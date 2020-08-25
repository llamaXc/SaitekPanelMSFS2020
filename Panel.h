#pragma once
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
#include <iostream>
#include <windows.h>


static enum LED_COLOR {
	OFF = 0x00,
	RED = 0x38,
	GREEN = 0x07,
	AMBER = 0x3f
};
static enum EVENT_ID {
	NONE = 0,
	KEY_MAGNETO1_OFF = 1,
	KEY_MAGNETO1_LEFT,
	KEY_MAGNETO1_RIGHT,
	KEY_MAGNETO1_BOTH,
	KEY_MAGNETO1_START,
	KEY_AVIONICS_MASTER_SET,
	KEY_TOGGLE_MASTER_ALTERNATOR,
	KEY_STROBES_SET,
	KEY_PITOT_HEAT_SET,
	KEY_LANDING_LIGHTS_SET,
	KEY_FUEL_PUMP,
	KEY_TOGGLE_STRUCTURAL_DEICE,
	KEY_PITOT_HEAT_TOGGLE,
	KEY_COWLFLAP1_SET,
	KEY_TOGGLE_NAV_LIGHTS,
	KEY_TOGGLE_TAXI_LIGHTS,
	KEY_TOGGLE_BEACON_LIGHTS,
	KEY_PANEL_LIGHTS_TOGGLE,

	SET_MASTER_BATTERY = 50,
	SET_LIGHT_NAV = 51,
	SET_AVIONICS_SWITCH = 52,
	SET_FUEL_PUMP = 53,
	SET_GEAR = 54,
	SET_LIGHTS = 55
//	SET_COWL_FLAP_PERCENT = 55
};

static enum  SWITCH_NAME {
	MASTER_BAT = 0,
	MASTER_ALT = 1,
	AVIONICS_MASTER = 2,
	FUEL_PUMP = 3,
	DE_ICE = 4,
	LANDING_GEAR = 5,
	PITOT_HEAT = 6,
	MAG_LEFT = 7,
	MAG_RIGHT = 8,
	MAG_OFF = 9,
	MAG_START = 10,
	MAG_ALL = 11,
	COWL_FLAP =12 ,
	LIGHT_PANEL = 13,
	LIGHT_BEACON = 14,
	LIGHT_NAV = 15,
	LIGHT_STROBE = 16,
	LIGHT_TAXI = 17,
	LIGHT_LANDING = 18,
	KEY_GEAR_UP = 19,
	KEY_GEAR_DOWN = 20
};

struct Switch {
	unsigned int bit;
	unsigned int times_switched;
	bool state;
	bool sim_state;
	bool last_state;
	bool toggle_only;
	bool is_event;
	EVENT_ID action_id;
	unsigned int row;
	bool update = false;
	SWITCH_NAME id;
	std::string string_id;
	bool correct_from_sim = false;
};


struct STATE {
	char state;
};


class Panel
{


public:
	std::map<SWITCH_NAME, Switch> switches;
	
	Panel();
	char is_switch_on( unsigned char buf, int bit);
	bool did_switch_change(Switch cur_switch);
	void update(unsigned char buf[256]);
	void print_states(HANDLE stdOut, bool clearScreen, bool printLatestPanelPriority);

};

