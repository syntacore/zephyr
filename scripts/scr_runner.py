"""
 @file scr_runner.py
 
 @brief Syntacore Smoke Test Runner
 
 @author amr-sc
 
 Copyright (c) 2023 Syntacore. All rights reserved.
 
 SPDX-License-Identifier: Apache-2.0
"""

import time, os, sys
import difflib
import argparse

# Additional debug output
DEBUG = 0

""""
  @brief Verifies console output in a configurable way.
  
  @param [in] console - console to iterate with Syntacore platform
  @param [in] expected_output - expected console output
  @param [in] relaxed - when True - checks that expected_output is present in the entire captured output, 
                        when False - checks that expected_output matches the entire captured output
  @param [in] monitor_mode - constantly monitors output for expected pattern; otherwise only the existing
                        data will be verified
						
  @return captured output on successfull match
"""
def verify_output(console, expected_output, relaxed = False, monitor_mode = False):
	
	text = ''
	
	while 42:
		try:
			text += console.read().decode()
		except:
			if not monitor_mode:
				print(" - FAILED! No console output detected")
				exit(-1)

		if text or not monitor_mode:
			if (relaxed and expected_output in text) or (text == expected_output):
					print(" + SUCCESS!")
					return text
			elif not monitor_mode:
					print(" - FAILED! Patterns don't match!\n Expected:" + expected_output + "\n Got:" + text)
					if DEBUG:
						for i, s in enumerate(difflib.ndiff(text, expected_output)):
							if s[0] == ' ': continue
							elif s[0] == '-':
								print(u'Delete "{}" from position {}'.format(list(s[-1]), i))
							elif s[0] == '+':
								print(u'Add "{}" to position {}'.format(s[-1], i))
					exit(-1)
		
		if not monitor_mode:
			return
	
""""
  @brief Interrupt Controller/UART test
  
  @param [in] console - console to iterate with Syntacore platform
"""
def check_irq(console):
	test_string = "Interrupt-driven console works!\r\n"
	
	print("[IRQ]: Checking Interrupts: UART, Interrupt Controller..")

	console.write(test_string.encode())
	time.sleep(1)
	verify_output(console, test_string)

""""
  @brief MTIMER test
  
  @param [in] console - console to iterate with Syntacore platform
"""
def check_timer(console):
	test = "MTIMER\r"
	expected_output = """MTIMER\r
Timer test begins\r
Seconds passed 1\r
Seconds passed 2\r
Seconds passed 3\r
Seconds passed 4\r
Seconds passed 5\r
Seconds passed 6\r
Seconds passed 7\r
Seconds passed 8\r
Seconds passed 9\r
Seconds passed 10\r\n"""
	
	print("[MTIMER]: Running MTIMER check..")
	
	console.write(test.encode())
	time.sleep(11)
	verify_output(console, expected_output)
	
""""
  @brief SMP/ICCM test
  
  @param [in] console - console to iterate with Syntacore platform
"""
def check_smp(console):
	test = "SMP\r"
	expected_output = 'SMP test completed'
	
	print("[SMP]: Checking SMP, ICCM..")
	console.write(test.encode())
	time.sleep(5)
	output = verify_output(console, expected_output, relaxed = True)
	
	print(output.splitlines()[1])
	
""""
  @brief Platform boot-up test
  
  @param [in] console - console to iterate with Syntacore platform
"""
def check_bootup(console):
	expected_output = "Syntacore Smoke Tests Utility"
	
	print("[BOOTUP]: Waiting for platform bootup, checking: Memory Protection, UART, Caches..")
	verify_output(console, expected_output, relaxed = True, monitor_mode = True)
	
""""
  @brief Runs specific test
  
  @param [in] test - test function
  @param [in] console - console to iterate with Syntacore platform
"""
def run_test(test, console):
	global tests_passed
	
	test(console)
	tests_passed += 1
		
""""
  @brief Console initialization
  
  @param [in] serial_tty - path to serial interface connected to Syntacore platform
  
  @return console to iterate with Syntacore platform
"""
def init_console(serial_tty):
	console = open(serial_tty, "wb+", buffering = 0)
	os.set_blocking(console.fileno(), False)
	return console
	
""""
  @brief Syntacore Smoke Test Runner entry point
"""
if __name__ == '__main__':
	
	tests_lookup = {
		'BOOTUP': check_bootup,
		'IRQ': check_irq,
		'MTIMER': check_timer,
		'SMP': check_smp,
	}

	parser = argparse.ArgumentParser(epilog = "Example: python3 scr_runner.py /dev/ttySCR BOOTUP IRQ MTIMER SMP")
	parser.add_argument("tty", help = "Serial console to iterate with SCR platform, e.g /dev/ttySCR")
	parser.add_argument("tests", help="List of tests to run", nargs = '+', choices = tests_lookup.keys())
	
	args = parser.parse_args()
	tests =  args.tests
	console = init_console(args.tty)
	
	print("""
________________________________       __   __
                                * * * //|| //||
Syntacore Smoke Test Runner    * * * // ||// ||
_____________________________ * * * //  |_/  ||
""")
	
	print("Scheduled tests: ", tests, "\n")
	
	for test in tests:
		tests_lookup[test](console)
		
	console.close()
	
	print("\n +++ SMOKE TESTS PASSED! +++\n")