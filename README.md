# ECN_M1-CORO_S2-Project
Measuring Power Consumption on the Linux Operating System

INSTALLATION:
	- Go to src folder and run "sudo make install".
	
USE:
	- Prerequisit: Go to src/build sub-folder and run "sudo insmod lpc-mod.ko".
	- Read data: data retrieved by the kernel module is contained in the /usr/share/ecn/lpc folder in the lpc.dat file.
	- Change interrupt value: write the new interrupt value in the batIntrptPeriod file located at /sys/kernel/config/ecn_lpc_mod.
	- Exit: run "sudo rmmod lpc-mod.ko" in the src/build sub-folder.
	
UNINSTALL:
	- run "sudo make clean" in the src folder.
