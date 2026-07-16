/* A/B slot bookkeeping shared by both MCUs' bootloaders. */
#include <stdint.h>
#include <stdbool.h>

typedef enum { SLOT_A = 0, SLOT_B = 1 } slot_t;

typedef struct {
    uint32_t magic;
    slot_t   active;
    uint32_t version[2];
    uint32_t attempts[2];
    bool     confirmed[2];
    bool     pending[2];
} slot_meta_t;

#define SLOT_MAGIC 0xAB0752A6u
#define MAX_ATTEMPTS 3u

slot_t slot_select_boot(slot_meta_t *m)
{
    if (m->magic != SLOT_MAGIC) {            /* first boot: default A */
        m->magic = SLOT_MAGIC;
        m->active = SLOT_A;
        m->confirmed[SLOT_A] = true;
    }

    const slot_t other = (m->active == SLOT_A) ? SLOT_B : SLOT_A;

    if (m->pending[other]) {                 /* an update is staged */
        m->pending[other] = false;
        m->attempts[other] = 0;
        m->active = other;
    }

    if (!m->confirmed[m->active]) {
        if (++m->attempts[m->active] > MAX_ATTEMPTS) {
            m->active = other;               /* rollback */
            m->attempts[other] = 0;
        }
    }
    return m->active;
}

void slot_confirm(slot_meta_t *m)
{
    m->confirmed[m->active] = true;
    m->attempts[m->active] = 0;
}
