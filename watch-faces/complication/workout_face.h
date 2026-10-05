/*
 * MIT License
 *
 * Copyright (c) 2026
 *
 */

#ifndef WORKOUT_FACE_H_
#define WORKOUT_FACE_H_

/*
 * WORKOUT STOPWATCH face (copy of fast_stopwatch_face renamed)
 */

#include "movement.h"

typedef struct {
    uint32_t timestamp;
    uint32_t elapsed; // elapsed time for the day in hundredths of a second
} workout_day_total_t;

typedef struct {
    workout_day_total_t *data;// the data points
    int length;               // current number of valid entries in the buffer
    int head_index;           // index of the most recent entry (-1 when empty)
    int max;                  // maximum capacity of the buffer
} workout_rolling_buffer_t;   //rolling buffer


typedef struct {
    rtc_counter_t start_counter; // rtc counter when the stopwatch was started
    rtc_counter_t stop_counter;  // rtc counter when the stopwatch was stopped
    uint8_t status;              // the status the stopwatch is in (idle, running, stopped, clear-confirm)
    uint8_t old_status;          // the status the stopwatch was in (idle, running, stopped, clear-confirm)
    int8_t log_index;            // index of the day currently being displayed in the log
    bool clear_yes;              // true when the clear confirmation has been set to yes
    bool sound_enabled;              // true if every 30 seconds there shall be a sound, and also a 30 sec countdown will be shown
    uint32_t sound_second;         // at which second shall the next sound be shown    
    workout_rolling_buffer_t buffer;
    struct {
        rtc_counter_t seconds;
        rtc_counter_t minutes;
        rtc_counter_t hours;
    } old_display;               // the digits currently being displayed on screen
} workout_state_t;

void workout_face_setup(uint8_t watch_face_index, void ** context_ptr);
void workout_face_activate(void *context);
bool workout_face_loop(movement_event_t event, void *context);
void workout_face_resign(void *context);

#define workout_face ((const watch_face_t){ \
    workout_face_setup, \
    workout_face_activate, \
    workout_face_loop, \
    workout_face_resign, \
    NULL, \
})

#endif // WORKOUT_FACE_H_