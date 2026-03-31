#ifndef ALARM_H
#define ALARM_H

#include "health.h"

// alarm thresholds
#define MICS5524_ALARM_THRESHOLD 5000 // ppm

/**
 * @brief Working set of system-level flags derived from sensor health (written by @c system_task).
 */
 typedef struct
 {
     bool as312_alarm;     /**< AS312 / motion alarm line (policy). */
     bool mics5524_alarm;  /**< Gas / MiCS alarm line (policy). */
 } alarm_local_state_t;

/**
 * @brief System alarm logic.
 * @param[in] TAG Log tag.
 * @param[in] state_copy Device state copy.
 * @param[in,out] local_state Local state.
 */
void systemAlarmLogic(const char *TAG, const device_state_t *state_copy, alarm_local_state_t *local_state);

#endif /* ALARM_H */