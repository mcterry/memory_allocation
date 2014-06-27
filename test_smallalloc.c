#include "smallalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define NUM_THREAD  (3)

static void * test_alloc_thread(void *data) {
    void *p[100];
    int i, size;
    printf("thread started...\n");

    for (size = 1; size <= 10000; size++) {
        for (i = 0; i < 100; i++) {
            p[i] = smallalloc_allocate(size);
            memset(p[i], 'c', size);
        }

        for (i = 0; i < 100; i++) {
            smallalloc_deallocate(p[i], size);
        }
    }

    printf("thread quit...\n");
    return NULL;
}

int main(int argc, char **argv) {
    smallalloc_init();

    int size = 18;
    char *str = smallalloc_allocate(size);

    snprintf(str, size, "Hello world!");
    printf("%s\n", str);

    smallalloc_deallocate(str, size);

    int i;
    pthread_t ids[NUM_THREAD]; 
    for (i = 0; i < NUM_THREAD; i++)
        pthread_create(&ids[i], NULL, test_alloc_thread, NULL);

    for (i = 0; i < NUM_THREAD; i++)
        pthread_join(ids[i], NULL);

    smallalloc_release();
    return 0;
}
