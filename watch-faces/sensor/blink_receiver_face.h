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
 * Receives data encoded as timed light pulses through the watch's light
 * sensor. Bright and dark pulses represent binary values, allowing a
 * computer or phone display to transmit commands to the watch. 
 * This can be used to set the time, date, and location, 
 * as well as to synchronize the watch clock to a received signal.
 *
 * TRANSMISSION
 *
 *   1. Press ALARM to listen. The receiver requests read_frequency sensor
 *      readings per second and waits for a bright start bit. Dark readings
 *      are ignored while it waits.
 * 
 *   2. After detecting the start bit, the receiver collects 32 data bits,
 *      most-significant bit first. For each bit it takes samples_per_bit
 *      sensor readings, making the readout more stable. 
 *      The transmission frequency of the sender is: 
 *      Transmission frequency = read_frequency / samples_per_bit
 * 
 *   3. After 32 bits, the checksum is verified. 
 *      If the immediate bit is set, the command is executed immediately. 
 *      If the multipacket bit is set in the command, the watch stays in receiving mode and
 *      waits for the next packet, which must again be indicated by a bright start bit. 
 *      When a packet without the multipacket bit is received, the buffered packets, including this final
 *      packet, are executed in packet order. Processing stops on the first
 *      command error. The receiver plays a success tone if all commands pass,
 *      or a failure tone if checksum or command processing fails. 
 *      It then enters idle mode.
 *
 * PACKET FORMAT
 *
 *   Header:
 *   bit 31       multipacket: The multipacket bit indicates that another packet follows. 
 *   bit 30       immediate: request execution as soon as the packet arrives
 *   bits 29..26  4-bit command type
 * 
 *   bits 25..8   18-bit payload
 * 
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
 *   TEST (7)         Dummy command for testing the checksum and command processing.
 *   PATCH (8)        Apply a signed time offset.
 *
 * Payload layouts are documented by the packet unions below. Related commands
 * can be sent in one multi-packet transmission; multipacket indicates whether
 * another packet follows, and immediate requests that a command shall be applied
 * before the transmission is complete.
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
 * ERROR CODES
 *
 *   MH  The packet contains an unsupported command type.
 *   MM  The packet limit was reached while another packet was indicated.
 *   MZ  SET_ZONE contained an invalid timezone index.
 *
 */

// Sensor threshold range.
#define DEFAULT_LIGHT_LEVEL_BORDER 65200
#define MIN_LIGHT_LEVEL_BORDER 65100
#define MAX_LIGHT_LEVEL_BORDER 65500

// Per-bit sensor sampling.
#define DEFAULT_SAMPLES_PER_BIT 8
#define MAX_SAMPLES_PER_BIT 32

// Watch tick-rate limits.
#define DEFAULT_IDLE_FREQUENCY 8
#define DEFAULT_READ_FREQUENCY 64
#define MAX_FREQUENCY 128

// Transmission capacity.
#define BITS_PER_PACKAGE 32
#define MAX_PACKAGE_COUNT 8

// Packet command identifiers.
#define BLINK_RECEIVER_HEADER_TIMESET 1
#define BLINK_RECEIVER_HEADER_DATESET 2
#define BLINK_RECEIVER_HEADER_HEARTBEAT 3
#define BLINK_RECEIVER_HEADER_LATITUDE 4
#define BLINK_RECEIVER_HEADER_LONGITUDE 5
#define BLINK_RECEIVER_HEADER_SET_ZONE 6
#define BLINK_RECEIVER_HEADER_TEST 7
#define BLINK_RECEIVER_HEADER_PATCH 8

// Receiver operating modes.
typedef enum {
  BLINK_RECEIVER_MODE_IDLE,
  BLINK_RECEIVER_MODE_DISPLAY_CURRENT_LIGHT,
  BLINK_RECEIVER_MODE_WAIT_FOR_RISING_EDGE,
  BLINK_RECEIVER_MODE_RECORD,
  BLINK_RECEIVER_MODE_SETTING,
  BLINK_RECEIVER_MODE_HISTORY
} blink_receiver_mode;

// Shared packet layouts.
typedef union {
    struct {
        uint32_t checksum : 8;
        uint32_t byte_3 : 8;
        uint32_t byte_2 : 8;
        uint32_t byte_1 : 8;
    } bytes; // Raw bytes
    struct {
        uint32_t _ : 26; // Payload and checksum
        uint32_t type : 4;
        uint32_t immediate: 1;
        uint32_t multipacket : 1;
    } header; // Common flags
    struct {
        uint32_t checksum : 8;
        uint32_t unused : 1;
        uint32_t hour : 5;
        uint32_t minute : 6;
        uint32_t second : 6;
        uint32_t header : 6;
    } time; // Time command
    struct {
        uint32_t checksum : 8;
        uint32_t unused : 2;
        uint32_t value : 16;
        uint32_t header : 6;
    } location; // Coordinate command
    struct {
        uint32_t checksum : 8;
        uint32_t unused : 12;
        uint32_t index : 6;
        uint32_t header : 6;
    } zone; // Timezone command
    struct {
        uint32_t checksum : 8;
        uint32_t unused : 3;
        uint32_t year : 6;
        uint32_t month : 4;
        uint32_t day : 5;
        uint32_t header : 6;
    } date; // Date command
    struct {
        uint32_t checksum : 8;
        uint32_t unused : 8;
        uint32_t offset : 8;
        uint32_t is_plus : 1;
        uint32_t is_smaller : 1;
        uint32_t header : 6;
    } patch; // Time adjustment
    uint32_t reg;
} blink_receiver_packet_data_t;

// Runtime receiver state.
typedef struct {
    blink_receiver_mode mode; // Current mode

    uint8_t settings_state; // Selected setting
    uint16_t light_level_border; // Sensor threshold
    uint8_t read_frequency; // Sampling rate
    uint8_t idle_frequency; // Display rate
    uint8_t samples_per_bit; // Samples per bit

    bool *samples; // Current bit samples
    uint32_t *packets; // Buffered packets
    uint8_t samples_received; // Samples collected
    int16_t bits_received; // Bits collected
    int8_t current_package; // Active packet index

    uint16_t *sample_history; // Captured sample groups
    uint8_t history_position; // History cursor

} blink_receiver_state_t;

// Watch-face lifecycle callbacks.
void blink_receiver_face_setup(uint8_t watch_face_index, void ** context_ptr);
void blink_receiver_face_activate(void *context);
bool blink_receiver_face_loop(movement_event_t event, void *context);
void blink_receiver_face_resign(void *context);

// Movement face descriptor.
#define blink_receiver_face ((const watch_face_t){ \
    blink_receiver_face_setup, \
    blink_receiver_face_activate, \
    blink_receiver_face_loop, \
    blink_receiver_face_resign, \
    NULL, \
})
