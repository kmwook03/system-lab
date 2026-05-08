#include <stdio.h>

struct student {
    int sno;
    char name[10];
    int point;
};

int main() {
    struct student s1, s2;
    FILE *fp;

    if ((fp = fopen("student.dat", "w")) == NULL) {
        perror("Failed to open file");
        return -1;
    }
    printf("Input SNO >> ");
    scanf("%d", &s1.sno);
    printf("Input Name >> ");
    scanf("%s", s1.name); // Not recommended
    printf("Input Point >> ");
    scanf("%d", &s1.point);

    if (fwrite(&s1, sizeof(struct student), 1, fp) != 1) {
        perror("Failed to write to file");
        fclose(fp);
        return -1;
    }
    fclose(fp);

    if ((fp = fopen("student.dat", "r")) == NULL) {
        perror("Failed to open file");
        return -1;
    }

    if (fread(&s2, sizeof(struct student), 1, fp) != 1) {
        perror("Failed to read from file");
        fclose(fp);
        return -1;
    }

    printf("\n NO        Name      Point\n");
    printf("============================\n");
    printf(" %d        %s        %d\n", s2.sno, s2.name, s2.point);
    fclose(fp);
    return 0;
}
