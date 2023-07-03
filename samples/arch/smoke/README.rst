.. _smoke:

Syntacore Smoke testing utility
###########

Overview
********

Performs basic Smoke tests to verify general system operation.
Should be driven by scr_runner.py 

Currently supported tests:
 - BOOTUP : minimal possible test, platform boot-up, checks Memory Protection, UART, Caches
 - IRQ: Interrupts testing using interrupt-driven console
 - MTIMER : coarse MTIMER accuracy verification
 - SMP : ICCM and SMP testing

Building and Running
********************

1. Testing on FPGA
------------------
1.1 Build Smoke Test Utility for particular platform with:
 > west build -p -b vcu118_scrXXX samples/arch/smoke
1.2 Flash the bitsteam to FPGA
1.3 Launch scripts/scr_runner.py specifying the path to FPGA console
   and reasonable combination of tests:
 > python3 scr_runner.py /dev/ttySCR BOOTUP IRQ MTIMER SMP
1.4 Launch OpenOCD server, connect to it, load and run Zephyr with
    Smoke Test Utility binary image using OpenOCD:
 > reset halt; load_image /path/to/zephyr.bin <addr>; resume <addr>

2. Testing on RTL Simulator
---------------------------
2.1 Build Smoke Test Utility for particular platform with:
 > west build -p -b vcu118_scrXXX samples/arch/smoke
2.2 Run Smoke Test Utility on RTL Simulator redirecting output to a file.
    E.g. if you use trace-player-runner to launch RTL Simulator:
 >  make APP_ELF=/path/to/smoke_test_utility/zephyr.elf run > /path/to/log
2.3 Since with RTL Simulator we can't interact with application,
    the only test which may be used is BOOTUP.
    Launch scripts/scr_runner.py specifying the path to a file to which
    the output of RTL Simulator is redirected:
> python3 scr_runner.py /path/to/log BOOTUP


Sample Output
*************

.. code-block:: console

> python3 scr_runner.py /dev/ttySCR BOOTUP IRQ MTIMER SMP
________________________________       __   __
                                * * * //|| //||
Syntacore Smoke Test Runner    * * * // ||// ||
_____________________________ * * * //  |_/  ||

Scheduled tests:  ['BOOTUP', 'IRQ', 'MTIMER', 'SMP'] 

[BOOTUP]: Waiting for platform bootup, checking: Memory Protection, UART, Caches..
 + SUCCESS!
[IRQ]: Checking Interrupts: UART, Interrupt Controller..
 + SUCCESS!
[MTIMER]: Running MTIMER check..
 + SUCCESS!
[SMP]: Checking SMP, ICCM..
 + SUCCESS!
 All 16 threads executed by 2 cores in 116 msec

 +++ SMOKE TESTS PASSED! +++

