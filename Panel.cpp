#include "Panel.h"
#include <iomanip>


char Panel::is_switch_on( unsigned char buf, int bit) {
	if (buf & 1 << bit) {
		return true;
	}else {
		return false;

	}
}

bool Panel::did_switch_change(Switch cur_switch) {
	return cur_switch.last_state != cur_switch.state;
}

void Panel::update(unsigned char buf[256]) {
	for (auto& cur_switch : switches) {

		//Save the last state
		cur_switch.second.last_state = cur_switch.second.state;
		cur_switch.second.state = is_switch_on(buf[cur_switch.second.row], cur_switch.second.bit);

		bool did_change = did_switch_change(cur_switch.second);

		if (did_change) {
			cur_switch.second.correct_from_sim = false;

			cur_switch.second.update = true;
		}
		
	}
}

void Panel::print_states(HANDLE stdOut, bool clearScreen, bool printLatestPanelPriority) {

	int alignMiddle = 25;
	int spaceNeeded = 0;
	
	//Used to keep console small and overwrite old text
	if (clearScreen) {
		CONSOLE_SCREEN_BUFFER_INFO cbsi;
		if (GetConsoleScreenBufferInfo(stdOut, &cbsi))
		{
			SetConsoleCursorPosition(stdOut, { 0, cbsi.dwCursorPosition.Y - 19 });
		}
	}

	//Print each switch 
	for (auto& cur_switch : switches) {
		spaceNeeded = alignMiddle - cur_switch.second.string_id.length();
		if (spaceNeeded <= 0) {
			spaceNeeded = 1;
		}

		std::cout << cur_switch.second.string_id << std::setw(spaceNeeded) << " : " << std::setw(10);
		if (cur_switch.second.toggle_only && printLatestPanelPriority == false) {
			std::cout << (cur_switch.second.state ? "\033[0;32m[PANEL_ON]\033[0m" : "\033[0;31m[PANEL_ON]\033[0m")
			<< std::setw(17) << (cur_switch.second.sim_state ? "\033[0;32m[SIM_ON]\033[0m" : "\033[0;31m[SIM_OFF]\033[0m") << std::endl;
		}else {
			std::cout << (cur_switch.second.state ? "\033[0;32m[PANEL_ON]\033[0m" : "\033[0;31m[PANEL_OFF]\033[0m")<< std::setw(17)
				<< (cur_switch.second.state ? "\033[0;32m[SIM_ON]\033[0m" : "\033[0;31m[SIM_OFF]\033[0m") << std::endl;
		}
		
	}
}



Panel::Panel() {
	Switch master_alt;
	master_alt.bit = 1;
	master_alt.state = 0;
	master_alt.sim_state = 0;
	master_alt.times_switched = 0;
	master_alt.last_state = 0;
	master_alt.toggle_only = 1;
	master_alt.is_event = 0;
	master_alt.row = 0;
	master_alt.id = SWITCH_NAME::MASTER_ALT;
	master_alt.action_id = EVENT_ID::KEY_TOGGLE_MASTER_ALTERNATOR;
	master_alt.string_id = "Master Alternator";

	Switch master_battery;
	master_battery.bit = 0;
	master_battery.state = 0;
	master_battery.sim_state = 0;
	master_battery.times_switched = 0;
	master_battery.last_state = 0;
	master_battery.toggle_only = 0;
	master_battery.is_event = 0;
	master_battery.row = 0;
	master_battery.id = SWITCH_NAME::MASTER_BAT;
	master_battery.action_id = EVENT_ID::SET_MASTER_BATTERY;
	master_battery.string_id = "Master Battery";

	Switch avionics_master;
	avionics_master.bit = 2;
	avionics_master.state = 0;
	avionics_master.sim_state = 0;
	avionics_master.times_switched = 0;
	avionics_master.last_state = 0;
	avionics_master.toggle_only = 0;
	avionics_master.is_event = 1;
	avionics_master.row = 0;
	avionics_master.id = SWITCH_NAME::AVIONICS_MASTER;
	avionics_master.action_id = EVENT_ID::KEY_AVIONICS_MASTER_SET;
	avionics_master.string_id = "Avionics Master";

	Switch fuel_pump;
	fuel_pump.bit = 3;
	fuel_pump.state = 0;
	fuel_pump.sim_state = 0;
	fuel_pump.times_switched = 0;
	fuel_pump.last_state = 0;
	fuel_pump.toggle_only = 1;
	fuel_pump.is_event = 1;
	fuel_pump.row = 0;
	fuel_pump.id = SWITCH_NAME::FUEL_PUMP;
	fuel_pump.action_id = EVENT_ID::KEY_FUEL_PUMP;
	fuel_pump.string_id = "Fuel Pump";

	Switch left_mag;
	left_mag.bit = 7;
	left_mag.state = 0;
	left_mag.sim_state = 0;
	left_mag.times_switched = 0;
	left_mag.last_state = 0;
	left_mag.toggle_only = 0;
	left_mag.is_event = 1;
	left_mag.row = 1;
	left_mag.id = SWITCH_NAME::MAG_LEFT;
	left_mag.action_id = EVENT_ID::KEY_MAGNETO1_LEFT;
	left_mag.string_id = "Mag Left";

	Switch right_mag;
	right_mag.bit = 6;
	right_mag.state = 0;
	right_mag.sim_state = 0;
	right_mag.times_switched = 0;
	right_mag.last_state = 0;
	right_mag.toggle_only = 0;
	right_mag.is_event = 1;
	right_mag.row = 1;
	right_mag.id = SWITCH_NAME::MAG_RIGHT;
	right_mag.action_id = EVENT_ID::KEY_MAGNETO1_RIGHT;
	right_mag.string_id = "Mag Right";

	Switch off_mag;
	off_mag.bit = 5;
	off_mag.state = 0;
	off_mag.sim_state = 0;
	off_mag.times_switched = 0;
	off_mag.last_state = 0;
	off_mag.toggle_only = 0;
	off_mag.is_event = 1;
	off_mag.row = 1;
	off_mag.id = SWITCH_NAME::MAG_OFF;
	off_mag.action_id = EVENT_ID::KEY_MAGNETO1_OFF;
	off_mag.string_id = "Mag Off";

	Switch all_mag;
	all_mag.bit = 0;
	all_mag.state = 0;
	all_mag.sim_state = 0;
	all_mag.times_switched = 0;
	all_mag.last_state = 0;
	all_mag.toggle_only = 0;
	all_mag.is_event = 1;
	all_mag.row = 2;
	all_mag.id = SWITCH_NAME::MAG_ALL;
	all_mag.action_id = EVENT_ID::KEY_MAGNETO1_BOTH;
	all_mag.string_id = "Mag All/Both";

	Switch start_mag;
	start_mag.bit = 1;
	start_mag.state = 0;
	start_mag.sim_state = 0;
	start_mag.times_switched = 0;
	start_mag.last_state = 0;
	start_mag.toggle_only = 0;
	start_mag.is_event = 1;
	start_mag.row = 2;
	start_mag.id = SWITCH_NAME::MAG_START;
	start_mag.action_id = EVENT_ID::KEY_MAGNETO1_START;
	start_mag.string_id = "Start Mag";

	Switch gear; //true when down
	gear.bit = 3;
	gear.state = 0;
	gear.sim_state = 0;
	gear.times_switched = 0;
	gear.last_state = 0;
	gear.toggle_only = 0;
	gear.is_event = 1;
	gear.row = 2;
	gear.id = SWITCH_NAME::LANDING_GEAR;
	gear.action_id = EVENT_ID::SET_GEAR;
	gear.string_id = "Gear Down";

	Switch deice;
	deice.bit = 4;
	deice.state = 0;
	deice.sim_state = 0;
	deice.times_switched = 0;
	deice.last_state = 0;
	deice.toggle_only = 1;
	deice.is_event = 1;
	deice.row = 0;
	deice.id = SWITCH_NAME::DE_ICE;
	deice.action_id = EVENT_ID::KEY_TOGGLE_STRUCTURAL_DEICE; 
	deice.string_id = "De-ice";

	Switch pitot_heat;
	pitot_heat.bit = 5;
	pitot_heat.state = 0;
	pitot_heat.sim_state = 0;
	pitot_heat.times_switched = 0;
	pitot_heat.last_state = 0;
	pitot_heat.toggle_only = 0;
	pitot_heat.is_event = 1;
	pitot_heat.row = 0;
	pitot_heat.id = SWITCH_NAME::PITOT_HEAT;
	pitot_heat.action_id = EVENT_ID::KEY_PITOT_HEAT_SET; 
	pitot_heat.string_id = "Pitot Heat";

	Switch cowl_flap;
	cowl_flap.bit = 6;
	cowl_flap.state = 0;
	cowl_flap.sim_state = 0;
	cowl_flap.times_switched = 0;
	cowl_flap.last_state = 0;
	cowl_flap.toggle_only = 0;
	cowl_flap.is_event = 1;
	cowl_flap.row = 0;
	cowl_flap.id = SWITCH_NAME::COWL_FLAP;
	cowl_flap.action_id = EVENT_ID::KEY_COWLFLAP1_SET; 
	cowl_flap.string_id = "Cowl Flap";

	Switch light_panel;
	light_panel.bit = 7;
	light_panel.state = 0;
	light_panel.sim_state = 0;
	light_panel.times_switched = 0;
	light_panel.last_state = 0;
	light_panel.toggle_only = 1;
	light_panel.is_event = 1;
	light_panel.row = 0;
	light_panel.id = SWITCH_NAME::LIGHT_PANEL;
	light_panel.action_id = EVENT_ID::KEY_PANEL_LIGHTS_TOGGLE;
	light_panel.string_id = "Panel Lights";

	Switch beacon_light;
	beacon_light.bit = 0;
	beacon_light.state = 0;
	beacon_light.sim_state = 0;
	beacon_light.times_switched = 0;
	beacon_light.last_state = 0;
	beacon_light.toggle_only = 1;
	beacon_light.is_event = 1;
	beacon_light.row = 1;
	beacon_light.id = SWITCH_NAME::LIGHT_BEACON;
	beacon_light.action_id = EVENT_ID::KEY_TOGGLE_BEACON_LIGHTS; 
	beacon_light.string_id = "Beacon Lights";

	Switch nav_light;
	nav_light.bit = 1;
	nav_light.state = 0;
	nav_light.sim_state = 0;
	nav_light.times_switched = 0;
	nav_light.last_state = 0;
	nav_light.toggle_only = 1;
	nav_light.is_event = 1;
	nav_light.row = 1;
	nav_light.id = SWITCH_NAME::LIGHT_NAV;
	nav_light.action_id = EVENT_ID::KEY_TOGGLE_NAV_LIGHTS; 
	nav_light.string_id = "Nav Lights";

	Switch strobe_light;
	strobe_light.bit = 2;
	strobe_light.state = 0;
	strobe_light.sim_state = 0;
	strobe_light.times_switched = 0;
	strobe_light.last_state = 0;
	strobe_light.toggle_only = 0;
	strobe_light.is_event = 1;
	strobe_light.row = 1;
	strobe_light.id = SWITCH_NAME::LIGHT_STROBE;
	strobe_light.action_id = EVENT_ID::KEY_STROBES_SET;
	strobe_light.string_id = "Strobe Lights";

	Switch taxi_light;
	taxi_light.bit = 3;
	taxi_light.state = 0;
	taxi_light.sim_state = 0;
	taxi_light.times_switched = 0;
	taxi_light.last_state = 0;
	taxi_light.toggle_only = 1;
	taxi_light.is_event = 1;
	taxi_light.row = 1;
	taxi_light.id = SWITCH_NAME::LIGHT_TAXI;
	taxi_light.action_id = EVENT_ID::KEY_TOGGLE_TAXI_LIGHTS;
	taxi_light.string_id = "Taxi Lights";

	Switch landing_light;
	landing_light.bit = 4;
	landing_light.state = 0;
	landing_light.sim_state = 0;
	landing_light.times_switched = 0;
	landing_light.last_state = 0;
	landing_light.toggle_only = 0;
	landing_light.is_event = 1;
	landing_light.row = 1;
	landing_light.id = SWITCH_NAME::LIGHT_LANDING;
	landing_light.action_id = EVENT_ID::KEY_LANDING_LIGHTS_SET; 
	landing_light.string_id = "Landing Lights";

	switches[SWITCH_NAME::MASTER_ALT] = master_alt;
	switches[SWITCH_NAME::MASTER_BAT] = master_battery;
	switches[SWITCH_NAME::AVIONICS_MASTER] = avionics_master;
	switches[SWITCH_NAME::MAG_LEFT] = left_mag;
	switches[SWITCH_NAME::MAG_RIGHT] = right_mag;
	switches[SWITCH_NAME::MAG_OFF] = off_mag;
	switches[SWITCH_NAME::MAG_ALL] = all_mag;
	switches[SWITCH_NAME::MAG_START] = start_mag;
	switches[SWITCH_NAME::LANDING_GEAR] = gear;
	switches[SWITCH_NAME::FUEL_PUMP] = fuel_pump;
	switches[SWITCH_NAME::DE_ICE] = deice;
	switches[SWITCH_NAME::PITOT_HEAT] = pitot_heat;
	switches[SWITCH_NAME::COWL_FLAP] = cowl_flap;
	switches[SWITCH_NAME::LIGHT_BEACON] = beacon_light;
	switches[SWITCH_NAME::LIGHT_NAV] = nav_light;
	switches[SWITCH_NAME::LIGHT_STROBE] = strobe_light;
	switches[SWITCH_NAME::LIGHT_TAXI] = taxi_light;
	switches[SWITCH_NAME::LIGHT_LANDING] = landing_light;
	switches[SWITCH_NAME::LIGHT_PANEL] = light_panel;
}