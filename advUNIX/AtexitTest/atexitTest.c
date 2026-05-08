// atexit testcode
#include <stdio.h>
#include <stdlib.h>

void func1(void)
{
    printf("func1 called\n");
}

void func2(void)
{
    printf("func2 called\n");
}

int main(void)
{
    atexit(func1);
    atexit(func2);
    printf("main function\n");
    return 0;
}
