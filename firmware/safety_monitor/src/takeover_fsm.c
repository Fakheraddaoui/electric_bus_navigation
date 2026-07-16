#include "takeover_fsm.h"

void sm_init(safety_monitor_t *sm, uint32_t frame_timeout_ms,
             uint32_t max_overruns, uint16_t cpu_limit_x10)
{
    sm->state = SM_NORMAL;
    sm->last_seq = 0;
    sm->last_frame_ms = 0;
    sm->frame_timeout_ms = frame_timeout_ms;
    sm->max_overruns = max_overruns;
    sm->cpu_limit_x10 = cpu_limit_x10;
    sm->seq_stall_count = 0;
    sm->fault_latch = 0;
}

void sm_on_frame(safety_monitor_t *sm, const health_frame_t *h, uint32_t now_ms)
{
    sm->last_frame_ms = now_ms;

    if (h->seq <= sm->last_seq && sm->last_seq != 0) {
        if (++sm->seq_stall_count >= 3)
            sm->fault_latch |= FAULT_SEQ_STALL;   /* primary frozen but timer alive */
    } else {
        sm->seq_stall_count = 0;
    }
    sm->last_seq = h->seq;

    if (h->task_overruns > sm->max_overruns) sm->fault_latch |= FAULT_OVERRUNS;
    if (h->cpu_load_pct_x10 > sm->cpu_limit_x10) sm->fault_latch |= FAULT_CPU_OVERLOAD;
    if (h->error_flags & (ERR_ETHERCAT_DOWN | ERR_SENSOR_TIMEOUT))
        sm->fault_latch |= FAULT_PRIMARY_ERROR;
}

sm_state_t sm_tick(safety_monitor_t *sm, uint32_t now_ms, float speed_mps)
{
    const bool frame_lost =
        (now_ms - sm->last_frame_ms) > sm->frame_timeout_ms;
    if (frame_lost) sm->fault_latch |= FAULT_FRAME_TIMEOUT;

    switch (sm->state) {
    case SM_NORMAL:
        if (sm->fault_latch & (FAULT_FRAME_TIMEOUT | FAULT_SEQ_STALL))
            sm->state = SM_TAKEOVER;              /* primary is gone */
        else if (sm->fault_latch)
            sm->state = SM_DEGRADED;
        break;

    case SM_DEGRADED:
        if (sm->fault_latch & (FAULT_FRAME_TIMEOUT | FAULT_SEQ_STALL))
            sm->state = SM_TAKEOVER;
        else if (sm->fault_latch & (FAULT_OVERRUNS | FAULT_PRIMARY_ERROR))
            sm->state = SM_CONTROLLED_STOP;       /* primary alive: ask it to stop */
        break;

    case SM_CONTROLLED_STOP:
        if (frame_lost) sm->state = SM_TAKEOVER;
        if (speed_mps < 0.1f) sm->state = SM_SAFE_STOPPED;
        break;

    case SM_TAKEOVER:
        if (speed_mps < 0.1f) sm->state = SM_SAFE_STOPPED;
        break;

    case SM_SAFE_STOPPED:
        /* Latched. Requires maintenance reset. */
        break;
    }
    return sm->state;
}

float sm_decel_command(const safety_monitor_t *sm)
{
    switch (sm->state) {
    case SM_CONTROLLED_STOP: return -1.2f;  /* comfortable */
    case SM_TAKEOVER:        return -2.0f;  /* firm but stable */
    case SM_SAFE_STOPPED:    return -0.5f;  /* hold */
    default:                 return 0.0f;
    }
}

bool sm_monitor_has_authority(const safety_monitor_t *sm)
{
    return sm->state == SM_TAKEOVER || sm->state == SM_SAFE_STOPPED;
}
