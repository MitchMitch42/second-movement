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

#include <stdlib.h>
#include <string.h>
#include "easteregg_face.h"
#include "watch.h"
#include "movement_custom_signal_tunes.h"

uint8_t red= 0;
uint8_t green= 0;
uint8_t blue= 0;
uint8_t tick=0;
bool led_enabled= false;
bool _clear;

uint8_t _com;
uint8_t _seg;

void easteregg_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(easteregg_state_t));
        memset(*context_ptr, 0, sizeof(easteregg_state_t));
        // Do any one-time tasks in here; the inside of this conditional happens only at boot.
    }
    // Do any pin or peripheral setup here; this will be called whenever the watch wakes from deep sleep.
}

void easteregg_face_activate(void *context) {
    easteregg_state_t *state = (easteregg_state_t *)context;

    watch_lcd_type_t lcd_type = watch_get_lcd_type();
    uint8_t num_com = 3;
    uint8_t num_seg = 27 - num_com;

    if (lcd_type == WATCH_LCD_TYPE_CUSTOM) {
        num_com = 4;
    }

    for (int com = 0; com < num_com; com++) {
        for (int seg = 0; seg < num_seg; seg++) {
            watch_set_pixel(com, seg);
        }
    }
}

bool easteregg_face_loop(movement_event_t event, void *context) {
    easteregg_state_t *state = (easteregg_state_t *)context;

    switch (event.event_type) {
        case EVENT_TICK:
            if(led_enabled) {

                if(_clear)
                    watch_clear_pixel(_com, _seg);
                else
                    watch_set_pixel(_com, _seg);

                _com++;
                if(_com == 3) {
                    _com = 0;
                    _seg++;

                    if(_seg == 24) {
                        _seg = 0;
                        _clear = !_clear;
                    }
                }

                movement_force_led_on(
                    red | red << 4,
                    green | green << 4,
                    blue | blue << 4);         
                if(tick < 15 * 1) red++;
                else if(tick < 15 * 2) green++;
                else if(tick < 15 * 3) red--;
                else if(tick < 15 * 4) blue++;
                else if(tick < 15 * 5) red++;
                else if(tick < 15 * 6) green--;
                else if(tick < 15 * 7) red--;
                else if(tick < 15 * 8) blue--;
                else tick = -1;
                tick++;
            }

            break;
        case EVENT_LIGHT_BUTTON_UP:
            led_enabled= !led_enabled; 
            if(led_enabled)
                movement_request_tick_frequency(16);
                watch_clear_display();
            break;

        case EVENT_LIGHT_LONG_PRESS:
            led_enabled= true; 
            if(led_enabled)
                movement_request_tick_frequency(32);
                watch_clear_display();
            break;

        case EVENT_ALARM_BUTTON_DOWN:
            movement_play_sequence(tunes_table[52], BUZZER_PRIORITY_ALARM);
            break;
        
        default:
            movement_default_loop_handler(event);
            break;
    }
    return true;
}

void easteregg_face_resign(void *context) {
    (void) context;

    // handle any cleanup before your watch face goes off-screen.
}

