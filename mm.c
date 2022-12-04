/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* 기본 상수와 매크로 */
#define WSIZE 4 // 워드(4byte) 와 헤더, 푸터 사이즈
#define DSIZE 8 // 더블 워드(8byte) 사이즈
#define CHUNKSIZE (1 << 12) // 초기 가용 블록과 힙 확장을 위한 기본 크기 (1_000_000_000_000)

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* 가용 리스트에 접근하고 방문하는 작은 매크로들 */
#define PACK(size, alloc) ((size) | (alloc)) // 크기와 할당 비트를 통합 -> 헤더와 푸터에 저장

#define GET(p) (*(unsigned int *)(p)) // p가 참조하는 워드 리턴 / 인자 p는 (void*) 이므로 역참조는 불가하다.
#define PUT(p, val) (*(unsigned int *)(p) = (val)) // p에 val을 저장

#define GET_SIZE(p) (GET(p) & ~0x7) // 헤더 또는 푸터의 사이즈 리턴
#define GET_ALLOC(p) (GET(p) & 0x1) // 할당 비트 리턴

#define HDRP(bp) ((char *)(bp) - WSIZE) // 블록 헤더를 가리키는 포인터 리턴
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) // 블록 푸터를 가리키는 포인터 리턴 
// ↳ HDRP(bp)에 헤더 사이즈(w)도 포함되어 있어서 DSIZE를 빼준다.

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE))) // 다음 블록의 포인터
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) // 이전 블록의 포인터

static void *extend_heap(size_t words);
static void *coalesce(void *bp);

static char* heap_listp;  


/******    메인 함수    ******/



/**
 * @brief mm_init - initialize the malloc package.
 * @details 초기화를 완료하고 할당과 반환 요청을 받을 준비 완료
 * 
 * @param void
 * @return int   
 */
int mm_init(void)
{
    heap_listp = mem_sbrk(4 * WSIZE); // 초기 상태를 저장하기 위한 사이즈 지정
    // heap_listp = 0
    // mem_brk = 4 * WSIZE

    if (heap_listp == (void *)-1) // 오류 (sbrk와 동일)
        return -1;

    // 1. 빈 가용 리스트를 만들 수 있도록 초기화한다.
    PUT(heap_listp, 0); // 미사용 패딩 워드
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); // 프롤로그 헤더
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); // 프롤로그 푸터
    PUT(heap_listp + (3*WSIZE), PACK(0, 1)); // 에필로그 헤더
    heap_listp += (2*WSIZE); // heap_listp 위치를 프롤로그 헤더 뒤로 옮긴다.

    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) // 2. 이후 일반 블록을 저장하기 위해 힙을 확장한다.
        return -1;

    return 0;
}


/**
 * @brief mm_malloc - Allocate a block by incrementing the brk pointer. @n   Always allocate a block whose size is a multiple of the alignment.
 * @details 
 * 
 * @param size_t size 
 * @return void* 
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}


/******    서브 함수    ******/

/**
 * @brief 힙의 크기를 확장한다.
 * 
 * @param size_t words 확장하려는 워드 개수
 * @return void* 새로 생긴 가용 블록의 포인터
 */
static void *extend_heap(size_t words)
{   
    char *bp; // 블록 포인터
    size_t size;

    // 더블 워드 정렬 제한 조건을 적용하기 위해 반드시 짝수 사이즈(8byte)의 메모리만 할당한다.(반올림)
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE; 
    bp = mem_sbrk(size); // mem_sbrk return : 이전 brk(epilogue block 뒤 포인터) 반환
    // ↳ 실제 brk : 확장 후 힙의 맨 끝 포인터

    if ((long)bp ==  -1) // mem_sbrk err return
        return NULL;

    PUT(HDRP(bp), PACK(size, 0)); // size 만큼의 가용 블록의 헤더를 생성한다.
    PUT(FTRP(bp), PACK(size, 0)); // size 만큼의 가용 블록의 푸터를 생성한다.
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // 새로운 에필로그 헤더를 생성한다.

    return coalesce(bp); // 이전 힙이 가용 블록이라면 연결 수행
}


static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))); // 이전 블록 푸터에서 할당 여부 파악
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp))); // 다음 블록 헤더에서 할당 여부 파악
    size_t size = GET_SIZE(HDRP(bp)); // (할당 비트를 제외한) 블록 사이즈

    // @pb : 이전 블록, @cb : 현재 블록, @nb : 다음 블록
    // [CASE 1] : pb, nb - 둘 다 할당 상태
    if (prev_alloc && next_alloc) 
    {
        return bp;
    }
    // [CASE 2] : pb - 할당 상태 / nb - 가용 상태
    // resize block size : cb + nb 
    else if (prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp))); // 현재 블록 사이즈에 다음 블록 사이즈 더함
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    // [CASE 3] : pb - 가용 상태 / nb - 할당 상태
    // resize block size : pb + cb 
    else if (!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    // [CASE 4] : pb - 가용 상태 / nb - 가용 상태
    // resize block size : pb + nb 
    else {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
            GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}


