// myprintf implementation
#include <stdio.h>
#include <stdarg.h>

void myprintf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char *ptr = (char *)format;
    while (*ptr) {
        if (*ptr == '%') {
            ptr++;
            switch (*ptr) {
                case 'd': {
                    int num = va_arg(args, int);
                    printf("%d", num);
                    break;
                }
                case 'c': {
                    int ch = va_arg(args, int); // char is promoted to int
                    printf("%c", ch);
                    break;
                }
                case 's': {
                    char *str = va_arg(args, const char *);
                    printf("%s", str);
                    break;
                }
                case 'f': {
                    double num = va_arg(args, double);
                    printf("%.2f", num);
                    break;
                }
                default: {
                    // Handle unsupported format specifiers
                    printf("Unsupported format specifier: %%%c\n", *ptr);
                    break;
                }
            }
        } else {
            putchar(*ptr);
        }
        ptr++;
    }
    va_end(args);
}

int main() {
    myprintf("The value of pi is approximately %f.\n", 3.14159);
    return 0;
}
