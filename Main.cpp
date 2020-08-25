#pragma once

#define MAX_STR 255

#include "./Panel.h"
#include <tchar.h>
#include <strsafe.h>
#include "./SimConnect.h"
#include <chrono>
#include <thread>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include "hidapi.h"
#include <string>
#include <map>
#include <io.h>
#include <wchar.h>
#include <chrono>

int     quit = 0; // Used to end the sim 
HANDLE  hSimConnect = NULL; // Simconnect connetion 
Panel panel;  // Saitek Panel with switches 
hid_device* handle; // Pointer to USB Saitek Panel 
int res;  // Result from reading USB data
unsigned char buf[256]; // Buffer to read USB data

SIMCONNECT_RECV* pData; // Receive data from SimConnect
DWORD cbData;  // Receive data from SimConnect
BOOL loaded = false; // Used to determine if we get latest data from MSFS2020 on load

BOOL debugTestMode = false;


/*************************************** Switch Request Setup ***************************/

//Request data for SimConnect
static enum DATA_DEFINE_ID {
	INITIAL_STATE
};

// A basic structure for a single item of returned data
struct InitialSwitchDatum {
	int        id;
	BOOL    value;
};

#define MAX_RETURNED_ITEMS   10

//Struct holding states of switches when sim loads 
//NOTE THIS MUST MATCH MAX ITEM RETURNED
struct InitialState {
	BOOL master_alt;
	BOOL fuel_pump;
	BOOL deice;
	BOOL strobe;
	BOOL nav;
	BOOL taxi;
	BOOL landing_light;
	BOOL beacon;
	BOOL panel_light; //10th item 
};

// A structure that can be used to receive Tagged data
struct InitialDatum {
	InitialSwitchDatum  datum[MAX_RETURNED_ITEMS];
};


static enum DATA_REQUEST_ID {
	INITIAL_STATE_REQUEST,
};

//Struct to send over SimConnect with button state 
struct SwitchStruct {
	BOOL state;
};


//Sets Landing Gear LED on panel
void set_led_color(LED_COLOR color) {
	buf[0] = 0x0;
	buf[1] = color;
	hid_send_feature_report(handle, buf, 17);
}

//Send our states when needed
//quick Assume Transmit
void transmit_states() {
	for (auto& cur_switch_value : panel.switches) {
		Switch* cur_switch = &cur_switch_value.second;
		BOOL allow_event_update = true;

		if (cur_switch->update || cur_switch->correct_from_sim) {
			if (cur_switch->is_event && !cur_switch->toggle_only) {

				//Moving a mag causes 2 switch update states. 
				//Since its a knob, only send the new state position
				if (cur_switch->id == SWITCH_NAME::MAG_ALL
					|| cur_switch->id == SWITCH_NAME::MAG_START 
					|| cur_switch->id == SWITCH_NAME::MAG_LEFT 
					|| cur_switch->id == SWITCH_NAME::MAG_RIGHT 
					|| cur_switch->id == SWITCH_NAME::MAG_OFF) {
					if (cur_switch->state == false) {
						allow_event_update = false;
					}
				}

				//Check for landing gear state and if we should send gear up or gear down handle requestt
				if (cur_switch->id == SWITCH_NAME::LANDING_GEAR) {
					if (cur_switch->state == 1) {
						set_led_color(LED_COLOR::GREEN);
						SimConnect_TransmitClientEvent(hSimConnect, 0, SWITCH_NAME::KEY_GEAR_DOWN, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
					}else {
						set_led_color(LED_COLOR::OFF);
						SimConnect_TransmitClientEvent(hSimConnect, 0, SWITCH_NAME::KEY_GEAR_UP, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
					}
				}

				if (allow_event_update) {
					SwitchStruct switch_to_send;
					switch_to_send.state = cur_switch->state;
					SimConnect_TransmitClientEvent(hSimConnect, 0, cur_switch->action_id, switch_to_send.state, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
				}
			}else if (cur_switch->toggle_only) {
				//sim state is lagging by 1 second 
				if (cur_switch->sim_state != cur_switch->state) {
					cur_switch->sim_state = cur_switch->state;
					SimConnect_TransmitClientEvent(hSimConnect, 0, cur_switch->action_id, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
				}
			}else {
				SwitchStruct switch_to_send;
				switch_to_send.state = cur_switch->state;
				SimConnect_SetDataOnSimObject(hSimConnect, cur_switch->action_id, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(switch_to_send), &switch_to_send);
			}

			cur_switch->update = false;
		}
	}
}

//Listen to sim to see if switches have changed in the cockpit.
bool getLatestToggleSwitchStates() {

	int result = SimConnect_GetNextDispatch(hSimConnect, &pData, &cbData);
	bool stillLooking = true;
	bool gotData = false;
	for (int i = 0; i < 10; i++) {
		if (SUCCEEDED(result) && stillLooking) {

			switch (pData->dwID) {

			case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:

				SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE* pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)pData;

				switch (pObjData->dwRequestID) {
				case INITIAL_STATE_REQUEST:
					DWORD ObjectID = pObjData->dwObjectID;
					InitialDatum* pS = (InitialDatum*)&pObjData->dwData;
					stillLooking = false;

					for (int i = 0; i < (int)pObjData->dwDefineCount; i++) {
						//		std::cout << panel.switches[static_cast<SWITCH_NAME>(pS->datum[i].id)].string_id << " " << panel.switches[static_cast<SWITCH_NAME>(pS->datum[i].id)].sim_state <<
						//		" " << panel.switches[static_cast<SWITCH_NAME>(pS->datum[i].id)].state << std::endl;
					
						gotData = true; // This is good, we can now read our data and sync with the sim
						panel.switches[static_cast<SWITCH_NAME>(pS->datum[i].id)].correct_from_sim = true;
						panel.switches[static_cast<SWITCH_NAME>(pS->datum[i].id)].sim_state = pS->datum[i].value;
						
					}

					break;

				} // End of switch case
			} // End of switch 

			Sleep(10);
		}
	}
	return gotData;
}

//Used for clearing console
//Got from windows api 
bool debug = false;
void fillConsoleWithSpace(HANDLE hConsole) {
	if (debug == false) {
		COORD coordScreen = { 0, 0 };    // home for the cursor 
		DWORD cCharsWritten;
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		DWORD dwConSize;
		if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
			return;
		}
		dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
		if (!FillConsoleOutputCharacter(hConsole,        // Handle to console screen buffer 
			(TCHAR)' ',     // Character to write to the buffer
			dwConSize,       // Number of cells to write 
			coordScreen,     // Coordinates of first cell 
			&cCharsWritten)) {
			return;
		}

		if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
			return;
		}
		if (!FillConsoleOutputAttribute(hConsole,         // Handle to console screen buffer 
			csbi.wAttributes, // Character attributes to use
			dwConSize,        // Number of cells to set attribute 
			coordScreen,      // Coordinates of first cell 
			&cCharsWritten)) {
			return;
		}
		SetConsoleCursorPosition(hConsole, coordScreen);
	}
}

//On load search for our panel
hid_device* findSaitekPanel() {
	struct hid_device_info* devs, * cur_dev;

	devs = hid_enumerate(0x0, 0x0);
	cur_dev = devs;

	const std::string saitek_panel_product = "Saitek Pro Flight Switch Panel";
	int product_id = 0;
	int vendor_id = 0;
	while (cur_dev) {
		printf("Found USB Device:      %ls\n", cur_dev->product_string);

		char buffer[256];
		int output = sprintf_s(buffer, "%ls", cur_dev->product_string);

		if (strcmp(buffer, saitek_panel_product.c_str()) == 0) {
			product_id = cur_dev->product_id;
			vendor_id = cur_dev->vendor_id;
			printf("Opening %ls Vendor ID: %d Product ID: %d\n", cur_dev->product_string, vendor_id, product_id);
			break;
		}

		cur_dev = cur_dev->next;
	}

	hid_free_enumeration(devs); //Free up device pointers 
	return hid_open(vendor_id, product_id, NULL); // Return Opened USB Device 
}

void shutdown() {

	hid_close(handle);
	hid_exit();

	SimConnect_Close(hSimConnect);
}

//Main
int __cdecl _tmain(int argc, _TCHAR* argv[]){

#ifdef WIN32
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
#endif
	HRESULT hr;

	//Attempt to see if sim is open
	if (debugTestMode == false && FAILED(SimConnect_Open(&hSimConnect, "Saitek Panel", NULL, 0, 0, 0))){
		printf("\n\n\033[0;31mWarning: \033[0mCould not find Microsoft Flight Simulator 2020\nPlease ensure it is running first then relaunch this application.\n\n");
		std::string ack;
		printf("\n Press any key and hit enter to close\n");
		std::cin >> ack;

		return -1;
	}

	/******************************************* CONFIGURE SIMCONNECT DATA FIELDS ******************************************************/
	//Define data for inital state request
	hr = SimConnect_AddToDataDefinition(hSimConnect, INITIAL_STATE, "GENERAL ENG MASTER ALTERNATOR", "BOOL", SIMCONNECT_DATATYPE_INT32, 0, SWITCH_NAME::MASTER_ALT);
	hr = SimConnect_AddToDataDefinition(hSimConnect, INITIAL_STATE, "GENERAL ENG FUEL PUMP SWITCH", "BOOL", SIMCONNECT_DATATYPE_INT32, 0, SWITCH_NAME::FUEL_PUMP);
	hr = SimConnect_AddToDataDefinition(hSimConnect, INITIAL_STATE, "STRUCTURAL DEICE SWITCH", "BOOL", SIMCONNECT_DATATYPE_INT32, 0, SWITCH_NAME::DE_ICE);
	hr = SimConnect_AddToDataDefinition(hSimConnect, INITIAL_STATE, "LIGHT STROBE", "BOOL", SIMCONNECT_DATATYPE_INT32, 0, SWITCH_NAME::LIGHT_STROBE);
	hr = SimConnect_AddToDataDefinition(hSimConnect, INITIAL_STATE, "LIGHT NAV", "BOOL", SIMCONNECT_DATATYPE_INT32, 0, SWITCH_NAME::LIGHT_NAV);
	hr = SimConnect_AddToDataDefinition(hSimConnect, INITIAL_STATE, "LIGHT TAXI", "BOOL", SIMCONNECT_DATATYPE_INT32, 0, SWITCH_NAME::LIGHT_TAXI);
	hr = SimConnect_AddToDataDefinition(hSimConnect, INITIAL_STATE, "LIGHT LANDING", "BOOL", SIMCONNECT_DATATYPE_INT32, 0, SWITCH_NAME::LIGHT_LANDING);
	hr = SimConnect_AddToDataDefinition(hSimConnect, INITIAL_STATE, "LIGHT BEACON", "BOOL", SIMCONNECT_DATATYPE_INT32, 0, SWITCH_NAME::LIGHT_BEACON);
	hr = SimConnect_AddToDataDefinition(hSimConnect, INITIAL_STATE, "LIGHT PANEL", "BOOL", SIMCONNECT_DATATYPE_INT32, 0, SWITCH_NAME::LIGHT_PANEL);

	//Set Simulation Definitions
	hr = SimConnect_AddToDataDefinition(hSimConnect, SET_AVIONICS_SWITCH, "AVIONICS MASTER SWITCH", NULL, SIMCONNECT_DATATYPE_INT32);
	hr = SimConnect_AddToDataDefinition(hSimConnect, SET_MASTER_BATTERY, "ELECTRICAL MASTER BATTERY", "BOOL", SIMCONNECT_DATATYPE_INT32);
	hr = SimConnect_AddToDataDefinition(hSimConnect, SET_GEAR, "GEAR POSITION", "BOOL", SIMCONNECT_DATATYPE_INT32);

	//Send Simulation Events 
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_AVIONICS_MASTER_SET, "AVIONICS_MASTER_SET");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_MAGNETO1_OFF, "MAGNETO1_OFF");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_MAGNETO1_LEFT, "MAGNETO1_LEFT");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_MAGNETO1_RIGHT, "MAGNETO1_RIGHT");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_MAGNETO1_BOTH, "MAGNETO1_BOTH");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_MAGNETO1_START, "MAGNETO1_START");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_STROBES_SET, "STROBES_SET");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_LANDING_LIGHTS_SET, "LANDING_LIGHTS_SET");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_TOGGLE_MASTER_ALTERNATOR, "TOGGLE_MASTER_ALTERNATOR");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_FUEL_PUMP, "FUEL_PUMP");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_TOGGLE_STRUCTURAL_DEICE, "TOGGLE_STRUCTURAL_DEICE");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_PITOT_HEAT_SET, "PITOT_HEAT_SET");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_COWLFLAP1_SET, "COWLFLAP1_SET");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_TOGGLE_NAV_LIGHTS, "TOGGLE_NAV_LIGHTS");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_TOGGLE_BEACON_LIGHTS, "TOGGLE_BEACON_LIGHTS");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_TOGGLE_TAXI_LIGHTS, "TOGGLE_TAXI_LIGHTS");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_PANEL_LIGHTS_TOGGLE, "PANEL_LIGHTS_TOGGLE");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_GEAR_UP, "GEAR_UP");
	hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_GEAR_DOWN, "GEAR_DOWN");


	hr = SimConnect_RequestDataOnSimObject(hSimConnect, INITIAL_STATE_REQUEST, INITIAL_STATE,
		SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SECOND,
		SIMCONNECT_DATA_REQUEST_FLAG_CHANGED | SIMCONNECT_DATA_REQUEST_FLAG_TAGGED);


	//Wait for sim to send first batch of switches
	bool loadStatus = false;
	for (int i = 0; i < 20; i++) {
		loadStatus = getLatestToggleSwitchStates();
		if (loadStatus == true) {
			break;
		}
		Sleep(100);
	}


	if (!loadStatus && debugTestMode == false) {
		printf("\n\nFailed to connect to sim and get inital states\n");
		std::string ack;
		printf("\n Press any key and hit enter to close\n");
		std::cin >> ack;
		shutdown();
		return 0;
	}

	/********************************* CONFIGURE HID USB DEVICE ***************************************/
	if (hid_init()) {
		printf("\nFailed to open HID device\n\n");
		std::string ack;
		printf("\n Press any key and hit enter to close\n");
		std::cin >> ack;
		shutdown();
		return 0;
	}

	handle = findSaitekPanel();
	if (!handle) {
		printf("\n\nFailed to locate \'Saitek Pro Flight Switch Panel\'\n");
		printf("Leave an issue and I wil fix this, please include the output from the application.\n\n");

		std::string ack;
		printf("\n Press any key and hit enter to close\n");
		std::cin >> ack;
		shutdown();
		return 0;
	}
	
	//Set up nonblocking and ready the bfufers
	hid_set_nonblocking(handle, 1);
	memset(buf, 0, sizeof(buf));

	bool end = false;
	//Store old previous state of USB device
	int row0state = -1;
	int row1state = -1;
	int row2state = -1;


	//Used to keep track of where in loading/sending proccess we are
	bool transmitStates = false;
	bool loadedInitialSwitchStates = false;
	bool initialStatePrinted = false;
	bool userConfirmed = false;
	bool cockpitUpdate = false;

	HANDLE hStdOut = GetStdHandle(-11);


	printf("\n\n======= Loading Panel Switches ======\n");
	printf("To load the panel, please move one switch!\n");

	std::chrono::steady_clock::time_point lastStateRequestTime = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

	while (end == false) {	 

		//Check for new changes in cockpit to reflect panel state at 2HZ
		currentTime = std::chrono::steady_clock::now();
		cockpitUpdate = false;
		double difference = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastStateRequestTime).count();
		if (difference > 500) {
			lastStateRequestTime = std::chrono::steady_clock::now();
			cockpitUpdate = getLatestToggleSwitchStates();
			if (cockpitUpdate) {

				panel.print_states(hStdOut, false, false);
				transmit_states();
			}
		}
		

		//Listen to Saitek Panel and read buffer states
		if (hid_read(handle, buf, sizeof(buf)) >= 2) {
	
			//If this is the very first load, then update the panel and configure the program to display correct states 
			//The next step in this program will ask user to toggle any switch to confirm. 
			if (loadedInitialSwitchStates == false) {
				panel.update(buf);
				row0state = buf[0];
				row1state = buf[1];
				row2state = buf[2];
				loadedInitialSwitchStates = true;
			}

			//Only update Saitek Panel internal states if a new state has been read.
			if (row0state != buf[0] || row1state != buf[1] || row2state != buf[2]) {

				panel.update(buf); // Update Logic in panel

				//Determine if this is the confirmation switch state
				//Begin sending out info to sim 
				if(loadedInitialSwitchStates && userConfirmed == false){
					loadedInitialSwitchStates = true;
					transmitStates = true;
					userConfirmed = true;
				}

				//If panel is configured then send out the state
				if (transmitStates) {
					transmit_states(); //Transmit to simconnect
				}

				//Store buffer to compare for new state events
				row0state = buf[0];
				row1state = buf[1];
				row2state = buf[2];

				//Clear and print to console 
				fillConsoleWithSpace(hStdOut);
				panel.print_states(hStdOut, true, true);

			}
		}

		//Show user the inital state of the switches it found 
		if (loadedInitialSwitchStates && initialStatePrinted == false) {
			printf("\n\033[0;36mIf these states are correct move any switch to begin\033[0m\n");

			panel.print_states(hStdOut, false, false);
			initialStatePrinted = true;
		}
		
		Sleep(10); // CPU Lives Matter 
	}

	shutdown();
	return 0;
}
