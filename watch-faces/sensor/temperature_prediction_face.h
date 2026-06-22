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

#pragma once

#include "movement.h"

/*
 * A DESCRIPTION OF YOUR WATCH FACE
 *
 * and a description of how use it
 *
 */

#define TEMPERATURE_PREDICTION_BUFFER_SIZE_MAX 90
#define TEMPERATURE_PREDICTION_AVERAGING_MAX 60
#define TEMPERATURE_PREDICTION_CALCULATION_EQUILIBRIUM_MINUTES 5 //defines how long the temperature shall be constant to determine that equilibrium has been reached.
#define TEMPERATURE_PREDICTION_CALCULATION_EQUILIBRIUM_TRESHOLD 0.1 //defines the maximum allowed temperature deviation to determine that equilibrium has been reached.
#define TEMPERATURE_PREDICTION_CALCULATION_IGNORE_START_MINUTES 5 
#define TEMPERATURE_PREDICTION_CALCULATION_END_TEMP_DELTA 2.0

typedef enum {
    temperature_prediction_waiting,
    temperature_prediction_running,
    temperature_prediction_setting,
    temperature_prediction_coefficient,
    temperature_prediction_show_coefficient,
    temperature_prediction_show_buffer
} temperature_prediction_mode_t;

typedef enum {
    temperature_prediction_algorithm_fixed, //algorithm 1: fixed delta between Tcurrent and Tstart, SMA over the last n calculated end temperatures
    temperature_prediction_algorithm_block, //algorithm 2: Tstart is the middle temperature of the last block, Tcurrent is the middle tempeprature of the current block
} temperature_prediction_algorithm_t;

typedef struct {
    float *data;             // the data points
    int head_index;          // index of the most recent entry (-1 when empty)
    int length;              // current number of valid entries in the buffer
    int max;                 // maximum capacity of the buffer
} temperature_prediction_rolling_buffer_t;

typedef struct {
    // buffers
    temperature_prediction_rolling_buffer_t buffer;                  // rolling buffer holding recent raw temperature samples
    temperature_prediction_rolling_buffer_t calculated_temperatures; // rolling buffer holding corrected temperatures
    
    // settings
    temperature_prediction_algorithm_t algorithm_type;               // selected correction algorithm
    float coefficient;                                               // heat transfer coefficient used for correction


    //settings for correction algorithm 1: fixed delta between Tcurrent and Tstart, SMA over the last n calculated end temperatures
    int buffer_size;                                                 // delta between Tcurrent and Tstart
    int average_count;                                               // number of calculated end temperatures to average

    //settings for correction algorithm 2: Tstart is the middle temperature of the last block, Tcurrent is the middle tempeprature of the current block
    int block_gap;                                                 // how many blocks between Tstart and Tcurrent (minimum 1)

    //for temp logging and coefficient calculation
    uint32_t last_second;                                             // last RTC second used for timed sampling

    bool bell_shown;                                                  // whether the bell indicator is currently shown
    temperature_prediction_mode_t mode;                               // current mode (waiting, running, setting, coefficient calculation, ...)
    uint8_t settings_state;                                           // selected sub-setting index when in settings mode
    uint8_t show_state;                                               // state index for showing the coefficient data after calculation
    uint8_t show_buffer_state;                                        // state index for showing the buffer data after calculation
    uint8_t tick_show_real_temperature;                               // if > 0: show the real temperature for the next few ticks
} temperature_prediction_state_t;

void temperature_prediction_face_setup(uint8_t watch_face_index, void ** context_ptr);
void temperature_prediction_face_activate(void *context);
bool temperature_prediction_face_loop(movement_event_t event, void *context);
void temperature_prediction_face_resign(void *context);

#define temperature_prediction_face ((const watch_face_t){ \
    temperature_prediction_face_setup, \
    temperature_prediction_face_activate, \
    temperature_prediction_face_loop, \
    temperature_prediction_face_resign, \
    NULL, \
})
