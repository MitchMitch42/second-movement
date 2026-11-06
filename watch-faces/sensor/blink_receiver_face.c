/*
 * MIT License
 *
 * Copyright (c) 2026 Mitch
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
#include "blink_receiver_face.h"
#include "tc.h"
#include "eic.h"
#include "usb.h"
#include "adc.h"
#include "watch.h"

//#ifdef HAS_IR_SENSOR

#define BR_DEBUG 0

// Date 0x4358C05B (2026-11-06), time 0x598800E1 (00:08:38).
// The first packet requests a following packet; each bit is repeated eight times.
bool debug_sample_array[] = {
    1, 1, 1, 1, 1, 1, 1, 1, // 0: start bit (1)

    0, 0, 0, 0, 0, 0, 0, 0, // 1: 0 (0x4)
    1, 1, 1, 1, 1, 1, 1, 1, // 2: 1
    0, 0, 0, 0, 0, 0, 0, 0, // 3: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 4: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 5: 0 (0x3)
    0, 0, 0, 0, 0, 0, 0, 0, // 6: 0
    1, 1, 1, 1, 1, 1, 1, 1, // 7: 1
    1, 1, 1, 1, 1, 1, 1, 1, // 8: 1
    0, 0, 0, 0, 0, 0, 0, 0, // 9: 0 (0x5)
    1, 1, 1, 1, 1, 1, 1, 1, // 10: 1
    0, 0, 0, 0, 0, 0, 0, 0, // 11: 0
    1, 1, 1, 1, 1, 1, 1, 1, // 12: 1
    1, 1, 1, 1, 1, 1, 1, 1, // 13: 1 (0x8)
    0, 0, 0, 0, 0, 0, 0, 0, // 14: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 15: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 16: 0
    1, 1, 1, 1, 1, 1, 1, 1, // 17: 1 (0xC)
    1, 1, 1, 1, 1, 1, 1, 1, // 18: 1
    0, 0, 0, 0, 0, 0, 0, 0, // 19: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 20: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 21: 0 (0x0)
    0, 0, 0, 0, 0, 0, 0, 0, // 22: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 23: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 24: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 25: 0 (0x5)
    1, 1, 1, 1, 1, 1, 1, 1, // 26: 1
    0, 0, 0, 0, 0, 0, 0, 0, // 27: 0
    1, 1, 1, 1, 1, 1, 1, 1, // 28: 1
    1, 1, 1, 1, 1, 1, 1, 1, // 29: 1 (0xB)
    0, 0, 0, 0, 0, 0, 0, 0, // 30: 0
    1, 1, 1, 1, 1, 1, 1, 1, // 31: 1
    1, 1, 1, 1, 1, 1, 1, 1, // 32: 1

    0, 0, 0, 0, 0, 0, 0, 0, // 0: stop bit (0)
    1, 1, 1, 1, 1, 1, 1, 1, // 0: start bit (1)

    0, 0, 0, 0, 0, 0, 0, 0, // 1: 0 (0x5)
    1, 1, 1, 1, 1, 1, 1, 1, // 2: 1
    0, 0, 0, 0, 0, 0, 0, 0, // 3: 0
    1, 1, 1, 1, 1, 1, 1, 1, // 4: 1
    1, 1, 1, 1, 1, 1, 1, 1, // 5: 1 (0x9)
    0, 0, 0, 0, 0, 0, 0, 0, // 40: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 41: 0
    1, 1, 1, 1, 1, 1, 1, 1, // 42: 1
    1, 1, 1, 1, 1, 1, 1, 1, // 43: 1 (0x8)
    0, 0, 0, 0, 0, 0, 0, 0, // 44: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 45: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 46: 0
    1, 1, 1, 1, 1, 1, 1, 1, // 47: 1 (0x8)
    0, 0, 0, 0, 0, 0, 0, 0, // 48: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 49: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 50: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 49: 0 (0x0)
    0, 0, 0, 0, 0, 0, 0, 0, // 50: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 51: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 52: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 53: 0 (0x0)
    0, 0, 0, 0, 0, 0, 0, 0, // 54: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 55: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 56: 0
    1, 1, 1, 1, 1, 1, 1, 1, // 57: 1 (0xE)
    1, 1, 1, 1, 1, 1, 1, 1, // 58: 1
    1, 1, 1, 1, 1, 1, 1, 1, // 59: 1
    0, 0, 0, 0, 0, 0, 0, 0, // 60: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 61: 0 (0x1)
    0, 0, 0, 0, 0, 0, 0, 0, // 62: 0
    0, 0, 0, 0, 0, 0, 0, 0, // 63: 0
    1, 1, 1, 1, 1, 1, 1, 1  // 64: 1
};

size_t blink_receiver_debug_idx = 0;

void blink_receiver_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(blink_receiver_state_t));
        memset(*context_ptr, 0, sizeof(blink_receiver_state_t));
        blink_receiver_state_t *state = (blink_receiver_state_t *) *context_ptr;

        state->samples = malloc(MAX_SAMPLES_PER_BIT * sizeof(*state->samples));
        state->sample_history = malloc((BITS_PER_PACKAGE + 1) * sizeof(*state->sample_history));
        state->data = malloc(MAX_PACKAGE_COUNT * sizeof(uint32_t));
        state->light_level_border = DEFAULT_LIGHT_LEVEL_BORDER; 
        state->samples_per_bit = DEFAULT_SAMPLES_PER_BIT;
        state->frequency = DEFAULT_FREQUENCY;
    }
}

void blink_receiver_face_activate(void *context) {
    (void) context;
    blink_receiver_state_t *state = (blink_receiver_state_t *)context;
    movement_request_tick_frequency(state->frequency);
}

uint16_t blink_receiver_read_sensor(void) {
    HAL_GPIO_IR_ENABLE_out();
    HAL_GPIO_IR_ENABLE_clr();
    HAL_GPIO_IRSENSE_pmuxen(HAL_GPIO_PMUX_ADC);
    adc_init();
    adc_enable();
    uint16_t ret= adc_get_analog_value(HAL_GPIO_IRSENSE_pin());
    if (BR_DEBUG) {
        ret = debug_sample_array[blink_receiver_debug_idx++ % (sizeof(debug_sample_array) / sizeof(debug_sample_array[0]))]
            ? 0 : UINT16_MAX;
    }
    adc_disable();
    HAL_GPIO_IRSENSE_pmuxdis();
    HAL_GPIO_IRSENSE_off();
    HAL_GPIO_IR_ENABLE_off();
    return ret;
}

static void blink_receiver_display_settings(blink_receiver_state_t *state) {
    char buf[6];
    watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");

    switch (state->settings_state) {
        case 0:
            watch_display_text(WATCH_POSITION_TOP_LEFT, "FC");
            snprintf(buf, sizeof(buf), "%2u", state->frequency);
            watch_display_string(buf, 2);
            break;
        case 1:
            watch_display_text(WATCH_POSITION_TOP_LEFT, "SA");
            snprintf(buf, sizeof(buf), "%2u", state->samples_per_bit);
            watch_display_string(buf, 2);
            break;
        case 2:
            watch_display_text(WATCH_POSITION_TOP_LEFT, "LL");
            watch_display_text(WATCH_POSITION_BOTTOM, "      ");
            snprintf(buf, sizeof(buf), "%5u", state->light_level_border);
            watch_display_string(buf, 4);
            break;
        default:
            break;
    }
}

static void blink_receiver_advance_setting(blink_receiver_state_t *state) {
    switch (state->settings_state) {
        case 0:
            state->frequency *= 2;
            if (state->frequency > MAX_FREQUENCY) state->frequency = 1;
            state->frequency_changed = true;
            break;
        case 1:
            state->samples_per_bit *= 2;
            if (state->samples_per_bit > MAX_SAMPLES_PER_BIT) state->samples_per_bit = 1;
            break;
        case 2:
            state->light_level_border += LIGHT_LEVEL_BORDER_STEP;
            if (state->light_level_border > MAX_LIGHT_LEVEL_BORDER) state->light_level_border = MIN_LIGHT_LEVEL_BORDER;
            break;
        default:
            break;
    }
}

static void blink_receiver_exit_settings(blink_receiver_state_t *state) {
    if (state->frequency_changed) {
        movement_request_tick_frequency(state->frequency);
        state->frequency_changed = false;
    }
    state->mode = BLINK_RECEIVER_MODE_DISPLAY_CURRENT_LIGHT;
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "BLI", "BL");
}

static void blink_receiver_display_history(blink_receiver_state_t *state) {
    char buf[11] = "          ";
    uint8_t bit_index = state->samples_per_bit > 8
        ? state->history_position / 2 : state->history_position;
    uint8_t sample_start = state->samples_per_bit > 8 && state->history_position % 2 ? 8 : 0;
    uint8_t visible_samples = state->samples_per_bit > sample_start
        ? state->samples_per_bit - sample_start : 0;
    if (visible_samples > 8) visible_samples = 8;

    snprintf(buf + 2, 3, "%2u", (unsigned)bit_index);
    for (uint8_t i = 0; i < visible_samples; i++) {
        uint8_t sample_index = sample_start + i;
        uint8_t shift = state->samples_per_bit - sample_index - 1;
        buf[i < 2 ? i : i + 2] = state->sample_history[bit_index] & (1u << shift) ? '1' : '0';
    }

    watch_display_string(buf, 0);
}

static void blink_receiver_move_history(blink_receiver_state_t *state, int8_t direction) {
    uint8_t history_length = (BITS_PER_PACKAGE + 1) * (state->samples_per_bit > 8 ? 2 : 1);
    state->history_position = (state->history_position + history_length + direction) % history_length;
    blink_receiver_display_history(state);
}

static uint16_t blink_receiver_pack_samples(uint8_t samples_per_bit, bool *samples) {
    uint16_t pattern = 0;
    for (uint8_t i = 0; i < samples_per_bit; i++) {
        pattern = (uint16_t)((pattern << 1) | (samples[i] ? 1u : 0u));
    }
    return pattern;
}

uint8_t blink_receiver_calculate_checksum(uint8_t data_1, uint8_t data_2, uint8_t data_3)
{
    uint32_t sum = (uint32_t)data_1 + (uint32_t)data_2 + (uint32_t)data_3;
    return (uint8_t)(sum & 0xFF);
}

void blink_receiver_handle_heartbeat(uint16_t ms) {
    if (ms > 0) {
        watch_rtc_enable(false);
        delay_ms(ms); // delay clock
        watch_rtc_enable(true);
    }

    movement_set_utc_timestamp(movement_get_utc_timestamp() + 1);
}

//set local time to received timestamp
void blink_receiver_handle_timeset(uint32_t packet)
{
    blink_receiver_packet_time_t data = { .reg = packet };
    watch_display_text(WATCH_POSITION_TOP_LEFT, "TI");
    watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");
    watch_display_text(WATCH_POSITION_BOTTOM, "      ");
    
    watch_date_time_t date_time = movement_get_local_date_time();
    date_time.unit.second = data.time.second;
    date_time.unit.minute = data.time.minute;
    date_time.unit.hour = data.time.hour;

    movement_set_local_date_time(date_time);

    char outputString2[3];
    snprintf(outputString2, 3, "%02d", data.time.hour);
    watch_display_string(outputString2, 4);
    snprintf(outputString2, 3, "%02d", data.time.minute);
    watch_display_string(outputString2, 6);
    snprintf(outputString2, 3, "%02d", data.time.second);
    watch_display_string(outputString2, 8);
}

//set local date to received date
void blink_receiver_handle_dateset(uint32_t packet)
{
    blink_receiver_packet_date_t data = { .reg = packet };
    watch_display_text(WATCH_POSITION_TOP_LEFT, "DA");
    watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");

    char outputString2[3];
    snprintf(outputString2, 3, "%2d", data.date.day);
    watch_display_string(outputString2, 2);

    snprintf(outputString2, 3, "%02d", data.date.month);
    watch_display_string(outputString2, 8);

    char outputString4[5];
    snprintf(outputString4, 5, "%04d", (data.date.year) + 2020);
    printf(outputString4);
    watch_display_string(outputString4, 4);
    
    watch_date_time_t date_time = movement_get_local_date_time();
    date_time.unit.day = data.date.day;
    date_time.unit.month = data.date.month;
    date_time.unit.year = data.date.year; 
    movement_set_local_date_time(date_time);
}

//set local date to received date and set local time to received timestamp
void blink_receiver_handle_datetimeset(uint32_t data1, uint32_t data2)
{
    blink_receiver_handle_timeset(data2);
    blink_receiver_handle_dateset(data1);
    watch_display_text(WATCH_POSITION_TOP_LEFT, "AI");
}

// Handle the received 32 bits. First 3 bytes are data, last byte is checksum. 
bool blink_receiver_handle_data(blink_receiver_state_t *state, uint32_t data) {
    char outputString9[9];
    snprintf(outputString9, 9, "%08x", data);
    watch_display_string(outputString9, 2);
    
    blink_receiver_packet_data_t packet = { .reg = data };
    uint8_t checksum = blink_receiver_calculate_checksum(packet.bytes.byte_1, packet.bytes.byte_2, packet.bytes.byte_3);
    if (checksum == packet.bytes.checksum) {
        switch(packet.bytes.byte_1 >> 4) { //header is the first 4 bits
            case BLINK_RECEIVER_HEADER_TIMESET:     
                blink_receiver_handle_timeset(data);
                break;
            case BLINK_RECEIVER_HEADER_DATESET:
                blink_receiver_handle_dateset(data); // date received
                break;
            case BLINK_RECEIVER_HEADER_DATETIME:
                blink_receiver_handle_datetimeset(state->data[0], state->data[1]);
                break;
            case BLINK_RECEIVER_HEADER_MULTIPLE_PACKETS:
                if (state->current_package + 1 < MAX_PACKAGE_COUNT) {
                    watch_display_text(WATCH_POSITION_TOP_LEFT, "PA"); //one of multiple packages received
                    return true;
                } else {
                    watch_display_text(WATCH_POSITION_TOP_LEFT, "PO"); // too many packages
                }
            default: 
                watch_display_text(WATCH_POSITION_TOP_LEFT, "__"); // unknown header
        }
    } else {
        //wrong checksum
        char outputString[3];
        snprintf(outputString, 3, "%02x", checksum);
        watch_display_text(WATCH_POSITION_TOP_LEFT, outputString);
    }

    return false;
}

bool blink_receiver_summarize_samples(uint8_t samples_per_bit, bool *samples) {
    uint8_t bright_sample_count = 0;
    for (int i = 0; i < samples_per_bit; i++) {
        if (samples[i]) {
            bright_sample_count++;
        }
    }
    return bright_sample_count >= (samples_per_bit / 2);
}

void blink_receiver_read_sample(bool waiting_for_start, blink_receiver_state_t *state) {
    bool bright = blink_receiver_read_sensor() < state->light_level_border;

    if (!waiting_for_start || state->samples_received > 0 || bright) {
        state->samples[state->samples_received] = bright;
        state->samples_received++;

        if(state->samples_received == 1) {
            watch_display_string("        ", 2);
        }
        if (state->samples_received <= 8) {
            watch_display_string(state->samples[state->samples_received - 1] ? "1" : "0", state->samples_received + 1);
        }

        if (state->samples_received == state->samples_per_bit) {
            //enough samples received for a single bit
            state->samples_received = 0;

            if (waiting_for_start) {
                if (blink_receiver_summarize_samples(state->samples_per_bit, state->samples)) {
                    //enough bright samples detected: transmission of package has started
                    state->bits_received = 0;
                    state->data[state->current_package] = 0;
                    for (int i = 0; i <= BITS_PER_PACKAGE; i++) {
                        state->sample_history[i] = 0;
                    }
                    state->sample_history[0] =
                        blink_receiver_pack_samples(state->samples_per_bit, state->samples);
                    state->mode = BLINK_RECEIVER_MODE_RECORD;
                }
            } else {
                //start signal received: record the next packet (32 bit, 32*samples_per_bit samples)
                state->sample_history[state->bits_received + 1] =
                    blink_receiver_pack_samples(state->samples_per_bit, state->samples);
                state->data[state->current_package] = state->data[state->current_package] << 1;
                if (blink_receiver_summarize_samples(state->samples_per_bit, state->samples)) {
                    state->data[state->current_package] += 1;
                }
                state->bits_received++;

                if (state->bits_received == BITS_PER_PACKAGE) { // 32bit package complete
                    if (blink_receiver_handle_data(state, state->data[state->current_package])) {
                        //another package is coming
                        state->bits_received = 0;
                        state->current_package++;
                        state->mode = BLINK_RECEIVER_MODE_WAIT_FOR_RISING_EDGE;
                    } else {
                        //transmission complete
                        state->mode = BLINK_RECEIVER_MODE_IDLE;
                    }
                }
            }
        }
    }
}

bool blink_receiver_face_loop(movement_event_t event, void *context) {
    blink_receiver_state_t *state = (blink_receiver_state_t *)context;
    char buf[7];
    switch (event.event_type) {
        case EVENT_NONE:
            break;
        case EVENT_ACTIVATE:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "BLI", "BL");
            state->mode = BLINK_RECEIVER_MODE_DISPLAY_CURRENT_LIGHT;
            break;
        case EVENT_TICK:
            switch (state->mode) {
                case BLINK_RECEIVER_MODE_DISPLAY_CURRENT_LIGHT:
                    // starting state: show current light value          
                    snprintf(buf, 7, "%-6d", blink_receiver_read_sensor());
                    watch_display_text(WATCH_POSITION_BOTTOM, buf);
                    break;            
                case BLINK_RECEIVER_MODE_WAIT_FOR_RISING_EDGE:     
                    blink_receiver_read_sample(true, state);
                    break;   
                case BLINK_RECEIVER_MODE_RECORD:     
                    blink_receiver_read_sample(false, state);
                    break;
                default: //IDLE and SETTINGS and HISTORY
                    break;
            }
            break;
        case EVENT_ALARM_BUTTON_UP:
            // Start or interrupt listening to incoming transmission
            switch (state->mode) {
                case BLINK_RECEIVER_MODE_DISPLAY_CURRENT_LIGHT:
                case BLINK_RECEIVER_MODE_IDLE:
                    // start listening
                    //watch_buzzer_play_note(BUZZER_NOTE_E5, 100);
                    state->samples_received = 0;
                    state->current_package = 0;
                    state->mode = BLINK_RECEIVER_MODE_WAIT_FOR_RISING_EDGE;
                    watch_display_string("        ", 2);
                    blink_receiver_debug_idx = 0;
                    break;
                case BLINK_RECEIVER_MODE_WAIT_FOR_RISING_EDGE:
                case BLINK_RECEIVER_MODE_RECORD:
                    // interrupt listening
                    //watch_buzzer_play_note(BUZZER_NOTE_E4, 200);
                    state->mode = BLINK_RECEIVER_MODE_IDLE;
                    break;
                default:
                    break;
            }
            break;
        case EVENT_ALARM_LONG_PRESS:
            if (state->mode == BLINK_RECEIVER_MODE_HISTORY) {
                //exit history
                state->mode = BLINK_RECEIVER_MODE_DISPLAY_CURRENT_LIGHT;
                watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "BLI", "BL");
                watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");
                watch_display_string("        ", 2);
                watch_display_text(WATCH_POSITION_BOTTOM, "      ");
            } else if (state->mode != BLINK_RECEIVER_MODE_SETTING
                && state->mode != BLINK_RECEIVER_MODE_WAIT_FOR_RISING_EDGE
                && state->mode != BLINK_RECEIVER_MODE_RECORD) {
                //enter history
                state->history_position = 0;
                state->mode = BLINK_RECEIVER_MODE_HISTORY;
                blink_receiver_display_history(state);
            }
            break;
        case EVENT_ALARM_BUTTON_DOWN:
            if (state->mode == BLINK_RECEIVER_MODE_HISTORY) {
                blink_receiver_move_history(state, 1);
            } else if (state->mode == BLINK_RECEIVER_MODE_SETTING) {
                blink_receiver_advance_setting(state);
                blink_receiver_display_settings(state);
            }
            break;
        case EVENT_LIGHT_LONG_PRESS:
            if (state->mode == BLINK_RECEIVER_MODE_HISTORY) {
                break;
            } else if (state->mode == BLINK_RECEIVER_MODE_SETTING) {
                blink_receiver_exit_settings(state);
            } else if (state->mode != BLINK_RECEIVER_MODE_WAIT_FOR_RISING_EDGE && state->mode != BLINK_RECEIVER_MODE_RECORD) {
                state->settings_state = 0;
                state->mode = BLINK_RECEIVER_MODE_SETTING;
                blink_receiver_display_settings(state);
            }
            break;
        case EVENT_LIGHT_BUTTON_DOWN:
            if (state->mode == BLINK_RECEIVER_MODE_HISTORY) {
                blink_receiver_move_history(state, -1);
            } else if (state->mode == BLINK_RECEIVER_MODE_SETTING) {
                state->settings_state++;
                if (state->settings_state > 2) {
                    blink_receiver_exit_settings(state);
                } else {
                    blink_receiver_display_settings(state);
                }
            }
            break;
        case EVENT_LIGHT_BUTTON_UP:
            break;
        case EVENT_TIMEOUT:
            //movement_move_to_face(0);
            break;
        default:
            return movement_default_loop_handler(event);
    }

    return false;
}

void blink_receiver_face_resign(void *context) {
    (void) context;
}

//#endif // HAS_IR_SENSOR
