/*
 * Copyright (C) 2025, Syntacore Ltd.
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

#ifndef SCR_GENERIC_CACHE_H_
#define SCR_GENERIC_CACHE_H_

#include <stdint.h>

/**
 * @enum gc_type
 * @brief Type of generic cache
 */
enum gc_type {
    GC_L1 = 0,
    GC_L1D,
    GC_L1I,
    GC_L2,
    GC_L3,
    GC_L4,
};

/**
 * @brief Initialization of GC
 */
int gc_init(enum gc_type gc_type);

#endif /* SCR_GENERIC_CACHE_H_ */
