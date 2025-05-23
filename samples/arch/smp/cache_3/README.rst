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

Syntacore Cache flush-invalidate test scenario #3
###########

1.1 Before each transaction fill test region with poison using core 0 in NC mode
1.2 All cores are synchronized, region is Cached for all cores.
1.3 For each cache line:
  write counter from each core to the part of cache line with stride,
  so that each core writes it's own byte. Each core flushes the CL after each write
1.4 The memory region contents is then getting checked with 3 passes:
  - 1 pass: each core remains in Cachable mode and verifies only bytes
    which was written by it
  - 2 pass: core0 waits until all the cores are done with writing
    (1.3 is completed by all the cores), then it verifies the entire region
  - 3 pass: core0 switches to NC and verifies the entire region
