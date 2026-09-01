#include "sim_engine.h"
#include "sim_algorithms.h"
#include "throw_errors.h"
#include <string.h>
#include <math.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#endif

///////////////////////////////////////////////////////////////////////////////////
//                            Engine Lifecycle
///////////////////////////////////////////////////////////////////////////////////

/**
 * Creates and initializes a new simulation engine instance.
 *
 * @return Pointer to new simulation engine, or NULL if creation failed
 */
sim_engine_t* sim_engine_create() {
    sim_engine_t* engine = calloc(1, sizeof(sim_engine_t));
    if (!engine) return NULL;

    engine->initialized = false;

    return engine;
}

/**
 * Destroys a simulation engine instance and frees all associated memory.
 * 
 * @param engine Pointer to the simulation engine to destroy
 */
void sim_engine_destroy(sim_engine_t* engine) {
    if (!engine) return;
    
    // Free components and fields
    for (int i = 0; i < engine->component_count; i++) {
        sim_component_t* comp = &engine->components[i];
        
        for (int j = 0; j < comp->field_count; j++) {
            sim_field_t* field = &comp->fields[j];
            
            free(field->field_name);
            free(field->component_name);
            
            if (field->params) {
                cJSON_Delete(field->params);
            }
            
            // Free dependency array
            for (int k = 0; k < field->depends_count; k++) {
                free(field->depends_on[k]);
            }
            free(field->depends_on);
        }
        
        free(comp->component_name);
        free(comp->fields);
    }
    
    free(engine->components);
    free(engine->update_order);
    free(engine->dcu_field_settings);
    free(engine->uia_field_settings);
    free(engine);
}

///////////////////////////////////////////////////////////////////////////////////
//                         Configuration Loading
///////////////////////////////////////////////////////////////////////////////////

/**
 * Loads predefined JSON simulation configuration files.
 * Uses a hardcoded list of configuration files instead of scanning directories.
 * 
 * @param engine Pointer to the simulation engine
 * @return true if all files were loaded successfully, false otherwise
 */
bool sim_engine_load_predefined_configs(sim_engine_t* engine) {
    if (!engine) return false;
    
    // Predefined list of configuration files to load
    // @TODO can generalize this more in some way?
    const char* config_files[] = {
        SIM_CONFIG_ROOT "/eva1.json",
        SIM_CONFIG_ROOT "/eva2.json"
    };
    const int config_count = sizeof(config_files) / sizeof(config_files[0]);
    
    bool success = true;
    int loaded_count = 0;
    
    // Load each configuration file in a loop
    for (int i = 0; i < config_count; i++) {
        if (sim_engine_load_component(engine, config_files[i])) {
            loaded_count++;
        } else {
            printf("Warning: Failed to load component file: %s\n", config_files[i]);
            success = false;
        }
    }
    
    if (loaded_count == 0) {
        printf("Error: No configuration files were loaded successfully.\n");
        return false;
    }
    
    return success;
}

/**
 * Loads a single JSON simulation component configuration file.
 * Parses the JSON and adds the component and its fields to the engine.
 * 
 * @param engine Pointer to the simulation engine
 * @param json_file_path Path to the JSON configuration file to load
 * @return true if the file was loaded successfully, false otherwise
 */
bool sim_engine_load_component(sim_engine_t* engine, const char* json_file_path) {
    if (!engine || !json_file_path) return false;
    
    // Read JSON file
    FILE* file = fopen(json_file_path, "r");
    if (!file) {
        printf("Error: Cannot open file: %s\n", json_file_path);
        return false;
    }
    
    // Get the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Read the entire JSON file into a string
    char* json_string = malloc(file_size + 1);
    fread(json_string, 1, file_size, file);
    json_string[file_size] = '\0';
    fclose(file);
    
    // Parse the string as JSON
    cJSON* root = cJSON_Parse(json_string);
    free(json_string);
    
    if (!root) {
        printf("Error: Invalid JSON in file: %s\n", json_file_path);
        return false;
    }
    
    // Extract component name e.g. "eva"
    cJSON* component_name_json = cJSON_GetObjectItem(root, "component_name");
    if (!component_name_json || !cJSON_IsString(component_name_json)) {
        printf("Error: Missing component_name in file: %s\n", json_file_path);
        cJSON_Delete(root);
        return false;
    }
    
    const char* component_name = cJSON_GetStringValue(component_name_json);
    
    // Get fields object
    cJSON* fields = cJSON_GetObjectItem(root, "fields");
    if (!fields || !cJSON_IsObject(fields)) {
        printf("Error: Missing or invalid fields object in file: %s\n", json_file_path);
        cJSON_Delete(root);
        return false;
    }
    
    // Count fields
    int field_count = cJSON_GetArraySize(fields);
    if (field_count == 0) {
        printf("Warning: No fields found in component: %s\n", component_name);
        cJSON_Delete(root);
        return true;
    }
    
    // Expand components array
    engine->components = realloc(engine->components, 
        (engine->component_count + 1) * sizeof(sim_component_t));
    
    sim_component_t* component = &engine->components[engine->component_count];
    component->component_name = strdup(component_name);
    component->field_count = field_count;
    component->fields = calloc(field_count, sizeof(sim_field_t));
    component->running = false;  // Start stopped by default
    component->simulation_time = 0.0f;  // Initialize component simulation time
    
    // Parse fields
    int field_idx = 0;
    cJSON* field_json = NULL;
    cJSON_ArrayForEach(field_json, fields) {
        sim_field_t* field = &component->fields[field_idx];
        
        field->field_name = strdup(field_json->string);
        field->component_name = strdup(component_name);
        
        // Parse algorithm
        cJSON* algorithm = cJSON_GetObjectItem(field_json, "algorithm");
        if (!algorithm || !cJSON_IsString(algorithm)) {
            printf("Error: Missing algorithm for field %s\n", field->field_name);
            continue;
        }
        
        field->algorithm = sim_algo_parse_type_string(cJSON_GetStringValue(algorithm));
        field->starting_algorithm = field->algorithm; // Store the starting algorithm for reset purposes

        //Parse min_value, max_value, and start_value (base_value) for use in algorithms and for reset purposes
        cJSON* min_value = cJSON_GetObjectItem(field_json, "min_value");
        cJSON* max_value = cJSON_GetObjectItem(field_json, "max_value");
        cJSON* start_value = cJSON_GetObjectItem(field_json, "start_value");
        cJSON* base_value = cJSON_GetObjectItem(field_json, "base_value");

        field->min_value.f = (min_value && cJSON_IsNumber(min_value)) ? cJSON_GetNumberValue(min_value) : 0.0f;
        field->max_value.f = (max_value && cJSON_IsNumber(max_value)) ? cJSON_GetNumberValue(max_value) : 100.0f;
        field->base_value.f = (base_value && cJSON_IsNumber(base_value)) ? cJSON_GetNumberValue(base_value) : 0.0f; 
        field->start_value.f = (start_value && cJSON_IsNumber(start_value)) ? cJSON_GetNumberValue(start_value) : 0.0f; //if start_value is not provided, use base_value as the starting value
        field->current_value.f = field->start_value.f; //initialize current value to start value

        //parse additional algorithm parameters for use in algorithms and store in field struct for reset purposes
        cJSON* amplitude = cJSON_GetObjectItem(field_json, "amplitude");
        cJSON* frequency = cJSON_GetObjectItem(field_json, "frequency");
        cJSON* phase_acc = cJSON_GetObjectItem(field_json, "phase");
        cJSON* rate = cJSON_GetObjectItem(field_json, "rate");
        

        field->amplitude.f = (amplitude && cJSON_IsNumber(amplitude)) ? cJSON_GetNumberValue(amplitude) : 0.0f;
        field->frequency.f = (frequency && cJSON_IsNumber(frequency)) ? cJSON_GetNumberValue(frequency) : 0.0f;
        field->phase_acc.f = (phase_acc && cJSON_IsNumber(phase_acc)) ? cJSON_GetNumberValue(phase_acc) : 0.0f;
        field->rate.f = (rate && cJSON_IsNumber(rate)) ? cJSON_GetNumberValue(rate) : 0.0f;
        field->starting_rate.f = field->rate.f;

        // Parse type (always float)
        field->type = SIM_TYPE_FLOAT;
        
        // Clone parameters for algorithm use
        field->params = cJSON_Duplicate(field_json, true);
        
        // Parse dependencies
        cJSON* depends_on = cJSON_GetObjectItem(field_json, "depends_on");
        if (depends_on && cJSON_IsArray(depends_on)) {
            field->depends_count = cJSON_GetArraySize(depends_on);
            field->depends_on = malloc(field->depends_count * sizeof(char*));
            
            for (int i = 0; i < field->depends_count; i++) {
                cJSON* dep = cJSON_GetArrayItem(depends_on, i);
                if (cJSON_IsString(dep)) {
                    field->depends_on[i] = strdup(cJSON_GetStringValue(dep));
                }
            }
        } else {
            field->depends_count = 0;
            field->depends_on = NULL;
        }

        field->active = true; //active by default, can be deactivated by DCU commands for certain fields
        field->initialized = false;
        field_idx++;
    }
    
    engine->component_count++;
    engine->total_field_count += field_count;
    
    cJSON_Delete(root);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////
//                            Dependency Sorting
///////////////////////////////////////////////////////////////////////////////////

/**
 * Checks if a field has any unresolved dependencies that do not exist in the config files.
 * 
 * @param field The field to check for dependencies
 * @param resolved_fields Array of fields that have already been resolved
 * @param resolved_count Number of fields in the resolved_fields array
 * @return true if the field has unresolved dependencies, false otherwise
 */
static bool has_unresolved_dependencies(sim_field_t* field, sim_field_t** resolved_fields, int resolved_count) {
    for (int i = 0; i < field->depends_count; i++) {
        bool found = false;
        for (int j = 0; j < resolved_count; j++) {
            if (strcmp(field->depends_on[i], resolved_fields[j]->field_name) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return true;
    }
    return false;
}

/**
 * Sorts all fields in the engine by their dependencies using topological sorting.
 * Fields with no dependencies are placed first, followed by fields that depend on them.
 * This ensures fields are updated in the correct order during simulation.
 * 
 * @param engine Pointer to the simulation engine
 * @return true if sorting was successful, false if circular dependencies were detected
 */
static bool sort_fields_by_dependencies(sim_engine_t* engine) {
    engine->update_order = malloc(engine->total_field_count * sizeof(sim_field_t*));
    sim_field_t** all_fields = malloc(engine->total_field_count * sizeof(sim_field_t*));
    
    // Collect all fields
    int field_idx = 0;
    for (int i = 0; i < engine->component_count; i++) {
        for (int j = 0; j < engine->components[i].field_count; j++) {
            all_fields[field_idx++] = &engine->components[i].fields[j];
        }
    }
    
    // Topological sort
    int resolved_count = 0;
    int iterations = 0;
    const int max_iterations = engine->total_field_count * 2;
    
    while (resolved_count < engine->total_field_count && iterations < max_iterations) {
        bool progress = false;
        
        for (int i = 0; i < engine->total_field_count; i++) {
            sim_field_t* field = all_fields[i];
            
            // Skip already resolved fields
            bool already_resolved = false;
            for (int j = 0; j < resolved_count; j++) {
                if (engine->update_order[j] == field) {
                    already_resolved = true;
                    break;
                }
            }
            if (already_resolved) continue;
            
            // Check if all dependencies are resolved
            if (!has_unresolved_dependencies(field, engine->update_order, resolved_count)) {
                engine->update_order[resolved_count++] = field;
                progress = true;
            }
        }
        
        if (!progress) {
            printf("Error: Circular dependency detected in simulation fields\n");
            free(all_fields);
            return false;
        }
        
        iterations++;
    }
    
    free(all_fields);
    return resolved_count == engine->total_field_count;
}

///////////////////////////////////////////////////////////////////////////////////
//                           Simulation Control
///////////////////////////////////////////////////////////////////////////////////

/**
 * Initializes the simulation engine after all components have been loaded.
 * Sorts fields by dependencies, sets initial values, and prepares for simulation.
 * Must be called after loading all configuration files and before updating.
 * 
 * @param engine Pointer to the simulation engine
 * @return true if initialization was successful, false otherwise
 */
bool sim_engine_initialize(sim_engine_t* engine) {
    if (!engine || engine->initialized) return false;
    
    // Sort fields by dependencies
    if (!sort_fields_by_dependencies(engine)) {
        return false;
    }

    //initialize the DCU field settings
        engine->dcu_field_settings = malloc(sizeof(sim_DCU_field_settings_t));
        engine->dcu_field_settings->battery_lu = false;
        engine->dcu_field_settings->battery_ps = false;
        engine->dcu_field_settings->fan = false;
        engine->dcu_field_settings->o2 = false;
        engine->dcu_field_settings->pump = false;
        engine->dcu_field_settings->co2 = false;
        printf("DCU field settings initialized\n");

    //initilize the UIA field settings
        engine->uia_field_settings = malloc(sizeof(sim_UIA_field_settings_t));
        engine->uia_field_settings->eva1_power = false;
        engine->uia_field_settings->eva1_oxy = false;
        engine->uia_field_settings->eva1_water_supply = false;
        engine->uia_field_settings->eva1_water_waste = false;
        engine->uia_field_settings->eva2_power = false;
        engine->uia_field_settings->eva2_oxy = false;
        engine->uia_field_settings->eva2_water_supply = false;
        engine->uia_field_settings->eva2_water_waste = false;
        engine->uia_field_settings->oxy_vent = false;
        engine->uia_field_settings->depress = false;
        printf("UIA field settings initialized\n");

        engine->error_time = time_to_throw_error();
        engine->num_task_board_errors = INITIAL_NUM_TASK_BOARD_ERRORS; //initialize number of task board errors to 0 at the start of each simulation run
        engine->time_to_complete_task_board = -10; //unreachable value
        engine->error_type = NUM_ERRORS; // set to NUM_ERRORS to signify no error, will be set to 0,1,2..NUM_ERRORS-1 to signify different errors when it's time to throw an error
    
    
        // Initialize all fields
    for (int i = 0; i < engine->total_field_count; i++) {
        sim_field_t* field = engine->update_order[i];


        // Set initial values based on algorithm
        switch (field->algorithm) {
            case SIM_ALGO_SINE_WAVE: {
                cJSON* base_value = cJSON_GetObjectItem(field->params, "base_value");
                if (base_value && cJSON_IsNumber(base_value)) {
                    field->current_value.f = (float)cJSON_GetNumberValue(base_value);
                }
                break;
            }
            case SIM_ALGO_LINEAR_DECAY: {
                cJSON* start_value = cJSON_GetObjectItem(field->params, "start_value");
                if (start_value && cJSON_IsNumber(start_value)) {
                    field->current_value.f = (float)cJSON_GetNumberValue(start_value);
                }
                break;
            }
            case SIM_ALGO_LINEAR_GROWTH: {
                cJSON* start_value = cJSON_GetObjectItem(field->params, "start_value");
                if (start_value && cJSON_IsNumber(start_value)) {
                    field->current_value.f = (float)cJSON_GetNumberValue(start_value);
                } else {
                    field->current_value.f = 0.0f;
                }
                break;
            }
            case SIM_ALGO_DEPENDENT_VALUE: {
                // Will be calculated during first update
                field->current_value.f = 0.0f;
                break;
            }
            case SIM_ALGO_EXTERNAL_VALUE: {
                // Will be calculated during first update
                field->current_value.f = 0.0f;
                break;
            }
            case SIM_ALGO_CONSTANT_VALUE: {
                cJSON* value = cJSON_GetObjectItem(field->params, "start_value");
                if (value && cJSON_IsNumber(value)) {
                    field->current_value.f = (float)cJSON_GetNumberValue(value);
                } else {                    field->current_value.f = 0.0f;
                }                break;
            } default: {
                field->current_value.f = 0.0f;
                return false;
                break;
            }
        }

        field->initialized = true;
    }
    
    engine->initialized = true;
    return true;
}

/**
 * Updates the simulation by one time step.
 * Advances simulation time and updates all fields in dependency order.
 * Throws error if battery voltage drops below 20% and
 * Throws pressure or fan error *randomly* once per 7 minutes of simulation time to simulate real-world issues.
 * Call this regularly to advance the simulation.
 * 
 * @param engine Pointer to the simulation engine
 * @param delta_time Time elapsed since last update (in seconds)
 */
void sim_engine_update(sim_engine_t* engine, float delta_time) {
    
    if (!engine || !engine->initialized) return;
    // First, advance simulation time for all running components
    for (int i = 0; i < engine->component_count; i++) {
        if (engine->components[i].running) {
            engine->components[i].simulation_time += delta_time;
        }
    }
 
    for(int i = 0; i < engine->total_field_count; i++) {
        sim_field_t* field = engine->update_order[i];

        // Find the component this field belongs to
        sim_component_t* component = NULL;
        for (int j = 0; j < engine->component_count; j++) {
            if (strcmp(engine->components[j].component_name, field->component_name) == 0) {
                component = &engine->components[j];
                break;
            }
        }
            
    }
    
    //determine if we need to throw additional errors
    bool eva_control_started = sim_engine_is_component_running(engine, "eva1");
    sim_component_t* eva1 = sim_engine_get_component(engine, "eva1");

    if(eva_control_started) {
        if(eva1 != NULL) {
            if(engine->num_task_board_errors == 0 && eva1->simulation_time == (engine->time_to_complete_task_board + engine->error_time)) {
                throw_random_error(engine);
                printf("Error thrown at simulation time: %.2f seconds\n", eva1->simulation_time);
            } else if(eva1->simulation_time == (engine->time_to_complete_task_board + engine->error_time)) {
                engine->time_to_complete_task_board = engine->time_to_complete_task_board+1;
            }
        } else {
            printf("Simulation tried to access non-existent component 'eva1'\n");
            exit(0);
        }
    }

    

    // Update all fields in dependency order (only for running components)
    for (int i = 0; i < engine->total_field_count; i++) {
        sim_field_t* field = engine->update_order[i];

        // Find the component this field belongs to
        sim_component_t* component = NULL;
        for (int j = 0; j < engine->component_count; j++) {
            if (strcmp(engine->components[j].component_name, field->component_name) == 0) {
                component = &engine->components[j];
                break;
            }
        }

        // Only update if component is running and DCU in correct state (if field depends on DCU commands)
        if (!component || !component->running) continue;

        switch (field->algorithm) {
            case SIM_ALGO_SINE_WAVE:
                field->current_value = sim_algo_sine_wave(field);
                break;
            case SIM_ALGO_LINEAR_DECAY:
                field->current_value = sim_algo_linear_decay(field);
                break;
            case SIM_ALGO_LINEAR_GROWTH:
                field->current_value = sim_algo_linear_growth(field);
                break;
            case SIM_ALGO_DEPENDENT_VALUE:
                field->current_value = sim_algo_dependent_value(field, engine);
                break;
            case SIM_ALGO_EXTERNAL_VALUE:
                field->current_value = sim_algo_external_value(field, engine);
                break;
            case SIM_ALGO_CONSTANT_VALUE:
                field->current_value = sim_algo_constant_value(field);
                break;
            default:
                break;
        }
    }
}

/**
 * Starts simulation updates for a specific component.
 *
 * @param engine Pointer to the simulation engine
 * @param component_name Name of the component to start (e.g., "eva1", "eva2")
 */
void sim_engine_start_component(sim_engine_t* engine, const char* component_name) {
    if (!engine || !engine->initialized || !component_name) return;

    for (int i = 0; i < engine->component_count; i++) {
        if (strcmp(engine->components[i].component_name, component_name) == 0) {
            engine->components[i].running = true;
            printf("Started component '%s' simulation\n", component_name);
            return;
        }
    }

    printf("Warning: Component '%s' not found\n", component_name);
}

/**
 * Stops simulation updates for a specific component.
 *
 * @param engine Pointer to the simulation engine
 * @param component_name Name of the component to stop (e.g., "eva1", "eva2")
 */
void sim_engine_stop_component(sim_engine_t* engine, const char* component_name) {
    if (!engine || !component_name) return;

    for (int i = 0; i < engine->component_count; i++) {
        if (strcmp(engine->components[i].component_name, component_name) == 0) {
            engine->components[i].running = false;
            printf("Stopped component '%s' simulation\n", component_name);
            return;
        }
    }

    printf("Warning: Component '%s' not found\n", component_name);
}

/**
 * Resets all fields of a specific component to their initial state and stops the component.
 * For external value fields with reset_value, calls the provided update_json function to update the data file.
 *
 * @param engine Pointer to the simulation engine
 * @param component_name Name of the component to reset (e.g., "eva1", "eva2")
 * @param update_json Function pointer to update JSON files (e.g., update_json_file from data.c)
 */
void sim_engine_reset_component(sim_engine_t* engine, const char* component_name,
                               void (*update_json)(const char*, const char*, const char*, char*)) {
    if (!engine || !engine->initialized || !component_name) return;

    engine->error_type = NUM_ERRORS; //reset error thrown

    // Find and stop the component
    sim_component_t* target_component = NULL;
    for (int i = 0; i < engine->component_count; i++) {
        if (strcmp(engine->components[i].component_name, component_name) == 0) {
            target_component = &engine->components[i];
            target_component->running = false;
            target_component->simulation_time = 0.0f;  // Reset component simulation time
            break;
        }
    }

    if (!target_component) {
        printf("Warning: Component '%s' not found\n", component_name);
        return;
    }


    //Reset all fields of the component to their initial state
    for (int j = 0; j < target_component->field_count; j++) {
        sim_field_t* field = &target_component->fields[j];
        field->algorithm = field->starting_algorithm; // Reset to starting algorithm
        field->rate = field->starting_rate;
        field->current_value.f = field->start_value.f; // Reset to starting value
        field->phase_acc.f = 0;
    }

    //send reset command to LTV
    if (engine->ltv_reset) {
        engine->ltv_reset(engine->ltv_ctx);
    }

    //also reset errors on our end just in case LTV isn't connected
    if (engine->reset_errors) {
        engine->reset_errors(engine->reset_ctx);
    }

    printf("Reset component '%s' simulation\n", component_name);
}

////////////////////////////////Error 

///////////////////////////////////////////////////////////////////////////////////
//                              Field Access
///////////////////////////////////////////////////////////////////////////////////

/**
 * Finds a field by name across all components.
 * Searches through all loaded components to find a field with the given name.
 * 
 * @param engine Pointer to the simulation engine
 * @param field_name Name of the field to find
 * @return Pointer to the field if found, NULL otherwise
 */
sim_field_t* sim_engine_find_field(sim_engine_t* engine, const char* field_name) {
    if (!engine || !field_name) return NULL;
    
    for (int i = 0; i < engine->component_count; i++) {
        for (int j = 0; j < engine->components[i].field_count; j++) {
            if (strcmp(engine->components[i].fields[j].field_name, field_name) == 0) {
                return &engine->components[i].fields[j];
            }
        }
    }
    
    return NULL;
}

/**
 * Finds a field by name across a specific component.
 * Searches through components to find a field with the given name.
 * 
 * @param engine Pointer to the component to search within
 * @param field_name Name of the field to find
 * @return Pointer to the field if found, NULL otherwise
 */
sim_field_t* sim_engine_find_field_within_component(sim_component_t* component, const char* field_name) {
    if (!component || !field_name) return NULL;
    
    for (int j = 0; j < component->field_count; j++) {
        if (strcmp(component->fields[j].field_name, field_name) == 0) {
                return &component->fields[j];
            }
    }
    
    return NULL;
}

/**
 * Gets the current value of a field by name.
 * 
 * @param engine Pointer to the simulation engine
 * @param field_name Name of the field to get the value for
 * @return The current value of the field, or a zero-initialized value if not found
 */
sim_value_t sim_engine_get_field_value(sim_engine_t* engine, const char* field_name) {
    sim_value_t empty = {0};
    
    sim_field_t* field = sim_engine_find_field(engine, field_name);
    if (!field) return empty;
    
    return field->current_value;
}

/**
* Returns a specific component from the simulation engine by name.
* @param engine Pointer to the simulation engine
* @param component_name Name of the component to retrieve
* @return Pointer to the component if found, NULL otherwise
*/
sim_component_t* sim_engine_get_component(sim_engine_t* engine, const char* component_name) {
    if (!engine || !component_name) return NULL;

    for (int i = 0; i < engine->component_count; i++) {
        if (strcmp(engine->components[i].component_name, component_name) == 0) {
            return &engine->components[i];
        }
    }

    return NULL;
}

/**
 * Checks if a specific component is currently running.
 *
 * @param engine Pointer to the simulation engine
 * @param component_name Name of the component to check
 * @return true if the component is running, false otherwise
 */
bool sim_engine_is_component_running(sim_engine_t* engine, const char* component_name) {
    if (!engine || !component_name) return false;

    for (int i = 0; i < engine->component_count; i++) {
        if (strcmp(engine->components[i].component_name, component_name) == 0) {
            return engine->components[i].running;
        }
    }
    return false;
}