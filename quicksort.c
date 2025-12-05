/*Filename: quicksort.c
 * Author: Arnav Gawas
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>


// Mutex is a good practice when it come to using threads. Helps with handling threads efficiently and safely.
static int thread_count = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


/**
 *size: number of integers in the array
 *data: pointer to the integer 
 */
typedef struct {
    size_t size;
    int *data;
} qs_args;

int *quicksort(size_t size, const int *data);
void *quicksort_threaded(void *ptr);

/*Non-threaded Quicksort*/
//Implements recursive, non-threaded Quicksort.
// returns  pointer to newly allocated sorted array
int *quicksort(size_t size, const int *data) {
    if (size == 0) return NULL;
    if (size == 1) {
        int *res = malloc(sizeof(int));
        res[0] = data[0];
        return res;
    }

    int pivot = data[0];
    size_t less_count = 0, same_count = 0, more_count = 0;

    // Count elements in each partition
    for (size_t i = 0; i < size; i++) {
        if (data[i] < pivot) less_count++;
        else if (data[i] > pivot) more_count++;
        else same_count++;
    }

    int *less = malloc(sizeof(int) * less_count);
    int *more = malloc(sizeof(int) * more_count);
    size_t l = 0, m = 0;

    // Fill partition arrays
    for (size_t i = 0; i < size; i++) {
        if (data[i] < pivot) less[l++] = data[i];
        else if (data[i] > pivot) more[m++] = data[i];
    }

    int *sorted_less = quicksort(less_count, less);
    int *sorted_more = quicksort(more_count, more);

    // Merge results into a new array
    int *result = malloc(sizeof(int) * size);
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

/*Threaded Quicksort*/
//Implements recursive Quicksort using POSIX threads
//returns pointer to newly allocated sorted array
void *quicksort_threaded(void *ptr) {
    qs_args *args = (qs_args *)ptr;
    size_t size = args->size;
    int *data = args->data;
    free(args);

    if (size == 0) pthread_exit(NULL);
    if (size == 1) {
        int *res = malloc(sizeof(int));
        res[0] = data[0];
        pthread_exit(res);
    }

    int pivot = data[0];
    size_t less_count = 0, same_count = 0, more_count = 0;

    // Count elements in partitions
    for (size_t i = 0; i < size; i++) {
        if (data[i] < pivot) less_count++;
        else if (data[i] > pivot) more_count++;
        else same_count++;
    }

    int *less = malloc(sizeof(int) * less_count);
    int *more = malloc(sizeof(int) * more_count);
    size_t l = 0, m = 0;

    for (size_t i = 0; i < size; i++) {
        if (data[i] < pivot) less[l++] = data[i];
        else if (data[i] > pivot) more[m++] = data[i];
    }

    pthread_t t1, t2;
    int create_left = 0, create_right = 0;
    int *sorted_less = NULL;
    int *sorted_more = NULL;

    // Left partition thread
    if (less_count > 0) {
        qs_args *a = malloc(sizeof(qs_args));
        a->size = less_count;
        a->data = less;

        pthread_mutex_lock(&lock);
        thread_count++;
        pthread_mutex_unlock(&lock);

        if (pthread_create(&t1, NULL, quicksort_threaded, a) == 0)
            create_left = 1;
        else {
            free(a);
            sorted_less = quicksort(less_count, less);
        }
    }

    // Right partition thread
    if (more_count > 0) {
        qs_args *b = malloc(sizeof(qs_args));
        b->size = more_count;
        b->data = more;

        pthread_mutex_lock(&lock);
        thread_count++;
        pthread_mutex_unlock(&lock);

        if (pthread_create(&t2, NULL, quicksort_threaded, b) == 0)
            create_right = 1;
        else {
            free(b);
            sorted_more = quicksort(more_count, more);
        }
    }

    if (create_left) pthread_join(t1, (void **)&sorted_less);
    if (create_right) pthread_join(t2, (void **)&sorted_more);

    // Merge results
    int *result = malloc(sizeof(int) * size);
    size_t idx = 0;
    if (sorted_less) for (size_t i = 0; i < less_count; i++) result[idx++] = sorted_less[i];
    for (size_t i = 0; i < same_count; i++) result[idx++] = pivot;
    if (sorted_more) for (size_t i = 0; i < more_count; i++) result[idx++] = sorted_more[i];

    free(sorted_less);
    free(sorted_more);
    free(less);
    free(more);

    pthread_exit(result);
}

/*Main Function*/
int main(int argc, char **argv) {
    int print_flag = 0;
    char *filename = NULL;

    // Parse command-line arguments
    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        print_flag = 1;
        filename = argv[2];
    } else if (argc == 2) {
        filename = argv[1];
    } else {
        fprintf(stderr, "usage: %s [-p] file_of_ints\n", argv[0]);
        return 1;
    }

    // Read integers from file
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "cannot open %s\n", filename);
        return 1;
    }

    size_t cap = 8, n = 0;
    int *arr = malloc(sizeof(int) * cap);
    while (1) {
        int x;
        if (fscanf(fp, "%d", &x) != 1) break;
        if (n >= cap) arr = realloc(arr, sizeof(int) * (cap *= 2));
        arr[n++] = x;
    }
    fclose(fp);

    // Print unsorted list before non-threaded quicksort
    if (print_flag) {
        printf("Unsorted list before non-threaded quicksort:  ");
        for (size_t i = 0; i < n; i++) {
            printf("%d", arr[i]);
            if (i < n - 1) printf(", ");
        }
        printf("\n");
    }

    // Non-threaded quicksort
    clock_t s1 = clock();
    int *sorted1 = quicksort(n, arr);
    clock_t s2 = clock();
    printf("Non-threaded time:  %.6f\n", (double)(s2 - s1) / CLOCKS_PER_SEC);

    if (print_flag) {
        printf("Resulting list:  ");
        for (size_t i = 0; i < n; i++) {
            printf("%d", sorted1[i]);
            if (i < n - 1) printf(", ");
        }
        printf("\n");
    }

    // Threaded quicksort
    if (print_flag) {
        printf("Unsorted list before threaded quicksort:  ");
        for (size_t i = 0; i < n; i++) {
            printf("%d", arr[i]);
            if (i < n - 1) printf(", ");
        }
        printf("\n");
    }

    thread_count = 1; // main thread counts as 1
    qs_args *root = malloc(sizeof(qs_args));
    root->size = n;
    root->data = arr;

    pthread_t main_thread;
    clock_t t1 = clock();
    pthread_create(&main_thread, NULL, quicksort_threaded, root);

    int *sorted2 = NULL;
    pthread_join(main_thread, (void **)&sorted2);
    clock_t t2 = clock();

    printf("Threaded time:      %.6f\n", (double)(t2 - t1) / CLOCKS_PER_SEC);
    printf("Threads spawned:    %d\n", thread_count);

    if (print_flag) {
        printf("Resulting list:  ");
        for (size_t i = 0; i < n; i++) {
            printf("%d", sorted2[i]);
            if (i < n - 1) printf(", ");
        }
        printf("\n");
    }

    free(arr);
    free(sorted1);
    free(sorted2);
    return 0;
}

