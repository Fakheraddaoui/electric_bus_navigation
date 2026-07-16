#ifndef HEALTH_REPORT_H
#define HEALTH_REPORT_H
/* Health frame the primary sends to the safety monitor every 10 ms. */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    uint32_t seq;               /* must increment monotonically */
    uint32_t task_overruns;     /* control loop deadline misses */
    uint16_t cpu_load_pct_x10;  /* 0..1000 */
    uint16_t error_flags;
    int16_t  accel_cmd_mm_s2;
    int16_t  steer_cmd_centideg;
    uint16_t crc;
} health_frame_t;

#define HEALTH_WIRE_SIZE 18u
#define ERR_ETHERCAT_DOWN   (1u << 0)
#define ERR_SENSOR_TIMEOUT  (1u << 1)
#define ERR_CONTROL_SAT     (1u << 2)

size_t health_encode(const health_frame_t *h, uint8_t *buf, size_t len);
bool   health_decode(const uint8_t *buf, size_t len, health_frame_t *out);

#endif
