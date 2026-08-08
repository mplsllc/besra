#include <QApplication>
#include <QSocketNotifier>
#include <QTimer>
#include <QObject>
#include <map>
#include <memory>
#include <iostream>

extern "C" {
#include <netsurf/netsurf.h>
#include <netsurf/content.h>
#include <netsurf/browser_window.h>
#include <content/fetch.h>
#include <utils/nsoption.h>
#include <desktop/bitmap.h>
void schedule_run(void);
}

#include "resources.h"
#include "dialogs.h"

static nserror set_defaults(struct nsoption_s *defaults)
{
    nsoption_set_charp(font_sans, strdup("Sans"));
    nsoption_set_charp(font_serif, strdup("Serif"));
    nsoption_set_charp(font_mono, strdup("Monospace"));
    nsoption_set_charp(font_cursive, strdup("Serif"));
    nsoption_set_charp(font_fantasy, strdup("Serif"));
    return NSERROR_OK;
}

namespace {

/**
 * Drives NetSurf's fetch layer from Qt's event loop.
 *
 * fetch_fdset() both services any fetcher that is ready (it calls
 * curl_multi_perform() internally) and reports the socket set the fetch
 * layer next wants woken up for. We mirror that set onto QSocketNotifier
 * objects, whose only job is to call back in here so the fdset gets
 * re-polled and re-serviced. A periodic heartbeat timer covers fetch
 * timeouts that have nothing to do with socket readiness (DNS, connect,
 * stalled transfers): libcurl needs fetch_fdset()/curl_multi_perform()
 * called periodically regardless of I/O activity to notice those.
 */
class FetchPump : public QObject {
public:
    explicit FetchPump(QObject *parent = nullptr) : QObject(parent)
    {
        heartbeat_.setInterval(200);
        connect(&heartbeat_, &QTimer::timeout, this, &FetchPump::pump);
        heartbeat_.start();
        pump();
    }

private:
    void pump()
    {
        fd_set read_fd_set, write_fd_set, exc_fd_set;
        int max_fd = -1;
        FD_ZERO(&read_fd_set);
        FD_ZERO(&write_fd_set);
        FD_ZERO(&exc_fd_set);
        fetch_fdset(&read_fd_set, &write_fd_set, &exc_fd_set, &max_fd);

        for (int i = 0; i <= max_fd; i++) {
            update_notifier(read_notifiers_, i, FD_ISSET(i, &read_fd_set),
                             QSocketNotifier::Read);
            update_notifier(write_notifiers_, i, FD_ISSET(i, &write_fd_set),
                             QSocketNotifier::Write);
        }

        prune_notifiers(read_notifiers_, max_fd);
        prune_notifiers(write_notifiers_, max_fd);

        schedule_run();
    }

    using NotifierMap = std::map<int, std::unique_ptr<QSocketNotifier>>;

    void update_notifier(NotifierMap &notifiers, int fd, bool wanted,
                          QSocketNotifier::Type type)
    {
        auto it = notifiers.find(fd);
        if (wanted) {
            if (it == notifiers.end()) {
                auto sn = std::make_unique<QSocketNotifier>(fd, type);
                connect(sn.get(), &QSocketNotifier::activated, this, &FetchPump::pump);
                notifiers[fd] = std::move(sn);
            } else {
                it->second->setEnabled(true);
            }
        } else if (it != notifiers.end()) {
            it->second->setEnabled(false);
        }
    }

    static void prune_notifiers(NotifierMap &notifiers, int max_fd)
    {
        for (auto it = notifiers.begin(); it != notifiers.end(); ) {
            it = (it->first > max_fd) ? notifiers.erase(it) : std::next(it);
        }
    }

    QTimer heartbeat_;
    NotifierMap read_notifiers_;
    NotifierMap write_notifiers_;
};

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    besra::load_messages();

    nsoption_init(set_defaults, &nsoptions, &nsoptions_default);

    // Set bitmap format to match Qt's Format_ARGB32_Premultiplied
    bitmap_fmt_t fmt = {
        .layout = BITMAP_LAYOUT_ARGB8888,
        .pma = true,
    };
    bitmap_set_format(&fmt);

    // Call a core entry point to force the linker to resolve all core dependencies.
    netsurf_init(NULL);

    const char *start_page = (argc > 1) ? argv[1] : "resource:welcome.html";
    nsurl *url;
    if (nsurl_create(start_page, &url) == NSERROR_OK) {
        browser_window_create(BW_CREATE_HISTORY, url, NULL, NULL, NULL);
        nsurl_unref(url);
    }

    FetchPump pump;

    QObject::connect(&app, &QApplication::aboutToQuit, [] {
        besra::finalizeHotlist();
    });

    return app.exec();
}
