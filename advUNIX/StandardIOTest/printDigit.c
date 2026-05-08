// variable parameter example
#include <stdio.h>
#include <stdarg.h>

// num 개의 정수를 출력하는 함수
void printDigit(int num, ...)
{
    va_list args; // 가변 인자 리스트를 위한 변수 선언
    va_start(args, num); // 가변 인자 리스트 초기화, num은 고정 인자
    for (int i = 0; i < num; i++)
    {
        printf("%d ", va_arg(args, int));
    }
    printf("\n");
    va_end(args);
}

int main()
{
    printDigit(3, 1, 2, 3);
    printDigit(5, 10, 20, 30, 40, 50);
    return 0;
}
