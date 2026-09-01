#ifndef SIM_ENGINE_H
#define SIM_ENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "../cjson/cJSON.h"

///////////////////////////////////////////////////////////////////////////////////
//                                  Constants
///////////////////////////////////////////////////////////////////////////////////

#define SIM_DATA_ROOT "data"
#define SIM_CONFIG_ROOT "src/lib/simulation/config"

#define INITIAL_NUM_TASK_BOARD_ERRORS 4

///////////////////////////////////////////////////////////////////////////////////
//                                  Data Types
///////////////////////////////////////////////////////////////////////////////////

typedef enum {
    SIM_TYPE_FLOAT
} sim_field_type_t;

typedef enum {
    SIM_ALGO_SINE_WAVE,
    SIM_ALGO_LINEAR_DECAY,
    SIM_ALGO_LINEAR_GROWTH,
    SIM_ALGO_DEPENDENT_VALUE,
    SIM_ALGO_EXTERNAL_VALUE,
    SIM_ALGO_CONSTANT_VALUE
} sim_algorithm_type_t;

typedef union {
    float f;
} sim_value_t;

//typedef for ltv reset function
typedef void (*ltv_reset_fn)(void* ctx);
typedef void (*sim_reset_errors_fn)(void* ctx);

typedef struct {
    char* field_name;
    char* component_name;
    sim_field_type_t type;
    sim_algorithm_type_t algorithm;
    sim_algorithm_type_t starting_algorithm; // Store the original algorithm for reset purposes
    sim_value_t current_value;
    sim_value_t min_value;
    sim_value_t max_value;
    sim_value_t start_value;
    sim_value_t base_value; // Used for sine wave algorithm
    sim_value_t amplitude; // Used for sine wave algorithm
    sim_value_t frequency; // Used for sine wave algorithm
    sim_value_t phase_acc; // Used for sine wave algorithm
    sim_value_t rate; // Used for linear growth/decay algorithms
    sim_value_t starting_rate;

    // Algorithm parameters (parsed from JSON)
    cJSON* params;

    // Dependencies
    char** depends_on;
    int depends_count;

    // Internal state for algorithms
    bool active; //whether the field should be actively updating (used for fields that depend on DCU commands)
    bool initialized;
} sim_field_t;

typedef struct {
    char* component_name;
    sim_field_t* fields;
    int field_count;
    bool running;  // Controls whether this component's fields update
    float simulation_time;  // Component-specific simulation time
} sim_component_t;

typedef struct {
    bool battery_lu;
    bool battery_ps;
    bool fan;
    bool o2;
    bool pump;
    bool co2;
} sim_DCU_field_settings_t;

typedef struct {
    bool eva1_power;
    bool eva1_oxy;
    bool eva1_water_supply;
    bool eva1_water_waste;
    bool eva2_power;
    bool eva2_oxy;
    bool eva2_water_supply;
    bool eva2_water_waste;
    bool oxy_vent;
    bool depress;
} sim_UIA_field_settings_t;

typedef struct {
    sim_component_t* components;
    int component_count;

    sim_field_t** update_order;  // Fields sorted by dependencies
    int total_field_count;

    //error throwing variables
    int num_task_board_errors;
    int time_to_complete_task_board;
    int error_time;
    int error_type;

    sim_DCU_field_settings_t* dcu_field_settings;
    sim_UIA_field_settings_t* uia_field_settings;

    ltv_reset_fn ltv_reset;
    void* ltv_ctx;

    sim_reset_errors_fn reset_errors;
    void* reset_ctx;

    bool initialized;
} sim_engine_t;

///////////////////////////////////////////////////////////////////////////////////
//                                 Engine API
///////////////////////////////////////////////////////////////////////////////////

// Engine lifecycle
sim_engine_t* sim_engine_create();
void sim_engine_destroy(sim_engine_t* engine);

// Configuration loading
bool sim_engine_load_predefined_configs(sim_engine_t* engine);
bool sim_engine_load_component(sim_engine_t* engine, const char* json_file_path);

// Simulation control
bool sim_engine_initialize(sim_engine_t* engine);
void sim_engine_update(sim_engine_t* engine, float delta_time);
void sim_engine_start_component(sim_engine_t* engine, const char* component_name);
void sim_engine_stop_component(sim_engine_t* engine, const char* component_name);
void sim_engine_reset_component(sim_engine_t* engine, const char* component_name,
                               void (*update_json)(const char*, const char*, const char*, char*));

// Field access
sim_value_t sim_engine_get_field_value(sim_engine_t* engine, const char* field_name);

// Component status
bool sim_engine_is_component_running(sim_engine_t* engine, const char* component_name);

//Get Component
sim_component_t* sim_engine_get_component(sim_engine_t* engine, const char* component_name);

// Utility functions
sim_field_t* sim_engine_find_field(sim_engine_t* engine, const char* field_name);
sim_field_t* sim_engine_find_field_within_component(sim_component_t* component, const char* field_name);

#endif // SIM_ENGINE_H