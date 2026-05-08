// int add(int a, int b) 
#include <stdio.h>

int add(int a, int b) {
	int c = a + b;
    return c;
}

int main() {
    int x = 10;
    int y = 20;
    int result = add(x, y);
    printf("The result of adding %d and %d is: %d\n", x, y, result);
    return 0;
}
