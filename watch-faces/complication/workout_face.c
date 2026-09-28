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

#define WORKOUT_BUFFER_SIZE_MAX 7

// Loosely implement the watch as a state machine
typedef enum {
    SW_STATUS_IDLE = 0,
    SW_STATUS_RUNNING,
    SW_STATUS_STOPPED,
    SW_STATUS_CLEAR,
    SW_STATUS_LOG,
} stopwatch_status_t;


// How quickly should the elapsing time be displayed?
// This is just for looks, timekeeping is always accurate to 128Hz
static const uint8_t DISPLAY_RUNNING_RATE = 32;

static void workout_face_beep() {
    watch_buzzer_play_note_with_volume(BUZZER_NOTE_C7, 50, movement_button_volume());
}

/// @brief reset a rolling buffer
/// @param usable_length maximum number of entries the buffer can hold
static void workout_face_init_rolling_buffer(workout_rolling_buffer_t *buffer, int usable_length) {
    buffer->head_index = -1;
    buffer->length = 0;
    buffer->max = usable_length;
}

/// @brief add a value to a rolling buffer
static void workout_face_add_to_rolling_buffer(workout_rolling_buffer_t *buffer, uint32_t elapsed, uint32_t timestamp) {
    buffer->head_index = (buffer->head_index + 1) % buffer->max;
    buffer->length = buffer->length + 1 < buffer->max ? buffer->length + 1 : buffer->max;
    buffer->data[buffer->head_index].elapsed = elapsed;
    buffer->data[buffer->head_index].timestamp = timestamp;
}

static void workout_face_show_log(workout_state_t *state) {
    char bottom[11];
    char top_right[3];
    if (state->buffer.length == 0) {
        watch_display_text_with_fallback(WATCH_POSITION_BOTTOM, "no dat", "no dat");
        watch_display_text_with_fallback(WATCH_POSITION_TOP_RIGHT, "  ", "  ");
        return;
    }
    uint32_t total_elapsed = state->buffer.data[state->log_index].elapsed;

    uint32_t hours = total_elapsed / 360000U;
    uint32_t minutes = (total_elapsed % 360000U) / 6000U;
    uint32_t seconds = (total_elapsed % 6000U) / 100U;
    sprintf(bottom, "%02lu%02lu%02lu", hours, minutes, seconds);
    watch_display_text_with_fallback(WATCH_POSITION_BOTTOM, bottom, bottom);
    watch_date_time_t dt = watch_utility_date_time_from_unix_time(state->buffer.data[state->log_index].timestamp * 86400U, movement_get_current_timezone_offset());
    sprintf(top_right, "%02u", dt.unit.day);
    watch_display_text_with_fallback(WATCH_POSITION_TOP_RIGHT, top_right, top_right);
}

static void workout_face_display(workout_state_t *state, uint32_t ticks) {
    if (state->status == SW_STATUS_CLEAR) {
        watch_display_text(WATCH_POSITION_BOTTOM, state->clear_yes ? "CLEAry" : "CLEArn");
        return;
    }

    if (state->status == SW_STATUS_LOG) {
        workout_face_show_log(state);
        return;
    }

    watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");

    char buf[3];
    uint32_t seconds = ticks >> 7;

    if (seconds == state->old_display.seconds) {
        return;
    }

    state->old_display.seconds = seconds;

    sprintf(buf, "%02lu", seconds % 60);
    watch_display_text(WATCH_POSITION_SECONDS, buf);

    uint32_t minutes = seconds / 60;

    if (minutes == state->old_display.minutes) {
        return;
    }

    state->old_display.minutes = minutes;

    sprintf(buf, "%02lu", minutes % 60);
    watch_display_text(WATCH_POSITION_MINUTES, buf);

    uint32_t hours = (minutes / 60) % 24;

    if (hours == state->old_display.hours) {
        return;
    }

    state->old_display.hours = hours;

    sprintf(buf, "%02lu", hours);
    watch_display_text(WATCH_POSITION_HOURS, buf);
}

static void workout_face_draw_colon(workout_state_t *state, movement_event_t event, uint32_t elapsed) {
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
        case SW_STATUS_CLEAR:
        case SW_STATUS_LOG:
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

        case SW_STATUS_CLEAR:
        case SW_STATUS_STOPPED:
            return state->stop_counter - state->start_counter;

        default:
            return 0;
    }
}

static void workout_face_fix_buffer(workout_state_t *state) {
    uint32_t today_timestamp = movement_get_utc_timestamp() / 86400U;

    if (state->buffer.length == 0) { 
        //first value: simply add
        workout_face_add_to_rolling_buffer(&state->buffer, 0, today_timestamp);
    } else { 
        //fill the gaps between latest value and today
        uint32_t latest_timestamp = state->buffer.data[state->buffer.head_index].timestamp;
        for (uint32_t timestamp = latest_timestamp + 1; timestamp <= today_timestamp; timestamp++) {
            workout_face_add_to_rolling_buffer(&state->buffer, 0, timestamp);
        }
    }
}

static void workout_face_add_elapsed_to_buffer(workout_state_t *state, uint32_t ticks) {
    if (ticks == 0) {
        return;
    }

    uint32_t elapsed = (ticks * 100U) / 128U;

    state->buffer.data[state->buffer.head_index].elapsed += elapsed;
}

static void state_transition(workout_state_t *state, rtc_counter_t counter, movement_event_type_t event_type) {
    switch (state->status) {
        case SW_STATUS_IDLE:
            switch (event_type) {
                case EVENT_ALARM_BUTTON_UP:
                    state->status = SW_STATUS_RUNNING;
                    state->start_counter = counter;
                    movement_request_tick_frequency(get_refresh_rate(state));
                    return;   
                case EVENT_LIGHT_BUTTON_UP:
                    state->old_status = state->status;
                    state->status = SW_STATUS_LOG;
                    state->log_index = 0;
                    return;
                default:
                    return;
            }

        case SW_STATUS_RUNNING:
            switch (event_type) {
                case EVENT_ALARM_BUTTON_UP:
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
                    return;
                case EVENT_ALARM_LONG_PRESS:
                    workout_face_add_elapsed_to_buffer(state, elapsed_time(state, counter));
                    state->status = SW_STATUS_IDLE;
                    state->start_counter = 0;
                    state->stop_counter = 0;
                    return;
                case EVENT_LIGHT_LONG_PRESS:
                    state->status = SW_STATUS_CLEAR;
                    state->clear_yes = false;
                    return;
                case EVENT_LIGHT_BUTTON_UP:
                    state->old_status = state->status;
                    state->status = SW_STATUS_LOG;
                    state->log_index = 0;
                    return;
                default:
                    return;
            }

        case SW_STATUS_CLEAR:
            switch (event_type) {
                case EVENT_ALARM_BUTTON_UP:
                    state->clear_yes = !state->clear_yes;
                    return;
                case EVENT_LIGHT_LONG_PRESS:
                case EVENT_LIGHT_BUTTON_UP:
                    //force full redraw
                    state->old_display.seconds = UINT_MAX;
                    state->old_display.minutes = UINT_MAX;
                    state->old_display.hours = UINT_MAX;
                    if (state->clear_yes) {
                        state->status = SW_STATUS_IDLE;
                        state->start_counter = 0;
                        state->stop_counter = 0;
                    } else {
                        state->status = SW_STATUS_STOPPED;
                    }
                    state->clear_yes = false;
                    return;
                default:
                    return;
            }

        case SW_STATUS_LOG:
            switch (event_type) {
                case EVENT_LIGHT_BUTTON_UP:
                    state->status = state->old_status;
                    state->old_display.seconds = UINT_MAX;
                    state->old_display.minutes = UINT_MAX;
                    state->old_display.hours = UINT_MAX;
                    return;
                case EVENT_ALARM_BUTTON_UP:
                    if (state->buffer.length != 0) {
                        state->log_index = (state->log_index + 1) % state->buffer.length;
                    }
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
        state->buffer.data = malloc(WORKOUT_BUFFER_SIZE_MAX * sizeof(workout_day_total_t));
        state->start_counter = 0;
        state->stop_counter = 0;
        state->status = SW_STATUS_IDLE;
        state->clear_yes = false;
        workout_face_init_rolling_buffer(&state->buffer, WORKOUT_BUFFER_SIZE_MAX);
    }
}

void workout_face_activate(void *context) {
    workout_state_t *state = (workout_state_t *) context;
    // force full re-draw
    state->old_display.seconds = UINT_MAX;
    state->old_display.minutes = UINT_MAX;
    state->old_display.hours = UINT_MAX;
    movement_request_tick_frequency(get_refresh_rate(state));
    workout_face_fix_buffer(state);
}

bool workout_face_loop(movement_event_t event, void *context) {
    workout_state_t *state = (workout_state_t *)context;

    rtc_counter_t counter = watch_rtc_get_counter();

    state_transition(state, counter, event.event_type);
    rtc_counter_t elapsed = elapsed_time(state, counter);

    switch (event.event_type) {
        case EVENT_ACTIVATE:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "WOR", "WO");
            workout_face_draw_colon(state, event, elapsed);
            workout_face_display(state, elapsed);
            break;
        case EVENT_LIGHT_BUTTON_DOWN:
            break; //no light
        case EVENT_ALARM_BUTTON_UP:
        case EVENT_ALARM_LONG_PRESS:
        case EVENT_LIGHT_BUTTON_UP:
        case EVENT_LIGHT_LONG_PRESS:
        case EVENT_TICK:
            workout_face_draw_colon(state, event, elapsed);
            workout_face_display(state, elapsed);
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