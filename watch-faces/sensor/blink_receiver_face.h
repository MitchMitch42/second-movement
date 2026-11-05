/*
 * MIT License
 *
 * Copyright (c) 2026 <#author_name#>
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

#define DEFAULT_LIGHT_LEVEL_BORDER 65200
#define MIN_LIGHT_LEVEL_BORDER 65100
#define MAX_LIGHT_LEVEL_BORDER 65500
#define LIGHT_LEVEL_BORDER_STEP 10

#define DEFAULT_SAMPLES_PER_BIT 8
#define MAX_SAMPLES_PER_BIT 16

#define DEFAULT_FREQUENCY 64
#define MAX_FREQUENCY 64

#define BITS_PER_PACKAGE 32
#define MAX_PACKAGE_COUNT 4

#define BLINK_RECEIVER_HEADER_TIMESET 1
#define BLINK_RECEIVER_HEADER_DATESET 2
#define BLINK_RECEIVER_HEADER_TIMESET_EXACT 3
#define BLINK_RECEIVER_HEADER_MULTIPLE_PACKETS 4
#define BLINK_RECEIVER_HEADER_DATETIME 5
#define BLINK_RECEIVER_HEADER_DATETIME_EXACT 6

typedef enum {
  BLINK_RECEIVER_MODE_IDLE,
  BLINK_RECEIVER_MODE_DISPLAY_CURRENT_LIGHT,
  BLINK_RECEIVER_MODE_WAIT_FOR_RISING_EDGE,
  BLINK_RECEIVER_MODE_RECORD,
  BLINK_RECEIVER_MODE_CALC,
  BLINK_RECEIVER_MODE_SETTING
} blink_receiver_mode;

typedef union {
    struct {
        uint32_t checksum : 8;
        uint32_t unused : 3;
        uint32_t hour : 5;
        uint32_t minute : 6;
        uint32_t second : 6;
        uint32_t header : 4;
    } time;
    uint32_t reg;
} blink_receiver_packet_time_t;

typedef union {
    struct {
        uint32_t checksum : 8;
        uint32_t unused : 5;
        uint32_t year : 6;
        uint32_t month : 4;
        uint32_t day : 5;
        uint32_t header : 4;
    } date;
    uint32_t reg;
} blink_receiver_packet_date_t;

typedef union {
    struct {
        uint32_t checksum : 8;
        uint32_t byte_3 : 8;
        uint32_t byte_2 : 8;
        uint32_t byte_1 : 8; //4 bits header + 4 bits data
    } bytes;
    uint32_t reg;
} blink_receiver_packet_data_t;

typedef struct {
    blink_receiver_mode mode; 

    uint16_t light_level_border;
    uint8_t frequency;
    uint8_t samples_per_bit;

    bool *samples;
    uint16_t *sample_history;
    uint32_t *data;
    uint8_t samples_received; 
    int16_t bits_received;  
    uint8_t current_package;
    uint8_t settings_state;

    uint8_t tick_cnt;
    uint32_t last_second;
} blink_receiver_state_t;

void blink_receiver_face_setup(uint8_t watch_face_index, void ** context_ptr);
void blink_receiver_face_activate(void *context);
bool blink_receiver_face_loop(movement_event_t event, void *context);
void blink_receiver_face_resign(void *context);

#define blink_receiver_face ((const watch_face_t){ \
    blink_receiver_face_setup, \
    blink_receiver_face_activate, \
    blink_receiver_face_loop, \
    blink_receiver_face_resign, \
    NULL, \
})
