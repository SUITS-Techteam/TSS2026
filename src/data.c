#include "data.h"
#include "lib/simulation/throw_errors.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/////////flags//////////
bool include_UIA_flag = false;
bool fan_error_flag = false;
bool o2_error_flag = false;

///////////////////////////////////////////////////////////////////////////////////
//                        Backend Lifecycle Management
///////////////////////////////////////////////////////////////////////////////////

/**
 * Initializes the backend data structure and simulation engine
 *
 * @return Pointer to the initialized backend data structure
 */
struct backend_data_t *init_backend() {
    // Allocate memory for backend
    struct backend_data_t *backend = malloc(sizeof(struct backend_data_t));
    memset(backend, 0, sizeof(struct backend_data_t));

    //initialize the JSON files
    if (!initialize_json_switch_states()) {
        printf("Warning: Failed to initialize JSON files\n");
    }

    // Set initial timing information
    backend->start_time = time(NULL);
    backend->server_up_time = 0;
    backend->time_since_last_ping = 0;

    // Initialize simulation engine
    backend->sim_engine = sim_engine_create();
    if (backend->sim_engine) {

        if (!sim_engine_load_predefined_configs(backend->sim_engine)) {
            printf("Warning: Failed to load simulation configurations\n");
        }

        if (!sim_engine_initialize(backend->sim_engine)) {
            printf("Warning: Failed to initialize simulation engine\n");
        }

        //if UIA is included, initialize the UIA override dependent values at the start of the simulation
        if(include_UIA_flag) {
            printf("UIA is included in this simulation run. Initializing dependent values...\n");
            initialize_UIA_override_dependent_values(backend->sim_engine);
        } else {
            printf("UIA override is not included in this simulation run.\n");
        }

    } else {
        printf("Warning: Failed to create simulation engine\n");
    }

    

    printf("Backend and simulation engine initialized successfully\n");

    return backend;
}

/** 
* Initializes all the JSON switch states
* @return true if initialization was successful, false otherwise
*/
bool initialize_json_switch_states() {
    bool eva_init = initialize_EVA_json_switch_states();
    bool ltv_errors_init = initialize_LTV_ERRORS_json_switch_states();

    return eva_init && ltv_errors_init;
}

/** 
* Initializes JSON switch states in EVA.json file
* @return true if initialization was successful, false otherwise
*/
bool initialize_EVA_json_switch_states() {
    cJSON* eva_json = get_json_file("EVA");
    if (!eva_json) {
        printf("Error: Failed to load EVA config file in initialize_json_switch_states\n");
        return false;
    }

    ///////////////set all UIA values to false///////////////
    cJSON* uia = cJSON_GetObjectItem(eva_json, "uia");
    if (!uia) {
        printf("Error: Failed to get uia from EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //change eva1_power to false
    cJSON_ReplaceItemInObject(uia, "eva1_power", cJSON_CreateBool(0));
    if (!cJSON_GetObjectItem(uia, "eva1_power")) {
        printf("Error: Failed to set eva1_power in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //change eva1_oxy to false
    cJSON_ReplaceItemInObject(uia, "eva1_oxy", cJSON_CreateBool(0));
    if (!cJSON_GetObjectItem(uia, "eva1_oxy")) {
        printf("Error: Failed to set eva1_oxy in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //change eva1_water_supply to false
    cJSON_ReplaceItemInObject(uia, "eva1_water_supply", cJSON_CreateBool(0));
    if (!cJSON_GetObjectItem(uia, "eva1_water_supply")) {
        printf("Error: Failed to set eva1_water_supply in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //change eva1_water_waste to false
    cJSON_ReplaceItemInObject(uia, "eva1_water_waste", cJSON_CreateBool(0));
    if (!cJSON_GetObjectItem(uia, "eva1_water_waste")) {
        printf("Error: Failed to set eva1_water_waste in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //change eva2_power to false
    cJSON_ReplaceItemInObject(uia, "eva2_power", cJSON_CreateBool(0));
    if (!cJSON_GetObjectItem(uia, "eva2_power")) {
        printf("Error: Failed to set eva2_power in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //change eva2_oxy to false
    cJSON_ReplaceItemInObject(uia, "eva2_oxy", cJSON_CreateBool(0));
    if (!cJSON_GetObjectItem(uia, "eva2_oxy")) {
        printf("Error: Failed to set eva2_oxy in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //change eva2_water_supply to false
    cJSON_ReplaceItemInObject(uia, "eva2_water_supply", cJSON_CreateBool(0));
    if (!cJSON_GetObjectItem(uia, "eva2_water_supply")) {
        printf("Error: Failed to set eva2_water_supply in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //change eva2_water_waste to false
    cJSON_ReplaceItemInObject(uia, "eva2_water_waste", cJSON_CreateBool(0));
    if (!cJSON_GetObjectItem(uia, "eva2_water_waste")) {
        printf("Error: Failed to set eva2_water_waste in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //change o2 vent to false
    cJSON_ReplaceItemInObject(uia, "oxy_vent", cJSON_CreateBool(0));
    if (!cJSON_GetObjectItem(uia, "oxy_vent")) {
        printf("Error: Failed to set oxy_vent in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //change depress to false
    cJSON_ReplaceItemInObject(uia, "depress", cJSON_CreateBool(0));
    if (!cJSON_GetObjectItem(uia, "depress")) {
        printf("Error: Failed to set depress in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    ///////////////set all DCU values to false///////////////
    cJSON* dcu = cJSON_GetObjectItem(eva_json, "dcu");
    if (!dcu) {
        printf("Error: Failed to get dcu from EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //get eva1 object from dcu
    cJSON* eva1_dcu = cJSON_GetObjectItem(dcu, "eva1");
    if (!eva1_dcu) {
        printf("Error: Failed to get eva1 from dcu in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //change oxy to false
    cJSON_ReplaceItemInObject(eva1_dcu, "oxy", cJSON_CreateBool(0));
    if (!cJSON_GetObjectItem(eva1_dcu, "oxy")) {
        printf("Error: Failed to set eva1.oxy in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //change fan to false
    cJSON_ReplaceItemInObject(eva1_dcu, "fan", cJSON_CreateBool(0));
    if (!cJSON_GetObjectItem(eva1_dcu, "fan")) {
        printf("Error: Failed to set eva1.fan in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //change pump to false
    cJSON_ReplaceItemInObject(eva1_dcu, "pump", cJSON_CreateBool(0));
    if (!cJSON_GetObjectItem(eva1_dcu, "pump")) {
        printf("Error: Failed to set eva1.pump in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //change co2 to false
    cJSON_ReplaceItemInObject(eva1_dcu, "co2", cJSON_CreateBool(0));
    if (!cJSON_GetObjectItem(eva1_dcu, "co2")) {
        printf("Error: Failed to set eva1.co2 in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //get batt object from eva1_dcu
    cJSON* batt_dcu = cJSON_GetObjectItem(eva1_dcu, "batt");
    if (!batt_dcu) {
        printf("Error: Failed to get batt from eva1 in dcu in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //change lu to false
    cJSON_ReplaceItemInObject(batt_dcu, "lu", cJSON_CreateBool(0));
    if (!cJSON_GetObjectItem(batt_dcu, "lu")) {
        printf("Error: Failed to set eva1.batt.lu in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //change ps to false
    cJSON_ReplaceItemInObject(batt_dcu, "ps", cJSON_CreateBool(0));
    if (!cJSON_GetObjectItem(batt_dcu, "ps")) {
        printf("Error: Failed to set eva1.batt.ps in EVA config file in initialize_json_switch_states\n");
        cJSON_Delete(eva_json);
        return false;
    }

    //write to JSON file
    char *json_string = cJSON_Print(eva_json);

    FILE *file = fopen("data/EVA.json", "w");
    if (!file) {
        printf("Error opening EVA.json for writing\n");
        free(json_string);
        cJSON_Delete(eva_json);
        return false;
    }

    fprintf(file, "%s", json_string);
    fclose(file);

    free(json_string);
    cJSON_Delete(eva_json);

    return true;
}

/**
* Initializes JSON error states in LTV_ERRORS.json file
* Sets all needs_resolved fields to true
* @return true if initialization was successful, false otherwise
*/
bool initialize_LTV_ERRORS_json_switch_states() {
    cJSON* errors_json = get_json_file("LTV_ERRORS");
    if (!errors_json) {
        printf("Error: Failed to load LTV_ERRORS config file in initialize_LTV_ERRORS_json_switch_states\n");
        return false;
    }

    cJSON* error_procedures = cJSON_GetObjectItem(errors_json, "error_procedures");
    if (!error_procedures || !cJSON_IsArray(error_procedures)) {
        printf("Error: Failed to get error_procedures array from LTV_ERRORS config file\n");
        cJSON_Delete(errors_json);
        return false;
    }

    cJSON* error = NULL;
    int index = 0;

    cJSON_ArrayForEach(error, error_procedures) {

        // Set index 4 and 5 to false, everything else true
        int value = (index == 4 || index == 5) ? 0 : 1;

        cJSON_ReplaceItemInObject(error, "needs_resolved", cJSON_CreateBool(value));

        if (!cJSON_GetObjectItem(error, "needs_resolved")) {
            printf("Error: Failed to set needs_resolved in LTV_ERRORS config file\n");
            cJSON_Delete(errors_json);
            return false;
        }

        index++;
    }

    char *json_string = cJSON_Print(errors_json);

    FILE *file = fopen("data/LTV_ERRORS.json", "w");
    if (!file) {
        printf("Error opening LTV_ERRORS.json for writing\n");
        free(json_string);
        cJSON_Delete(errors_json);
        return false;
    }

    fprintf(file, "%s", json_string);
    fclose(file);

    free(json_string);
    cJSON_Delete(errors_json);

    return true;
}

/**
* initialize all values where initialization value depends on whether including UIA override
* @param sim_engine Pointer to the simulation engine to update
*/
void initialize_UIA_override_dependent_values(sim_engine_t* sim_engine) {
    if (!sim_engine) {
        return;
    }

    // Get pointer to EVA1 component for easy access to its fields
    sim_component_t* eva1 = sim_engine_get_component(sim_engine, "eva1");

    //set eva.fields.coolant_storage to 0 at the start of the simulation and set algorithm to constant value to keep at 0 until ready to grow
    sim_field_t* coolant_storage_field = sim_engine_find_field_within_component(eva1, "coolant_storage");
    if (coolant_storage_field) {
        coolant_storage_field->current_value.f = 0.0f;
        coolant_storage_field->algorithm = SIM_ALGO_CONSTANT_VALUE;
    } else {
        printf("Simulation tried to access non-existent field 'eva1.coolant_storage' for UIA override initialization\n");
    }

    //set eva1.suit_pressure_oxy to 0 to keep at 0 until ready to grow
    sim_field_t* suit_pressure_oxy_field = sim_engine_find_field_within_component(eva1, "suit_pressure_oxy");
    if (suit_pressure_oxy_field) {
        suit_pressure_oxy_field->current_value.f = 0.0f;
        suit_pressure_oxy_field->algorithm = SIM_ALGO_CONSTANT_VALUE;
    } else {
        printf("Simulation tried to access non-existent field 'eva1.suit_pressure_oxy' for UIA override initialization\n");
    }

    //make sure that suit_pressure_co2 stays at 0 during the simulation to keep suit_pressure_total just = to suit_pressure_oxy
    sim_field_t* suit_pressure_co2_field = sim_engine_find_field_within_component(eva1, "suit_pressure_co2");
    if (suit_pressure_co2_field) {
        suit_pressure_co2_field->current_value.f = 0.0f;
        suit_pressure_co2_field->algorithm = SIM_ALGO_CONSTANT_VALUE;
    } else {
        printf("Simulation tried to access non-existent field 'eva1.suit_pressure_co2' for UIA override initialization\n");
    }

}

/**
* Returns whether UIA is connected
* @param sim_engine Pointer to the simulation engine to check
* @return true if UIA is connected, false otherwise
*/
bool is_UIA_connected(sim_engine_t* sim_engine) {
    if (!sim_engine) {
        return false;
    }

    bool uia_power_supply_connected_eva1 = sim_engine->uia_field_settings->eva1_power;
    bool dcu_using_umbilical_power_eva1 = sim_engine->dcu_field_settings->battery_lu;

    return (uia_power_supply_connected_eva1 && dcu_using_umbilical_power_eva1);
}

/**
* Updates fan values if on or not based on DCU state
* If the DCU command for the fan is set to true, the fan value will be set to 30000.0 when turned on.
* If the DCU command for the fan is set to false, the fan value will be set to 0.0 when turned off.
* @param sim_engine Pointer to the simulation engine to update
*/
void update_fan_values(sim_engine_t* sim_engine) {
    if (!sim_engine) {
        return;
    }

    sim_component_t* eva1 = sim_engine_get_component(sim_engine, "eva1");
    if (eva1 == NULL) {
        printf("Simulation tried to access non-existent component 'eva1' for fan value update\n");
        return;
    }

    sim_field_t* fan_pri_rpm_field = sim_engine_find_field_within_component(eva1, "fan_pri_rpm");
    sim_field_t* fan_sec_rpm_field = sim_engine_find_field_within_component(eva1, "fan_sec_rpm");


    //if in error state, need to store the last value
    if(sim_engine->error_type == FAN_RPM_LOW) {
        if (fan_pri_rpm_field && fan_sec_rpm_field) {
            if (sim_engine->dcu_field_settings->fan) {
                fan_pri_rpm_field->current_value.f = 0.0f;
                fan_sec_rpm_field->current_value.f = 0.0f;
            } else {
                fan_pri_rpm_field->current_value.f = 0.0f;
                fan_sec_rpm_field->current_value.f = 30000.0f;
            }
        } else {
            printf("Simulation tried to access non-existent field 'eva1.fan_pri_rpm' or 'eva1.fan_sec_rpm' for fan value update\n");
        }
    }

    //otherwise just switch between 0 and 30,000 RPM
    if(sim_engine->error_type != FAN_RPM_LOW) {
        if (fan_pri_rpm_field && fan_sec_rpm_field) {
            if (sim_engine->dcu_field_settings->fan) {
                fan_pri_rpm_field->current_value.f = 30000.0f;
                fan_sec_rpm_field->current_value.f = 0.0f;
            } else {
                fan_pri_rpm_field->current_value.f = 0.0f;
                fan_sec_rpm_field->current_value.f = 30000.0f;
            }
        } else {
            printf("Simulation tried to access non-existent field 'eva1.fan_pri_rpm' or 'eva1.fan_sec_rpm' for fan value update\n");
        }
    }
}

/**
* Updates active states of all applicable fields based on whether UIA is connected and the current DCU command settings
* @param sim_engine Pointer to the simulation engine to update
 */

 void update_sim_active_states(sim_engine_t* sim_engine) {
    if (!sim_engine) {
        return;
    }

    //find all fields that are running
    for(int i = 0; i < sim_engine->total_field_count; i++) {
        sim_field_t* field = sim_engine->update_order[i];

        // Find the component this field belongs to
        sim_component_t* component = NULL;
        for (int j = 0; j < sim_engine->component_count; j++) {
            if (strcmp(sim_engine->components[j].component_name, field->component_name) == 0) {
                component = &sim_engine->components[j];
                break;
            }
        }

        
        field->active = true; //set all fields to active by default, then set to false if they depend on a DCU command that is not active

        if(component && strncmp(component->component_name, "eva", 3) == 0) { //if the field is within an EVA component, check DCU command dependencies to determine active state
        //set active to true by default, will be set to false for ***EVA*** fields that depend on DCU commands until the correct command is received
        if(strncmp(field->field_name, "primary_battery_level", 21) == 0 && !(sim_engine->dcu_field_settings->battery_lu == false && sim_engine->dcu_field_settings->battery_ps == true)) {
            field->active = false;
        } else if(strncmp(field->field_name, "secondary_battery_level", 23) == 0 && !(sim_engine->dcu_field_settings->battery_lu == false && sim_engine->dcu_field_settings->battery_ps == false)) {
            field->active = false;
        } else if(strncmp(field->field_name, "oxy_pri_storage", 15) == 0 && (sim_engine->dcu_field_settings->o2 == false)) {
            field->active = false;
        } else if(strncmp(field->field_name, "oxy_sec_storage", 15) == 0 && (sim_engine->dcu_field_settings->o2 == true)) {
            field->active = false;
        } else if(strncmp(field->field_name, "fan_pri_rpm", 11) == 0 && (sim_engine->dcu_field_settings->fan == false)) {
            field->active = false;
        } else if(strncmp(field->field_name, "fan_sec_rpm", 11) == 0 && (sim_engine->dcu_field_settings->fan == true)) {
            field->active = false;
        } else if(strncmp(field->field_name, "coolant_liquid_pressure", 23) == 0 && (sim_engine->dcu_field_settings->pump == false)) {
            field->active = false;
        } else {
            field->active = true;
        }

        //error overrides active status, so if an error is active for a field, it should be active regardless of DCU settings
        if((sim_engine->error_type == SUIT_PRESSURE_OXY_LOW) && (strcmp(field->field_name, "oxy_pri_storage") == 0)) {
            field->active = true;
        } else if((sim_engine->error_type == SUIT_PRESSURE_OXY_HIGH) && (strcmp(field->field_name, "oxy_pri_storage") == 0)) {
            field->active = true;
        } else if((sim_engine->error_type == FAN_RPM_LOW) && (strcmp(field->field_name, "fan_pri_rpm") == 0)) {
            field->active = true;
        }
    }


    }


    //if UIA is connected, override active states based on UIA states
    if(is_UIA_connected(sim_engine)) {
        //check if o2 vent is open and make suit_pressure_oxy fields active if so
        bool uia_oxy_vent_open = sim_engine->uia_field_settings->oxy_vent;

        if (uia_oxy_vent_open) {
            sim_component_t* component = sim_engine_get_component(sim_engine, "eva1");
            sim_field_t* suit_pressure_oxy_field = sim_engine_find_field_within_component(component, "oxy_pri_storage");
            if (suit_pressure_oxy_field) {
                suit_pressure_oxy_field->active = true; //make the field active so it starts updating based on the new algorithm
            } else {
                printf("Simulation tried to access non-existent field 'eva1.oxy_pri_storage' for UIA override\n");
            }

            sim_field_t* suit_pressure_oxy_field2 = sim_engine_find_field_within_component(component, "oxy_sec_storage");
            if (suit_pressure_oxy_field2) {
                suit_pressure_oxy_field2->active = true; //make the field active so it starts updating based on the new algorithm
            } else {
                printf("Simulation tried to access non-existent field 'eva1.oxy_sec_storage' for UIA override\n");
            }
        }
    }
}

/**
* UIA override function overrides EVA simulation values.
* If the UIA is connected, the update will be based on 
* ingress/egress procedures and UIA states.
* @param sim_engine Pointer to the simulation engine to update
* @return true if the UIA is connected and overrides were applied, false otherwise
*/
bool update_sim_UIA_connected(sim_engine_t* sim_engine) {

    if (!sim_engine) {
        return false;
    }

    // Get pointer to EVA1 component for easy access to its fields
    sim_component_t* eva1 = sim_engine_get_component(sim_engine, "eva1");
    if (eva1 == NULL) {
        printf("Simulation tried to access non-existent component 'eva1' for UIA override\n");
        return false;
    }

    //check if UIA is connected by checking the DCU command for UIA connection
    if (is_UIA_connected(sim_engine)) {
        //check if o2 vent is open and drain o2 if so
        bool uia_oxy_vent_open = sim_engine->uia_field_settings->oxy_vent;

        //check if o2 vent is closed and oxygen EMU1 is open
        //if so, fill 
        bool uia_oxy_vent_closed = !sim_engine->uia_field_settings->oxy_vent;
        bool uia_oxy_emu1_open = sim_engine->uia_field_settings->eva1_oxy;
        bool uia_oxy_emu1_closed = !sim_engine->uia_field_settings->eva1_oxy;

        bool dcu_using_primary_oxygen = sim_engine->dcu_field_settings->o2 == true;
        bool dcu_using_secondary_oxygen = sim_engine->dcu_field_settings->o2 == false;

        if (uia_oxy_vent_open) {
            sim_field_t* suit_pressure_oxy_field = sim_engine_find_field_within_component(eva1, "oxy_pri_storage");
            if (suit_pressure_oxy_field) {
                suit_pressure_oxy_field->active = true; //make the field active so it starts updating based on the new algorithm
                suit_pressure_oxy_field->algorithm = SIM_ALGO_LINEAR_DECAY;
                suit_pressure_oxy_field->rate.f = OXY_VENT_RATE; //set the rate of decay to 0.5 units per second when the vent is open
                
            } else {
                printf("Simulation tried to access non-existent field 'eva1.oxy_pri_storage' for UIA override\n");
            }

            sim_field_t* suit_pressure_oxy_field2 = sim_engine_find_field_within_component(eva1, "oxy_sec_storage");
            if (suit_pressure_oxy_field2) {
                suit_pressure_oxy_field2->active = true; //make the field active so it starts updating based on the new algorithm
                suit_pressure_oxy_field2->algorithm = SIM_ALGO_LINEAR_DECAY;
                suit_pressure_oxy_field2->rate.f = OXY_VENT_RATE; //set the rate of decay to 0.5 units per second when the vent is open
            } else {
                printf("Simulation tried to access non-existent field 'eva1.oxy_sec_storage' for UIA override\n");
            }

        } else if(uia_oxy_vent_closed && uia_oxy_emu1_open) {
            //on primary oxygen
            if(dcu_using_primary_oxygen) {
                sim_field_t* oxy_pri_storage = sim_engine_find_field_within_component(eva1, "oxy_pri_storage");
                if (oxy_pri_storage) {
                    oxy_pri_storage->algorithm = SIM_ALGO_LINEAR_GROWTH;
                    oxy_pri_storage->rate.f = OXY_FILL_RATE;
                } else {
                    printf("Simulation tried to access non-existent field 'eva1.oxy_pri_storage' for UIA override\n");
                }

            //put secondary oxygen back on decay
                sim_field_t* oxy_sec_storage = sim_engine_find_field_within_component(eva1, "oxy_sec_storage");
                    if (oxy_sec_storage) {
                        oxy_sec_storage->algorithm = SIM_ALGO_LINEAR_DECAY;
                        oxy_sec_storage->rate.f = OXY_RATE;
                    } else {
                        printf("Simulation tried to access non-existent field 'eva1.oxy_sec_storage' for UIA override\n");
                    }
            } 

            //on secondary oxygen
            if(dcu_using_secondary_oxygen) {
                sim_field_t* oxy_sec_storage = sim_engine_find_field_within_component(eva1, "oxy_sec_storage");
                if (oxy_sec_storage) {
                    oxy_sec_storage->algorithm = SIM_ALGO_LINEAR_GROWTH;
                    oxy_sec_storage->rate.f = OXY_FILL_RATE;
                } else {
                    printf("Simulation tried to access non-existent field 'eva1.oxy_sec_storage' for UIA override\n");
                }

                //put primary oxygen back on decay
                sim_field_t* oxy_pri_storage = sim_engine_find_field_within_component(eva1, "oxy_pri_storage");
                    if (oxy_pri_storage) {
                        oxy_pri_storage->algorithm = SIM_ALGO_LINEAR_DECAY;
                        oxy_pri_storage->rate.f = OXY_RATE;
                    } else {
                        printf("Simulation tried to access non-existent field 'eva1.oxy_pri_storage' for UIA override\n");
                    }
            }
        } else {
            //if vent is closed and oxygen EMU1 is closed, make sure that oxygen storage fields are not growing
            sim_field_t* oxy_pri_storage = sim_engine_find_field_within_component(eva1, "oxy_pri_storage");
            sim_field_t* oxy_sec_storage = sim_engine_find_field_within_component(eva1, "oxy_sec_storage");
            if (dcu_using_primary_oxygen && oxy_pri_storage && oxy_sec_storage) {
                    oxy_pri_storage->algorithm = SIM_ALGO_LINEAR_DECAY;
                    oxy_pri_storage->rate.f = OXY_RATE;

                    oxy_sec_storage->algorithm = SIM_ALGO_LINEAR_DECAY;
                    oxy_sec_storage->rate.f = 0;
            } 

            if (dcu_using_secondary_oxygen && oxy_pri_storage && oxy_sec_storage) {
                    oxy_sec_storage->algorithm = SIM_ALGO_LINEAR_DECAY;
                    oxy_sec_storage->rate.f = OXY_RATE;

                    oxy_pri_storage->algorithm = SIM_ALGO_LINEAR_DECAY;
                    oxy_pri_storage->rate.f = 0;
            } 
        }

        
        float current_oxy_pressure = 0.0f;
        //check that current O2 pressure is above 2900 before filling to prevent filling when not supposed to due to a vent open command being sent while suit pressure is still high from a previous fill
        if(dcu_using_primary_oxygen) {
            sim_field_t* oxy_pressure = sim_engine_find_field_within_component(eva1, "oxy_pri_pressure");
            current_oxy_pressure = 0.0f;
            if (oxy_pressure) {
                current_oxy_pressure = oxy_pressure->current_value.f;
            } else {
                printf("Simulation tried to access non-existent field 'eva1.oxy_pri_pressure' for UIA override\n");
            }
        } else {
            sim_field_t* oxy_pressure = sim_engine_find_field_within_component(eva1, "oxy_sec_pressure");
            current_oxy_pressure = 0.0f;
            if (oxy_pressure) {
                current_oxy_pressure = oxy_pressure->current_value.f;
            } else {
                printf("Simulation tried to access non-existent field 'eva1.oxy_sec_pressure' for UIA override\n");
            }
        }

        //fill the suit pressures and O2 pressure until they are both 4
        //only do so if primary O2 pressure > 2900
        if(uia_oxy_vent_closed && uia_oxy_emu1_closed && dcu_using_primary_oxygen && current_oxy_pressure > 1000.0f) {
            sim_field_t* suit_pressure_oxy_field = sim_engine_find_field_within_component(eva1, "suit_pressure_oxy");
            if (suit_pressure_oxy_field) {
                if(suit_pressure_oxy_field->current_value.f < 4.0f ) {
                suit_pressure_oxy_field->algorithm = SIM_ALGO_LINEAR_GROWTH;
                } else {
                    suit_pressure_oxy_field->current_value.f = 4.0f;
                    suit_pressure_oxy_field->algorithm = SIM_ALGO_CONSTANT_VALUE;
                }
            } else {
                printf("Simulation tried to access non-existent field 'eva1.suit_pressure_oxy' for UIA override\n");
            }
        }

        //make sure that suit_pressure_co2 stays at 0 during the simulation to keep suit_pressure_total just = to suit_pressure_oxy
        sim_field_t* suit_pressure_co2_field = sim_engine_find_field_within_component(eva1, "suit_pressure_co2");
        if (suit_pressure_co2_field) {
            suit_pressure_co2_field->current_value.f = 0.0f;
            suit_pressure_co2_field->algorithm = SIM_ALGO_CONSTANT_VALUE;
        } else {
            printf("Simulation tried to access non-existent field 'eva1.suit_pressure_co2' for UIA override initialization\n");
        }

        //prep coolant tank
        //check whether DCU pump is open
        bool dcu_pump_open = sim_engine->dcu_field_settings->pump == true;
        
        //check whether UIA EVA1 valves are open or closed
        bool uia_supply_water_tank_valve_closed = !sim_engine->uia_field_settings->eva1_water_supply;
        bool uia_waste_water_tank_valve_open  = sim_engine->uia_field_settings->eva1_water_waste == true;
        bool uia_supply_water_tank_valve_open = sim_engine->uia_field_settings->eva1_water_supply == true;
        bool uia_waste_water_tank_valve_closed  = !sim_engine->uia_field_settings->eva1_water_waste;
        
        //drain water tank
        if(dcu_pump_open && uia_supply_water_tank_valve_closed && uia_waste_water_tank_valve_open) {
            sim_field_t* coolant_storage = sim_engine_find_field_within_component(eva1, "coolant_storage");
            if (coolant_storage) {
                coolant_storage->algorithm = SIM_ALGO_LINEAR_DECAY;
                coolant_storage->rate.f = COOLANT_DRAIN_RATE;
            } else {
                printf("Simulation tried to access non-existent field 'eva1.coolant_storage' for UIA override\n");
            }
        }

        //fill coolant tank
        else if(dcu_pump_open && uia_supply_water_tank_valve_open && uia_waste_water_tank_valve_closed) {
            sim_field_t* coolant_storage = sim_engine_find_field_within_component(eva1, "coolant_storage");
            if (coolant_storage) {
                    coolant_storage->algorithm = SIM_ALGO_LINEAR_GROWTH;
                    coolant_storage->rate.f = COOLANT_FILL_RATE;
                } else {
                    printf("Simulation tried to access non-existent field 'eva1.coolant_storage' for UIA override\n");
                }
        } else {
            //make coolant storage decrease normally
            sim_field_t* coolant_storage = sim_engine_find_field_within_component(eva1, "coolant_storage");
            if (coolant_storage) {
                    coolant_storage->algorithm = SIM_ALGO_LINEAR_DECAY;
                    coolant_storage->rate.f = COOLANT_RATE;
            } else {
                printf("Simulation tried to access non-existent field 'eva1.coolant_storage' for UIA override\n");
            }
        }

        return true;
    }

    return false;
}

/**
* calls individual functions to update error states for each system based on the current DCU field settings. 
* This function is called in data.c before sim_engine_update to ensure that error states are updated before each simulation update, 
* allowing the simulation to reflect any changes in error conditions based on DCU commands.
* @param sim_engine Pointer to the simulation engine to update
 */
void update_EVA_error_simulation_error_states(sim_engine_t* sim_engine) {
    if (!sim_engine) {
        return;
    }

    update_O2_error_state(sim_engine);
    update_fan_error_state(sim_engine);
    update_power_error_state(sim_engine);

}

/**
* Update the number of LTV errors still thrown
* based on the LTV_ERRORS number of "needs_resolved" values still true in the JSON file. 
* If there are no more "needs_resolved" values that are true, 
* the number of remaining LTV errors will be set to 0.
* @param engine Pointer to the simulation engine
*/

void update_num_remaining_errors_LTV(sim_engine_t* engine) {
    //check how many LTV errors still need to be resolved by checking the LTV_ERRORS JSON file for any "needs_resolved" values that are still true for LTV errors
    int remaining_errors = 0;

    //count the number of values in the LTV json file under "errors" that are set to true to indicate that those errors are still being thrown, and update the number of task board errors accordingly
    cJSON* ltv_errors_json = get_json_file("LTV_ERRORS");
    if (!ltv_errors_json) {
        printf("Error: Failed to load LTV_ERRORS config file in update_num_remaining_errors_LTV\n");
        return;
    }

    cJSON* error_array = cJSON_GetObjectItem(ltv_errors_json, "error_procedures");
    if (error_array == NULL) {
        printf("Failed to get 'errors' array from LTV_ERRORS JSON file for updating remaining errors\n");
        cJSON_Delete(ltv_errors_json);
        return;
    }

    int error_count = cJSON_GetArraySize(error_array);
    for (int i = 0; i < error_count; i++) {
        cJSON* error_item = cJSON_GetArrayItem(error_array, i);
        if (error_item == NULL) {
            printf("Failed to get error item from LTV_ERRORS JSON file for updating remaining errors\n");
            continue;
        }

        cJSON* needs_resolved = cJSON_GetObjectItem(error_item, "needs_resolved");
        if (needs_resolved == NULL) {
            printf("Failed to get 'needs_resolved' from error item in LTV_ERRORS JSON file for updating remaining errors\n");
            continue;
        }

        if (cJSON_IsTrue(needs_resolved)) {
            remaining_errors++;
        }
    }

    sim_component_t* eva1 = sim_engine_get_component(engine, "eva1");
     if (eva1 == NULL) {
        printf("Simulation tried to access non-existent component 'eva1' for updating remaining errors\n");
        cJSON_Delete(ltv_errors_json);
        return;
    }
    if(remaining_errors == 0 && engine->time_to_complete_task_board == -10 && eva1->running == true) { //if all errors have been resolved, set time to complete task board to current simulation time to track how long it took to resolve all errors, but only if the task board is currently running and time to complete task board has not already been set
        engine->time_to_complete_task_board = eva1->simulation_time; //set error time to current time when all errors have been resolved to track how long it took to resolve all errors
    }

    engine->num_task_board_errors = remaining_errors;
    cJSON_Delete(ltv_errors_json);
}

void update_ltv_error_dependencies() {
    cJSON* root = get_json_file("LTV_ERRORS");
    if (!root) return;

    cJSON* arr = cJSON_GetObjectItem(root, "error_procedures");
    if (!cJSON_IsArray(arr)) {
        cJSON_Delete(root);
        return;
    }

    cJSON* poor_comms = NULL;     // 3452
    cJSON* comms_reboot = NULL;   // 2441
    cJSON* subsystem_bus = NULL;  // 4968

    // Find the relevant errors by code
    cJSON* item = NULL;
    cJSON_ArrayForEach(item, arr) {
        cJSON* code = cJSON_GetObjectItem(item, "code");
        if (!cJSON_IsString(code)) continue;

        if (strcmp(code->valuestring, "3452") == 0) {
            poor_comms = item;
        } else if (strcmp(code->valuestring, "2441") == 0) {
            comms_reboot = item;
        } else if (strcmp(code->valuestring, "4968") == 0) {
            subsystem_bus = item;
        }
    }

    if (!poor_comms || !comms_reboot || !subsystem_bus) {
        printf("Error: Missing required error codes\n");
        cJSON_Delete(root);
        return;
    }

    cJSON* poor_val = cJSON_GetObjectItem(poor_comms, "needs_resolved");
    cJSON* reboot_val = cJSON_GetObjectItem(comms_reboot, "needs_resolved");

    if (!cJSON_IsBool(poor_val) || !cJSON_IsBool(reboot_val)) {
        cJSON_Delete(root);
        return;
    }


    bool poor_resolved = !cJSON_IsTrue(poor_val);
    bool reboot_resolved = !cJSON_IsTrue(reboot_val);

    static bool poor_resolved_previous = 0;
    static bool reboot_resolved_previous = 1;

    if ((poor_resolved && poor_resolved_previous!=poor_resolved) || (reboot_resolved && reboot_resolved_previous!=reboot_resolved)) {
        // Turn ON subsystem power bus error (needs_resolved = true)
        cJSON_ReplaceItemInObject(subsystem_bus, "needs_resolved", cJSON_CreateBool(1));
        printf("Subsystem Power Bus Error triggered due to dependencies\n");
    }

    if ((!poor_resolved && poor_resolved_previous!=poor_resolved) || (!reboot_resolved && reboot_resolved_previous!=reboot_resolved)) {
        // Turn ON subsystem power bus error (needs_resolved = true)
        cJSON_ReplaceItemInObject(subsystem_bus, "needs_resolved", cJSON_CreateBool(0));
        printf("Subsystem Power Bus Error untriggered due to dependencies\n");
    }

    poor_resolved_previous = poor_resolved;
    reboot_resolved_previous = reboot_resolved;

    // Write back to file
    char* out = cJSON_Print(root);
    FILE* fp = fopen("data/LTV_ERRORS.json", "w");
    if (fp) {
        fputs(out, fp);
        fclose(fp);
    }

    free(out);
    cJSON_Delete(root);
}

/**
* updates the O2 error state based on the current DCU field settings and o2 value.
* If the DCU command for O2 is set to false, the O2 error state will be set to false (no error).
* If the O2 error is thrown and the DCU command for O2 is set to true, the O2 error state will be set to true (error present).
* The function also updates the algorithm and rate for the suit_pressure_oxy field to simulate the consequences of the O2 error based on the current DCU command.
* @param sim_engine Pointer to the simulation engine to update
*/
void update_O2_error_state(sim_engine_t* sim_engine) {
    if (!sim_engine) {
        return;
    }

    //check if the O2 error is currently thrown by checking if the algorithm for the O2 storage field is set to linear decay
    sim_component_t* eva1 = sim_engine_get_component(sim_engine, "eva1");
    if (eva1 == NULL) {
        printf("Simulation tried to access non-existent component 'eva1' for O2 error state update\n");
        return;
    }

    sim_field_t* field = sim_engine_find_field_within_component(eva1, "suit_pressure_oxy");
    if (field == NULL) {
        printf("Simulation tried to access non-existent field 'suit_pressure_oxy' for O2 error state update\n");
        return;
    }   

    bool o2_error_thrown = (sim_engine->error_type == SUIT_PRESSURE_OXY_LOW || sim_engine->error_type == SUIT_PRESSURE_OXY_HIGH);  
    bool o2_low_error_thrown = sim_engine->error_type == SUIT_PRESSURE_OXY_LOW;
    bool o2_high_error_thrown = sim_engine->error_type == SUIT_PRESSURE_OXY_HIGH;

    //update the oxy_error state and equation in the JSON file based on the current error state and DCU command
    if (sim_engine->dcu_field_settings->o2 == false) { //on secondary oxygen
        o2_error_flag = true;
        update_json_file("EVA", "error", "oxy_error", "false");
        if(o2_low_error_thrown) {
            field->algorithm = SIM_ALGO_LINEAR_GROWTH;
            field->rate.f = OXY_RATE;
            field->max_value.f = 4.0f; //nominal suit o2 pressure
        }

        if(o2_high_error_thrown) {
            field->algorithm = SIM_ALGO_LINEAR_DECAY;
            field->rate.f = OXY_RATE;
            field->min_value.f = 4.0f; //nominal suit o2 pressure
        }

    } else if (o2_error_thrown && sim_engine->dcu_field_settings->o2 == true) {
        update_json_file("EVA", "error", "oxy_error", "true");
        //rethrow the error if switched back to primary oxygen while an O2 error is still thrown to simulate the consequences of switching back to primary oxygen with an unresolved O2 error
        if(o2_low_error_thrown) {
            field->algorithm = SIM_ALGO_LINEAR_DECAY;
            field->rate.f = OXY_ERROR_RATE;
            field->min_value.f = 0.0f; //allow suit o2 pressure to drop to 0 to simulate a critical low O2 pressure in the suit
            if(o2_error_flag == true) {
                throw_O2_suit_pressure_low_error(sim_engine);
                o2_error_flag = false; //reset flag so that error is not thrown again until conditions are met again
            }
        }

        if(o2_high_error_thrown) {
            field->algorithm = SIM_ALGO_LINEAR_GROWTH;
            field->rate.f = OXY_ERROR_RATE;
            field->max_value.f = 4.2f; //allow suit o2 pressure to rise above nominal to simulate a critical high O2 pressure in the suit
            if(o2_error_flag == true) {
                throw_O2_suit_pressure_high_error(sim_engine);
                o2_error_flag = false; //reset flag so that error is not thrown again until conditions are met again
            }
        }
    } else { 
        update_json_file("EVA", "error", "oxy_error", "false");
    }
}

/**
* switches between which EVA scrubber is increasing linearly and decreasing linearly based on the current DCU command for CO2 scrubber.
* also switches whether suit_pressure_co2 is increasing or decreasing based on scrubber value and DCU command, to simulate the relationship between CO2 scrubber performance and suit CO2 pressure.
* If the DCU command for CO2 scrubber is set to true, scrubber_a_co2_storage will be set to linear_growth and scrubber_b_co2_storage will be set to linear_decay.
* If the DCU command for CO2 scrubber is set to false, scrubber_a_co2_storage will be set to linear_decay and scrubber_b_co2_storage will be set to linear_growth.
* If the increasing scrubber co2 storage value is above 30, the suit_pressure_co2 field will be set to linear_growth, simulating a buildup of CO2 in the suit due to poor scrubber performance. 
* If the increasing scrubber co2 storage value is below 30, the suit_pressure_co2 field will be set to linear_decay, simulating effective CO2 scrubbing and a decrease in suit CO2 pressure.
* @param sim_engine Pointer to the simulation engine to update
*/
void update_scrubber_state_EVA(sim_engine_t* sim_engine) {
    if (!sim_engine) {
        return;
    }

    sim_component_t* eva1 = sim_engine_get_component(sim_engine, "eva1");
    if (eva1 == NULL) {
        printf("Simulation tried to access non-existent component 'eva1' for scrubber error state update\n");
        return;
    }

    sim_field_t* scrubber_a_field = sim_engine_find_field_within_component(eva1, "scrubber_a_co2_storage");
    sim_field_t* scrubber_b_field = sim_engine_find_field_within_component(eva1, "scrubber_b_co2_storage");
    sim_field_t* suit_co2_pressure_field = sim_engine_find_field_within_component(eva1, "suit_pressure_co2");

    if (scrubber_a_field == NULL || scrubber_b_field == NULL || suit_co2_pressure_field == NULL) {
        printf("Simulation tried to access non-existent scrubber or suit pressure fields for scrubber error state update\n");
        return;
    }

    if (sim_engine->dcu_field_settings->co2 == true) {
        scrubber_a_field->algorithm = SIM_ALGO_LINEAR_GROWTH;
        scrubber_b_field->algorithm = SIM_ALGO_LINEAR_DECAY;
    } else {
        scrubber_a_field->algorithm = SIM_ALGO_LINEAR_DECAY;
        scrubber_b_field->algorithm = SIM_ALGO_LINEAR_GROWTH;
    }

    

    //if the increasing scrubber co2 storage value is above 30, set suit_pressure_co2 to linear growth, otherwise set it to linear decay
    if ((scrubber_a_field->algorithm == SIM_ALGO_LINEAR_GROWTH && scrubber_a_field->current_value.f > 30.0f) || (scrubber_b_field->algorithm == SIM_ALGO_LINEAR_GROWTH && scrubber_b_field->current_value.f > 30.0f)) {
        suit_co2_pressure_field->algorithm = SIM_ALGO_LINEAR_GROWTH;
    } else {
        suit_co2_pressure_field->algorithm = SIM_ALGO_LINEAR_DECAY;
    } 

    //update scrubber_error state in JSON file based on scrubber performance
    if ((scrubber_a_field->current_value.f > 60.0f && sim_engine->dcu_field_settings->co2 == true) || (scrubber_b_field->current_value.f > 60.0f && sim_engine->dcu_field_settings->co2 == false)) {
        update_json_file("EVA", "error", "scrubber_error", "true");
    } else {
        update_json_file("EVA", "error", "scrubber_error", "false");
    }
}

/**
* updates the fan error states based on the current DCU field settings and fan value.
* If the DCU command for the fan is set to false, the fan error states will be set to false (no error).
* If the fan RPM high error is thrown and the DCU command for the fan is set to true, the fan RPM error state will be set to true (error present).
* If the fan RPM low error is thrown and the DCU command for the fan is set to true, the fan RPM error state will be set to true (error present).
* The helmet_pressure_CO2 value will build up upon fan error and go back to normal upon fan error resolution
* @param sim_engine Pointer to the simulation engine to update
*/
void update_fan_error_state(sim_engine_t* sim_engine) {
    if (!sim_engine) {
        return;
    }

    //check if the fan RPM is below 30000
    sim_component_t* eva1 = sim_engine_get_component(sim_engine, "eva1");
    if (eva1 == NULL) {
        printf("Simulation tried to access non-existent component 'eva1' for fan error state update\n");
        return;
    }
    sim_field_t* field = sim_engine_find_field_within_component(eva1, "fan_pri_rpm");
    if (field == NULL) {
        printf("Simulation tried to access non-existent field 'fan_pri_rpm' for fan error state update\n");
        return;
    }

    sim_field_t* field_helmet_pressure_co2 = sim_engine_find_field_within_component(eva1, "helmet_pressure_co2");
    if (field_helmet_pressure_co2 == NULL) {
        printf("Simulation tried to access non-existent field 'helmet_pressure_co2' for fan error state update\n");
        return;
    }

    bool fan_error_thrown = (field->algorithm == SIM_ALGO_LINEAR_DECAY);


    //update the fan_error state in the JSON file based on the current error state and DCU command
    if (sim_engine->dcu_field_settings->fan == false) { //on backup fan
        fan_error_flag = false; //reset fan error flag when switching to backup fan to allow errors to be thrown again if switch back to primary fan with an error condition
        update_json_file("EVA", "error", "fan_error", "false");
        if(fan_error_thrown) {
            field_helmet_pressure_co2->algorithm = SIM_ALGO_LINEAR_DECAY;
            field_helmet_pressure_co2->rate.f = 1000.0;
        }
    } else if (fan_error_thrown && sim_engine->dcu_field_settings->fan == true) {
        update_json_file("EVA", "error", "fan_error", "true");
        if(fan_error_thrown) {
            if(fan_error_flag == true) {
                throw_fan_RPM_low_error(sim_engine);
                fan_error_flag = false; //reset flag so that error is not thrown again until conditions are met again
            }
        }

    } else {
        update_json_file("EVA", "error", "fan_error", "false");
    }
}

/**
* updates the power error state based on the current DCU field settings and battery values.
* If the DCU command for the battery.lu is set to true or battery.ps is set to false, the power error state will be set to false (no error).
* If the power error is thrown and the DCU command for battery.lu is set to false and battery.ps is set to true, the power error state will be set to true (error present).
* @param sim_engine Pointer to the simulation engine to update
 */
void update_power_error_state(sim_engine_t* sim_engine) {
    if (!sim_engine) {
        return;
    }

    //check if the power level is below the error threshold by checking the current value of the primary battery level field
    sim_component_t* eva1 = sim_engine_get_component(sim_engine, "eva1");
    if (eva1 == NULL) {
        printf("Simulation tried to access non-existent component 'eva1' for power error state update\n");
        return;
    }

    sim_field_t* field = sim_engine_find_field_within_component(eva1, "primary_battery_level");
    if (field == NULL) {
        printf("Simulation tried to access non-existent field 'primary_battery_level' for power error state update\n");
        return;
    }  

    bool power_error_thrown = (field->current_value.f < 20.0f);  //error threshold for battery level is 20%

    //update the power_error state in the JSON file based on the current error state and DCU commands
    if (sim_engine->dcu_field_settings->battery_lu == true || sim_engine->dcu_field_settings->battery_ps == false) {
        update_json_file("EVA", "error", "power_error", "false");
    } else if (power_error_thrown && sim_engine->dcu_field_settings->battery_lu == false && sim_engine->dcu_field_settings->battery_ps == true) {
        update_json_file("EVA", "error", "power_error", "true");
    } else {
        update_json_file("EVA", "error", "power_error", "false");
    }
}

/**
 * Updates error states in the simulation engine based on the current JSON values and updates JSON values accordingly to reflect any changes in the error states. This ensures that the simulation engine's error conditions are in sync with the JSON data.
 * For example, if a certain error condition is met in the simulation engine, it can update the corresponding field in the JSON file to reflect that error, and vice versa. This function acts as a bridge to keep the simulation engine and JSON data in sync regarding error states.
 * This function is called before each simulation update to ensure the engine reflects the latest error conditions
 *
 * @param sim_engine Pointer to the simulation engine to update
 */
void update_error_states(sim_engine_t* sim_engine) {

    update_EVA_error_simulation_error_states(sim_engine);
    update_scrubber_state_EVA(sim_engine);
}

/**
 * Calls the simulation engine to update all telemetry data based on elapsed time
 *
 * @param backend Backend data structure containing all telemetry and simulation engines
 */
void increment_simulation(struct backend_data_t *backend) {
    // Increment server time
    int new_time = time(NULL) - backend->start_time;
    bool time_incremented = false;
    if (new_time != backend->server_up_time) {
        backend->server_up_time = new_time;
        time_incremented = true;
    }

    

    // Update simulation engine once per second
    if (time_incremented) {

        // if UIA is connected, update the simulation values based on UIA states and ingress/egress procedures
        if(backend->sim_engine) {
            // Update simulation engine with elapsed time
            float delta_time = 1.0f;  // 1 second per update

            //update simulation engine DCU field settings based on the new values received from UDP commands
            
            update_sim_DCU_field_settings(backend->sim_engine);
            update_sim_UIA_field_settings(backend->sim_engine);
            update_sim_active_states(backend->sim_engine);
            update_error_states(backend->sim_engine);
            update_fan_values(backend->sim_engine);
            update_sim_UIA_connected(backend->sim_engine);
            sim_engine_update(backend->sim_engine, delta_time);
            update_ltv_error_dependencies();
            update_num_remaining_errors_LTV(backend->sim_engine);
            

            // Update EVA station timing
            update_eva_station_timing();
        }
    }
}

/**
 * Cleans up backend data structure and frees resources from memory, called in server.c
 * 
 * @param backend Backend data structure to cleanup
 */
void cleanup_backend(struct backend_data_t *backend) {
    if (!backend) return;

    // Cleanup simulation engine
    if (backend->sim_engine) {
        sim_engine_destroy(backend->sim_engine);
    }

    // Free backend data structure
    free(backend);
}

/** 
* checks if recovery mode is resolved by checking the LTV_ERRORS JSON file for needs_resolved value under Recovery Mode
* If the recovery mode is resolved, the function will return true, allowing the LTV_ERRORS data to be sent in response to UDP GET requests.
* If the recovery mode is not resolved, the function will return false, preventing the LTV_ERRORS data from being sent in response to UDP GET requests
*/
bool is_recovery_mode_resolved() {
    cJSON* ltv_errors_json = get_json_file("LTV_ERRORS");
    if (!ltv_errors_json) return false;

    bool result = false;
    cJSON* error_array = cJSON_GetObjectItem(ltv_errors_json, "error_procedures");
    
    if (cJSON_IsArray(error_array)) {
        int count = cJSON_GetArraySize(error_array);
        for (int i = 0; i < count; i++) {
            cJSON* error_item = cJSON_GetArrayItem(error_array, i);
            if (!error_item) continue;

            cJSON* desc_obj = cJSON_GetObjectItem(error_item, "description");
            
            // Explicit check to ensure valuestring exists
            if (cJSON_IsString(desc_obj) && desc_obj->valuestring != NULL) {
                if (strcmp(desc_obj->valuestring, "Exit Recovery Mode (ERM)") == 0) {
                    cJSON* needs_resolved = cJSON_GetObjectItem(error_item, "needs_resolved");
                    if (cJSON_IsBool(needs_resolved)) {
                        result = !cJSON_IsTrue(needs_resolved);
                    }
                    break; 
                }
            }
        }
    }

    cJSON_Delete(ltv_errors_json);
    return result;
}


/**
 * Just send JSON file recovery mode information part
 * @param filename Name of file with the recovery mode information
 * @param data Response buffer to populate with requested data
 */
void send_recovery_mode_json_file(const char* filename, unsigned char* data) {
    // Load full JSON as cJSON object
    cJSON* json = get_json_file(filename);
    if (json == NULL) {
        printf("Error: Could not load JSON file %s\n", filename);
        return;
    }

    // Get the error_procedures array directly
    cJSON* error_array = cJSON_GetObjectItem(json, "error_procedures");
    if (!error_array || !cJSON_IsArray(error_array)) {
        printf("Error: error_procedures not found or is not an array\n");
        cJSON_Delete(json);
        return;
    }

    // Get the first object in the array (Recovery Mode)
    cJSON* first_error = cJSON_GetArrayItem(error_array, 0);
    if (!first_error) {
        printf("Error: error_procedures array is empty\n");
        cJSON_Delete(json);
        return;
    }

    // Create a new JSON object with only the first error
    cJSON* send_json = cJSON_CreateObject();
    cJSON* new_array = cJSON_CreateArray();
    cJSON_AddItemToArray(new_array, cJSON_Duplicate(first_error, 1)); // deep copy
    cJSON_AddItemToObject(send_json, "error_procedures", new_array);

    // Convert new JSON object to string
    char* json_str = cJSON_Print(send_json);
    if (json_str == NULL) {
        printf("Error: Failed to convert JSON to string\n");
        cJSON_Delete(send_json);
        cJSON_Delete(json);
        return;
    }

    // Copy JSON string to data buffer
    size_t json_len = strlen(json_str);
    memcpy(data, json_str, json_len);
    data[json_len] = '\0'; // Null terminate

    // Cleanup
    free(json_str);
    cJSON_Delete(send_json);
    cJSON_Delete(json);
}

///////////////////////////////////////////////////////////////////////////////////
//                             UDP Request Handlers
///////////////////////////////////////////////////////////////////////////////////


/**
 * Handles UDP GET requests for data retrieval
 * 
 * @param command Command identifier for the GET request
 * @param data Response buffer to populate with requested data
 * @param backend Backend data structure containing all telemetry and simulation engines
 */
void handle_udp_get_request(unsigned int command, unsigned char* data, struct backend_data_t* backend) {
    // Handle different GET requests
    switch (command) {
        case 0: // EVA telemetry
            printf("Getting EVA telemetry data.\n");
            send_json_file("EVA", data);
            break;
        case 1: //LTV_ERRORS data
            //only print this data if the Recovery Mode is resolved
            if(is_recovery_mode_resolved()) {
                printf("Getting LTV error data.\n");
                send_json_file("LTV_ERRORS", data);
            } else {
                send_recovery_mode_json_file("LTV_ERRORS", data);
            }
            break;


        default:
            printf("Invalid GET command: %u\n", command);
            break;
    }
}

/**
 * Handles UDP POST requests for data updates using command-to-path mapping
 *
 * @param command Command identifier for the POST request
 * @param data Request buffer containing data to update
 * @param backend Backend data structure containing all telemetry and simulation engines
 * 
 * @return true if the update was successful, false otherwise
 */
bool handle_udp_post_request(unsigned int command, unsigned char* data, struct backend_data_t* backend) {
    // Find the mapping for this command
    const udp_command_mapping_t* mapping = NULL;
    for (int i = 0; udp_command_mappings[i].path != NULL; i++) {
        if (udp_command_mappings[i].command == command) {
            mapping = &udp_command_mappings[i];
            break;
        }
    }

    if (!mapping) {
        printf("Invalid UDP POST command: %u\n", command);
        return false;
    }
    
    // Extract value from UDP data
    char value_str[32];

    if (mapping->data_type == "bool") {
        // Special case for Unreal Engine commands where bool is sent as float
        float bool_float;
        memcpy(&bool_float, data, 4);
        bool value = bool_float != 0.0f;
        strcpy(value_str, value ? "true" : "false");
    } else {
        // Other commands extract float value from packet format
        if (strcmp(mapping->data_type, "bool") == 0) {
            bool value = extract_bool_value(data);

            strcpy(value_str, value ? "true" : "false");
        } else if (strcmp(mapping->data_type, "float") == 0) {
            float value = extract_float_value(data);
            sprintf(value_str, "%.6f", value);
        }
    }

    // Create request content in the same format as HTML forms
    char request_content[256];
    sprintf(request_content, "%s=%s", mapping->path, value_str);

    // Reuse existing html_form_json_update function for all the JSON and simulation logic
    //printf("Processing UDP command %u: %s = %s\n", command, mapping->path, value_str);

    bool result = html_form_json_update(request_content, backend);

    return result;
}

///////////////////////////////////////////////////////////////////////////////////
//                              Data Management
///////////////////////////////////////////////////////////////////////////////////

/**
 * Updates a field within the specified JSON file (supports both simple and nested field paths)
 * 
 * @param filename Name of the JSON file to update (e.g., "EVA")
 * @param section Section within the JSON file to update (e.g., "telemetry")
 * @param field_path Field path within the section to update (e.g., "batt_time_left" or "eva1.batt")
 * @param new_value New value to set for the specified field
 */
void update_json_file(const char* filename, const char* section, const char* field_path, char* new_value) {
    char file_path[100];
    snprintf(file_path, sizeof(file_path), "data/%s.json", filename);

    FILE *fp = fopen(file_path, "r");
    if (!fp) {
        printf("Error: Unable to open %s for reading.\n", file_path);
        return;
    }

    fseek(fp, 0L, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    char *file_buffer = malloc(file_size + 1);
    fread(file_buffer, 1, file_size, fp);
    file_buffer[file_size] = '\0';
    fclose(fp);

    cJSON *json = cJSON_Parse(file_buffer);
    free(file_buffer);

    if (!json) {
        printf("Error: Failed to parse JSON from %s\n", file_path);
        return;
    }

    cJSON *section_json = cJSON_GetObjectItemCaseSensitive(json, section);
    if (!section_json) {
        printf("Error: Section %s not found in JSON.\n", section);
        cJSON_Delete(json);
        return;
    }

    char path_copy[256];
    strncpy(path_copy, field_path, sizeof(path_copy)-1);
    path_copy[sizeof(path_copy)-1] = '\0';

    char* token;
    char* field_parts[20];
    int field_count = 0;

    token = strtok(path_copy, ".");
    while (token && field_count < 20) {
        field_parts[field_count++] = token;
        token = strtok(NULL, ".");
    }

    cJSON* current = section_json;

    for (int i = 0; i < field_count - 1; i++) {
        char* part = field_parts[i];

        // Check if this part is an array index
        char* endptr;
        long idx = strtol(part, &endptr, 10);

        if (*endptr == '\0') {
            // It's a number → array index
            if (!cJSON_IsArray(current)) {
                printf("Error: Expected array at %s but found object.\n", part);
                cJSON_Delete(json);
                return;
            }
            current = cJSON_GetArrayItem(current, idx);
            if (!current) {
                printf("Error: Array index %ld out of bounds.\n", idx);
                cJSON_Delete(json);
                return;
            }
        } else {
            // It's a string → object key
            current = cJSON_GetObjectItemCaseSensitive(current, part);
            if (!current) {
                printf("Error: Field path not found at %s.\n", part);
                cJSON_Delete(json);
                return;
            }
        }
    }

    // Handle final part (can also be array index or object key)
    char* final_part = field_parts[field_count - 1];
    cJSON* new_value_json = NULL;

    if (strcmp(new_value, "true") == 0) {
        new_value_json = cJSON_CreateTrue();
    } else if (strcmp(new_value, "false") == 0) {
        new_value_json = cJSON_CreateFalse();
    } else if (new_value[0] == '[') {
        // Parse as array of numbers
        cJSON* arr = cJSON_CreateArray();
        char* copy = strdup(new_value);
        char* item = strtok(copy + 1, ",]");
        while (item) {
            double n = strtod(item, NULL);
            cJSON_AddItemToArray(arr, cJSON_CreateNumber(n));
            item = strtok(NULL, ",]");
        }
        free(copy);
        new_value_json = arr;
    } else {
        char* endptr;
        double num = strtod(new_value, &endptr);
        if (*endptr == '\0') new_value_json = cJSON_CreateNumber(num);
        else new_value_json = cJSON_CreateString(new_value);
    }

    // Check if final part is array index
    char* endptr;
    long idx = strtol(final_part, &endptr, 10);

    if (*endptr == '\0') {
        if (!cJSON_IsArray(current)) {
            printf("Error: Expected array for final part but found object.\n");
            cJSON_Delete(json);
            return;
        }
        cJSON_ReplaceItemInArray(current, idx, new_value_json);
    } else {
        cJSON_ReplaceItemInObject(current, final_part, new_value_json);
    }

    char* out = cJSON_Print(json);
    fp = fopen(file_path, "w");
    if (!fp) {
        printf("Error: Unable to write to %s\n", file_path);
        free(out);
        cJSON_Delete(json);
        return;
    }
    fputs(out, fp);
    fclose(fp);
    free(out);
    cJSON_Delete(json);
}

/**
 * Loads and returns a cJSON object from the specified JSON file
 *
 * @param filename Name of the JSON file to load (e.g., "EVA")
 * @return Pointer to the cJSON object representing the file content, or NULL on failure
 */
cJSON* get_json_file(const char* filename) {
    // Construct the file path
    char file_path[100];
    snprintf(file_path, sizeof(file_path), "data/%s.json", filename);

    // Read existing JSON file
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        printf("Error: Unable to open the file %s for reading.\n", file_path);
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    char *file_buffer = malloc(file_size + 1);
    fread(file_buffer, 1, file_size, fp);
    file_buffer[file_size] = '\0'; // Null-terminate the buffer
    fclose(fp);

    // Parse the JSON content
    cJSON *json = cJSON_Parse(file_buffer);
    free(file_buffer);

    if (json == NULL) {
        printf("Error: Failed to parse JSON from file %s., check data folder for existing files. Accidental edits to those files can be resolved with git checkout data\n", file_path);
        return NULL;
    }

    return json;
}

/**
 * Sends the entire JSON file content as response data
 *
 * @param filename Name of the JSON file (e.g., "EVA")
 * @param data Response buffer to populate with JSON string
 */
void send_json_file(const char* filename, unsigned char* data) {
    cJSON* json = get_json_file(filename);
    if (json == NULL) {
        printf("Error: Could not load JSON file %s\n", filename);
        return;
    }
    
    // Convert JSON to string
    char* json_str = cJSON_Print(json);
    if (json_str == NULL) {
        printf("Error: Failed to convert JSON to string\n");
        cJSON_Delete(json);
        return;
    }
    
    // Copy JSON string to response data buffer
    size_t json_len = strlen(json_str);
    memcpy(data, json_str, json_len);
    data[json_len] = '\0'; // Null terminate
    
    // Cleanup
    free(json_str);
    cJSON_Delete(json);
}


/**
 * Synchronizes the simulation engine data to the corresponding JSON files
 *
 * @param backend Backend data structure containing telemetry and simulation engine
 */
void sync_simulation_to_json(struct backend_data_t* backend) {
    sim_engine_t* engine = backend->sim_engine;

    if (engine == NULL) {
        printf("Error: Simulation engine is NULL.\n");
        return;
    }

    // Load EVA JSON file
    cJSON* root = get_json_file("EVA");
    if (root == NULL) {
        printf("Error: Could not load EVA.json\n");
        return;
    }
    
    // Get or create the status section and update existing started field
    cJSON* status = cJSON_GetObjectItemCaseSensitive(root, "status");
    if (status == NULL) {
        status = cJSON_CreateObject();
        cJSON_AddItemToObject(root, "status", status);
    }

    // Update existing started field (check if eva1 OR eva2 is running)
    bool eva_running = sim_engine_is_component_running(engine, "eva1") || sim_engine_is_component_running(engine, "eva2");
    cJSON* started_field = cJSON_GetObjectItemCaseSensitive(status, "started");
    if (started_field != NULL) {
        cJSON_SetBoolValue(started_field, eva_running);
    } else {
        cJSON_AddBoolToObject(status, "started", eva_running);
    }

    // Get or create the telemetry section
    cJSON* telemetry = cJSON_GetObjectItemCaseSensitive(root, "telemetry");
    if (telemetry == NULL) {
        telemetry = cJSON_CreateObject();
        cJSON_AddItemToObject(root, "telemetry", telemetry);
    }
    
    // Get or create eva1 and eva2 sections under telemetry
    cJSON* eva1_section = cJSON_GetObjectItemCaseSensitive(telemetry, "eva1");
    if (eva1_section == NULL) {
        eva1_section = cJSON_CreateObject();
        cJSON_AddItemToObject(telemetry, "eva1", eva1_section);
    }
    
    cJSON* eva2_section = cJSON_GetObjectItemCaseSensitive(telemetry, "eva2");
    if (eva2_section == NULL) {
        eva2_section = cJSON_CreateObject();
        cJSON_AddItemToObject(telemetry, "eva2", eva2_section);
    }
    
    // Update simulation fields in their respective sections
    for (int i = 0; i < engine->total_field_count; i++) {
        sim_field_t* field = engine->update_order[i];
        if (field != NULL) {
            double value = field->current_value.f;

            // Determine target section based on component name
            cJSON* target_section = NULL;
            if (strcmp(field->component_name, "eva1") == 0) {
                target_section = eva1_section;
            } else if (strcmp(field->component_name, "eva2") == 0) {
                target_section = eva2_section;
            }
    
            if (target_section != NULL) {
                // Check if field already exists and replace it, otherwise add new
                cJSON* existing_field = cJSON_GetObjectItemCaseSensitive(target_section, field->field_name);
                if (existing_field != NULL) {
                    cJSON_SetNumberValue(existing_field, value);
                } else {
                    cJSON_AddNumberToObject(target_section, field->field_name, value);
                }
            }
        }
    }
    
    // Write EVA file
    char filepath[100];
    snprintf(filepath, sizeof(filepath), "data/EVA.json");
    
    char* json_str = cJSON_Print(root);
    FILE* fp = fopen(filepath, "w");
    if (fp) {
        fputs(json_str, fp);
        fclose(fp);
    }
    
    free(json_str);
    cJSON_Delete(root);
}

void backend_reset_errors(void* ctx) {
    struct backend_data_t* backend = ctx;

    update_json_file("LTV_ERRORS", "error_procedures", "0.needs_resolved", "true");
    update_json_file("LTV_ERRORS", "error_procedures", "1.needs_resolved", "true");
    update_json_file("LTV_ERRORS", "error_procedures", "2.needs_resolved", "true");
    update_json_file("LTV_ERRORS", "error_procedures", "3.needs_resolved", "true");
    update_json_file("LTV_ERRORS", "error_procedures", "4.needs_resolved", "false");
    update_json_file("LTV_ERRORS", "error_procedures", "5.needs_resolved", "false");
    //update_json_file("LTV_ERRORS", "error_procedures", "6.needs_resolved", "true");
    //update_json_file("LTV_ERRORS", "error_procedures", "7.needs_resolved", "true");

    printf("LTV errors reset via update_json_file\n");
}

/**
 * Updates a field in a JSON file based on a route-style request (for example, "eva.error.fan_error=true") from a HTML form submission
 * The request content is parsed and matched to the appropriate JSON file and field.
 * 
 * // @TODO look into this more
 * 
 * @example request_content: "eva.error.fan_error=true" -> EVA.json, section "error", field "fan_error", value true
 * @param request_content String containing the route-based update request
 * @param backend Backend data structure
 * @return true if update was successful, false otherwise
 */
bool html_form_json_update(char* request_content, struct backend_data_t* backend) {
    // Parse URL-encoded data: "route=value"
    char* route = NULL;
    char* value = NULL;

    // Create a copy to work with since strtok modifies the string
    char request_copy[1024];
    strncpy(request_copy, request_content, sizeof(request_copy) - 1);
    request_copy[sizeof(request_copy) - 1] = '\0';

    // Parse URL-encoded parameters
    char* param = strtok(request_copy, "&");
    while (param != NULL) {
        char* equals_pos = strchr(param, '=');
        if (equals_pos != NULL) {
            *equals_pos = '\0';  // Split parameter name and value
            char* param_name = param;
            char* param_value = equals_pos + 1;

            // route parameter e.g. "eva.error.fan_error"
            route = param_name;
            value = param_value;
            break;  // Take the first parameter as the route
        }
        param = strtok(NULL, "&");
    }

    if (route == NULL || value == NULL) {
        printf("Error: Invalid format, missing route or value in request: %s\n", request_content);
        return false;
    }

    // Parse the route (split by dots)
    char route_copy[256];
    strncpy(route_copy, route, sizeof(route_copy) - 1);
    route_copy[sizeof(route_copy) - 1] = '\0';
    
    // Split route into parts
    char* route_parts[10];  // Support up to 10 levels deep
    int part_count = 0;
    
    char* token = strtok(route_copy, ".");
    while (token != NULL && part_count < 10) {
        route_parts[part_count++] = token;
        token = strtok(NULL, ".");
    }
    
    if (part_count < 2) {
        printf("Error: Route must have at least 2 parts (file.section): %s\n", route);
        return false;
    }
    
    // Determine file type and construct JSON path
    const char* filename = NULL;
    if (strcmp(route_parts[0], "eva") == 0) {
        filename = "EVA";
    } else if (strcmp(route_parts[0], "ltv") == 0) {
        filename = "LTV";
    } else if (strcmp(route_parts[0], "ltv_errors") == 0) {
        filename = "LTV_ERRORS";
    } 
    else {
        printf("Error: Unsupported file type '%s'. Use 'eva', 'ltv_errors', or 'ltv'\n", route_parts[0]);
        return false;
    }
    
    // Build the JSON path from remaining route parts

    if (part_count == 3) {
        // Simple case: file.section.field
        const char* section = route_parts[1];
        const char* field = route_parts[2];
        
        update_json_file(filename, section, field, value);

        if (strcmp(filename, "EVA") == 0 && strcmp(section, "status") == 0 && strcmp(field, "started") == 0) {
            if (backend->sim_engine) {
                if (strcmp(value, "true") == 0) {
                    sim_engine_start_component(backend->sim_engine, "eva1");
                    sim_engine_start_component(backend->sim_engine, "eva2");
                    printf("Started EVA simulation\n");
                } else {
                    sim_engine_reset_component(backend->sim_engine, "eva1", update_json_file);
                    backend->sim_engine->time_to_complete_task_board = -10;
                    sim_engine_reset_component(backend->sim_engine, "eva2", update_json_file);
                    reset_eva_station_timing();
                    printf("Reset EVA simulation\n");
                }
            }
        }

        return true;
    } else if (part_count > 3) {
        // Nested case: file.section.subsection.field or deeper
        const char* section = route_parts[1];
        
        // Rebuild the nested field path from remaining parts
        char nested_field[256] = "";
        for (int i = 2; i < part_count; i++) {
            if (i > 2) {
                strcat(nested_field, ".");
            }
            strcat(nested_field, route_parts[i]);
        }
        
        // For now, handle nested updates by directly updating the JSON
        // This requires extending update_json_file to handle nested paths
        update_json_file(filename, section, nested_field, value);

        return true;
    } else {
        printf("Error: Invalid route format: %s\n", route);
        return false;
    }
}

/**
* Updates sim_UIA_field_settings based on the current state of the UIA station
* @param sim_engine Pointer to the simulation engine
*/
void update_sim_UIA_field_settings(sim_engine_t* sim_engine) {
    if (!sim_engine || !sim_engine->uia_field_settings) {
        return;
    }

    sim_UIA_field_settings_t* settings = sim_engine->uia_field_settings;

    // Update eva1_power setting
    settings->eva1_power = (get_field_from_json("EVA", "uia.eva1_power", 0.0) == 1.0);

    // Update eva2_power setting
    settings->eva2_power = (get_field_from_json("EVA", "uia.eva2_power", 0.0) == 1.0);

    // Update eva1_oxy setting
    settings->eva1_oxy = (get_field_from_json("EVA", "uia.eva1_oxy", 0.0) == 1.0);

    // Update eva2_oxy setting
    settings->eva2_oxy = (get_field_from_json("EVA", "uia.eva2_oxy", 0.0) == 1.0);

    // Update eva1_water_supply setting
    settings->eva1_water_supply = (get_field_from_json("EVA", "uia.eva1_water_supply", 0.0) == 1.0);

    // Update eva2_water_supply setting
    settings->eva2_water_supply = (get_field_from_json("EVA", "uia.eva2_water_supply", 0.0) == 1.0);

    // Update eva1_water_waste setting
    settings->eva1_water_waste = (get_field_from_json("EVA", "uia.eva1_water_waste", 0.0) == 1.0);

    // Update eva2_water_waste setting
    settings->eva2_water_waste = (get_field_from_json("EVA", "uia.eva2_water_waste", 0.0) == 1.0);

    // Update oxy_vent setting
    settings->oxy_vent = (get_field_from_json("EVA", "uia.oxy_vent", 0.0) == 1.0);

    //Update depress setting
    settings->depress = (get_field_from_json("EVA", "uia.depress", 0.0) == 1.0);



    
}

/**
* Updates sim_DCU_field_settings based on the current state of the DCU station
* @param sim_engine Pointer to the simulation engine
*/
void update_sim_DCU_field_settings(sim_engine_t* sim_engine) {
    if (!sim_engine || !sim_engine->dcu_field_settings) {
        return;
    }

    sim_DCU_field_settings_t* settings = sim_engine->dcu_field_settings;

    // Update battery_lu setting
    settings->battery_lu = (get_field_from_json("EVA", "dcu.eva1.batt.lu", 0.0) == 1.0);

    // Update battery_ps setting
    settings->battery_ps = (get_field_from_json("EVA", "dcu.eva1.batt.ps", 0.0) == 1.0);

    // Update fan setting
    settings->fan = (get_field_from_json("EVA", "dcu.eva1.fan", 0.0) == 1.0);

    // Update o2 setting
    settings->o2 = (get_field_from_json("EVA", "dcu.eva1.oxy", 0.0) == 1.0);

    //update pump setting
    settings->pump = (get_field_from_json("EVA", "dcu.eva1.pump", 0.0) == 1.0);

    //update co2 setting
    settings->co2 = (get_field_from_json("EVA", "dcu.eva1.co2", 0.0) == 1.0);

}


/**
 * Gets a field value from a JSON file using a dot-separated path
 *
 * @param filename Name of the JSON file (e.g. "EVA")
 * @param field_path Dot-separated path to the field (e.g., "pr_telemetry.brakes" or "telemetry.eva1.batt")
 * @param default_value Default value to return if field is not found or invalid
 * @return Field value as double, or default_value if not found
 */
double get_field_from_json(const char* filename, const char* field_path, double default_value) {
    cJSON* json = get_json_file(filename);
    if (json == NULL) {
        return default_value;
    }
    
    // Parse the field path
    char path_copy[256];
    strncpy(path_copy, field_path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';
    
    cJSON* current_object = json;
    char* field_parts[10];
    int field_part_count = 0;
    
    // Split field path by dots
    char* token = strtok(path_copy, ".");
    while (token != NULL && field_part_count < 10) {
        field_parts[field_part_count++] = token;
        token = strtok(NULL, ".");
    }
    
    // Navigate through the JSON path
    for (int i = 0; i < field_part_count; i++) {
        cJSON* next_object = cJSON_GetObjectItemCaseSensitive(current_object, field_parts[i]);
        if (next_object == NULL) {
            cJSON_Delete(json);
            return default_value;
        }
        current_object = next_object;
    }
    
    // Extract the value based on type
    double result = default_value;
    if (cJSON_IsNumber(current_object)) {
        result = cJSON_GetNumberValue(current_object);
    } else if (cJSON_IsBool(current_object)) {
        result = cJSON_IsTrue(current_object) ? 1.0 : 0.0;
    } else if (cJSON_IsString(current_object)) {
        // Try to parse string as number
        char* endptr;
        double parsed_value = strtod(cJSON_GetStringValue(current_object), &endptr);
        if (*endptr == '\0') {
            result = parsed_value;
        }
    }
    
    cJSON_Delete(json);
    return result;
}

/**
 * Updates EVA station timing based on started states
 * Increments time for stations that are started and marks completed when stopped
 */
void update_eva_station_timing(void) {
    // Load current EVA JSON data
    cJSON* eva_json = get_json_file("EVA");
    if (eva_json == NULL) {
        return;
    }

    // Get the status section
    cJSON* status = cJSON_GetObjectItemCaseSensitive(eva_json, "status");
    if (status == NULL) {
        cJSON_Delete(eva_json);
        return;
    }

    bool json_modified = false;
    const char* stations[] = {"uia", "dcu", "spec"};
    int num_stations = sizeof(stations) / sizeof(stations[0]);

    // Process each station
    for (int i = 0; i < num_stations; i++) {
        const char* station_name = stations[i];

        // Get station object
        cJSON* station = cJSON_GetObjectItemCaseSensitive(status, station_name);
        if (station == NULL) {
            continue;
        }

        // Get started and time fields
        cJSON* started_field = cJSON_GetObjectItemCaseSensitive(station, "started");
        cJSON* time_field = cJSON_GetObjectItemCaseSensitive(station, "time");
        cJSON* completed_field = cJSON_GetObjectItemCaseSensitive(station, "completed");

        if (started_field == NULL || time_field == NULL) {
            continue;
        }

        bool is_started = cJSON_IsTrue(started_field);
        double current_time = cJSON_GetNumberValue(time_field);

        // If station is started, increment time
        if (is_started) {
            cJSON_SetNumberValue(time_field, current_time + 1.0);
            json_modified = true;
        }

        // Check if station was just stopped (completed should be set when started changes from true to false)
        // This is handled by the frontend toggle logic, but we can ensure completed status is consistent
        if (!is_started && completed_field != NULL && cJSON_IsFalse(completed_field) && current_time > 0) {
            cJSON_SetBoolValue(completed_field, true);
            json_modified = true;
        }
    }

    // Save changes if JSON was modified
    if (json_modified) {
        char filepath[100];
        snprintf(filepath, sizeof(filepath), "data/EVA.json");

        char* json_str = cJSON_Print(eva_json);
        FILE* fp = fopen(filepath, "w");
        if (fp) {
            fputs(json_str, fp);
            fclose(fp);
        }
        free(json_str);
    }

    cJSON_Delete(eva_json);
}

/**
 * Resets EVA station timing by setting all station times to 0 and completed status to false
 */
void reset_eva_station_timing(void) {
    // Load current EVA JSON data
    cJSON* eva_json = get_json_file("EVA");
    if (eva_json == NULL) {
        return;
    }

    // Get the status section
    cJSON* status = cJSON_GetObjectItemCaseSensitive(eva_json, "status");
    if (status == NULL) {
        cJSON_Delete(eva_json);
        return;
    }

    bool json_modified = false;
    const char* stations[] = {"uia", "dcu", "spec"};
    int num_stations = sizeof(stations) / sizeof(stations[0]);

    // Reset each station
    for (int i = 0; i < num_stations; i++) {
        const char* station_name = stations[i];

        // Get station object
        cJSON* station = cJSON_GetObjectItemCaseSensitive(status, station_name);
        if (station == NULL) {
            continue;
        }

        // Get time and completed fields
        cJSON* time_field = cJSON_GetObjectItemCaseSensitive(station, "time");
        cJSON* completed_field = cJSON_GetObjectItemCaseSensitive(station, "completed");

        // Reset time to 0
        if (time_field != NULL) {
            cJSON_SetNumberValue(time_field, 0.0);
            json_modified = true;
        }

        // Reset completed status to false
        if (completed_field != NULL) {
            cJSON_SetBoolValue(completed_field, false);
            json_modified = true;
        }
    }

    // Save changes if JSON was modified
    if (json_modified) {
        char filepath[100];
        snprintf(filepath, sizeof(filepath), "data/EVA.json");

        char* json_str = cJSON_Print(eva_json);
        FILE* fp = fopen(filepath, "w");
        if (fp) {
            fputs(json_str, fp);
            fclose(fp);
        }
        free(json_str);
    }

    cJSON_Delete(eva_json);
}

///////////////////////////////////////////////////////////////////////////////////
//                              Helper Functions
///////////////////////////////////////////////////////////////////////////////////

/**
 * Reverses the byte order of a 4-byte value for endianness conversion
 * 
 * @param bytes Pointer to 4-byte array to reverse
 */
void reverse_bytes(unsigned char *bytes) {
    // expects 4 bytes to be flipped
    char temp;
    for (int i = 0; i < 2; i++) {
        temp = bytes[i];
        bytes[i] = bytes[3 - i];
        bytes[3 - i] = temp;
    }
}

/**
 * Determines if the system uses big-endian byte ordering
 *
 * @return true if system is big-endian, false if little-endian
 */
bool big_endian() {
    unsigned int i = 1;
    unsigned char temp[4];

    memcpy(temp, &i, 4);

    if (temp[0] == 1) {
        // System is little-endian
        return false;
    } else {
        // System is big-endian
        return true;
    }
}

/**
 * Extracts boolean value from UDP data (bytes 4-7 as float, non-zero = true)
 *
 * @param data UDP data buffer
 * @return Boolean value
 */
bool extract_bool_value(unsigned char* data) {
    float bool_float;
    memcpy(&bool_float, data, 4);
    return bool_float != 0.0f;
}

/**
 * Extracts float value from UDP data (bytes 4-7)
 *
 * @param data UDP data buffer
 * @return Float value
 */
float extract_float_value(unsigned char* data) {
    float value;
    memcpy(&value, data, 4);
    return value;
}