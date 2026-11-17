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
 * BLINK RECEIVER
 *
 * Receives 32-bit packets encoded as timed light pulses.
 *
 * Multi-packet transmissions allow multiple commands to be sent together.
 * Commands marked immediate are applied as they arrive; other commands are
 * applied when the transmission is complete. 
 * 
 * PACKET FORMAT
 *
 *   6 bit Header:
 *   bit 31       0 = single packet, 1 = multipacket (means another packet follows)
 *   bit 30       1 = immediate: apply this command immediately, even if another packet follows
 *   bits 29..26  command type
 * 
 *   18 bit Payload:
 *   bits 25..8   command-specific payload
 * 
 *   8 bit Checksum:
 *   bits 7..0    checksum
 *
 * COMMANDS
 *
 *   TIMESET (1)      Set the watch's local time.
 *   DATESET (2)      Set the watch's local date.
 *   HEARTBEAT (3)    Synchronize the watch clock to the received signal.
 *   LATITUDE (4)     Store the latitude in the watch's location record.
 *   LONGITUDE (5)    Store the longitude in the watch's location record.
 *   SET_ZONE (6)     Select and persist a Movement timezone.
 *   TEST (7)         Confirm packet reception with a test command.
 *   PATCH (8)        Adjust the watch time by a signed offset
 *
 * OPERATION
 *
 *   Idle             The bottom row displays the current light-sensor reading.
 *   Listening        The sensor is sampled at the configured read frequency;
 *                    samples_per_bit readings are grouped to decode each bit.
 *   Settings         Adjust reception and display parameters.
 *   History          Inspect the recorded samples from packet reception.
 *
 * BUTTONS
 *
 *   ALARM press      Start listening, or cancel an active reception.
 *   LIGHT press      Open settings. 
 *   ALARM long press Open or close sample history
 *
 * SETTINGS
 *
 *   FC  Frequency for sampling.
 *   SA  Samples per bit. 
 *   LL  Light threshold. 
 *   DF  Frequency when showing the current light level.
 *
 */

#define DEFAULT_LIGHT_LEVEL_BORDER 65200
#define MIN_LIGHT_LEVEL_BORDER 65100
#define MAX_LIGHT_LEVEL_BORDER 65500

#define DEFAULT_SAMPLES_PER_BIT 8
#define MAX_SAMPLES_PER_BIT 32

#define DEFAULT_IDLE_FREQUENCY 8
#define DEFAULT_READ_FREQUENCY 64
#define MAX_FREQUENCY 128

#define BITS_PER_PACKAGE 32
#define MAX_PACKAGE_COUNT 8

#define BLINK_RECEIVER_HEADER_TIMESET 1
#define BLINK_RECEIVER_HEADER_DATESET 2
#define BLINK_RECEIVER_HEADER_HEARTBEAT 3
#define BLINK_RECEIVER_HEADER_LATITUDE 4
#define BLINK_RECEIVER_HEADER_LONGITUDE 5
#define BLINK_RECEIVER_HEADER_SET_ZONE 6
#define BLINK_RECEIVER_HEADER_TEST 7
#define BLINK_RECEIVER_HEADER_PATCH 8

typedef enum {
  BLINK_RECEIVER_MODE_IDLE,
  BLINK_RECEIVER_MODE_DISPLAY_CURRENT_LIGHT,
  BLINK_RECEIVER_MODE_WAIT_FOR_RISING_EDGE,
  BLINK_RECEIVER_MODE_RECORD,
  BLINK_RECEIVER_MODE_SETTING,
  BLINK_RECEIVER_MODE_HISTORY
} blink_receiver_mode;

typedef union {
    struct {
        uint32_t checksum : 8;
        uint32_t unused : 1;
        uint32_t hour : 5;
        uint32_t minute : 6;
        uint32_t second : 6;
        uint32_t header : 6;
    } time;
    uint32_t reg;
} blink_receiver_packet_time_t;

typedef union {
    struct {
        uint32_t checksum : 8;
        uint32_t unused : 2;
        uint32_t value : 16;
        uint32_t header : 6;
    } location;
    uint32_t reg;
} blink_receiver_packet_location_t;

typedef union {
    struct {
        uint32_t checksum : 8;
        uint32_t unused : 12;
        uint32_t index : 6;
        uint32_t header : 6;
    } zone;
    uint32_t reg;
} blink_receiver_packet_zone_t;

typedef union {
    struct {
        uint32_t checksum : 8;
        uint32_t unused : 3;
        uint32_t year : 6;
        uint32_t month : 4;
        uint32_t day : 5;
        uint32_t header : 6;
    } date;
    uint32_t reg;
} blink_receiver_packet_date_t;

typedef union {
    struct {
        uint32_t checksum : 8;
        uint32_t unused : 8;
        uint32_t offset : 8;
        uint32_t is_plus : 1;
        uint32_t is_smaller : 1;
        uint32_t header : 6;
    } patch;
    uint32_t reg;
} blink_receiver_packet_patch_t;

typedef union {
    struct {
        uint32_t checksum : 8;
        uint32_t byte_3 : 8;
        uint32_t byte_2 : 8;
        uint32_t byte_1 : 8;
    } bytes;
    struct {
        uint32_t _ : 26;
        uint32_t type : 4;
        uint32_t immediate: 1;
        uint32_t multipacket : 1;
    } header;
    uint32_t reg;
} blink_receiver_packet_data_t;

typedef struct {
    blink_receiver_mode mode; 

    uint8_t settings_state;
    uint16_t light_level_border;
    uint8_t read_frequency;
    uint8_t idle_frequency;
    uint8_t samples_per_bit;

    bool *samples;
    uint32_t *packets;
    uint8_t samples_received; 
    int16_t bits_received;  
    int8_t current_package;

    uint16_t *sample_history;
    uint8_t history_position;

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
