#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define PRODUCER_COUNT 2
#define CONSUMER_COUNT 2
#define ITEM_COUNT 4 // 각 스레드가 생산/소비할 아이템 수

// 공유 자원 (원형 큐 버퍼)
int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;

// 세마포어 선언
sem_t empty;
sem_t full;
sem_t mutex;

// 생산자(Producer) 스레드 함수
void* producer(void* arg) {
    int id = *((int*)arg);
    
    for (int i = 0; i < ITEM_COUNT; i++) {
        int item = rand() % 100; // 0~99 사이의 임의의 데이터 생성

        // 1. 빈 공간이 있을 때까지 대기 (비어있는 슬롯 감소)
        sem_wait(&empty);
        // 2. 임계구역(Critical Section) 진입을 위해 락 획득
        sem_wait(&mutex);

        // --- 임계구역 시작 ---
        buffer[in] = item;
        printf("[생산자 %d] 아이템 %2d 생산 (버퍼 인덱스 %d)\n", id, item, in);
        in = (in + 1) % BUFFER_SIZE;
        // --- 임계구역 종료 ---

        // 3. 임계구역 락 해제
        sem_post(&mutex);
        // 4. 채워진 공간이 하나 늘어났음을 알림 (채워진 슬롯 증가)
        sem_post(&full);

        sleep(1); // 생산 간격 시뮬레이션
    }
    return NULL;
}

// 소비자(Consumer) 스레드 함수
void* consumer(void* arg) {
    int id = *((int*)arg);
    
    for (int i = 0; i < ITEM_COUNT; i++) {
        // 1. 소비할 데이터가 있을 때까지 대기 (채워진 슬롯 감소)
        sem_wait(&full);
        // 2. 임계구역(Critical Section) 진입을 위해 락 획득
        sem_wait(&mutex);

        // --- 임계구역 시작 ---
        int item = buffer[out];
        printf("  [소비자 %d] 아이템 %2d 소비 (버퍼 인덱스 %d)\n", id, item, out);
        out = (out + 1) % BUFFER_SIZE;
        // --- 임계구역 종료 ---

        // 3. 임계구역 락 해제
        sem_post(&mutex);
        // 4. 빈 공간이 하나 늘어났음을 알림 (비어있는 슬롯 증가)
        sem_post(&empty);

        sleep(2); // 소비 간격 시뮬레이션
    }
    return NULL;
}

int main() {
    pthread_t producers[PRODUCER_COUNT];
    pthread_t consumers[CONSUMER_COUNT];
    int prod_ids[PRODUCER_COUNT];
    int cons_ids[CONSUMER_COUNT];

    // 난수 시드 초기화
    srand(time(NULL));

    // 세마포어 초기화 (0: 스레드 간 공유, 초기값 지정)
    sem_init(&empty, 0, BUFFER_SIZE); 
    sem_init(&full, 0, 0);            
    sem_init(&mutex, 0, 1);           

    printf("생산자-소비자 시뮬레이션 시작...\n\n");

    // 생산자 스레드 생성
    for (int i = 0; i < PRODUCER_COUNT; i++) {
        prod_ids[i] = i + 1;
        pthread_create(&producers[i], NULL, producer, &prod_ids[i]);
    }

    // 소비자 스레드 생성
    for (int i = 0; i < CONSUMER_COUNT; i++) {
        cons_ids[i] = i + 1;
        pthread_create(&consumers[i], NULL, consumer, &cons_ids[i]);
    }

    // 스레드 종료 대기
    for (int i = 0; i < PRODUCER_COUNT; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < CONSUMER_COUNT; i++) {
        pthread_join(consumers[i], NULL);
    }

    // 세마포어 리소스 해제
    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mutex);

    printf("\n모든 작업이 완료되었습니다.\n");
    return 0;
}
