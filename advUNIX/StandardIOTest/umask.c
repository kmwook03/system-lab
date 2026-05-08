#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main() {

    umask(0); // Set the file mode creation mask to 0
    if (creat("foo", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) < 0) {
        perror("Failed to create foo");
        exit(-1);
    }

    umask(S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH); // set the file mode rw-------
    if (creat("bar", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) < 0) {
        perror("Failed to create bar");
        exit(-1);
    }
    
    exit(0);
}