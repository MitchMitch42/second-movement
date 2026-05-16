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

void blink_receiver_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(blink_receiver_state_t));
        memset(*context_ptr, 0, sizeof(blink_receiver_state_t));
        blink_receiver_state_t *state = (blink_receiver_state_t *) *context_ptr;

        state->light_level_border = 65440; 
        state->frequency = 8;
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
void blink_receiver_handle_timeset(blink_receiver_state_t *state)
{
    watch_display_text(WATCH_POSITION_TOP_LEFT, "TI");
    watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");

    char outputString2[3];
    snprintf(outputString2, 3, "%02d", state->data >> 11 & 0b11111);
    watch_display_string(outputString2, 4);
    snprintf(outputString2, 3, "%02d", state->data >> 16 & 0b111111);
    watch_display_string(outputString2, 6);
    snprintf(outputString2, 3, "%02d", state->data >> 22 & 0b111111);
    watch_display_string(outputString2, 8);
    watch_set_colon();
    
    watch_date_time_t date_time = movement_get_local_date_time();
    date_time.unit.second = state->data >> 22 & 0b111111;
    date_time.unit.minute = state->data >> 16 & 0b111111;
    date_time.unit.hour = state->data >> 11 & 0b11111;
    movement_set_local_date_time(date_time);
}

//set local date to received date
void blink_receiver_handle_dateset(blink_receiver_state_t *state)
{
    watch_display_text(WATCH_POSITION_TOP_LEFT, "DA");
    watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");

    char outputString2[3];
    snprintf(outputString2, 3, "%02d", state->data >> 23 & 0b11111);
    watch_display_string(outputString2, 2);

    snprintf(outputString2, 3, "%02d", state->data >> 19 & 0b1111);
    watch_display_string(outputString2, 8);

    char outputString4[5];
    snprintf(outputString4, 5, "%04d", (state->data >> 13 & 0b111111) + 2020);
    printf(outputString4);
    watch_display_string("    ", 4);
    
    watch_date_time_t date_time = movement_get_local_date_time();
    date_time.unit.day = state->data >> 23 & 0b11111;
    date_time.unit.month = state->data >> 19 & 0b1111;
    date_time.unit.year = state->data >> 13 & 0b111111; //todo?
    movement_set_local_date_time(date_time);
}

// Handle the received 4 bytes. First 3 bytes are data, last byte is checksum. 
void blink_receiver_handle_data(blink_receiver_state_t *state) {
    uint8_t checksum = blink_receiver_calculate_checksum(state->data >> 24 & 0xFF, state->data >> 16 & 0xFF, state->data >> 8 & 0xFF);
    if (checksum == (uint8_t)(state->data & 0xFF)) { //checksum ist last 8 bits
        switch (state->data >> 28 & 0b1111) { //header is first 4 bits
            case 1:
                // timestamp received
                blink_receiver_handle_timeset(state); 
                break;        
            case 2:
                // date received
                blink_receiver_handle_dateset(state); 
                break;
            default: 
                // unknown header
                watch_display_text(WATCH_POSITION_TOP_LEFT, "__");
                break;
        }
    } else {
        //wrong checksum
        char outputString[3];
        snprintf(outputString, 3, "%02x", checksum);
        watch_display_text(WATCH_POSITION_TOP_LEFT, outputString);
    }
}

bool blink_receiver_face_loop(movement_event_t event, void *context) {
    blink_receiver_state_t *state = (blink_receiver_state_t *)context;

    switch (event.event_type) {
        case EVENT_NONE:
            break;
        case EVENT_ACTIVATE:
            watch_display_text_with_fallback(WATCH_POSITION_TOP, "BLINK", "BL");
            state->mode = BLINK_RECEIVER_MODE_DISPLAY_CURRENT_LIGHT;         
            break;
        case EVENT_TICK:
            switch (state->mode) {
                case BLINK_RECEIVER_MODE_DISPLAY_CURRENT_LIGHT:
                    // starting state: show current light value
                    state->light_level = adc_get_analog_value(HAL_GPIO_IRSENSE_pin());
                    char buf[7];
                    snprintf(buf, 7, "%-6d", state->light_level);
                    watch_display_text(WATCH_POSITION_BOTTOM, buf);
                    break;            
                case BLINK_RECEIVER_MODE_WAIT_FOR_RISING_EDGE:        
                    // watch is polling sensor with increased frequency, waiting for a riging edge (dark -> bright)
                    state->light_level = adc_get_analog_value(HAL_GPIO_IRSENSE_pin());      
                    if (state->light_level < state->light_level_border) { //rising edge detected: transmission has started
                        //watch_buzzer_play_note(BUZZER_NOTE_E6, 100);  
                        state->bits_received = 0;
                        state->data = 0;
                        state->mode = BLINK_RECEIVER_MODE_RECORD;
                        movement_request_tick_frequency(state->frequency); //switch back to normal frequency for receiving the data
                    }
                    break;
                case BLINK_RECEIVER_MODE_RECORD:     
                        // read light sensor and save value (dark = 0, bright = 1) to corresponding bit in state->data
                        state->light_level = adc_get_analog_value(HAL_GPIO_IRSENSE_pin());
                        state->data = state->data << 1;
                        if (state->light_level < state->light_level_border) {         
                            state->data += 1;              
                        }
                        state->bits_received++;
                        if (state->bits_received == 32) { // transmission complete                 
                            //watch_buzzer_play_note(BUZZER_NOTE_E6, 100);
                            state->mode = BLINK_RECEIVER_MODE_IDLE;   
                            char outputString9[9];
                            snprintf(outputString9, 9, "%08x", state->data);
                            watch_display_string(outputString9, 2);
                            blink_receiver_handle_data(state);
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
                    state->mode = BLINK_RECEIVER_MODE_WAIT_FOR_RISING_EDGE;
                    movement_request_tick_frequency(state->frequency * 2); //increase frequency to make sure that the first rising edge is detected as fast as possible
                    watch_display_string("        ", 2);
                    watch_clear_colon();
                    break;
                case BLINK_RECEIVER_MODE_WAIT_FOR_RISING_EDGE:
                case BLINK_RECEIVER_MODE_RECORD:
                    // interrupt listening
                    //watch_buzzer_play_note(BUZZER_NOTE_E4, 200);
                    state->mode = BLINK_RECEIVER_MODE_IDLE;
                    movement_request_tick_frequency(state->frequency); //switch back to normal frequency
                    break;
                default:
                    break;
            }
            break;
        case EVENT_LIGHT_LONG_PRESS:
            //change frequency (1 Hz - 32 Hz)
            state->frequency *= 2;
            if (state->frequency > 32)
                state->frequency = 1;
            char outputString3[3];
            snprintf(outputString3, 3, "%2d", state->frequency);
            watch_display_string(outputString3, 2);
            movement_request_tick_frequency(state->frequency);
            break;
        case EVENT_LIGHT_BUTTON_UP:
            //change light level border (65400 - 65500)
            state->light_level_border += 1;
            if (state->light_level_border > 65500)
                state->light_level_border = 65400;
            char outputString6[6];
            snprintf(outputString6, 6, "%5d", state->light_level_border);
            watch_display_string(outputString6, 4);
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
