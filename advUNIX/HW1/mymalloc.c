#include <unistd.h>
#include <stdio.h>
#include <string.h>

/*
    메모리 블록 앞에 저장되는 메타데이터 구조체
    size : 사용자에게 제공되는 메모리 크기
    free : 해당 블록이 사용 중인지 여부
    next : 다음 메모리 블록을 가리키는 포인터
*/
typedef struct block {
    size_t size;
    int free;
    struct block *next;
} block_t;

#define BLOCK_SIZE sizeof(block_t)

/* 연결 리스트의 시작 */
static block_t *head = NULL;

/*
    사용 가능한 free 블록을 찾는 함수
*/
block_t *find_free_block(size_t size)
{
    block_t *current = head;
    while (current) {
        /*
        BLANK 1
        블록이 free 상태이고
        요청된 size보다 크거나 같은지 검사
        */
         // 첫번째 빈칸에 ?? == 1 보다 더 간단하게 기술... 
        if ( current->free && current->size >= size )
            return current;

        current = current->next;
    }

    return NULL;
}

/*
    sbrk()를 이용하여 운영체제에 메모리를 요청하는 함수
*/
block_t *request_space(block_t *last, size_t size)
{
    void *request;
    /*
    BLANK 2
    metadata + user memory 만큼 sbrk 호출
    */
    request = sbrk(size + BLOCK_SIZE);

    if (request == (void*)-1)
        return NULL;

    block_t *block = (block_t*)request;
    block->size = size;

    /*
    BLANK 3
    새로 할당된 블록은 free가 아님
    */

    block->free = 0;
    block->next = NULL;

    if (last)
        last->next = block;

    return block;
}

/*
    사용자 정의 malloc 함수
*/

void *my_malloc(size_t size)
{
    block_t *block;

    if (size <= 0)
        return NULL;

    if (!head) {
        /* 첫 번째 메모리 할당 */
        block = request_space(NULL, size);

        if (!block)
            return NULL;

        head = block;
    } else {
        block_t *last = head;
        block = find_free_block(size);

        if (!block) {
            while (last->next)
                last = last->next;

            block = request_space(last, size);

            if (!block)
                return NULL;
        } else {
            /*
            BLANK 4
            free 블록을 다시 사용
            */
            block->free = 0;
        }
    }

    /* metadata 다음 주소를 반환 */
    return (block + 1);
}

/*
    간단한 free 구현
*/
void my_free(void *ptr)
{
    if (!ptr)
        return;

    /*
    BLANK 5
    사용자 포인터에서 metadata 주소 계산
    */
    block_t *block = (block_t*)ptr - 1;
    block->free = 1;
}

/*
    테스트 프로그램
*/
int main()
{
    char *p = (char*) my_malloc(20);
    strcpy(p, "Hello Heap");
    printf("%s\n", p);
    my_free(p);
    return 0;
}
