#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define STACK_SIZE (1024 * 1024)    // 컨테이너용 1MB 스택 할당
#define CGROUP_PATH "/sys/fs/cgroup/mini_container"

int checkpoint[2]; // 부모와 자식 간의 동기화를 위한 파이프

int run_cmd(char *const argv[]) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork 실패");
        return -1;
    }
    
    if (pid == 0) { // 자식 프로세스: 외부 바이너리 직접 실행
        execvp(argv[0], argv);
        perror("execvp 실패");
        exit(EXIT_FAILURE);
    } else { // 부모 프로세스: 명령어 실행이 끝날 때 까지 대기
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return 0;
        }
        return -1;
    }

    return 0; // 이 줄은 도달 불가하지만, 컴파일러 경고 방지용으로 추가
}

void setup_cgroup_v2(pid_t child_pid) {
    const char *cgroup_path = CGROUP_PATH;
    char path[256];
    char value[64];
    int fd;

    // cgroup 디렉토리 생성
    if (mkdir(cgroup_path, 0755) == -1) {
        if (errno != EEXIST) {
            perror("=> [Error] cgroup 디렉토리 생성 실패");
            return;
        }
    }

    // cgroup.procs에 타겟 프로세스 등록
    // 호스트 관점 PID를 사용하여 자식 프로세스를 cgroup에 추가한다.
    snprintf(path, sizeof(path), "%s/cgroup.procs", cgroup_path);
    fd = open(path, O_WRONLY);
    if (fd != -1) {
        snprintf(value, sizeof(value), "%d", child_pid);
        if (write(fd, value, strlen(value)) == -1) {
            perror("=> [Error] cgroup.procs write 실패");
        }
        close(fd);
    } else {
        perror("=> [Error] cgroup 등록 실패 (권한 문제 혹은 cgroup v2 마운트 확인 필요)");
    }

    // memory.max 제한
    snprintf(path, sizeof(path), "%s/memory.max", cgroup_path);
    fd = open(path, O_WRONLY);
    if (fd != -1) {
        if (write(fd, "52428800", 8) == -1) { // 50MB
            perror("=> [Error] memory.max write 실패");
        }
        close(fd);
    }

    // memory.swap.max 제한
    snprintf(path, sizeof(path), "%s/memory.swap.max", cgroup_path);
    fd = open(path, O_WRONLY);
    if (fd != -1) {
        if (write(fd, "0", 1) == -1) {
            perror("=> [Error] memory.swap.max write 실패");
        }
        close(fd);
    } else {
        perror("=> [Error] memory.swap.max 설정 실패 (권한 문제 혹은 cgroup v2 마운트 확인 필요)");
        close(fd);
    }

    // cpu.max 제한
    snprintf(path, sizeof(path), "%s/cpu.max", cgroup_path);
    fd = open(path, O_WRONLY);
    const char *cpu_limit = "20000 100000"; // 20% CPU (20ms 사용, 100ms 주기)
    if (fd != -1) {
        if (write(fd, cpu_limit, strlen(cpu_limit)) == -1) { // 20% CPU
            perror("=> [Error] cpu.max write 실패");
        }
        close(fd);
    }

    printf("=> [Parent] cgroup v2 설정 완료 (PID: %d)\n", child_pid);

    return;
}

void cleanup_cgroup_v2(const char *cgroup_path) {
    char path[256];
    int fd;

    // cgroup.kill을 활용한 커널 레벨 원자적 킬링
    snprintf(path, sizeof(path), "%s/cgroup.kill", cgroup_path);
    fd = open(path, O_WRONLY);
    if (fd != -1) {
        // "1" -> cgroup 내의 모든 프로세스에 SIGKILL 시그널 보냄
        if (write(fd, "1", 1) == -1) {
            perror("=> [Error] cgroup.kill write 실패");
        }
        close(fd);
        usleep(100000); // 100ms 대기하여 커널이 프로세스 종료 처리할 시간 확보
    } else {
        perror("=> [Error] cgroup.kill 실패 (권한 문제 혹은 cgroup v2 마운트 확인 필요)");
    }
}

// ioctl 시스템 콜을 사용하여 네트워크 인터페이스 설정
int setup_interface(const char *ifname, const char *ip) {
    struct ifreq ifr;
    int sockfd;

    // IPv4 제어용 UDP 소켓 생성
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return -1;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

    if (ip != NULL) {
        struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_addr;
        addr->sin_family = AF_INET;
        inet_pton(AF_INET, ip, &addr->sin_addr);

        if (ioctl(sockfd, SIOCSIFADDR, &ifr) < 0) {
            perror("=> [Error] IP 주소 할당 실패");
            close(sockfd);
            return -1;
        }

        struct sockaddr_in *netmask = (struct sockaddr_in *)&ifr.ifr_netmask;
        netmask->sin_family = AF_INET;
        inet_pton(AF_INET, "255.255.255.0", &netmask->sin_addr);

        if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) < 0) {
            perror("=> [Error] 서브넷 마스크 할당 실패");
            close(sockfd);
            return -1;
        }
    }

    // 인터페이스 활성화 (IFF_UP | IFF_RUNNING 플래그 설정)
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
        close(sockfd);
        return -1;
    }
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) {
        perror("인터페이스 활성화 실패");
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0;
}

static int child_fn(void *arg) {
    (void)arg; // 사용하지 않는 매개변수 무시

    char c;
    close(checkpoint[1]); // 자식은 쓰기 끝을 닫는다.

    if (read(checkpoint[0], &c, 1) == -1) {
        perror("=> [Error] 파이프 읽기 실패");
        return -1;
    }

    printf("=> [Child] 격리된 공간 안으로 들어왔습니다!\n");
    printf("=> [Child] PID: %d\n", getpid());

    // 네트워크 인터페이스 설정
    if (setup_interface("lo", "127.0.0.1") == -1) {
        fprintf(stderr, "lo 인터페이스 설정 실패\n");
    }
    if (setup_interface("veth1", "10.0.0.2") == -1) {
        fprintf(stderr, "veth1 인터페이스 설정 실패\n");
    }

    // 1. 마운트 전파 속성을 private로 변경
    // 마운트 한 내용이 호스트로 전파되지 않도록 한다.
    if (mount(NULL, "/", NULL, MS_PRIVATE | MS_REC, NULL) == -1) {
        perror("mount private 실패");
        return -1;
    }

    // 2. 새로운 루트는 마운트 포인트여야 한다.
    // 이를 위해 bind mount를 사용한다.
    if (mount("./rootfs", "./rootfs", "bind", MS_BIND | MS_REC, NULL) == -1) {
        perror("bind 마운트 실패");
        return -1;
    }

    // 3. 기존 호스트의 루트 파일 시스템을 새로운 위치로 이동한다.
    mkdir("./rootfs/oldroot", 0700);

    // 4. pivot_root 시스템 콜을 사용하여 새로운 루트로 전환한다.
    if (syscall(SYS_pivot_root, "./rootfs", "./rootfs/oldroot") == -1) {
        perror("pivot_root 실패"); return -1;
    }

    // 5. 작업 디렉토리를 새로운 루트로 변경한다.
    if (chdir("/") == -1) {
        perror("chdir 실패");
        return -1;
    }

    // 6. 새로운 환경에 맞는 /proc 파일 시스템을 마운트한다.
    if (mount("proc", "/proc", "proc", 0, NULL) == -1) {
        perror("mount /proc 실패"); return -1;
    }

    // 7. 기존 호스트의 루트 파일 시스템을 언마운트하고 제거한다.
    // 호스트 파일 시스템 노출과 컨테이너 탈옥을 막기 위한 조치.
    if (umount2("/oldroot", MNT_DETACH) == -1) {    // MNT_DETACH <- 사용 중이더라도 강제로 언마운트
        perror("umount2 oldroot 실패"); return -1;
    }
    rmdir("/oldroot");

    printf("=> [Child] pivot_root 성공! 완벽한 격리 완성.\n");

    char *args[] = {"/bin/sh", NULL};
    execvp(args[0], args);

    perror("execvp 실패");
    return -1;
}

int main() {
    printf("=> [Parent] 호스트 환경입니다. (PID: %d)\n", getpid());

    if (pipe2(checkpoint, O_CLOEXEC) == -1) {
        perror("파이프 생성 실패");
        exit(1);
    }

    char *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, 
                        MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    mprotect(stack, sysconf(_SC_PAGESIZE), PROT_NONE); // 스택 오버플로우 방지용 가드 페이지 설정
    if (stack == MAP_FAILED) {
        perror("mmap 실패");
        exit(EXIT_FAILURE);
    }

    pid_t child_pid = clone(child_fn, (char *)stack + STACK_SIZE, 
                            CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | CLONE_NEWCGROUP | SIGCHLD, 
                            NULL);

    if (child_pid == -1) {
        perror("clone 실패");
        munmap(stack, STACK_SIZE);
        exit(1);
    }

    printf("=> [Parent] veth 네트워크 쌍을 생성하고 연결합니다...\n");
    // veth 쌍 생성: veth0 (호스트 측) <-> veth1 (컨테이너 측)
    char *cmd_add_veth[] = {"ip", "link", "add", "veth0", "type", 
                            "veth", "peer", "name", "veth1", NULL};
    run_cmd(cmd_add_veth);

    // veth1을 자식 프로세스의 네트워크 네임스페이스로 이동
    char pid_str[16];
    snprintf(pid_str, sizeof(pid_str), "%d", child_pid);
    char *cmd_move_netns[] = {"ip", "link", "set", "veth1", "netns", pid_str, NULL};
    run_cmd(cmd_move_netns);

    // 호스트 측 veth0에 IP 주소 할당 및 활성화
    char *cmd_set_ip[] = {"ip", "addr", "add", "10.0.0.1/24", "dev", "veth0", NULL};
    run_cmd(cmd_set_ip);

    char *cmd_set_up[] = {"ip", "link", "set", "veth0", "up", NULL};
    run_cmd(cmd_set_up);

    setup_cgroup_v2(child_pid);

    // 네트워크 설정 완료
    close(checkpoint[0]);
    if (write(checkpoint[1], "1", 1) == -1) {
        perror("=> [Error] 파이프 쓰기 실패");
    }
    close(checkpoint[1]);

    waitpid(child_pid, NULL, 0);
    printf("=> [Parent] 자식 프로세스가 종료되었습니다.\n");

    cleanup_cgroup_v2(CGROUP_PATH);
    if (rmdir(CGROUP_PATH) == -1) {
        perror("=> [Parent] cgroup 디렉토리 제거 실패");
    }

    if (munmap(stack, STACK_SIZE) == -1) {
        perror("=> [Error] 스택 메모리 해제 실패");
    }

    return 0;
}
