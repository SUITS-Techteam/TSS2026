#ifndef DATA_H
#define DATA_H

#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include "lib/cjson/cJSON.h"
#include "lib/simulation/sim_engine.h"
#include <stdlib.h>
#include <stdio.h>  

// UDP command mapping structure
typedef struct {
    unsigned int command;
    const char* path;        // full dot-separated path like "eva.uia.eva1_power"
    const char* data_type;   // bool or float, this makes the parsing easier
} udp_command_mapping_t;

struct backend_data_t {
    // Timing information
    uint32_t start_time;
    uint32_t server_up_time;
    uint32_t time_since_last_ping;

    // Simulation engine
    sim_engine_t* sim_engine;
};

// Backend Lifecycle Functions
struct backend_data_t* init_backend();
void increment_simulation(struct backend_data_t* backend);
void cleanup_backend(struct backend_data_t*  backend);

// UDP Request Handlers
void handle_udp_get_request(unsigned int command, unsigned char* data, struct backend_data_t* backend);
bool handle_udp_post_request(unsigned int command, unsigned char* data, struct backend_data_t* backend);

// Data management
bool initialize_json_switch_states(void);
bool initialize_EVA_json_switch_states(void);
bool initialize_LTV_ERRORS_json_switch_states(void);
void update_json_file(const char* filename, const char* section, const char* field_path, char* new_value);
void sync_simulation_to_json(struct backend_data_t* backend);
cJSON* get_json_file(const char* filename);
void send_json_file(const char* filename, unsigned char* data);
void send_recovery_mode_json_file(const char* filename, unsigned char* data);
void update_eva_station_timing(void);
void reset_eva_station_timing(void);
void backend_reset_errors(void* ctx);
void update_sim_DCU_field_settings(sim_engine_t* sim_engine);
void update_error_states(sim_engine_t* sim_engine);
void update_EVA_error_simulation_error_states(sim_engine_t* sim_engine);
void update_O2_error_state(sim_engine_t* sim_engine);
void update_fan_error_state(sim_engine_t* sim_engine);
void update_power_error_state(sim_engine_t* sim_engine);
void update_scrubber_state_EVA(sim_engine_t* sim_engine);
void update_num_remaining_errors_LTV(sim_engine_t* engine);
void update_ltv_error_dependencies();

//UIA related functions
void update_sim_UIA_field_settings(sim_engine_t* sim_engine);
bool update_sim_UIA_connected(sim_engine_t* sim_engine);
void update_sim_active_states(sim_engine_t* sim_engine);
void initialize_UIA_override_dependent_values(sim_engine_t* sim_engine);

// Helper functions
void reverse_bytes(unsigned char* bytes);
bool big_endian();
bool html_form_json_update(char* request_content, struct backend_data_t* backend);
double get_field_from_json(const char* filename, const char* field_path, double default_value);

// UDP data extraction helpers
bool extract_bool_value(unsigned char* data);
float extract_float_value(unsigned char* data);

// UDP command to JSON path mapping table
// NOTE: most of these commands have been reused from the TSS 2025 project to help support backwards compatibility. In the future, it may be recommended to standardize these.
static const udp_command_mapping_t udp_command_mappings[] = {
    // UIA commands (sent from the peripheral device over UDP)
    {2001, "eva.uia.eva1_power", "bool"},
    {2002, "eva.uia.eva1_oxy", "bool"},
    {2003, "eva.uia.eva1_water_supply", "bool"},
    {2004, "eva.uia.eva1_water_waste", "bool"},
    {2005, "eva.uia.eva2_power", "bool"},
    {2006, "eva.uia.eva2_oxy", "bool"},
    {2007, "eva.uia.eva2_water_supply", "bool"},
    {2008, "eva.uia.eva2_water_waste", "bool"},
    {2009, "eva.uia.oxy_vent", "bool"},
    {2010, "eva.uia.depress", "bool"},

    // DCU commands (sent from the peripheral device over UDP)
    {2011, "eva.dcu.eva1.batt.lu", "bool"},
    {2012, "eva.dcu.eva1.oxy", "bool"},
    {2013, "eva.dcu.eva1.batt.ps", "bool"},
    {2014, "eva.dcu.eva1.fan", "bool"},
    {2015, "eva.dcu.eva1.pump", "bool"},
    {2016, "eva.dcu.eva1.co2", "bool"},

    // IMU position commands from the TSS-Location-App Python server
    {2017, "eva.imu.eva1.posx", "float"},
    {2018, "eva.imu.eva1.posy", "float"},
    {2019, "eva.imu.eva1.heading", "float"},
    {2020, "eva.imu.eva2.posx", "float"},
    {2021, "eva.imu.eva2.posy", "float"},
    {2022, "eva.imu.eva2.heading", "float"},

    //LTV Error commands
    {2023, "ltv_errors.error_procedures.0.needs_resolved", "bool"},
    {2024, "ltv_errors.error_procedures.1.needs_resolved", "bool"},
    {2025, "ltv_errors.error_procedures.2.needs_resolved", "bool"},
    {2026, "ltv_errors.error_procedures.3.needs_resolved", "bool"},
    {2027, "ltv_errors.error_procedures.4.needs_resolved", "bool"},
    {2028, "ltv_errors.error_procedures.5.needs_resolved", "bool"},
    {2029, "ltv_errors.error_procedures.6.needs_resolved", "bool"},
    {2030, "ltv_errors.error_procedures.7.needs_resolved", "bool"},

    {0, NULL, NULL} // Sentinel
};

#endif // DATA_H