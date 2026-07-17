/*
 * MIT License
 *
 * Copyright (c) 2025 Mitch42
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
#include <math.h>
#include "temperature_prediction_face.h"

// Default initial values
#define TEMPERATURE_PREDICTION_DEFAULT_COEFFICIENT 0.002F
#define TEMPERATURE_PREDICTION_DEFAULT_BUFFER_SIZE 60
#define TEMPERATURE_PREDICTION_DEFAULT_AVERAGE_COUNT 30

// Constants for showing the predicted temperature
#define TEMPERATURE_PREDICTION_BUFFER_SIZE_MAX 120 //todo: check how many kb this takes
#define TEMPERATURE_PREDICTION_AVERAGING_MAX 90
#define TEMPERATURE_PREDICTION_AVERAGING_ERROR 60

// Constants for coefficient calculation
#define TEMPERATURE_PREDICTION_CALCULATION_EQUILIBRIUM_MINUTES 5 //defines how long the temperature shall be constant to determine that equilibrium has been reached.
#define TEMPERATURE_PREDICTION_CALCULATION_EQUILIBRIUM_TRESHOLD 0.1 //defines the maximum allowed temperature deviation to determine that equilibrium has been reached.
#define TEMPERATURE_PREDICTION_CALCULATION_IGNORE_START_MINUTES 5 //if you start coefficient calculation, the watch is waiting for n minutes to ensure that the temperature flow is stable
#define TEMPERATURE_PREDICTION_CALCULATION_END_TEMP_DELTA 2.0 //ignore everything below (end_temp - 2.0), as the temperature is too close to the end temperature, making the temperature flow become unstable

// int debug_index = 0;
// float debug_data[] = { 29.2, 29.2, 29.2, 29.2, 29.2, 29.2, 29.1, 29.1, 29.1, 29.1, 29.1, 29.1, 29.1, 29.1, 29.1, 29.1, 29.1, 29.0, 29.0, 29.0, 29.0, 29.0, 29.0, 29.0, 29.0, 29.0, 29.0, 28.9, 28.9, 28.9, 28.9, 28.9, 28.9, 28.9, 28.9, 28.9, 28.9, 28.8, 28.8, 28.8, 28.8, 28.8, 28.8, 28.8, 28.8, 28.8, 28.8, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.6, 28.6, 28.6, 28.6, 28.6, 28.6, 28.6, 28.6, 28.6, 28.6, 28.5, 28.5, 28.5, 28.5, 28.5, 28.5, 28.5, 28.5, 28.5, 28.5, 28.4, 28.4, 28.4, 28.4, 28.4, 28.4, 28.3, 28.3, 28.3, 28.3, 28.3, 28.3, 28.3, 28.3, 28.2, 28.2, 28.2, 28.2, 28.2, 28.2, 28.2, 28.2, 28.2, 28.2, 28.1, 28.1, 28.1, 28.1, 28.1, 28.1, 28.1, 28.1, 28.1, 28.1, 28.1, 28.0, 28.0, 28.0, 28.0, 28.0, 28.0, 28.0, 28.0, 28.0, 28.0, 28.0, 28.0, 27.9, 27.9, 27.9, 27.9, 27.9, 27.9, 27.9, 27.9, 27.9, 27.9, 27.9, 27.8, 27.8, 27.8, 27.8, 27.8, 27.8, 27.8, 27.8, 27.8, 27.8, 27.8, 27.8, 27.7, 27.7, 27.7, 27.7, 27.7, 27.7, 27.7, 27.7, 27.7, 27.7, 27.7, 27.6, 27.6, 27.6, 27.6, 27.6, 27.6, 27.6, 27.6, 27.6, 27.6, 27.6, 27.5, 27.5, 27.5, 27.5, 27.5, 27.5, 27.5, 27.5, 27.5, 27.5, 27.5, 27.4, 27.4, 27.4, 27.4, 27.4, 27.4, 27.4, 27.4, 27.4, 27.4, 27.4, 27.4, 27.4, 27.3, 27.3, 27.3, 27.3, 27.3, 27.3, 27.3, 27.2, 27.2, 27.2, 27.2, 27.2, 27.2, 27.2, 27.2, 27.2, 27.2, 27.2, 27.2, 27.2, 27.1, 27.1, 27.1, 27.1, 27.1, 27.1, 27.1, 27.1, 27.1, 27.1, 27.1, 27.1, 27.1, 27.1, 27.0, 27.0, 27.0, 27.0, 27.0, 27.0, 27.0, 27.0, 27.0, 27.0, 27.0, 27.0, 27.0, 27.0, 26.9, 26.9, 26.9, 26.9, 26.9, 26.9, 26.9, 26.9, 26.9, 26.9, 26.9, 26.9, 26.9, 26.8, 26.8, 26.8, 26.8, 26.8, 26.8, 26.8, 26.8, 26.8, 26.8, 26.8, 26.8, 26.8, 26.8, 26.7, 26.7, 26.7, 26.7, 26.7, 26.7, 26.7, 26.7, 26.7, 26.7, 26.7, 26.7, 26.7, 26.7, 26.7, 26.6, 26.6, 26.6, 26.6, 26.6, 26.6, 26.6, 26.6, 26.6, 26.6, 26.6, 26.6, 26.6, 26.6, 26.5, 26.5, 26.5, 26.5, 26.5, 26.5, 26.5, 26.5, 26.5, 26.5, 26.5, 26.5, 26.5, 26.5, 26.5, 26.4, 26.4, 26.4, 26.4, 26.4, 26.4, 26.4, 26.4, 26.4, 26.4, 26.4, 26.4, 26.4, 26.4, 26.4, 26.3, 26.3, 26.3, 26.3, 26.3, 26.3, 26.3, 26.3, 26.3, 26.3, 26.3, 26.3, 26.3, 26.3, 26.3, 26.3, 26.3, 26.2, 26.2, 26.2, 26.2, 26.2, 26.2, 26.2, 26.2, 26.2, 26.2, 26.2, 26.2, 26.2, 26.2, 26.2, 26.2, 26.2, 26.1, 26.1, 26.1, 26.1, 26.1, 26.1, 26.1, 26.1, 26.1, 26.1, 26.1, 26.1, 26.1, 26.1, 26.1, 26.1, 26.0, 26.0, 26.0, 26.0, 26.0, 26.0, 26.0, 26.0, 26.0, 26.0, 26.0, 26.0, 26.0, 25.9, 25.9, 25.9, 25.9, 25.9, 25.9, 25.9, 25.9, 25.9, 25.9, 25.9, 25.9, 25.9, 25.9, 25.9, 25.9, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.8, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.7, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.6, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.5, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.4, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.3, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.2, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.1, 25.0, 25.1, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.9, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.8, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.7, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.6, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.5, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.4, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.3, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.2, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.1, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 24.0, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.9, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.8, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.7, 23.6, 23.7, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.6, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.5, 23.4, 23.5, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.4, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.3, 23.2, 23.3, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.2, 23.1, 23.2, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.1, 23.0, 23.1, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 23.0, 22.9, 23.0, 23.0, 23.0, 22.9, 22.9, 22.9, 22.9, 23.0, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.9, 22.8, 22.9, 22.9, 22.9, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.7, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.8, 22.7, 22.7, 22.7, 22.8, 22.8, 22.7, 22.7, 22.7, 22.8, 22.7 };

// float movement_get_temperature(void) {
//     float ret= debug_data[debug_index];
//     debug_index = (debug_index + 1) % 2168;
//     return ret;
// }

/// @brief reset a rolling buffer
/// @param usable_length maximum number of entries the buffer can hold
static void temperature_prediction_face_init_rolling_buffer(temperature_prediction_rolling_buffer_t *buffer, int usable_length) {
    buffer->head_index = -1;
    buffer->length = 0;
    buffer->max = usable_length;
}

/// @brief add a value to a rolling buffer
static void temperature_prediction_face_add_to_rolling_buffer(temperature_prediction_rolling_buffer_t *buffer, float value) {
    buffer->head_index = (buffer->head_index + 1) % buffer->max;
    buffer->length = buffer->length + 1 < buffer->max ? buffer->length + 1 : buffer->max;
    buffer->data[buffer->head_index] = value;
}

/// @brief calculate end temperature with a closed-form least squares solution of Newton's law of cooling, performing a linear regression on the linearized exponential temperature curve
static float temperature_prediction_face_calculate_end_temperature(temperature_prediction_rolling_buffer_t *buffer, float coefficient) {
    const float alpha = expf(-coefficient); // dt = 1s
    float numerator = 0.0f;
    float denominator = 0.0f;
    float e = 1.0f;
    uint16_t index = (buffer->head_index + 1) % buffer->length;
    const float t0 = buffer->data[index];
    
    for (uint16_t i = 0; i < buffer->length; i++) {
        const float w = 1.0f - e;
        numerator   += w * (buffer->data[index] - e * t0);
        denominator += w * w;
        e *= alpha;
        index = (index + 1) % buffer->length;
    }
    return denominator <= 0.0f ? t0 : (numerator / denominator);
}

static void temperature_prediction_face_calculate_end_temperature_ema(temperature_prediction_state_t *state) {
    if (state->buffer.length > 1) {
        float end_temperature = temperature_prediction_face_calculate_end_temperature(&state->buffer, state->coefficient);
        float alpha = 2.0f / (state->average_count + 1); // Alpha = 2 / (N + 1), where N is the number of periods
        state->ema = state->ema == -999 ? end_temperature : ((end_temperature * alpha) + (state->ema * (1 - alpha))); //EMA = (Value * Alpha) + (EMA_Before * (1 - Alpha))
        temperature_prediction_face_add_to_rolling_buffer(&state->calculated_averages, state->ema);
    }
}

/// @brief calculate temperature deviation
static float temperature_correction_face_calculate_error(temperature_prediction_state_t *state) {
    if (state->calculated_averages.length < 1) {
        return -1; 
    }
    float min = state->calculated_averages.data[0];
    float max = state->calculated_averages.data[0];
    for (int i = 1; i < state->calculated_averages.length; i++) {
        if (state->calculated_averages.data[i] < min) min = state->calculated_averages.data[i]; // Update minimum
        if (state->calculated_averages.data[i] > max) max = state->calculated_averages.data[i]; // Update maximum
    }
    return max - min;
}      

/// @brief calculate heat transfer coefficient of Newtons law of cooling, using all points and performing a linear regression of the transformed logarithmic curve
static float temperature_prediction_face_calculate_coefficient_with_linear_regression(temperature_prediction_rolling_buffer_t *buffer, int end_temp_cnt, int ignore_start_cnt, float ignore_delta_temp) {
    //determine end temperature
    float temp_end = 0;
    for (int i = buffer->length - end_temp_cnt; i < buffer->length; i++) {
        temp_end += buffer->data[i];
    }
    temp_end /= end_temp_cnt;

    //calculate values for linear regression, formula is: -m=(nΣxy-ΣxΣy)/(mΣx²-(Σx)²)
    float sum_x = 0;
    float sum_y = 0;
    float sum_xx = 0;
    float sum_xy = 0;
    int n = 0; 
    for (int i = ignore_start_cnt; i < buffer->length; i++) {
        if (fabs(buffer->data[i] - temp_end) <= ignore_delta_temp) {
            break; //temperature is near end temperature, becoming unstable
        } else {
            float x = (i - ignore_start_cnt) * 60.0; //x = delta time in seconds
            float y = logf(buffer->data[i] - temp_end); //y = ln(T - Tend)
            sum_x += x;
            sum_y += y;
            sum_xx += x * x;
            sum_xy += x * y;
            n++;
        }
    }

    if (n * sum_xx - sum_x * sum_x == 0) return 0;
    else return (float)(-((n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x)));
}

/// @brief check if max-min of the last n values of buffer is <= TEMPERATURE_PREDICTION_CALCULATION_EQUILIBRIUM_TRESHOLD, with n = TEMPERATURE_PREDICTION_CALCULATION_EQUILIBRIUM_MINUTES
static bool temperature_prediction_face_equilibrium_reached(temperature_prediction_rolling_buffer_t *buffer) {
    //we know that buffer->length < buffer->max, and also that the buffer has at least TEMPERATURE_PREDICTION_CALCULATION_EQUILIBRIUM_MINUTES values
    float min = buffer->data[buffer->length - TEMPERATURE_PREDICTION_CALCULATION_EQUILIBRIUM_MINUTES];
    float max = buffer->data[buffer->length - TEMPERATURE_PREDICTION_CALCULATION_EQUILIBRIUM_MINUTES];
    for (int i = buffer->length - TEMPERATURE_PREDICTION_CALCULATION_EQUILIBRIUM_MINUTES + 1; i < buffer->length; i++) {
        if (buffer->data[i] < min) min = buffer->data[i]; // Update minimum
        if (buffer->data[i] > max) max = buffer->data[i]; // Update maximum
    }
    return max - min <= TEMPERATURE_PREDICTION_CALCULATION_EQUILIBRIUM_TRESHOLD;
}

/// @brief show the coefficient at the bottom line
static void temperature_prediction_face_display_coefficient(float coeff_f) {
    watch_display_text(WATCH_POSITION_BOTTOM, "      ");
    char buf[8];
    int coeff = (int)(coeff_f * 100000 + 0.5); // 0.0013240584 -> 000132
    sprintf(buf, "%06d", coeff); 
    watch_display_text(WATCH_POSITION_BOTTOM, buf);
}

/// @brief display a temperature
/// @param temperature_c temperature in Celsius
/// @param in_fahrenheit true to display in Fahrenheit, false to display Celsius
static void temperature_prediction_face_display_temperature(float temperature_c ) {
    if (movement_use_imperial_units()) watch_display_float_with_best_effort(temperature_c * 1.8 + 32.0, "#F");
    else watch_display_float_with_best_effort(temperature_c, "#C");
}

static uint8_t temperature_prediction_face_get_next_settings_state(temperature_prediction_state_t *state) {
    uint8_t next_state = state->settings_state + 1;
    return next_state;
}

/// @brief display current settings on the watch face
/// @param state face state containing settings values
/// @param subsecond current subsecond value used for blink timing
static void temperature_prediction_face_display_settings(temperature_prediction_state_t *state, uint8_t subsecond) {
    char buf[8];
    watch_display_text_with_fallback(WATCH_POSITION_BOTTOM, "      ", "      ");
    watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");

    switch (state->settings_state) {
        case 0:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "BUF", "BU");
            sprintf(buf, "%6d", state->buffer_size);
            if (subsecond % 2) watch_display_text(WATCH_POSITION_BOTTOM, buf);
            else watch_display_text(WATCH_POSITION_BOTTOM, "      ");
            break;
        case 1:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "AVG", "AV");
            sprintf(buf, "%6d", state->average_count);
            if (subsecond % 2) watch_display_text(WATCH_POSITION_BOTTOM, buf);
            else watch_display_text(WATCH_POSITION_BOTTOM, "      ");
            break;
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "COE", "CO");
            temperature_prediction_face_display_coefficient(state->coefficient);
            if (subsecond % 2) 
                watch_display_string(" ", state->settings_state + 2);
            break;
        case 8:
            watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "COE", "CO");
            watch_display_text(WATCH_POSITION_BOTTOM, "CALC  ");
            if (subsecond % 2) 
                watch_display_text(WATCH_POSITION_SECONDS, state->start_coefficient_calculation ? " y" : " n");
            break;
        default:
            break;
    }
}

/// @brief advance the active setting value up or down
/// @param state face state containing the selected setting
/// @param forward true to increment, false to decrement
static void temperature_prediction_face_advance_settings(temperature_prediction_state_t *state, bool forward) {
    int coeff;
    int step;
    
    switch (state->settings_state) {
        case 0:
            if (forward) state->buffer_size = state->buffer_size + 1 > TEMPERATURE_PREDICTION_BUFFER_SIZE_MAX ? 2 : state->buffer_size + 1;
            else state->buffer_size = state->buffer_size - 1 < 2 ? TEMPERATURE_PREDICTION_BUFFER_SIZE_MAX : state->buffer_size - 1;
            break;
        case 1:
            if (forward) state->average_count = state->average_count + 1 > TEMPERATURE_PREDICTION_AVERAGING_MAX ? 1 : state->average_count + 1;
            else state->average_count = state->average_count - 1 < 1 ? TEMPERATURE_PREDICTION_AVERAGING_MAX : state->average_count - 1;
            break;
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            //set coefficient, increasing or decreasing one digit at a time, with wrap-around
            coeff = (int)(state->coefficient * 100000 + 0.5); // 0.0013240584 -> 000132
            step = 1;
            for (int i = 0; i < 7 - state->settings_state; i++) step *= 10;
            coeff += forward ? ((coeff / step) % 10 == 9 ? -9 * step : step) : ((coeff / step) % 10 == 0 ? 9 * step : -step);
            state->coefficient = ((float)coeff) / 100000;
            break;
        case 8:
            state->start_coefficient_calculation = !state->start_coefficient_calculation;
            break;
        default:
            break;
    }
}

/// @brief update WATCH_POSITION_TOP_RIGHT and WATCH_POSITION_BOTTOM according to state (running / coefficient calculation)
static void temperature_prediction_face_update_display(temperature_prediction_state_t *state, float temperature) {
    char buf[8];

    if (state->mode == temperature_prediction_coefficient) {         
        //WATCH_POSITION_BOTTOM
        if (state->show_real_temperature) {
            temperature_prediction_face_display_temperature(temperature == -999 ? movement_get_temperature() : temperature);
        } else {
            watch_display_text(WATCH_POSITION_BOTTOM, "COEFF ");    
        }

        //WATCH_POSITION_TOP_RIGHT   
        sprintf(buf, "%2d", state->buffer.length);   
        watch_display_text(WATCH_POSITION_TOP_RIGHT, buf);
    } else if (state->mode == temperature_prediction_running) {
        //WATCH_POSITION_BOTTOM
        if (temperature != -999) {
            temperature_prediction_face_display_temperature(temperature);
        } else if (state->show_real_temperature || state->ema == -999) {
            temperature_prediction_face_display_temperature(movement_get_temperature());
        } else {
            temperature_prediction_face_display_temperature(state->ema);
        }

        //WATCH_POSITION_TOP_RIGHT   
        if (state->show_real_temperature) {
            watch_display_text(WATCH_POSITION_TOP_RIGHT, "  ");
        } else {   
            float error = temperature_correction_face_calculate_error(state);          
            if (error < 1.0) {
                int err = (int)(error * 10.0);
                sprintf(buf, ",%1d", err);   
            } else {
                int err = (int)(error + 0.5);
                sprintf(buf, "%2d", err);   
            }
            watch_display_text(WATCH_POSITION_TOP_RIGHT, buf);
        }
    } 
}

/// @brief update WATCH_POSITION_TOP_LEFT according to state (running+temp / running+calc / coefficient calculation)
static void temperature_prediction_face_update_display_top_left(temperature_prediction_state_t *state) {
    if (state->mode == temperature_prediction_coefficient) {
        watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "COE", "CO");
    } else if (state->show_real_temperature) {
        watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "TEM", "TE");
    } else {
        watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "EST", "ET");
    }
}

/// @brief (re)start temperature logging or coefficient calculation
static void temperature_prediction_face_start_logging(temperature_prediction_state_t *state, bool coefficient_calculation) {
    if (coefficient_calculation) {
        temperature_prediction_face_init_rolling_buffer(&state->buffer, TEMPERATURE_PREDICTION_BUFFER_SIZE_MAX);
        state->show_real_temperature = false;
        state->mode = temperature_prediction_coefficient;
    } else {
        temperature_prediction_face_init_rolling_buffer(&state->buffer, state->buffer_size);
        temperature_prediction_face_init_rolling_buffer(&state->calculated_averages, TEMPERATURE_PREDICTION_AVERAGING_ERROR);
        state->ema = -999;
        state->mode = temperature_prediction_running;
    }
    temperature_prediction_face_update_display_top_left(state);
    temperature_prediction_face_update_display(state, -999); 
}

/// @brief stop logging of temperatures
static void temperature_prediction_face_stop_logging(temperature_prediction_state_t *state) {
    state->mode = temperature_prediction_waiting;
}

void temperature_prediction_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;

    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(temperature_prediction_state_t));
        memset(*context_ptr, 0, sizeof(temperature_prediction_state_t));

        temperature_prediction_state_t *state = (temperature_prediction_state_t *)*context_ptr;       
        state->buffer.data = malloc(TEMPERATURE_PREDICTION_BUFFER_SIZE_MAX * sizeof(float));
        state->calculated_averages.data = malloc(TEMPERATURE_PREDICTION_AVERAGING_ERROR * sizeof(float));
        
        state->coefficient = TEMPERATURE_PREDICTION_DEFAULT_COEFFICIENT;
        state->buffer_size = TEMPERATURE_PREDICTION_DEFAULT_BUFFER_SIZE;
        state->average_count = TEMPERATURE_PREDICTION_DEFAULT_AVERAGE_COUNT;
        state->show_real_temperature = true;
    }
}

void temperature_prediction_face_activate(void *context) {
    movement_request_tick_frequency(1);
}

bool temperature_prediction_face_loop(movement_event_t event, void *context) {
    temperature_prediction_state_t *state = (temperature_prediction_state_t *)context;

    switch (event.event_type) {
        case EVENT_ACTIVATE: 
            temperature_prediction_face_start_logging(state, false);
            break;
        case EVENT_LIGHT_BUTTON_DOWN:
            switch (state->mode) {
                case temperature_prediction_running:
                case temperature_prediction_coefficient:              
                case temperature_prediction_waiting:
                    movement_illuminate_led();
                    break;
                case temperature_prediction_setting: // flip through settings
                    state->settings_state = temperature_prediction_face_get_next_settings_state(state);
                    temperature_prediction_face_display_settings(state, event.subsecond);
                    if (state->settings_state > 8) { //last setting
                        movement_request_tick_frequency(1);
                        temperature_prediction_face_start_logging(state, state->start_coefficient_calculation);
                    }
                    break;
            }
            break;
        // case EVENT_LIGHT_BUTTON_UP:
        //     if(state->debug) {
        //         temperature_prediction_face_start_logging(state); //restart logging for debug only! TODO: make restart availabe for normal user?
        //     }
        //     break;
        case EVENT_LIGHT_LONG_PRESS:
            switch (state->mode) {
                case temperature_prediction_coefficient:    
                    break; //no settings in coefficient, as exiting settings starts temperature_prediction_running
                case temperature_prediction_waiting: //fallthrough
                case temperature_prediction_running: //enter settings
                    temperature_prediction_face_stop_logging(state);
                    state->mode = temperature_prediction_setting;
                    state->settings_state = 0;
                    state->start_coefficient_calculation = false;
                    movement_request_tick_frequency(4); // we need to blink in settings
                    temperature_prediction_face_display_settings(state, event.subsecond);
                    break;
                case temperature_prediction_setting: //exit settings
                    movement_request_tick_frequency(1);
                    temperature_prediction_face_start_logging(state, state->start_coefficient_calculation);
                    break;
            }
            break;
        case EVENT_ALARM_BUTTON_UP: 
            switch (state->mode) {
                case temperature_prediction_waiting: // start logging
                    temperature_prediction_face_start_logging(state, false);
                    break;
                case temperature_prediction_coefficient: //fallthrough
                case temperature_prediction_running: //toggle "show real temp"
                    state->show_real_temperature = !state->show_real_temperature;
                    temperature_prediction_face_update_display_top_left(state);
                    temperature_prediction_face_update_display(state, -999);
                    break;
                case temperature_prediction_setting:
                    temperature_prediction_face_advance_settings(state, true); //TODO: settings only in one direction (forward), but with fast mode
                    temperature_prediction_face_display_settings(state, event.subsecond);
                    break;
            }
            break;
        case EVENT_ALARM_LONG_PRESS:
            switch (state->mode) {
                case temperature_prediction_coefficient: // stop logging
                    temperature_prediction_face_start_logging(state, false);
                    break;
                case temperature_prediction_setting:
                    temperature_prediction_face_advance_settings(state, false);
                    temperature_prediction_face_display_settings(state, event.subsecond);
                    break;
                case temperature_prediction_running: //toggle Celsius and Fahrenheit
                    movement_set_use_imperial_units(!movement_use_imperial_units());
                    temperature_prediction_face_update_display(state, -999);
                    break;
                case temperature_prediction_waiting: 
                    break;
            }
            break;
        case EVENT_TICK:
            switch (state->mode) {
                case temperature_prediction_waiting:
                    break;
                case temperature_prediction_running: 
                    if(watch_rtc_get_date_time().unit.second % 2) watch_clear_indicator(WATCH_INDICATOR_SIGNAL);
                    else watch_set_indicator(WATCH_INDICATOR_SIGNAL);       

                    temperature_prediction_face_add_to_rolling_buffer(&state->buffer, movement_get_temperature()); 
                    temperature_prediction_face_calculate_end_temperature_ema(state);
                    temperature_prediction_face_update_display(state, state->show_real_temperature || state->ema == -999 ? state->buffer.data[state->buffer.head_index] : state->ema);
                    break;
                case temperature_prediction_setting: 
                    temperature_prediction_face_display_settings(state, event.subsecond);
                    break;
                case temperature_prediction_coefficient:
                        if(watch_rtc_get_date_time().unit.second % 2) watch_set_indicator(WATCH_INDICATOR_SIGNAL);
                        else watch_clear_indicator(WATCH_INDICATOR_SIGNAL);              
                        float temperature= movement_get_temperature();

                        if (watch_rtc_get_date_time().unit.second == 0) { //once a minute
                            temperature_prediction_face_add_to_rolling_buffer(&state->buffer, temperature);
                            if (state->buffer.length == state->buffer.max) { //buffer full
                                temperature_prediction_face_stop_logging(state);
                                watch_display_text_with_fallback(WATCH_POSITION_BOTTOM, "FULL  ", " FULL ");
                                break;
                            } else if (state->buffer.length >= TEMPERATURE_PREDICTION_CALCULATION_EQUILIBRIUM_MINUTES) { //only after n minutes
                                if (temperature_prediction_face_equilibrium_reached(&state->buffer)) { //temperature is stable: stop calculation
                                    state->coefficient = temperature_prediction_face_calculate_coefficient_with_linear_regression(&state->buffer, TEMPERATURE_PREDICTION_CALCULATION_EQUILIBRIUM_MINUTES, TEMPERATURE_PREDICTION_CALCULATION_IGNORE_START_MINUTES, TEMPERATURE_PREDICTION_CALCULATION_END_TEMP_DELTA);
                                    //coefficient must be positive and shall only have 1 integer place and 5 decimal places, otherwise we can not properly show it 
                                    state->coefficient = roundf(state->coefficient * 100000.0f) / 100000.0f;
                                    if (state->coefficient < 0) {
                                        state->coefficient = 0;
                                    } else if (state->coefficient > 9.99999) {
                                        state->coefficient = 9.99999;
                                    }
                                    temperature_prediction_face_stop_logging(state);
                                    temperature_prediction_face_display_coefficient(state->coefficient);
                                    break;
                                }
                            }
                        }
                        temperature_prediction_face_update_display(state, temperature);
                    break;
            }
            break;
        case EVENT_TIMEOUT:
            //no timeout
            break;
        default:
            return movement_default_loop_handler(event);
    }

    return true;
}

void temperature_prediction_face_resign(void *context) {
    (void) context;

    // handle any cleanup before your watch face goes off-screen.
}

