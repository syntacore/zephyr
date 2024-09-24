# Copyright (C) 2022, Syntacore Ltd.
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

.. _mrt_dump:

Syntacore Memory Routing Table (MRT) dumping utility
###########

Overview
********

Dumps MRT contents

Building and Running
********************

This application can be built and executed on QEMU as follows:

.. zephyr-app-commands::
   :zephyr-app: samples/arch/mrt
   :host-os: unix
   :board: vcu118_scr7_l3cl
   :goals: run
   :compact:


Sample Output
=============

.. code-block:: console

MRT regions:16                                                    
\#    CTRL               BASE            MASK                      
0   0x8F000001   0x0000FFFF00000000   0x0000FFFFFFFF8000          
1   0x00000001   0x0000000000000000   0x00FFFFFFC0000000          
2   0x00000001   0x000000003F000000   0x00FFFFFFFF000000          
3   0x00000001   0xFFFFFFFF80000000   0x00FFFFFFFFFFE000          
4   0x00000001   0xFFFFFFFFFF000000   0x00FFFFFFFF800000          
5   0x0F000001   0xFFFFFFFFE0000000   0x00FFFFFFFF800000          
6   0x0F000001   0xFFFFFFFFFC000000   0x00FFFFFFFF000000          
7   0x00000001   0xFFFFFFFFFFFF0000   0x00FFFFFFFFFF0000    

Exit QEMU by pressing :kbd:`CTRL+A` :kbd:`x`.
