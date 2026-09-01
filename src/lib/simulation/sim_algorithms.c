#include "sim_algorithms.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

///////////////////////////////////////////////////////////////////////////////////
//                           Algorithm Implementations
///////////////////////////////////////////////////////////////////////////////////

/**
 * Sine wave algorithm for oscillating values (progresses one step per call)
 * 
 * @param field Pointer to the field containing algorithm parameters
 * @return Calculated value based on sine wave function
 */
sim_value_t sim_algo_sine_wave(sim_field_t* field) {
    sim_value_t result = {0};
    
    if (!field || !field->params) return result;
    
    // Check if field is active before applying decay
    if (field->active) {
    // Get parameters for the sine wave calculation
    float base = field->base_value.f;
    float amp = field->amplitude.f;
    float freq = field->frequency.f;
    
    // Initialize phase accumulator if not already present
    if (field->phase_acc.f < 0.0f) {
        field->phase_acc.f = 0.0f;
    }

    // Calculate sine value based on current phase
    float sine_value = base + amp * sinf(field->phase_acc.f);

    // Advance phase for next call
    field->phase_acc.f += freq;

    // Keep phase in [0, 2π] for numerical stability
    if (field->phase_acc.f > 2.0f * 3.1415f) {
        field->phase_acc.f = 0.0f;
    }

    result.f = sine_value;
    } else {
        result.f = field->current_value.f;
    }
    return result;
}

/**
* Linear decay algorithm for decreasing values over time.
 * Values decrease linearly from current_value by decay rate each call, until they reach min_value.
 * 
 * @param field Pointer to the field containing algorithm parameters
 * @return Calculated value based on linear decay
 */
sim_value_t sim_algo_linear_decay(sim_field_t* field) {
    sim_value_t result = {0};
    
    if (!field || !field->params) return result;
    
    // Get parameters
    float current_value = field->current_value.f;
    float min_value = field->min_value.f;
    float decay_rate = field->rate.f;
    
    // Check if field is active before applying decay
    if (field->active) {
        // Calculate decayed value based on decay rate current value
        float decayed_value = current_value - decay_rate;

        if (decayed_value < min_value) {
            decayed_value = min_value;
        }

        result.f = decayed_value;

    } else {
        result.f = current_value;
    }

    return result;
}

/**
 * Linear growth algorithm for increasing values over time.
 * Values increase linearly from current_value by growth rate each call, until they reach max_value.
 * 
 * @param field Pointer to the field containing algorithm parameters
 * @return Calculated value based on linear growth
 */
sim_value_t sim_algo_linear_growth(sim_field_t* field) {
    sim_value_t result = {0};
    
    if (!field || !field->params) return result;
    
    // Get parameters
    float current_value = field->current_value.f;
    float max_value = field->max_value.f;
    float growth_rate = field->rate.f;
    
    // Check if field is active before applying growth
    if (field->active) {
        // Calculate grown value based on growth rate current value
        float grown_value = current_value + growth_rate;

        if (grown_value > max_value) {
            grown_value = max_value;
        }

        result.f = grown_value;

    } else {
        result.f = current_value;
    }

    return result;
}

/**
* Constant value algorithm that keeps the field at a constant value specified by the "constant_value" parameter.
 * 
 * @param field Pointer to the field containing algorithm parameters
 * @return Constant value specified in parameters
 */
sim_value_t sim_algo_constant_value(sim_field_t* field) {
    sim_value_t result = {0};
    
    if (!field || !field->params) {
        return result;
    } 
    
    
    result.f = field->current_value.f;

    return result;
}

/**
 * Implements dependent value algorithm for fields calculated from other fields.
 * Evaluates a mathematical formula using values from other fields as inputs.
 * 
 * @param field Pointer to the field containing algorithm parameters
 * @param engine Pointer to the simulation engine for accessing other field values
 * @return Calculated value based on formula evaluation
 */
sim_value_t sim_algo_dependent_value(sim_field_t* field, sim_engine_t* engine) {
    sim_value_t result = {0};
    
    if (!field || !field->params || !engine) return result;

    if(field->active == false) {
        result.f = field->current_value.f;
        return result;
    }

    // Get formula parameter
    cJSON* formula = cJSON_GetObjectItem(field->params, "formula");
    if (!formula || !cJSON_IsString(formula)) {
        printf("Warning: No formula specified for dependent field %s\n", field->field_name);
        return result;
    }
    
    const char* formula_str = cJSON_GetStringValue(formula);
    float calculated_value = sim_algo_evaluate_formula(formula_str, engine);

    result.f = calculated_value;

    return result;
}



/**
 * External value algorithm for fetching values from data JSON files.
 * Reads the specified field from data/{file_path}.
 *
 * @param field Pointer to the field containing algorithm parameters
 * @param engine Pointer to the simulation engine
 * @return Value fetched from external JSON file
 */
sim_value_t sim_algo_external_value(sim_field_t* field, sim_engine_t* engine) {
    sim_value_t result = {0};

    if (!field || !field->params || !engine) return result;

    // Get parameters
    cJSON* file_path_param = cJSON_GetObjectItem(field->params, "file_path");
    cJSON* field_path_param = cJSON_GetObjectItem(field->params, "field_path");

    if (!file_path_param || !cJSON_IsString(file_path_param)) {
        printf("Warning: No file_path specified for external_value field %s\n", field->field_name);
        return result;
    }

    if (!field_path_param || !cJSON_IsString(field_path_param)) {
        printf("Warning: No field_path specified for external_value field %s\n", field->field_name);
        return result;
    }

    const char* file_path = cJSON_GetStringValue(file_path_param);
    const char* field_path = cJSON_GetStringValue(field_path_param);

    // Construct full file path: data/{file_path}
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "data/%s", file_path);

    // Read and parse JSON file
    FILE* file = fopen(full_path, "r");
    if (!file) {
        printf("Warning: Cannot open external data file: %s\n", full_path);
        return result;
    }

    // Get file size and read content
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* json_string = malloc(file_size + 1);
    if (!json_string) {
        fclose(file);
        return result;
    }

    fread(json_string, 1, file_size, file);
    json_string[file_size] = '\0';
    fclose(file);

    // Parse JSON
    cJSON* root = cJSON_Parse(json_string);
    free(json_string);

    if (!root) {
        printf("Warning: Invalid JSON in external data file: %s\n", full_path);
        return result;
    }

    // Navigate through field path using dot notation (e.g., "telemetry.eva1.temperature")
    cJSON* current_obj = root;
    char* path_copy = strdup(field_path);
    char* token = strtok(path_copy, ".");

    while (token && current_obj) {
        current_obj = cJSON_GetObjectItem(current_obj, token);
        token = strtok(NULL, ".");
    }

    // Extract value
    if (current_obj) {
        if (cJSON_IsNumber(current_obj)) {
            double value = cJSON_GetNumberValue(current_obj);
            result.f = (float)value;
        } else if (cJSON_IsBool(current_obj)) {
            // Convert boolean to number (true = 1.0, false = 0.0)
            double value = cJSON_IsTrue(current_obj) ? 1.0 : 0.0;
            result.f = (float)value;
        } else {
            printf("Warning: Field '%s' in %s is not a number or boolean\n", field_path, full_path);
        }
    } else {
        printf("Warning: Could not find field '%s' in %s\n", field_path, full_path);
    }

    free(path_copy);
    cJSON_Delete(root);
    return result;
}


///////////////////////////////////////////////////////////////////////////////////
//                            Utility Functions
///////////////////////////////////////////////////////////////////////////////////

/**
 * Returns operator precedence level for order of operations.
 * Higher values indicate higher precedence.
 *
 * @param op Operator character (+, -, *, /)
 * @return Precedence level (3 for * /, 2 for + -, 0 for others)
 */
static int get_precedence(char op) {
    switch(op) {
        case '*':
        case '/':
            return 3;
        case '+':
        case '-':
            return 2;
        default:
            return 0;
    }
}

/**
 * Applies a binary operator to two operands
 *
 * @param op Operator character (+, -, *, /)
 * @param a Left operand
 * @param b Right operand
 * @return Result of applying operator to operands
 */
static float apply_operator(char op, float a, float b) {
    switch(op) {
        case '+':
            return a + b;
        case '-':
            return a - b;
        case '*':
            return a * b;
        case '/':
            return (b != 0.0f) ? a / b : 0.0f;
        default:
            return 0.0f;
    }
}

/**
 * Parses a token as either a numeric literal or field name lookup
 *
 * @param token Token string to parse
 * @param engine Simulation engine for field value lookup
 * @return Numeric value of token
 */
static float parse_token_value(const char* token, sim_engine_t* engine) {
    if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
        return atof(token);
    }
    sim_value_t field_value = sim_engine_get_field_value(engine, token);
    return field_value.f;
}

/**
 * Evaluates a mathematical formula string using current field values.
 * Supports arithmetic operations (+, -, *, /) and parentheses.
 * Implements proper operator precedence (* / before + -) and parentheses grouping.
 * Example: "90.0 + (temperature - 21.1) * 0.36"
 *
 * @param formula String containing the mathematical formula to evaluate
 * @param engine Pointer to the simulation engine for field value lookup
 * @return Calculated result of the formula evaluation
 */
float sim_algo_evaluate_formula(const char* formula, sim_engine_t* engine) {
    if (!formula || !engine) return 0.0f;

    // Stacks for two-stack algorithm
    float value_stack[64];
    int value_top = -1;

    char op_stack[64];
    int op_top = -1;

    // Tokenize and process
    char* formula_copy = strdup(formula);
    char* token;
    char* rest = formula_copy;

    while ((token = strtok_r(rest, " ", &rest))) {
        // Skip commas
        if (strcmp(token, ",") == 0) {
            continue;
        }

        // Handle opening parenthesis
        if (token[0] == '(' && token[1] == '\0') {
            op_stack[++op_top] = '(';
            continue;
        }

        // Handle closing parenthesis
        if (token[0] == ')' && token[1] == '\0') {
            // Pop operators until '('
            while (op_top >= 0 && op_stack[op_top] != '(') {
                char op = op_stack[op_top--];
                if (value_top < 1) break;  // Safety check
                float b = value_stack[value_top--];
                float a = value_stack[value_top--];
                value_stack[++value_top] = apply_operator(op, a, b);
            }

            // Remove '(' from stack
            if (op_top >= 0 && op_stack[op_top] == '(') {
                op_top--;
            }
            continue;
        }

        // Handle operators
        if (strlen(token) == 1 && strchr("+-*/", token[0])) {
            char op = token[0];
            int prec = get_precedence(op);

            // Pop higher or equal precedence operators
            while (op_top >= 0 && op_stack[op_top] != '(' &&
                   get_precedence(op_stack[op_top]) >= prec) {
                char prev_op = op_stack[op_top--];
                if (value_top < 1) break;  // Safety check
                float b = value_stack[value_top--];
                float a = value_stack[value_top--];
                value_stack[++value_top] = apply_operator(prev_op, a, b);
            }

            op_stack[++op_top] = op;
            continue;
        }

        // Handle numbers and field names
        float value = parse_token_value(token, engine);
        value_stack[++value_top] = value;
    }

    // Pop remaining operators
    while (op_top >= 0) {
        char op = op_stack[op_top--];
        if (op == '(') continue;  // Skip unmatched parens
        if (value_top < 1) break;  // Safety check
        float b = value_stack[value_top--];
        float a = value_stack[value_top--];
        value_stack[++value_top] = apply_operator(op, a, b);
    }

    free(formula_copy);
    return value_top >= 0 ? value_stack[value_top] : 0.0f;
}

/**
 * Parses an algorithm type string into its corresponding enum value.
 * Converts JSON algorithm type strings into internal algorithm type enums.
 * 
 * @param algo_string String name of the algorithm (e.g., "sine_wave", "linear_decay")
 * @return Corresponding algorithm type enum, or SIM_ALGO_SINE_WAVE if not recognized
 */
sim_algorithm_type_t sim_algo_parse_type_string(const char* algo_string) {
    if (!algo_string) return SIM_ALGO_SINE_WAVE;
    
    if (strcmp(algo_string, "sine_wave") == 0) {
        return SIM_ALGO_SINE_WAVE;
    } else if (strcmp(algo_string, "linear_decay") == 0) {
        return SIM_ALGO_LINEAR_DECAY;
    } else if (strcmp(algo_string, "linear_growth") == 0) {
        return SIM_ALGO_LINEAR_GROWTH;
    } else if (strcmp(algo_string, "dependent_value") == 0) {
        return SIM_ALGO_DEPENDENT_VALUE;
    } else if (strcmp(algo_string, "external_value") == 0) {
        return SIM_ALGO_EXTERNAL_VALUE;
    } else if (strcmp(algo_string, "constant_value") == 0) {
        return SIM_ALGO_CONSTANT_VALUE;
    }
    
    return SIM_ALGO_SINE_WAVE;  // Default algorithm
}

/**
 * Converts an algorithm type enum to its string representation.
 * Used for debugging and status display purposes.
 * 
 * @param type Algorithm type enum value
 * @return String representation of the algorithm type
 */
const char* sim_algo_type_to_string(sim_algorithm_type_t type) {
    switch (type) {
        case SIM_ALGO_SINE_WAVE:
            return "sine_wave";
        case SIM_ALGO_LINEAR_DECAY:
            return "linear_decay";
        case SIM_ALGO_LINEAR_GROWTH:
            return "linear_growth";
        case SIM_ALGO_DEPENDENT_VALUE:
            return "dependent_value";
        case SIM_ALGO_EXTERNAL_VALUE:
            return "external_value";
        case SIM_ALGO_CONSTANT_VALUE:
            return "constant_value";
        default:
            return "unknown";
    }
}