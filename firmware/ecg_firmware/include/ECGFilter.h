// ECGFilter.h
// This header file defines the ECGFilter class for applying a simple moving average filter to ECG data.

#ifndef ECG_FILTER_H
#define ECG_FILTER_H

#include <Arduino.h>
#include <deque>

/**
 * @brief A class to apply a simple moving average filter to a stream of integer data.
 *
 * This filter helps smooth out noisy signals by averaging a specified number of
 * recent samples. It's suitable for basic noise reduction in sensor readings.
 */
class ECGFilter {
public:
    /**
     * @brief Constructor for the ECGFilter class.
     * @param windowSize The number of samples to include in the moving average calculation.
     * A larger window provides more smoothing but introduces more lag.
     * Minimum windowSize is 1.
     */
    ECGFilter(unsigned int windowSize);

    /**
     * @brief Adds a new raw data point to the filter and returns the smoothed (filtered) value.
     * The oldest sample is discarded if the buffer is full.
     * @param rawValue The new raw integer data point to filter.
     * @return The filtered (averaged) integer value.
     */
    int filter(int rawValue);

    /**
     * @brief Clears all samples from the filter's internal buffer.
     * This should be called if there's a discontinuity in the data or
     * when starting a new reading session to prevent old samples from affecting new ones.
     */
    void clear();

private:
    unsigned int _windowSize;      // Size of the moving average window
    std::deque<int> _buffer;       // Deque to store samples for the moving average
    long _currentSum;              // Sum of values currently in the buffer (use long to prevent overflow)
};

// // ECGFilter.cpp
// // This file implements the methods defined in the ECGFilter class.

// // #include "ECGFilter.h" // Include the corresponding header file

// ECGFilter::ECGFilter(unsigned int windowSize) : _currentSum(0) {
//     // Ensure minimum window size is 1 to avoid division by zero or empty buffer issues.
//     _windowSize = (windowSize > 0) ? windowSize : 1;
// }

// int ECGFilter::filter(int rawValue) {
//     // Add the new value to the sum.
//     _currentSum += rawValue;
//     // Add the new value to the back of the buffer.
//     _buffer.push_back(rawValue);

//     // If the buffer has exceeded the window size, remove the oldest value from the front.
//     if (_buffer.size() > _windowSize) {
//         _currentSum -= _buffer.front(); // Subtract the value being removed from the sum.
//         _buffer.pop_front();            // Remove the oldest value.
//     }

//     // Calculate the average. If the buffer is empty (shouldn't happen with rawValue added), return 0.
//     if (_buffer.empty()) {
//         return 0;
//     }
//     return _currentSum / _buffer.size(); // Return the calculated moving average.
// }

// void ECGFilter::clear() {
//     _buffer.clear();   // Clear all elements from the deque.
//     _currentSum = 0;   // Reset the sum.
}
