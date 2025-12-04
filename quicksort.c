#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int print_flag = 0;
    char *filename = NULL;

    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        print_flag = 1;
        filename = argv[2];
    } else if (argc == 2) {
        filename = argv[1];
    } else {
        fprintf(stderr, "usage: %s [-p] <inputfile>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "cannot open %s\n", filename);
        return 1;
    }

    int capacity = 4;
    int *arr = malloc(capacity * sizeof(int));
    int n = 0;

    while (1) {
        int x;
        if (fscanf(fp, "%d", &x) != 1) break;
        if (n >= capacity) {
            capacity *= 2;
            arr = realloc(arr, capacity * sizeof(int));
        }
        arr[n++] = x;
    }

    fclose(fp);

    if (print_flag) {
        for (int i = 0; i < n; i++) {
            printf("%d", arr[i]);
            if (i < n - 1) printf(",");
        }
        printf("\n");
    }

    free(arr);
    return 0;
}

