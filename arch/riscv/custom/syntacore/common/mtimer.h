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

/*
 * @file mtimer.h
 *
 * @brief Syntacore fixup for standard RISC-V MTIMER
 */

#ifndef SCR_MTIMER_H
#define SCR_MTIMER_H

/******************************************************************************
 *                              Prototypes section
 */
/*
 * @brief Fixes-up MTIMER registers which couldn't be initialized
 *		with standard driver API
 */
void mtimer_early_init(void);

#endif /* SCR_MTIMER_H */
