// print /proc/self/maps content
#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *file = fopen("/proc/self/maps", "r");
    if (file == NULL) {
        perror("fopen");
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }

    fclose(file);
    return 0;
}
