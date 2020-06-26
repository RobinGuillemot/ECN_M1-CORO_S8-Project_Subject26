# ECN_M1-CORO_S2-Project
Measuring Power Consumption on the Linux Operating System

New updates to come at https://github.com/RobinGuillemot/ECN_M1-CORO_S8-Project_Subject26

INSTALLATION:
	- Go to src folder and run "sudo make install".
	
USE:
	- Start data recovery and save : Go to src/build sub-folder and run "sudo insmod lpc-mod.ko".
	- Read data: Go to /usr/lib/ecn and run the lpc executable in a terminal. If the data recovering is in 
	progress, new data will be displayed as soon as it is available.
	- Change interrupt value: write the new interrupt value in the batIntrptPeriod file located at /sys/kernel/config/ecn_lpc_mod.
	- Exit: run "sudo rmmod lpc-mod.ko" in the src/build sub-folder.
	
UNINSTALL:
	- run "sudo make clean" in the src folder.
