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
    rtc_counter_t start_counter; // rtc counter when the stopwatch was started
    rtc_counter_t stop_counter;  // rtc counter when the stopwatch was stopped
    uint8_t status;              // the status the stopwatch is in (idle, running, stopped)
    bool slow_refresh;           // update the display slowly (same 128Hz timekeeping accuracy)
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