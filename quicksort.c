#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int *quicksort(size_t size, const int *data) {
    if (size == 0) return NULL;
    if (size == 1) {
        int *res = malloc(sizeof(int));
        res[0] = data[0];
        return res;
    }

    int pivot = data[0];
    size_t less_count = 0, same_count = 0, more_count = 0;

    for (size_t i = 0; i < size; i++) {
        if (data[i] < pivot) less_count++;
        else if (data[i] > pivot) more_count++;
        else same_count++;
    }

    int *less = malloc(less_count * sizeof(int));
    int *more = malloc(more_count * sizeof(int));
    size_t l = 0, m = 0;

    for (size_t i = 0; i < size; i++) {
        if (data[i] < pivot) less[l++] = data[i];
        else if (data[i] > pivot) more[m++] = data[i];
    }

    int *sorted_less = quicksort(less_count, less);
    int *sorted_more = quicksort(more_count, more);

    int *result = malloc(size * sizeof(int));
    size_t idx = 0;

    for (size_t i = 0; i < less_count; i++) result[idx++] = sorted_less[i];
    for (size_t i = 0; i < same_count; i++) result[idx++] = pivot;
    for (size_t i = 0; i < more_count; i++) result[idx++] = sorted_more[i];

    free(less);
    free(more);
    free(sorted_less);
    free(sorted_more);

    return result;
}

int main(int argc, char *argv[]) {
    int print_flag = 0;
    char *filename = NULL;

    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        print_flag = 1;
        filename = argv[2];
    } else if (argc == 2) {
        filename = argv[1];
    } else {
        fprintf(stderr, "usage: %s [-p] file_of_ints\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "cannot open %s\n", filename);
        return 1;
    }

    size_t capacity = 4;
    int *arr = malloc(capacity * sizeof(int));
    size_t n = 0;

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
        printf("Unsorted list before non-threaded quicksort:  ");
        for (size_t i = 0; i < n; i++) {
            printf("%d", arr[i]);
            if (i < n - 1) printf(", ");
        }
        printf("\n");
    }

    int *sorted = quicksort(n, arr);

    if (print_flag) {
        printf("Resulting list:  ");
        for (size_t i = 0; i < n; i++) {
            printf("%d", sorted[i]);
            if (i < n - 1) printf(", ");
        }
        printf("\n");
    }

    free(arr);
    free(sorted);

    return 0;
}

