#include <glib.h>
#include <stdlib.h>
#include <stdbool.h>

#include "utils/errors.h"
#include "utils/log.h"

#include "netsurf/misc.h"

/** Killable callback closure embodiment. */
typedef struct {
        void (*callback)(void *);       /**< The callback function. */
        void *context;                  /**< The context for the callback. */
        bool callback_killed;           /**< Whether or not this was killed. */
} _nsgtk_callback_t;

/** List of callbacks which have occurred and are pending running. */
static GList *pending_callbacks = NULL;
/** List of callbacks which are queued to occur in the future. */
static GList *queued_callbacks = NULL;
/** List of callbacks which are about to be run in this ::schedule_run. */
static GList *this_run = NULL;

static gboolean
nsgtk_schedule_generic_callback(gpointer data)
{
        _nsgtk_callback_t *cb = (_nsgtk_callback_t *)(data);
        if (cb->callback_killed) {
                /* This callback instance has been killed. */
        }
        queued_callbacks = g_list_remove(queued_callbacks, cb);
        pending_callbacks = g_list_append(pending_callbacks, cb);
        return FALSE;
}

static void
nsgtk_schedule_kill_callback(void *_target, void *_match)
{
        _nsgtk_callback_t *target = (_nsgtk_callback_t *)_target;
        _nsgtk_callback_t *match = (_nsgtk_callback_t *)_match;
        if ((target->callback == match->callback) &&
            (target->context == match->context)) {
                target->callback = NULL;
                target->context = NULL;
                target->callback_killed = true;
                match->callback_killed = true;
        }
}

/**
 * remove a matching callback and context tuple from all lists
 *
 * \param callback The callback to match
 * \param cbctx The callback context to match
 * \return NSERROR_OK if the tuple was removed from at least one list else NSERROR_NOT_FOUND
 */
static nserror
schedule_remove(void (*callback)(void *p), void *cbctx)
{
        _nsgtk_callback_t cb_match = {
                .callback = callback,
                .context = cbctx,
                .callback_killed = false,
        };

        g_list_foreach(queued_callbacks,
                       nsgtk_schedule_kill_callback, &cb_match);
        g_list_foreach(pending_callbacks,
                       nsgtk_schedule_kill_callback, &cb_match);
        g_list_foreach(this_run,
                       nsgtk_schedule_kill_callback, &cb_match);

        if (cb_match.callback_killed == false) {
                return NSERROR_NOT_FOUND;
        }
        return NSERROR_OK;
}

nserror gui_misc_schedule(int t, void (*callback)(void *p), void *cbctx)
{
        _nsgtk_callback_t *cb;
        nserror res;

        /* Kill any pending schedule of this kind. */
        res = schedule_remove(callback, cbctx);

        /* only removal */
        if (t < 0) {
                return res;
        }

        cb = malloc(sizeof(_nsgtk_callback_t));
        cb->callback = callback;
        cb->context = cbctx;
        cb->callback_killed = false;
        /* Prepend is faster right now. */
        queued_callbacks = g_list_prepend(queued_callbacks, cb);
        g_timeout_add(t, nsgtk_schedule_generic_callback, cb);

        return NSERROR_OK;
}

void schedule_run(void);
void schedule_run(void)
{
        /* Capture this run of pending callbacks into the list. */
        this_run = pending_callbacks;

        if (this_run == NULL)
                return; /* Nothing to do */

        /* Clear the pending list. */
        pending_callbacks = NULL;

        /* Run all the callbacks which made it this far. */
        while (this_run != NULL) {
                _nsgtk_callback_t *cb = (_nsgtk_callback_t *)(this_run->data);
                this_run = g_list_remove(this_run, this_run->data);
                if (!cb->callback_killed)
                        cb->callback(cb->context);
                free(cb);
        }
}
