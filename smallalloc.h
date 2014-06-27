/* 
 * @file  smallalloc.h
 * @author Terry Lei 
 * @emal mcterrylei@gmail.com
 * @brief: For small memory allocation. It is thread safe!
 */
#ifndef _SMALLALLOC_H_
#define _SMALLALLOC_H_

#ifdef __cplusplus
extern "C" {
#endif

void   smallalloc_init();
void   smallalloc_release();

void * smallalloc_allocate(unsigned int size);
void   smallalloc_deallocate(void *mem, unsigned int size);

#ifdef __cplusplus
}
#endif

#endif

