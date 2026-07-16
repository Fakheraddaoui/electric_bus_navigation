#ifndef TAKEOVER_FSM_H
#define TAKEOVER_FSM_H
/*
 * Safety-monitor takeover state machine (runs on the independent MCU).
 * Watches the primary's health frames; on fault, commands a controlled stop
 * and (if the primary is unresponsive) takes over actuator authority.
 */
#include <stdint.h>
#include <stdbool.h>
#include "health_report.h"

typedef enum {
    SM_NORMAL = 0,        /* primary healthy, monitor passive */
    SM_DEGRADED,          /* soft fault: warn, tighter limits */
    SM_CONTROLLED_STOP,   /* commanding smooth deceleration to standstill */
    SM_TAKEOVER,          /* primary dead: monitor owns actuators */
    SM_SAFE_STOPPED       /* at standstill, parking brake applied */
} sm_state_t;

typedef struct {
    sm_state_t state;
    uint32_t   last_seq;
    uint32_t   last_frame_ms;
    uint32_t   frame_timeout_ms;
    uint32_t   max_overruns;
    uint16_t   cpu_limit_x10;
    uint32_t   seq_stall_count;
    uint32_t   fault_latch;      /* bitmask of observed faults */
} safety_monitor_t;

#define FAULT_FRAME_TIMEOUT (1u << 0)
#define FAULT_SEQ_STALL     (1u << 1)
#define FAULT_OVERRUNS      (1u << 2)
#define FAULT_CPU_OVERLOAD  (1u << 3)
#define FAULT_PRIMARY_ERROR (1u << 4)

void sm_init(safety_monitor_t *sm, uint32_t frame_timeout_ms,
             uint32_t max_overruns, uint16_t cpu_limit_x10);
/* Feed each received health frame */
void sm_on_frame(safety_monitor_t *sm, const health_frame_t *h, uint32_t now_ms);
/* Periodic tick (10 ms). speed_mps from independent sensor. Returns state. */
sm_state_t sm_tick(safety_monitor_t *sm, uint32_t now_ms, float speed_mps);
/* Deceleration command the monitor outputs when it has authority */
float sm_decel_command(const safety_monitor_t *sm);
bool  sm_monitor_has_authority(const safety_monitor_t *sm);

#endif
