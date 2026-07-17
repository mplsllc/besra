#include <QApplication>

extern "C" {
#include <netsurf/netsurf.h>
void schedule_run(void);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Call a core entry point to force the linker to resolve all core dependencies.
    // This will explode with undefined references to the frontend API,
    // which serves as our Qt6 implementation worklist.
    netsurf_init(NULL);
    
    // Custom event loop to emulate GTK's batching queue draining at a known point
    while (true) {
        app.processEvents(QEventLoop::WaitForMoreEvents);
        schedule_run();
    }
    
    return 0;
}
