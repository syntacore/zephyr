# Copyright (C) 2025, Syntacore Ltd.
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


.. _game_of_life-sample:

Game of Life
###################

Overview
********

An implementation of a solution Game of Life. The implementation is based on https://github.com/andre-j3sus/game-of-life/tree/main


Building and Running
********************

This project outputs to the console.  It can be built and executed on QEMU as follows:

.. zephyr-app-commands::
   :zephyr-app: samples/game_of_life
   :host-os: unix
   :board: qemu_x86
   :goals: run
   :compact:

Sample Output
=============

.. code-block::

Game of life on Zephyr 3.7.0


      X




  X     X   X
      X X X   X
X       X X   X X
X   X X X X   X     X
X     X X   X
  X   X X
Generation: 19

Exit QEMU by pressing :kbd:`CTRL+A` :kbd:`x`.

