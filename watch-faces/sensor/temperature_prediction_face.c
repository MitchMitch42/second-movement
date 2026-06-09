/*
 * MIT License
 *
 * Copyright (c) 2025 Mitch42
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "temperature_correction_face.h"

// Default initial values for the temperature correction face
#define TEMPERATURE_CORRECTION_DEFAULT_COEFFICIENT 0.002F
#define TEMPERATURE_CORRECTION_DEFAULT_BUFFER_SIZE 60
#define TEMPERATURE_CORRECTION_DEFAULT_AVERAGE_COUNT 30

//int debug_index = -1;
//float debug_data[] = {31.5, 31.4, 31.4, 31.4, 31.4, 31.4, 31.4, 31.4, 31.4, 31.4, 31.4, 31.4, 31.3, 31.3, 31.3, 31.3, 31.3, 31.3, 31.3, 31.3, 31.3, 31.3, 31.2, 31.2, 31.2, 31.2, 31.2, 31.2, 31.2};
//float debug_data[] = { 15.7, 15.6, 15.6, 15.6, 15.6, 15.6, 15.5, 15.5, 15.5, 15.5, 15.5, 15.4, 15.4, 15.4, 15.4, 15.3, 15.3, 15.3, 15.3, 15.2, 15.2, 15.2, 15.2, 15.2, 15.1, 15.1, 15.1, 15.1, 15.1, 15.0, 15.0, 15.0, 15.0, 14.9, 14.9, 14.9, 14.9, 14.9, 14.8, 14.8, 14.8, 14.8, 14.8, 14.7, 14.7, 14.7, 14.7, 14.7, 14.6, 14.6, 14.6, 14.6, 14.5, 14.5 };

/// @brief add a value to a rolling buffer
/// @param buffer rolling buffer to update
/// @param value value to append
static void temperature_correction_face_add_to_rolling_buffer(temperature_correction_rolling_buffer_t *buffer, float value, bool display) {
    buffer->head_index = (buffer->head_index + 1) % buffer->max;
    buffer->length = buffer->length + 1 < buffer->max ? buffer->length + 1 : buffer->max;
    buffer->data[buffer->head_index] = value;
    if (display) { //display buffer item count
        char buf[8];
        sprintf(buf, "%2d", buffer->length);
        watch_display_text(WATCH_POSITION_TOP_RIGHT, buf);
    }
}

/// @brief calculate heat transfer coefficient of Newtons law of cooling, using two points
static float temperature_correction_face_calculate_coefficient(int delta, float temperature_start, float temperature_current, float temperature_end ) {
    // =(1/G28)*LN((J28-I28)/(H28-I28))
    return (1.0 / (float)delta) * logf((temperature_start - temperature_end) / (temperature_current - temperature_end));
}

/// @brief calculate heat transfer coefficient of Newtons law of cooling, using all points and performing a linear regression of the transformed logarithmic curve
static float temperature_correction_face_calculate_coefficient_with_linear_regression(temperature_correction_rolling_buffer_t *buffer, int end_temp_cnt, int ignore_start_cnt, float ignore_delta_temp) {
    //determine end temperature
    float temp_end = 0;
    for (int i = buffer->length - end_temp_cnt; i < buffer->length; i++) {
        temp_end += buffer->data[i];
    }
    temp_end /= end_temp_cnt;

    //calculate values for linear regression, formula is: -m=(nΣxy-ΣxΣy)/(mΣx²-(Σx)²)
    float sum_x = 0;
    float sum_y = 0;
    float sum_xx = 0;
    float sum_xy = 0;
    int n = 0; 
    for (int i = ignore_start_cnt; i < buffer->length; i++) {
        if (fabs(buffer->data[i] - temp_end) <= ignore_delta_temp) {
            break; //temperature is near end temperature, becoming unstable
        } else {
            float x = (i - ignore_start_cnt) * 60.0; //x = delta time in seconds
            float y = logf(buffer->data[i] - temp_end); //y = ln(T - Tend)
            sum_x += x;
            sum_y += y;
            sum_xx += x * x;
            sum xy += x * y;
            n++;
        }
    }
    if (n * sum_xx - sum_x * sum_x == 0) {
        return 0;
    } else {
        return (float)(-((n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x)));
    }
}

/// @brief calculate end temperature using Newton's law of cooling
/// @param state face state containing temperature history and coefficient
/// @return corrected end temperature based on buffered data
static float temperature_correction_face_calculate_end_temperature(temperature_correction_state_t *state) {
    float temperature_current = state->buffer.data[state->buffer.head_index];
    int start_index = state->buffer.length < state->buffer.max || state->buffer.head_index + 1 == state->buffer.max ? 0 : state->buffer.head_index + 1;
    float temperature_start = state->buffer.data[start_index];
    float ex =  expf(-state->coefficient * (float)(state->buffer.length - 1)));
    return (temperature_current - temperature_start * ex) / (1 - ex);
}

/// @brief display a temperature
/// @param temperature_c temperature in Celsius
/// @param in_fahrenheit true to display in Fahrenheit, false to display Celsius
static void temperature_correction_face_show_temperature(float temperature_c, bool in_fahrenheit) {
    if (in_fahrenheit) watch_display_float_with_best_effort(temperature_c * 1.8 + 32.0, "#F");
    else watch_display_float_with_best_effort(temperature_c, "#C");
}

/// @brief calculate average value of the last n values in a rolling buffer
/// @param buffer rolling buffer containing the values
/// @return average value
static float temperature_correction_face_calculate_average(temperature_correction_rolling_buffer_t *buffer) {
    float sum = 0;
    for (int i = 0; i < buffer->length; i++)
        sum += buffer->data[i];
    return sum / buffer->length;
}

/// @brief check if max-min of the last n values of buffer is <= TEMPERATURE_CORRECTION_CALCULATION_TRESHOLD, with n = TEMPERATURE_CORRECTION_CALCULATION_EQUILIBRIUM_MINUTES
static bool temperature_correction_face_equilibrium_reached(temperature_correction_rolling_buffer_t *buffer) {
    //we know that buffer->length < buffer->max, and also that the buffer has at least TEMPERATURE_CORRECTION_CALCULATION_EQUILIBRIUM_MINUTES values
    float min = buffer->data[buffer->length - TEMPERATURE_CORRECTION_CALCULATION_EQUILIBRIUM_MINUTES];
    float max = buffer->data[buffer->length - TEMPERATURE_CORRECTION_CALCULATION_EQUILIBRIUM_MINUTES];
    for (int i = buffer->length - TEMPERATURE_CORRECTION_CALCULATION_EQUILIBRIUM_MINUTES + 1; i < buffer->length; i++) {
        if (buffer->data[i] < min) min = buffer->data[i]; // Update minimum
        if (buffer->data[i] > max) max = buffer->data[i]; // Update maximum
    }
    return max - min <= TEMPERATURE_CORRECTION_CALCULATION_TRESHOLD;
} 

/// @brief calculate corrected temperature and display it
/// @param state face state used to compute and display the temperature
/// @param display true if temperature shall be displayed
static void temperature_correction_face_calculate_temperature_and_display(temperature_correction_state_t *state, bool display) {
    if (state->buffer.length > 1) {
        float end_temperature = temperature_correction_face_calculate_end_temperature(state);
        temperature_correction_face_add_to_rolling_buffer(&state->calculated_temperatures, end_temperature);
        float average_temperature = temperature_correction_face_calculate_average(&state->calculated_temperatures);
        if(display) temperature_correction_face_show_temperature(average_temperature, movement_use_imperial_units());
    }
}

/// @brief reset a rolling buffer
/// @param buffer rolling buffer to initialize
/// @param usable_length maximum number of entries the buffer can hold
static void temperature_correction_face_init_rolling_buffer(temperature_correction_rolling_buffer_t *buffer, int usable_length) {
    buffer->head_index = -1;
    buffer->length = 0;
    buffer->max = usable_length;
}

/// @brief stop logging of temperatures
static void temperature_correction_face_stop_logging(temperature_correction_state_t *state) {
    watch_clear_indicator(WATCH_INDICATOR_SIGNAL); 
    watch_clear_indicator(WATCH_INDICATOR_BELL); 
    watch_clear_indicator(WATCH_INDICATOR_LAP);
    state->bell_shown = false;
    state->mode = temperature_correction_waiting;
}

/// @brief show the coefficient at the bottom line
static void temperature_correction_face_display_coefficient(float coeff_f) {
    int coeff = (int)(coeff_f * 100000 + 0.5); // 0.0013240584 -> 000132
    sprintf(buf, "%06d", coeff); 
    watch_display_text(WATCH_POSITION_BOTTOM, buf);
}

static void temperature_correction_face_display_coefficient(temperature_correction_state_t *state) {
    char buf[8];
    watch_display_text_with_fallback(WATCH_POSITION_BOTTOM, "      ", "      ");

    switch (state->show_state) {
        case 0:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "STE", "ST");
            temperature_correction_face_show_temperature(state->temperature_start, movement_use_imperial_units());
            break;
        case 1:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "ETE", "ET");
            temperature_correction_face_show_temperature(state->temperature_end, movement_use_imperial_units());
            break;
        case 2:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "DEL", "DE");
            sprintf(buf, "%06d", state->delta); 
            watch_display_text(WATCH_POSITION_BOTTOM, buf);
            break;
        default:
            watch_display_text_with_fallback(WATCH_POSITION_TOP, "TCO", "TC");
            break;
    }
}

/// @brief display current settings on the watch face
/// @param state face state containing settings values
/// @param subsecond current subsecond value used for blink timing
static void temperature_correction_face_display_settings(temperature_correction_state_t *state, uint8_t subsecond) {
    char buf[8];
    watch_display_text_with_fallback(WATCH_POSITION_BOTTOM, "      ", "      ");

    switch (state->settings_state) {
        case 0:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "BUF", "BU");
            sprintf(buf, "%2d", state->buffer_size);
            if (subsecond % 2) watch_display_text(WATCH_POSITION_MINUTES, buf);
            else watch_display_text(WATCH_POSITION_MINUTES, "  ");
            break;
        case 1:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "AVG", "AV");
            sprintf(buf, "%2d", state->average_count);
            if (subsecond % 2) watch_display_text(WATCH_POSITION_MINUTES, buf);
            else watch_display_text(WATCH_POSITION_MINUTES, "  ");
            break;
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "COE", "CO");
            temperature_correction_face_display_coefficient(state->coefficient);
            if (subsecond % 2) 
                watch_display_string(" ", state->settings_state + 2);
            break;
        default:
            watch_display_text_with_fallback(WATCH_POSITION_TOP, "TCO", "TC");
            break;
    }
}

/// @brief advance the active setting value up or down
/// @param state face state containing the selected setting
/// @param forward true to increment, false to decrement
static void temperature_correction_face_advance_settings(temperature_correction_state_t *state, bool forward) {
    int coeff;
    float decim;
    int digit;

    switch (state->settings_state) {
        case 0:
            if (forward) state->buffer_size = state->buffer_size + 1 > TEMPERATURE_CORRECTION_BUFFER_SIZE_MAX ? 0 : state->buffer_size + 1;
            else state->buffer_size = state->buffer_size - 1 < 2 ? TEMPERATURE_CORRECTION_BUFFER_SIZE_MAX : state->buffer_size - 1;
            break;
        case 1:
            if (forward) state->average_count = state->average_count + 1 > TEMPERATURE_CORRECTION_AVERAGING_MAX ? 0 : state->average_count + 1;
            else state->average_count = state->average_count - 1 < 1 ? TEMPERATURE_CORRECTION_AVERAGING_MAX : state->average_count - 1;
            break;
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            coeff = (int)(state->coefficient * 100000 + 0.5); // 0.0013240584 -> 000132      
            decim = (float)pow(10, abs((int)state->settings_state - 7) + 1); // for 6: 100
            digit = (((float)coeff / decim) - ((int)((float)coeff / decim))) * 10;
            if(forward && digit < 9) coeff = coeff + pow(10, abs((int)state->settings_state - 7));
            else if(!forward && digit > 0) coeff = coeff - pow(10, abs((int)state->settings_state - 7));
            state->coefficient = ((float)coeff) / 100000;
            if(state->coefficient > 9) state->coefficient = TEMPERATURE_CORRECTION_DEFAULT_COEFFICIENT;
        default:
            break;
    }
}

void temperature_correction_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;

    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(temperature_correction_state_t));
        memset(*context_ptr, 0, sizeof(temperature_correction_state_t));

        temperature_correction_state_t *state = (temperature_correction_state_t *)*context_ptr;       
        state->buffer.data = malloc(TEMPERATURE_CORRECTION_BUFFER_SIZE_MAX * sizeof(float));
        state->calculated_temperatures.data = malloc(TEMPERATURE_CORRECTION_AVERAGING_MAX * sizeof(float));
        
        state->coefficient = TEMPERATURE_CORRECTION_DEFAULT_COEFFICIENT;
        state->buffer_size = TEMPERATURE_CORRECTION_DEFAULT_BUFFER_SIZE;
        state->average_count = TEMPERATURE_CORRECTION_DEFAULT_AVERAGE_COUNT;
    }
}

void temperature_correction_face_activate(void *context) {
    temperature_correction_state_t *state = (temperature_correction_state_t *)context;
    movement_request_tick_frequency(4); // we need to manually blink some pixels
}

bool temperature_correction_face_loop(movement_event_t event, void *context) {
    temperature_correction_state_t *state = (temperature_correction_state_t *)context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
            watch_display_text_with_fallback(WATCH_POSITION_TOP, "TCO", "TC");
            if(state->mode == temperature_correction_setting) { 
                state->mode = temperature_correction_waiting;
            }
            break;
        case EVENT_LIGHT_BUTTON_DOWN:
            break; //no light
        case EVENT_LIGHT_LONG_PRESS:
            switch (state->mode) {
                case temperature_correction_waiting:   
                    state->mode = temperature_correction_show_coefficient;
                    state->show_state = 0;
                    temperature_correction_face_display_coefficient(state);
                    break;  
                case temperature_correction_coefficient:     
                case temperature_correction_show_coefficient: 
                case temperature_correction_running:
                case temperature_correction_setting:
                    break;
            }
            break;
        case EVENT_LIGHT_BUTTON_UP:     
            switch (state->mode) {
                case temperature_correction_waiting: // enter settings
                    state->mode = temperature_correction_setting;
                    state->settings_state = 0;
                    temperature_correction_face_display_settings(state, event.subsecond);
                    break;      
                case temperature_correction_coefficient: //fallthrough
                case temperature_correction_running: //show real temp for a few seconds
                    temperature_correction_face_show_temperature(movement_get_temperature(), movement_use_imperial_units());
                    watch_set_indicator(WATCH_INDICATOR_LAP);
                    state->tick_show_real_temperature = 2;
                    break;
                case temperature_correction_setting: // flip through settings
                    state->settings_state++;
                    temperature_correction_face_display_settings(state, event.subsecond);
                    if (state->settings_state > 7) state->mode = temperature_correction_waiting;
                    break;
                case temperature_correction_show_coefficient:
                    state->show_state++;
                    temperature_correction_face_display_coefficient(state);
                    if (state->show_state > 2) state->mode = temperature_correction_waiting;
                    break;
            }
            break;
        case EVENT_TICK:
            switch (state->mode) {
                case temperature_correction_show_coefficient: //fallthrough
                case temperature_correction_waiting:
                    break;
                case temperature_correction_running: 
                    if (watch_rtc_get_date_time().unit.second != state->last_second) { 
                        state->last_second = watch_rtc_get_date_time().unit.second;                          
                        state->bell_shown = !state->bell_shown;
                        if(state->bell_shown) watch_set_indicator(WATCH_INDICATOR_BELL);
                        else watch_clear_indicator(WATCH_INDICATOR_BELL);               
                        temperature_correction_face_add_to_rolling_buffer(&state->buffer, movement_get_temperature(), true);
                        temperature_correction_face_calculate_temperature_and_display(state, state->tick_show_real_temperature == 0);
                        if (state->tick_show_real_temperature == 0) watch_clear_indicator(WATCH_INDICATOR_LAP);
                        else state->tick_show_real_temperature--;
                    }
                    break;
                case temperature_correction_setting: 
                    temperature_correction_face_display_settings(state, event.subsecond);
                    break;
                case temperature_correction_coefficient:
                     if (watch_rtc_get_date_time().unit.second != state->last_second) { 
                        state->last_second = watch_rtc_get_date_time().unit.second;                          
                        state->bell_shown = !state->bell_shown;
                        if(state->bell_shown) watch_set_indicator(WATCH_INDICATOR_BELL);
                        else watch_clear_indicator(WATCH_INDICATOR_BELL);              
                        
                        //TODO: show something here, maybe precalculate coeff

                        if (state->last_second == 42) {//once a minute (TODO: this is ugly)
                            temperature_correction_face_add_to_rolling_buffer(&state->buffer, movement_get_temperature(), true);
                            if (state->buffer->length == state->buffer->max) { //buffer full
                                temperature_correction_face_stop_logging(state);
                                watch_display_text_with_fallback(WATCH_POSITION_BOTTOM, "FULL  ", " FULL ");
                                break;
                            } else if (state->buffer.length >= TEMPERATURE_CORRECTION_CALCULATION_EQUILIBRIUM_MINUTES) { //only after n minutes
                                if (temperature_correction_face_equilibrium_reached(&state->buffer)) { //temperature is stable: stop calculation
                                    state->coefficient = temperature_correction_face_calculate_coefficient_with_linear_regression(&state->buffer, TEMPERATURE_CORRECTION_CALCULATION_EQUILIBRIUM_MINUTES, TEMPERATURE_CORRECTION_CALCULATION_IGNORE_START_MINUTES, TEMPERATURE_CORRECTION_CALCULATION_END_TEMP_DELTA);
                                    //TODO: coefficient shall only have 5 decimal places, otherwise we have a different coeff than what we show and adjust
                                    temperature_correction_face_stop_logging(state);
                                    temperature_correction_face_display_coefficient(state->coefficient);
                                    break;
                                }
                            }
                        }

                        if (state->tick_show_real_temperature == 0) watch_clear_indicator(WATCH_INDICATOR_LAP);
                        else state->tick_show_real_temperature--;
                    }
                    break;
            }
            break;
        case EVENT_ALARM_BUTTON_UP: 
            switch (state->mode) {
                case temperature_correction_waiting: // start logging
                    state->last_second = watch_rtc_get_date_time().unit.second; // start logging at next second   
                    watch_set_indicator(WATCH_INDICATOR_SIGNAL);
                    temperature_correction_face_init_rolling_buffer(&state->buffer, state->buffer_size);
                    temperature_correction_face_init_rolling_buffer(&state->calculated_temperatures, state->average_count);
                    tick_show_real_temperature = 0;
                    state->mode = temperature_correction_running;
                    break;
                case temperature_correction_coefficient: //falltrough
                case temperature_correction_running: // stop logging
                    temperature_correction_face_stop_logging(state);
                    break;
                case temperature_correction_setting:
                    temperature_correction_face_advance_settings(state, true);
                    temperature_correction_face_display_settings(state, watch_rtc_get_date_time().unit.second);
                    break;
                case temperature_correction_show_coefficient:
                    break;
            }
            break;
        case EVENT_ALARM_LONG_PRESS:
            switch (state->mode) {
                case temperature_correction_waiting: // start coefficient calculation
                    state->last_second = watch_rtc_get_date_time().unit.second; // start logging at next second   
                    watch_set_indicator(WATCH_INDICATOR_SIGNAL);
                    temperature_correction_face_init_rolling_buffer(&state->buffer, TEMPERATURE_CORRECTION_BUFFER_SIZE_MAX);          
                    tick_show_real_temperature = 0;
                    state->mode = temperature_correction_coefficient;
                    break;
                case temperature_correction_coefficient: //fallthrough
                case temperature_correction_running: // stop logging
                    temperature_correction_face_stop_logging(state);
                    break;
                case temperature_correction_setting:
                    temperature_correction_face_advance_settings(state, false);
                    temperature_correction_face_display_settings(state, watch_rtc_get_date_time().unit.second);
                    break;
                case temperature_correction_show_coefficient: 
                    break;
            }
            break;
        case EVENT_TIMEOUT:
            //no timeout
            break;
        default:
            return movement_default_loop_handler(event);
    }

    return true;
}

void temperature_correction_face_resign(void *context) {
    (void) context;

    // handle any cleanup before your watch face goes off-screen.
}

