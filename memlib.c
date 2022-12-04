/*
 * memlib.c - a module that simulates the memory system.  Needed because it 
 *            allows us to interleave calls from the student's malloc package 
 *            with the system's malloc package in libc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#include "memlib.h"
#include "config.h"

/* private variables */
static char *mem_start_brk;  /* points to first byte of heap */
static char *mem_brk;        /* points to last byte of heap */
static char *mem_max_addr;   /* largest legal heap address */ 

/**
 * @brief mem_init - initialize the memory system model
 * @details mem_start_brk : 할당된 가상 메모리 @n mem_brk : 미할당 가상 메모리 @n mem_max_addr : 최대 힙 주소
 * 
 * @return void
 */
void mem_init(void)
{
    /* allocate the storage we will use to model the available VM */
    if ((mem_start_brk = (char *)malloc(MAX_HEAP)) == NULL) {
	fprintf(stderr, "mem_init_vm: malloc error\n");
	exit(1);
    }

    mem_max_addr = mem_start_brk + MAX_HEAP;  /* max legal heap address */
    mem_brk = mem_start_brk;                  /* heap is empty initially */
}

/* 
 * mem_deinit - free the storage used by the memory system model
 */
void mem_deinit(void)
{
    free(mem_start_brk);
}

/*
 * mem_reset_brk - reset the simulated brk pointer to make an empty heap
 */
void mem_reset_brk()
{
    mem_brk = mem_start_brk;
}

/* 

 * 
 */

/**
 * @brief mem_sbrk - simple model of the sbrk function. Extends the heap @n by incr bytes and returns the start address of the new area. @n In this model, the heap cannot be shrunk.
 * @details 힙을 축소하라는 요청을 거부하는 것만 제외하고는 시스템의 sbrk 함수와 @n 동일한 의미뿐만 아니라 동일한 인터페이스를 갖는다.
 * 
 * @param int incr incr 바이트만큼 올린다.
 * @return void* 성공 시 이전 brk, 실패 시 -1
 */
void *mem_sbrk(int incr) 
{
    char *old_brk = mem_brk; // mem_brk 를 가져와서 작업

    // [err] incr 가 음수인 경우, 최대 힙 크기를 넘는 경우
    if ( (incr < 0) || ((mem_brk + incr) > mem_max_addr)) {
        errno = ENOMEM;
        fprintf(stderr, "ERROR: mem_sbrk failed. Ran out of memory...\n");
        return (void *)-1;
    }
    mem_brk += incr; // mem_brk를 incr만큼 확장 시킨다.
    return (void *)old_brk; // 이전 brk(*old_brk) 리턴
}

/*
 * mem_heap_lo - return address of the first heap byte
 */
void *mem_heap_lo()
{
    return (void *)mem_start_brk;
}

/* 
 * mem_heap_hi - return address of last heap byte
 */
void *mem_heap_hi()
{
    return (void *)(mem_brk - 1);
}

/*
 * mem_heapsize() - returns the heap size in bytes
 */
size_t mem_heapsize() 
{
    return (size_t)(mem_brk - mem_start_brk);
}

/*
 * mem_pagesize() - returns the page size of the system
 */
size_t mem_pagesize()
{
    return (size_t)getpagesize();
}
