#include "health_report.h"
#include <string.h>

static uint16_t crc16(const uint8_t *d, size_t n)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < n; i++) {
        crc ^= (uint16_t)d[i] << 8;
        for (int b = 0; b < 8; b++)
            crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
    }
    return crc;
}

static void wr16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void wr32(uint8_t *p, uint32_t v) { wr16(p, (uint16_t)v); wr16(p + 2, (uint16_t)(v >> 16)); }
static uint16_t rd16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static uint32_t rd32(const uint8_t *p) { return (uint32_t)rd16(p) | ((uint32_t)rd16(p + 2) << 16); }

size_t health_encode(const health_frame_t *h, uint8_t *buf, size_t len)
{
    if (len < HEALTH_WIRE_SIZE) return 0;
    wr32(&buf[0], h->seq);
    wr32(&buf[4], h->task_overruns);
    wr16(&buf[8], h->cpu_load_pct_x10);
    wr16(&buf[10], h->error_flags);
    wr16(&buf[12], (uint16_t)h->accel_cmd_mm_s2);
    wr16(&buf[14], (uint16_t)h->steer_cmd_centideg);
    wr16(&buf[16], crc16(buf, 16));
    return HEALTH_WIRE_SIZE;
}

bool health_decode(const uint8_t *buf, size_t len, health_frame_t *out)
{
    if (len < HEALTH_WIRE_SIZE) return false;
    if (crc16(buf, 16) != rd16(&buf[16])) return false;
    out->seq = rd32(&buf[0]);
    out->task_overruns = rd32(&buf[4]);
    out->cpu_load_pct_x10 = rd16(&buf[8]);
    out->error_flags = rd16(&buf[10]);
    out->accel_cmd_mm_s2 = (int16_t)rd16(&buf[12]);
    out->steer_cmd_centideg = (int16_t)rd16(&buf[14]);
    out->crc = rd16(&buf[16]);
    return true;
}
