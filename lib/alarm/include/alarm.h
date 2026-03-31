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
     const char *co2_alarm_level; /**< CO2 alarm level (policy). */ 
 } alarm_local_state_t;

/**
 * @brief CO2 alarm levels.
 */
#define CO2_LEVEL_OPTIMAL    "optimal"
#define CO2_LEVEL_GOOD       "good"
#define CO2_LEVEL_MODERATE   "moderate"
#define CO2_LEVEL_POOR       "poor"
#define CO2_LEVEL_VERY_POOR  "very poor"
#define CO2_LEVEL_CRITICAL   "critical"

/**
 * @brief System alarm logic.
 * @param[in] TAG Log tag.
 * @param[in] state_copy Device state copy.
 * @param[in,out] local_state Local state.
 */
void systemAlarmLogic(const char *TAG, const device_state_t *state_copy, alarm_local_state_t *local_state);

#endif /* ALARM_H */