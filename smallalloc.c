/* 
 * @file smallalloc.c
 * @author Terry Lei 
 * @email mcterrylei@gmail.com
 * @brief Implementation of types defined in smallalloc.h
 */
#include "smallalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#define CHUNK_LIMIT     (16384) 
#define MAX_NUMBER      (64)
#define ALIGN_SIZE      (8)
#define CHUNK_NUMBER    (CHUNK_LIMIT/ALIGN_SIZE)

struct memory_list {
    struct memory_list *next;
};

struct chunk_list {
    struct chunk_list *next;
    struct memory_list *data;
};

static struct memory_list * _free_list[CHUNK_NUMBER] = {0};
static pthread_mutex_t _free_list_locks[CHUNK_NUMBER];
static struct chunk_list * _chunk_list = NULL;
static pthread_mutex_t _chunk_list_lock;

void smallalloc_init() {
    memset(_free_list, 0, sizeof(_free_list));
    int i;
    for (i = 0; i < CHUNK_NUMBER; i++)
        pthread_mutex_init(&_free_list_locks[i], NULL);
    pthread_mutex_init(&_chunk_list_lock, NULL);
}

void smallalloc_release() {
    /* for the memory of chunk_list node is allocated from
     * memory list (via smallalloc_allocate());
     * so we should allocate the memory to store the nodes,
     * before free the memory list.
     */
    int count = 0;
    struct chunk_list *tmp = _chunk_list;
    while (tmp != NULL) {
        count++;
        tmp = tmp->next;
    }

    struct chunk_list *list = malloc(sizeof(struct chunk_list)*count);
    int i;    
    for (i=0,tmp=_chunk_list; i < count; i++,tmp=tmp->next) 
        memcpy(list+i, tmp, sizeof(struct chunk_list)); 
    for (i = 0; i < count; i++) 
        free(list[i].data);
    free(list);

    /* destroy locks */
    for (i = 0; i < CHUNK_NUMBER; i++)
        pthread_mutex_destroy(&_free_list_locks[i]);
    pthread_mutex_destroy(&_chunk_list_lock);
}

static inline unsigned int __chunk_index(unsigned int size) {
    unsigned int index = (size-1) / ALIGN_SIZE; 
    assert(index >= 0 && index < CHUNK_NUMBER);
    return index;
}

static inline int __min(int a, int b) {
    return a < b? a : b;
}

static struct memory_list * __alloc_chunk(unsigned int index) {
    assert(_free_list[index] == NULL);
    int node_size = (index+1) * ALIGN_SIZE; 
    int chunk_size = __min(node_size*MAX_NUMBER, CHUNK_LIMIT/node_size*node_size);
    struct memory_list *list = malloc(chunk_size);
    int i;
    struct memory_list *iter = list;
    /* Notice: chunk_size - 2*node_size may less than 0; 
     * so use int instead of unsigned int.
     */
    for (i = 0; i <= chunk_size-2*node_size; i+=node_size) 
        iter = iter->next = (struct memory_list *)((char *)iter + node_size);
    iter->next = NULL;
    return list;
}

void * smallalloc_allocate(unsigned int size) {
    unsigned int index = __chunk_index(size);  
    pthread_mutex_lock(&_free_list_locks[index]); 
    if (_free_list[index] == NULL) {
        struct memory_list *new_chunk = __alloc_chunk(index);
        _free_list[index] = new_chunk;
         
        struct chunk_list *node; 
        if (__chunk_index(sizeof(struct chunk_list)) == index) {  
            /* this chunk is right fit, so use it */
            node = (struct chunk_list *)_free_list[index];
            _free_list[index] = _free_list[index]->next;
        }
        else {
            node = smallalloc_allocate(sizeof(struct chunk_list));  
        }

        /* add to list head */
        pthread_mutex_lock(&_chunk_list_lock); 
        node->data = new_chunk;
        node->next = _chunk_list;
        _chunk_list = node;
        pthread_mutex_unlock(&_chunk_list_lock); 
    }
    struct memory_list *ret = _free_list[index];
    _free_list[index] = ret->next;
    pthread_mutex_unlock(&_free_list_locks[index]); 
    return (void *)ret;
}

void smallalloc_deallocate(void *mem, unsigned int size) {
    unsigned int index = __chunk_index(size);
    pthread_mutex_lock(&_free_list_locks[index]); 
    struct memory_list *tmp = mem;
    tmp->next = _free_list[index];
    _free_list[index] = tmp;
    pthread_mutex_unlock(&_free_list_locks[index]); 
}

