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

#define DEFAULT_LIGHT_LEVEL_BORDER 65200
#define DEFAULT_SAMPLES_PER_BIT 8
#define DEFAULT_FREQUENCY 64 

#define MAX_SAMPLES_PER_BIT 16
#define MAX_FREQUENCY 64
#define MAX_PACKAGE_COUNT 4

#define BLINK_RECEIVER_HEADER_TIMESET 1
#define BLINK_RECEIVER_HEADER_DATESET 2
#define BLINK_RECEIVER_HEADER_TIMESET_EXACT 3
#define BLINK_RECEIVER_HEADER_MULTIPLE_PACKETS 4
#define BLINK_RECEIVER_HEADER_DATETIME 5
#define BLINK_RECEIVER_HEADER_DATETIME_EXACT 6

void blink_receiver_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(blink_receiver_state_t));
        memset(*context_ptr, 0, sizeof(blink_receiver_state_t));
        blink_receiver_state_t *state = (blink_receiver_state_t *) *context_ptr;

        state->samples = malloc(MAX_SAMPLES_PER_BIT * sizeof(bool));
        state->data = malloc(MAX_PACKAGE_COUNT * sizeof(uint32_t));
        state->light_level_border = DEFAULT_LIGHT_LEVEL_BORDER; 
        state->samples_per_bit = DEFAULT_SAMPLES_PER_BIT;
        state->frequency = DEFAULT_FREQUENCY;
    }
}

void blink_receiver_face_activate(void *context) {
    (void) context;
    HAL_GPIO_IR_ENABLE_out();
    HAL_GPIO_IR_ENABLE_clr();
    HAL_GPIO_IRSENSE_pmuxen(HAL_GPIO_PMUX_ADC);
    adc_init();
    adc_enable();

    blink_receiver_state_t *state = (blink_receiver_state_t *)context;
    movement_request_tick_frequency(state->frequency);
}

uint8_t blink_receiver_calculate_checksum(uint8_t data_1, uint8_t data_2, uint8_t data_3)
{
    uint32_t sum = (uint32_t)data_1 + (uint32_t)data_2 + (uint32_t)data_3;
    return (uint8_t)(sum & 0xFF);
}

//set local time to received timestamp
void blink_receiver_handle_timeset(bool exact, uint32_t data, uint8_t tick_cnt, uint8_t frequency)
{
    watch_display_text(WATCH_POSITION_TOP_LEFT, exact ? "TE" : "TI");
    watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");
    watch_display_text(WATCH_POSITION_BOTTOM, "      ");
    
    watch_date_time_t date_time = movement_get_local_date_time();
    date_time.unit.second = data >> 22 & 0b111111;
    date_time.unit.minute = data >> 16 & 0b111111;
    date_time.unit.hour = data >> 11 & 0b11111;

    if(exact) movement_set_local_date_time_exact(date_time, (uint16_t)((((double)tick_cnt * 1000.0) / (double)frequency) + 0.5));
    else movement_set_local_date_time(date_time);

    char outputString2[3];
    snprintf(outputString2, 3, "%02d", data >> 11 & 0b11111);
    watch_display_string(outputString2, 4);
    snprintf(outputString2, 3, "%02d", data >> 16 & 0b111111);
    watch_display_string(outputString2, 6);
    snprintf(outputString2, 3, "%02d", data >> 22 & 0b111111);
    watch_display_string(outputString2, 8);
    watch_set_colon();
}

//set local date to received date
void blink_receiver_handle_dateset(uint32_t data)
{
    watch_display_text(WATCH_POSITION_TOP_LEFT, "DA");
    watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");

    char outputString2[3];
    snprintf(outputString2, 3, "%02d", data >> 23 & 0b11111);
    watch_display_string(outputString2, 2);

    snprintf(outputString2, 3, "%02d", data >> 19 & 0b1111);
    watch_display_string(outputString2, 8);

    char outputString4[5];
    snprintf(outputString4, 5, "%04d", (data >> 13 & 0b111111) + 2020);
    printf(outputString4);
    watch_display_string(outputString4, 4);
    
    watch_date_time_t date_time = movement_get_local_date_time();
    date_time.unit.day = data >> 23 & 0b11111;
    date_time.unit.month = data >> 19 & 0b1111;
    date_time.unit.year = data >> 13 & 0b111111; //todo?
    movement_set_local_date_time(date_time);
}

//set local date to received date and set local time to received timestamp
void blink_receiver_handle_datetimeset(bool exact, uint32_t data1, uint32_t data2, uint8_t tick_cnt, uint8_t frequency)
{
    blink_receiver_handle_timeset(exact, data2, tick_cnt, frequency);
    blink_receiver_handle_dateset(data1);
    watch_display_text(WATCH_POSITION_TOP_LEFT, exact ? "AE" : "AI");
}

// Handle the received 32 bits. First 3 bytes are data, last byte is checksum. 
bool blink_receiver_handle_data(blink_receiver_state_t *state, uint32_t data) {
    uint8_t checksum = blink_receiver_calculate_checksum(data >> 24 & 0xFF, data >> 16 & 0xFF, data >> 8 & 0xFF);
    if (checksum == (uint8_t)(data & 0xFF)) { //checksum ist last 8 bits
        uint8_t header = (uint8_t)(data >> 28 & 0b1111); //header is first 4 bits
        switch(header) {
            case BLINK_RECEIVER_HEADER_TIMESET:     
                blink_receiver_handle_timeset(data, false, state->tick_cnt, state->frequency); // timestamp received
                break;
            case BLINK_RECEIVER_HEADER_TIMESET_EXACT:
                blink_receiver_handle_timeset(data, true, state->tick_cnt, state->frequency); // exact timestamp received (ms == 0)
                break;
            case BLINK_RECEIVER_HEADER_DATESET:
                blink_receiver_handle_dateset(data); // date received
                break;
            case BLINK_RECEIVER_HEADER_DATETIME:
                blink_receiver_handle_datetimeset(false, state->data[0], state->data[1], state->tick_cnt, state->frequency); // date and timestamp received
                break;
            case BLINK_RECEIVER_HEADER_DATETIME_EXACT:
                blink_receiver_handle_datetimeset(true, state->data[0], state->data[1], state->tick_cnt, state->frequency); // date and exact timestamp received (ms == 0)
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

bool blink_receiver_summarize_samples(blink_receiver_state_t *state) {
    uint8_t bright_sample_count = 0;
    for (int i = 0; i < state->samples_per_bit; i++) {
        if (state->samples[i]) {
            bright_sample_count++;
        }
    }
    return bright_sample_count >= (state->samples_per_bit / 2);
}

void blink_receiver_read_sample(bool waiting_for_start, blink_receiver_state_t *state) {
    bool bright = adc_get_analog_value(HAL_GPIO_IRSENSE_pin()) < state->light_level_border;

    if (waiting_for_start) { //waiting for start signal
        if (state->samples_received > 0 || bright) { //only record samples if at least one bright sample has been received
            state->samples[state->samples_received] = bright;
            state->samples_received++;

            if(state->samples_received <= 8) {
                watch_display_string(state->samples[state->samples_received - 1] ? "1" : "0", state->samples_received + 1);
            }

            if (state->samples_received == state->samples_per_bit) {
                state->samples_received = 0;
                if (blink_receiver_summarize_samples(state)) { //enough bright samples detected: transmission has started
                    state->bits_received = 0;
                    state->current_package = 0;
                    for (int i = 0; i < MAX_PACKAGE_COUNT; i++) {
                        state->data[i] = 0;
                    }
                    state->mode = BLINK_RECEIVER_MODE_RECORD;
                }
            }
        }
    } else { //start signal received: record the next 32 bit (32*samples_per_bit samples)
        state->samples[state->samples_received] = bright;
        state->samples_received++;   

        if (state->samples_received == state->samples_per_bit) {
            state->samples_received = 0;
            state->data[state->current_package] = state->data[state->current_package] << 1;
            if (blink_receiver_summarize_samples(state)) {         
                state->data[state->current_package] += 1;              
            }
            state->bits_received++;
            if (state->bits_received == 32) { // transmission of 32bit package complete   
                if(blink_receiver_handle_data(state, state->data[state->current_package])) {
                    //another package is coming
                    state->bits_received = 0;
                    state->current_package++;
                } else {
                    //transmission complete
                    state->mode = BLINK_RECEIVER_MODE_IDLE;   
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
            watch_display_text_with_fallback(WATCH_POSITION_TOP, "BLINK", "BL");
            state->samples_received = 0;
            state->last_second = 62;
            state->mode = BLINK_RECEIVER_MODE_CALC;         
            break;
        case EVENT_TICK:
            state->tick_cnt = (state->tick_cnt + 1) % state->frequency;        
            switch (state->mode) {
                case BLINK_RECEIVER_MODE_DISPLAY_CURRENT_LIGHT:
                    // starting state: show current light value
                    
                    snprintf(buf, 7, "%-6d", adc_get_analog_value(HAL_GPIO_IRSENSE_pin()));
                    watch_display_text(WATCH_POSITION_BOTTOM, buf);
                    break;            
                case BLINK_RECEIVER_MODE_WAIT_FOR_RISING_EDGE:     
                    blink_receiver_read_sample(true, state);
                    break;   
                case BLINK_RECEIVER_MODE_RECORD:     
                    blink_receiver_read_sample(false, state);
                    break;
                case BLINK_RECEIVER_MODE_CALC:
                    watch_display_text(WATCH_POSITION_BOTTOM, "CALC"); 
                    if (state->last_second == 62) {
                        state->last_second = movement_get_local_date_time().unit.second;
                    }
                    else if (movement_get_local_date_time().unit.second != state->last_second) {
                        state->tick_cnt = 0;
                        state->mode = BLINK_RECEIVER_MODE_DISPLAY_CURRENT_LIGHT;
                    }
                    break;     
                default: //IDLE
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
                    state->mode = BLINK_RECEIVER_MODE_WAIT_FOR_RISING_EDGE;
                    watch_display_string("        ", 2);
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
            //change frequency (1 Hz - 64 Hz)
            state->frequency *= 2;
            if (state->frequency > MAX_FREQUENCY)
                state->frequency = 1;
            snprintf(buf, 3, "%02d", state->frequency);
            watch_display_string(buf, 2);
            movement_request_tick_frequency(state->frequency);
            state->last_second = 62;
            state->mode = BLINK_RECEIVER_MODE_CALC;   
            break;
        case EVENT_LIGHT_BUTTON_UP:
            //changes samples_per_bit (1 to 16)
            state->samples_per_bit *= 2;
            if (state->samples_per_bit > MAX_SAMPLES_PER_BIT)
                state->samples_per_bit = 1;
            snprintf(buf, 3, "%02d", state->samples_per_bit);
            watch_display_string(buf, 2);
            state->mode = BLINK_RECEIVER_MODE_IDLE;
            break;
        case EVENT_LIGHT_LONG_PRESS:
            //change light level border (65400 - 65500)
            state->light_level_border += 10;
            if (state->light_level_border > 65500)
                state->light_level_border = 65100;
            snprintf(buf, 6, "%5d", state->light_level_border);
            watch_display_string("      ", 4);
            watch_display_string(buf, 4);
            break;
        case EVENT_LIGHT_BUTTON_DOWN:
            break; //do not illuminate led
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

    adc_disable();
    HAL_GPIO_IRSENSE_pmuxdis();
    HAL_GPIO_IRSENSE_off();
    HAL_GPIO_IR_ENABLE_off();
}

//#endif // HAS_IR_SENSOR
