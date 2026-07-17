#include <QTimer>
#include <QObject>
#include <QCoreApplication>
#include <vector>
#include <algorithm>

extern "C" {
#include "utils/errors.h"
#include "utils/log.h"
#include "netsurf/misc.h"
}

struct qt_callback_t {
    void (*callback)(void *);
    void *context;
    bool callback_killed;
    QTimer *timer;
};

static std::vector<qt_callback_t*> pending_callbacks;
static std::vector<qt_callback_t*> queued_callbacks;
static std::vector<qt_callback_t*> this_run;

static nserror schedule_remove(void (*callback)(void *p), void *cbctx) {
    bool killed = false;
    auto kill_match = [&](qt_callback_t *cb) {
        if (cb->callback == callback && cb->context == cbctx) {
            cb->callback = nullptr;
            cb->context = nullptr;
            cb->callback_killed = true;
            if (cb->timer) {
                cb->timer->stop();
                cb->timer->deleteLater();
                cb->timer = nullptr;
            }
            killed = true;
        }
    };
    
    for (auto cb : queued_callbacks) kill_match(cb);
    for (auto cb : pending_callbacks) kill_match(cb);
    for (auto cb : this_run) kill_match(cb);

    return killed ? NSERROR_OK : NSERROR_NOT_FOUND;
}

extern "C" nserror gui_misc_schedule(int t, void (*callback)(void *p), void *cbctx) {
    schedule_remove(callback, cbctx);

    if (t < 0) {
        return NSERROR_OK;
    }

    qt_callback_t *cb = new qt_callback_t{callback, cbctx, false, nullptr};
    queued_callbacks.push_back(cb);

    QTimer *timer = new QTimer();
    timer->setSingleShot(true);
    cb->timer = timer;

    QObject::connect(timer, &QTimer::timeout, [cb, timer]() {
        if (!cb->callback_killed) {
            auto it = std::find(queued_callbacks.begin(), queued_callbacks.end(), cb);
            if (it != queued_callbacks.end()) {
                queued_callbacks.erase(it);
            }
            pending_callbacks.push_back(cb);
        }
        if (cb->timer == timer) {
            cb->timer = nullptr;
        }
        timer->deleteLater();
    });

    timer->start(t);

    return NSERROR_OK;
}

extern "C" void schedule_run(void) {
    if (pending_callbacks.empty()) {
        return;
    }

    this_run = pending_callbacks;
    pending_callbacks.clear();

    for (auto cb : this_run) {
        if (!cb->callback_killed && cb->callback) {
            cb->callback(cb->context);
        }
        delete cb;
    }
    this_run.clear();
}
