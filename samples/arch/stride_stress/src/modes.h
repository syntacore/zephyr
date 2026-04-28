/*
 * Copyright (C) 2023, Syntacore Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *     http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * @file modes.h
 *
 * @author vdsh-sc
 */

/******************************************************************************
 *                              Defines section
 */

/* * * OPTIONS * * */

/* Also, you can define CONFIG_HEAP_MEM_POOL_SIZE for dynamic memory */

#define LOWER           512
#define ITERATIONS      (1000LLU)

/* We put canaries on the sides of the array - you can adjust their number */
#define CANARIES_ONE_SIDE 2

#define STACK_SIZE (512 * 4 * 4)
#define CACHE_LINE (64)
#define CACHE_SIZE (1024 * 1024) /* L2 or L3 */
#define NUM_THREADS 4

/* * * * * * * * * */

#define HUNDRED_TIMES 100

#define STRIDE CACHE_LINE
#define CACHE_ENTRIES (CACHE_SIZE / CACHE_LINE)

/* We take an array twice as large as the cache + a little more */
#define NUM_ENTRIES (CACHE_ENTRIES * 2 + 1)
#define MAX_RANGE (CACHE_LINE * NUM_ENTRIES)

#define ONE p = (char** volatile) *p;
#define FIVE    ONE ONE ONE ONE ONE
#define TEN     FIVE FIVE
#define FIFTY   TEN TEN TEN TEN TEN
#define HUNDRED FIFTY FIFTY

/******************************************************************************
 *                              Prototypes section
 */

struct mem_raw {
    char left_canary[MAX_RANGE][CANARIES_ONE_SIDE];
    char main_array[MAX_RANGE];
    char right_canary[MAX_RANGE][CANARIES_ONE_SIDE];
};

struct mem_descriptor {
    char*   raw_ptr;    /*!< raw pointer returned by malloc */
    char*   start_ptr;  /*!< start of array, around - canary */
    size_t  range;      /*!< length of array on current step */
    size_t  max_range;  
    size_t  stride;     /*!< often - cache line size */
    struct k_mutex* mutex; /*!< One thread takes full control of 
                                    the memory for the duration of one circle */
    size_t iterations; /*!< num of cycles */
    void (*action) (struct mem_descriptor* descriptor, size_t* hash_errors);
};

struct region_crc {
    const uint8_t * ptr;
    size_t         size;
};

struct __aligned(STRIDE) base_stride_struct {
    struct base_stride_struct* next;
};

struct __aligned(STRIDE) dirty_stride_struct {
    struct base_stride_struct* next;
    uint32_t filler;
    size_t poisoned;
};

void cache_lines_clean(struct mem_descriptor* descriptor, size_t* hash_errors);
void cache_lines_dirty(struct mem_descriptor* descriptor, size_t* hash_errors);
void thread_routine(struct mem_descriptor* descriptor, size_t* hash_errors);
