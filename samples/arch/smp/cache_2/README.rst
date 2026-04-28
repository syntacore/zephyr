# Copyright (C) 2023, Syntacore Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#     http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

.. _cache:

Syntacore Cache flush-invalidate test#2
###########
All cores run independently accessing memory regions with
offset of (core_id * L2D_SIZE) and performing test procedures below.
Cacheable/uncacheable accesses configured via PMP.

1. Set test region NC
2. Fill region with pattern
3. Set region Cacheable, fill region with different pattern,
   invalidate (CL), check that initial pattern still exists and
   no cache eviction corrupted data in DDR
4. Iteratively fill region with test sequence, covering multiple
   cache-lines per-iteration:

4.1 In Cached mode fill N cache lines with test sequence
4.2 Flush N cache lines
4.3 Set region NC
4.4 Check N lines are indeed written to DDR
4.5 Return region to Cached
