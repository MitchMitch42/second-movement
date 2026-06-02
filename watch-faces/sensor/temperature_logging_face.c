/*
 * MIT License
 *
 * Copyright (c) 2022 Joey Castillo
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
#include "temperature_logging_face.h"
#include "watch.h"

static bool skip = false;

static void _temperature_logging_face_log_data(temperature_logging_state_t *logger_state) {
    if (logger_state->data_points >= TEMPERATURE_LOGGING_NUM_DATA_POINTS) 
        return;
    
    float temp = movement_get_temperature();

    if (logger_state->data_points > 0 && logger_state->data[logger_state->data_points - 1].temperature_c == temp) 
        return

    logger_state->data[logger_state->data_points].delta_seconds = logger_state->delta_seconds;
    logger_state->data[logger_state->data_points].temperature_c = temp;
    logger_state->data_points++;
}

static void _temperature_logging_face_update_display(temperature_logging_state_t *logger_state, bool in_fahrenheit, bool clock_mode_24h) {
    int8_t pos = (logger_state->data_points - 1 - logger_state->display_index) % TEMPERATURE_LOGGING_NUM_DATA_POINTS;
    char buf[7];

    watch_clear_indicator(WATCH_INDICATOR_24H);
    watch_clear_indicator(WATCH_INDICATOR_PM);
    watch_clear_colon();

    if (pos < 0) {
        // no data at this index
        watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "LOG", "TL");
        watch_display_text(WATCH_POSITION_BOTTOM, "no dat");
        sprintf(buf, "%2d", logger_state->display_index);
        watch_display_text(WATCH_POSITION_TOP_RIGHT, buf);
    } else {
        // we are displaying the temperature
        watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "LOG", "TL");
        sprintf(buf, "%2d", logger_state->data[pos].delta_seconds);
        watch_display_text(WATCH_POSITION_TOP_RIGHT, buf);
        if (in_fahrenheit) {
            watch_display_float_with_best_effort(logger_state->data[pos].temperature_c * 1.8 + 32.0, "#F");
        } else {
            watch_display_float_with_best_effort(logger_state->data[pos].temperature_c, "#C");
        }
    }
}

void temperature_logging_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;

    // if temperature is invalid, we don't have a temperature sensor which means we shouldn't be here.
    if (movement_get_temperature() == 0xFFFFFFFF) skip = true;

    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(temperature_logging_state_t));
        memset(*context_ptr, 0, sizeof(temperature_logging_state_t));
    }
}

void temperature_logging_face_activate(void *context) {
    temperature_logging_state_t *logger_state = (temperature_logging_state_t *)context;
    logger_state->display_index = 0;
}

bool temperature_logging_face_loop(movement_event_t event, void *context) {
    temperature_logging_state_t *logger_state = (temperature_logging_state_t *)context;
    switch (event.event_type) {
        case EVENT_TIMEOUT:
            //no timeout!
            break;
        case EVENT_LIGHT_LONG_PRESS:
            //TODO
            break;
        case EVENT_ALARM_LONG_PRESS: //start/stop logging
            if (!logger_state->is_logging) {
                //Start
                logger_state->is_logging = true;
                logger_state->last_second = 66;
                logger_state->delta_seconds = 0;
                logger_state->data_points = 0;
                watch_set_indicator(WATCH_INDICATOR_SIGNAL);
            } else {
                //Stop
                logger_state->is_logging = false;
                logger_state->bell_shown = false;
                watch_clear_indicator(WATCH_INDICATOR_BELL);
                watch_clear_indicator(WATCH_INDICATOR_SIGNAL);
            }
            break;
        case EVENT_LIGHT_BUTTON_UP: //show previous datapoint
            logger_state->display_index = (logger_state->display_index - 1) % TEMPERATURE_LOGGING_NUM_DATA_POINTS;
            _temperature_logging_face_update_display(logger_state, movement_use_imperial_units(), movement_clock_mode_24h());
            break;
        case EVENT_ALARM_BUTTON_UP: //show next datapoint
            logger_state->display_index = (logger_state->display_index + 1) % TEMPERATURE_LOGGING_NUM_DATA_POINTS;
            _temperature_logging_face_update_display(logger_state, movement_use_imperial_units(), movement_clock_mode_24h());
            break;
        case EVENT_ACTIVATE:
            _temperature_logging_face_update_display(logger_state, movement_use_imperial_units(), movement_clock_mode_24h());
            break;
        case EVENT_TICK:
            if (logger_state->is_logging && watch_rtc_get_date_time().unit.second != logger_state->last_second) { 
                logger_state->last_second = watch_rtc_get_date_time().unit.second;                          
                logger_state->bell_shown = !state->bell_shown;
                if(logger_state->bell_shown) watch_set_indicator(WATCH_INDICATOR_BELL);
                else watch_clear_indicator(WATCH_INDICATOR_BELL); 
                _temperature_logging_face_log_data(logger_state);
                _temperature_logging_face_update_display(logger_state);
                logger_state->delta_seconds++;
            }
            break;
        case EVENT_BACKGROUND_TASK:
            //no bg task
            break;
        default:
            movement_default_loop_handler(event);
            break;
    }

    return true;
}

void temperature_logging_face_resign(void *context) {
    (void) context;
}

movement_watch_face_advisory_t temperature_logging_face_advise(void *context) {
    (void) context;
    movement_watch_face_advisory_t retval = { 0 };
    retval.wants_background_task = false;
    return retval;
}
