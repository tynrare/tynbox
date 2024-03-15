#include "include/tynstage_template.h"
#include <stddef.h>
#include <stdlib.h>

static void _dispose(TynstageTemplate0State* state);
static STAGEFLAG _step(TynstageTemplate0State* state, STAGEFLAG flags);
static void _draw(TynstageTemplate0State* state);

TynstageTemplate0State* TynstageTemplate0Init(TynStage* stage) {
    TynstageTemplate0State* state = malloc(sizeof(TynstageTemplate0State));
    if (state == NULL) {
        return NULL;
    }

    stage->state = state;
    stage->frame =
        (TynFrame){ &_dispose, &_step, &_draw };

    return stage->state;
}

void _dispose(TynstageTemplate0State* state)
{
}

STAGEFLAG _step(TynstageTemplate0State* state, STAGEFLAG flags)
{
    return flags;
}

void _draw(TynstageTemplate0State* state)
{
}
