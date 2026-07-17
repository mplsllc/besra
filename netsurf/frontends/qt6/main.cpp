#include <QApplication>
#include <QSocketNotifier>
#include <map>
#include <memory>
#include <iostream>

extern "C" {
#include <netsurf/netsurf.h>
#include <netsurf/content.h>
#include <netsurf/browser_window.h>
#include <content/fetch.h>
void schedule_run(void);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Call a core entry point to force the linker to resolve all core dependencies.
    netsurf_init(NULL);
    
    nsurl *url;
    if (nsurl_create("file:///home/patrick/Webs/Besra/test.html", &url) == NSERROR_OK) {
        browser_window_create(BW_CREATE_HISTORY, url, NULL, NULL, NULL);
        nsurl_unref(url);
    }
    
    std::map<int, std::unique_ptr<QSocketNotifier>> read_notifiers;
    std::map<int, std::unique_ptr<QSocketNotifier>> write_notifiers;

    while (true) {
        fd_set read_fd_set, write_fd_set, exc_fd_set;
        int max_fd = -1;
        FD_ZERO(&read_fd_set);
        FD_ZERO(&write_fd_set);
        FD_ZERO(&exc_fd_set);
        fetch_fdset(&read_fd_set, &write_fd_set, &exc_fd_set, &max_fd);

        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fd_set)) {
                if (read_notifiers.find(i) == read_notifiers.end()) {
                    auto sn = std::make_unique<QSocketNotifier>(i, QSocketNotifier::Read);
                    read_notifiers[i] = std::move(sn);
                }
            } else {
                read_notifiers.erase(i);
            }

            if (FD_ISSET(i, &write_fd_set)) {
                if (write_notifiers.find(i) == write_notifiers.end()) {
                    auto sn = std::make_unique<QSocketNotifier>(i, QSocketNotifier::Write);
                    write_notifiers[i] = std::move(sn);
                }
            } else {
                write_notifiers.erase(i);
            }
        }

        // Clean up notifiers for FDs larger than max_fd
        for (auto it = read_notifiers.begin(); it != read_notifiers.end(); ) {
            if (it->first > max_fd) it = read_notifiers.erase(it);
            else ++it;
        }
        for (auto it = write_notifiers.begin(); it != write_notifiers.end(); ) {
            if (it->first > max_fd) it = write_notifiers.erase(it);
            else ++it;
        }

        app.processEvents(QEventLoop::WaitForMoreEvents);
        schedule_run();
    }
    
    return 0;
}
