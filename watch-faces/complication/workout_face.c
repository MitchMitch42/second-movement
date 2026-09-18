/*
 * MIT License
 *
 * Copyright (c) 2026
 *
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "workout_face.h"
#include "watch.h"
#include "watch_common_display.h"
#include "watch_utility.h"
#include "watch_rtc.h"
#include "slcd.h"

// Loosely implement the watch as a state machine
typedef enum {
    SW_STATUS_IDLE = 0,
    SW_STATUS_RUNNING,
    SW_STATUS_STOPPED,
    
} stopwatch_status_t;

static inline void _button_beep() {
    if (movement_button_should_sound()) watch_buzzer_play_note_with_volume(BUZZER_NOTE_C7, 50, movement_button_volume());
}

// How quickly should the elapsing time be displayed?
// This is just for looks, timekeeping is always accurate to 128Hz
static const uint8_t DISPLAY_RUNNING_RATE = 32;

/// @brief Display minutes, seconds and fractions derived from 128 Hz tick counter
///        on the lcd.
/// @param ticks
static void _display_elapsed(workout_state_t *state, uint32_t ticks) {
    char buf[3];
    uint8_t sec_100 = (ticks & 0x7F) * 100 / 128;

    watch_display_character_lp_seconds('0' + sec_100 / 10, 8);
    watch_display_character_lp_seconds('0' + sec_100 % 10, 9);

    uint32_t seconds = ticks >> 7;

    if (seconds == state->old_display.seconds) {
        return;
    }

    state->old_display.seconds = seconds;

    sprintf(buf, "%02lu", seconds % 60);
    watch_display_text(WATCH_POSITION_MINUTES, buf);

    uint32_t minutes = seconds / 60;

    if (minutes == state->old_display.minutes) {
        return;
    }

    state->old_display.minutes = minutes;

    sprintf(buf, "%02lu", minutes % 60);
    watch_display_text(WATCH_POSITION_HOURS, buf);

    uint32_t hours = (minutes / 60) % 24;

    if (hours == state->old_display.hours) {
        return;
    }

    state->old_display.hours = hours;

    if (hours) {
        sprintf(buf, "%2lu", hours);
        watch_display_text(WATCH_POSITION_TOP_RIGHT, buf);
    } else {
        watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");
    }
}

static void _draw_indicators(workout_state_t *state, movement_event_t event, uint32_t elapsed) {
    uint8_t subsecond;
    bool tock;

    switch (state->status) {
        case SW_STATUS_RUNNING:
            subsecond = elapsed & 127;
            tock = subsecond >= 64;

            if (tock) {
                watch_clear_colon();
            } else {
                watch_set_colon();
            }

            return;

        case SW_STATUS_STOPPED:
        case SW_STATUS_IDLE:
        default:
            watch_set_colon();
            return;
    }
}

static uint8_t get_refresh_rate(workout_state_t *state) {
    switch (state->status) {
        case SW_STATUS_RUNNING:
            return DISPLAY_RUNNING_RATE;
        case SW_STATUS_STOPPED:
        case SW_STATUS_IDLE:
        default:
            return 1;
    }
}

static uint32_t elapsed_time(workout_state_t *state, rtc_counter_t counter) {
    switch (state->status) {
        case SW_STATUS_IDLE:
            return 0;

        case SW_STATUS_RUNNING:
            return counter - state->start_counter;

        case SW_STATUS_STOPPED:
            return state->stop_counter - state->start_counter;

        default:
            return 0;
    }
}

static void workout_face_add_elapsed_to_day_buffer(workout_state_t *state, uint32_t elapsed_seconds) {
    if (elapsed_seconds == 0) {
        return;
    }

    uint32_t today_day_index = movement_get_utc_timestamp() / 86400U;
    uint8_t match_index = UINT8_MAX;

    for (uint8_t i = 0; i < state->day_count; i++) {
        if (state->day_totals[i].day_index == today_day_index) {
            match_index = i;
            break;
        }
    }

    if (match_index != UINT8_MAX) {
        state->day_totals[match_index].total_seconds += elapsed_seconds;
        return;
    }

    if (state->day_count < WORKOUT_HISTORY_DAYS) {
        state->day_totals[state->day_count].day_index = today_day_index;
        state->day_totals[state->day_count].total_seconds = elapsed_seconds;
        state->day_count++;
        return;
    }

    uint8_t oldest_index = 0;
    for (uint8_t i = 1; i < WORKOUT_HISTORY_DAYS; i++) {
        if (state->day_totals[i].day_index < state->day_totals[oldest_index].day_index) {
            oldest_index = i;
        }
    }

    state->day_totals[oldest_index].day_index = today_day_index;
    state->day_totals[oldest_index].total_seconds = elapsed_seconds;
}

static void state_transition(workout_state_t *state, rtc_counter_t counter, movement_event_type_t event_type) {
    switch (state->status) {
        case SW_STATUS_IDLE:
            switch (event_type) {
                case EVENT_ALARM_BUTTON_DOWN:
                    state->status = SW_STATUS_RUNNING;
                    state->start_counter = counter;
                    movement_request_tick_frequency(get_refresh_rate(state));
                    return;
                default:
                    return;
            }

        case SW_STATUS_RUNNING:
            switch (event_type) {
                case EVENT_ALARM_BUTTON_DOWN:
                    state->status = SW_STATUS_STOPPED;
                    state->stop_counter = counter;
                    movement_request_tick_frequency(get_refresh_rate(state));
                    return;
                default:
                    return;
            }

        case SW_STATUS_STOPPED:
            switch (event_type) {
                case EVENT_ALARM_BUTTON_UP:
                    state->status = SW_STATUS_RUNNING;
                    state->start_counter = counter - state->stop_counter + state->start_counter;
                    movement_request_tick_frequency(get_refresh_rate(state));
                    state->clear_confirm = false;
                    return;
                case EVENT_ALARM_LONG_PRESS: 
                    state->status = SW_STATUS_IDLE;
                    workout_face_add_elapsed_to_day_buffer(state, elapsed_time(state, counter));
                    state->clear_confirm = false;
                    return;
                case EVENT_LIGHT_LONG_PRESS:
                    if (state->clear_confirm) {
                        state->status = SW_STATUS_IDLE;
                        state->start_counter = 0;
                        state->stop_counter = 0;
                        state->clear_confirm = false;
                        return;
                    }
                    state->clear_confirm = true;
                    return;
                default:
                    return;
            }

        default:
            return;
    }
}

void workout_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(workout_state_t));
        memset(*context_ptr, 0, sizeof(workout_state_t));
        workout_state_t *state = (workout_state_t *)*context_ptr;
        state->start_counter = 0;
        state->stop_counter = 0;
        state->status = SW_STATUS_IDLE;
        state->clear_confirm = false;
    }
}

void workout_face_activate(void *context) {
    workout_state_t *state = (workout_state_t *) context;
    // force full re-draw
    state->old_display.seconds = UINT_MAX;
    state->old_display.minutes = UINT_MAX;
    state->old_display.hours = UINT_MAX;
    movement_request_tick_frequency(get_refresh_rate(state));
}

bool workout_face_loop(movement_event_t event, void *context) {
    workout_state_t *state = (workout_state_t *)context;

    rtc_counter_t counter = watch_rtc_get_counter();

    state_transition(state, counter, event.event_type);
    rtc_counter_t elapsed = elapsed_time(state, counter);

    switch (event.event_type) {
        case EVENT_ACTIVATE:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "STW", "ST");
            _draw_indicators(state, event, elapsed);
            _display_elapsed(state, elapsed);
            if (state->clear_confirm) {
                watch_display_text_with_fallback(WATCH_POSITION_BOTTOM, "CLEAR", "CL");
            }
            break;
        case EVENT_ALARM_BUTTON_DOWN:
        case EVENT_ALARM_BUTTON_UP:
        case EVENT_ALARM_LONG_PRESS:
        case EVENT_LIGHT_LONG_PRESS:
            _button_beep();
            if (state->clear_confirm) {
                watch_display_text_with_fallback(WATCH_POSITION_BOTTOM, "CLEAR", "CL");
            }
            // fall through
        case EVENT_TICK:
            _draw_indicators(state, event, elapsed);
            _display_elapsed(state, elapsed);
            if (state->clear_confirm) {
                watch_display_text_with_fallback(WATCH_POSITION_BOTTOM, "CLEAR", "CL");
            }
            break;
        default:
            movement_default_loop_handler(event);
            break;
    }

    return true;
}

void workout_face_resign(void *context) {
    (void) context;
    movement_request_tick_frequency(1);
}