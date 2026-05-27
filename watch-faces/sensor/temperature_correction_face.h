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

#define TEMPERATURE_CORRECTION_BUFFER_SIZE_MAX 60
#define TEMPERATURE_CORRECTION_AVERAGING_MAX 60

typedef enum {
    temperature_correction_waiting,
    temperature_correction_running,
    temperature_correction_setting,
} temperature_correction_mode_t;

typedef struct {
    int head_index;
    int length;
    int max;
} temperature_correction_rolling_buffer_t;

typedef struct {
    temperature_correction_rolling_buffer_t buffer;
    temperature_correction_rolling_buffer_t calculated_temperatures;
    float coefficient;
    int buffer_size;
    int average_count;
    bool bell_shown;
    uint32_t last_second;
    temperature_correction_mode_t mode;
    uint8_t settings_state;
} temperature_correction_state_t;

void temperature_correction_face_setup(uint8_t watch_face_index, void ** context_ptr);
void temperature_correction_face_activate(void *context);
bool temperature_correction_face_loop(movement_event_t event, void *context);
void temperature_correction_face_resign(void *context);

#define temperature_correction_face ((const watch_face_t){ \
    temperature_correction_face_setup, \
    temperature_correction_face_activate, \
    temperature_correction_face_loop, \
    temperature_correction_face_resign, \
    NULL, \
})
