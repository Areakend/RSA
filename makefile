all:	checkPorts.c
	gcc -o checkPorts checkPorts.c
	sudo ./checkPorts
	rm checkPorts
