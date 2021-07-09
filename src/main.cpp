
#include <modbus/modbus.h>

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>

static void show_help();

int main(int argc, char** argv) {
	const char* ip = nullptr;
	int port = -1;

	bool railLayout = false;
	bool pdoMapping = false;
	bool settings = false;

	// Very cool argument parsing
	for(int i = 1; i < argc; i++) {
		const char* arg = argv[i];

		if(!strcmp(arg,"-l")) {
			railLayout = true;
		}
		else if(!strcmp(arg, "-p")) {
			pdoMapping = true;
		}
		else if(!strcmp(arg, "-h")) {
			show_help();
			exit(0);
		}
		else if(!strcmp(arg, "-s")) {
			settings = true;
		}
		else {
			if(!ip) {
				ip = arg;
			}
			else if(port == -1) {
				port = strtol(arg, nullptr, 10);
				if(errno) {
					printf("Invalid port number: %s\n", arg);
					exit(1);
				}
			}
			else {
				printf("Unknown argument '%s'. Use -h for help", arg);
				exit(1);
			}
		}
	}

	if(!ip || port == -1) {
		show_help();
		exit(1);
	}

	modbus_t* mb = modbus_new_tcp("10.0.0.3", 502);
	if(modbus_connect(mb)) {
		printf("Could not connect to %s:%d\n", ip, port);
		modbus_free(mb);
		exit(1);
	}

	// Display rail layout
	if(railLayout) {
		uint16_t terminals[0xFF];
		*terminals = 0;
		modbus_read_input_registers(mb, 0x6000, 125, terminals);
		modbus_read_input_registers(mb, 0x607D, 125, terminals + 125);
		modbus_read_input_registers(mb, 0x60FA, 5, terminals + 250);

		printf("Rail layout:\n");
		for(int i = 0; i < 0xFF; i++) {
			if(terminals[i] == 0) 
				break;
			printf(" %d\n", (int)terminals[i]);
		}
	}

	if(pdoMapping) {
		// 0x1010 analout
		// 0x1011 analin
		// 0x1012 digout
		// 0x1013 digin
		uint16_t pdoSizes[4] = {};
		modbus_read_input_registers(mb, 0x1010, 0x4, pdoSizes);

		printf("PDO Sizes:\n");
		printf(" Analog output PDO size: %d\n", pdoSizes[0]);
		printf(" Analog input PDO size: %d\n", pdoSizes[1]);
		printf(" Digital output PDO size: %d\n", pdoSizes[2]);
		printf(" Digital input PDO size: %d\n", pdoSizes[3]);
	}

	// Prints settings
	if(settings) {
		uint16_t settingsData[0x4];
		modbus_read_registers(mb, 0x1120, 0x4, settingsData);

		const char* wdtType = "invalid";
		uint16_t wdtTypeI = settingsData[2];
		switch(wdtTypeI) {
			case 2: wdtType = "disable"; break;
			case 1: wdtType = "telegram (default)"; break;
			case 0: wdtType = "write telegram"; break;
			default: break;
		}

		const char* fallbackMode = "invalid";
		switch(settingsData[3]) {
			case 2: fallbackMode = "stop EBus"; break;
			case 1: fallbackMode = "freeze"; break;
			case 0: fallbackMode = "set to 0 (default)"; break;
			default: break;
		}

		printf("Settings:\n");
		printf(" Watchdog time: %dms\n", settingsData[0]);
		printf(" Watchdog type: %s\n", wdtType);
		printf(" Fallback mode: %s\n", fallbackMode);
	}

	modbus_close(mb);
	modbus_free(mb);

	return 0;
}

static void show_help() {
	printf("USAGE: ek9k-ctl [-h] [-l] [-p]\n");
	printf("       -l    - List terminals on rail\n");
	printf("       -p    - Display PDO mapping\n");
	printf("       -s    - Display settings\n");
	printf("       -h    - Display help text\n");
}
