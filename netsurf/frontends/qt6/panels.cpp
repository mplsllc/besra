/*
 * History, bookmarks (hotlist), and cookie manager panels. All three are
 * the core's generic treeview UI rendered through a CoreWindowWidget; each
 * function here just wires one feature's init/redraw/keypress/mouse_action
 * to a CoreWindowWidget-backed dialog, lazily created and reused (matching
 * each feature's own singleton lifetime in the core).
 */

#include "corewindow.h"
#include "dialogs.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QDir>
#include <QByteArray>

extern "C" {
#include "utils/errors.h"
#include "utils/nsurl.h"
#include "netsurf/content.h"
#include "netsurf/mouse.h"
#include "netsurf/browser_window.h"
#include "desktop/global_history.h"
#include "desktop/hotlist.h"
#include "desktop/cookie_manager.h"
}

namespace {

/** A QDialog wrapping one CoreWindowWidget, created once and reused. */
QDialog *panelDialog(QWidget *parent, const QString &title, CoreWindowWidget **out_widget,
        CoreWindowWidget::RedrawFn redraw, CoreWindowWidget::KeyFn key,
        CoreWindowWidget::MouseFn mouse)
{
    auto *dialog = new QDialog(parent);
    dialog->setWindowTitle(title);
    dialog->resize(700, 500);

    auto *layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(0, 0, 0, 0);
    *out_widget = new CoreWindowWidget(std::move(redraw), std::move(key), std::move(mouse), dialog);
    layout->addWidget(*out_widget);

    return dialog;
}

/** The path Besra stores the bookmarks file at: <config dir>/hotlist. Kept
 * as a static QByteArray so the const char* handed to the core stays valid
 * for the process lifetime (hotlist_init doesn't copy the save_path it's
 * given more than once; safest to just keep the backing storage alive). */
const char *hotlistPath()
{
    static QByteArray path;
    if (path.isEmpty()) {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        QDir().mkpath(dir);
        path = QDir(dir).filePath(QStringLiteral("hotlist")).toUtf8();
    }
    return path.constData();
}

/**
 * The bookmarks panel, created eagerly the first time bookmarks are touched
 * at all (via either "Show Bookmarks" or "Add Bookmark"), so both entry
 * points share one fully-initialized hotlist_manager_init() state rather
 * than "Add Bookmark" improvising a lighter partial setup of its own.
 *
 * Note: hotlist_add_url() only succeeds for URLs the core's URL database
 * will track (real http/https/file schemes with a host or path); it
 * correctly declines internal pseudo-pages like resource:welcome.html,
 * the same way a real browser won't let you bookmark about:blank.
 */
QDialog *&bookmarksDialog()
{
    static QDialog *dialog = nullptr;
    return dialog;
}

void ensureHotlistInit()
{
    if (bookmarksDialog() != nullptr) {
        return;
    }
    hotlist_init(hotlistPath(), hotlistPath());

    CoreWindowWidget *widget = nullptr;
    bookmarksDialog() = panelDialog(nullptr, QObject::tr("Bookmarks"), &widget,
        [](int x, int y, const struct rect *clip, const struct redraw_context *ctx) {
            hotlist_redraw(x, y, const_cast<struct rect *>(clip),
                const_cast<struct redraw_context *>(ctx));
        },
        [](uint32_t key) { return hotlist_keypress(key); },
        [](browser_mouse_state mouse, int x, int y) {
            hotlist_mouse_action(mouse, x, y);
        });
    hotlist_manager_init(static_cast<void *>(widget->handle()));
}

} // namespace

namespace besra {

void showHistory(QWidget *parent)
{
    static QDialog *dialog = nullptr;
    if (dialog == nullptr) {
        CoreWindowWidget *widget = nullptr;
        dialog = panelDialog(parent, QObject::tr("History"), &widget,
            [](int x, int y, const struct rect *clip, const struct redraw_context *ctx) {
                global_history_redraw(x, y, const_cast<struct rect *>(clip),
                    const_cast<struct redraw_context *>(ctx));
            },
            [](uint32_t key) { return global_history_keypress(key); },
            [](browser_mouse_state mouse, int x, int y) {
                global_history_mouse_action(mouse, x, y);
            });
        global_history_init(static_cast<void *>(widget->handle()));
    }
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void showBookmarks(QWidget *parent)
{
    (void)parent;
    ensureHotlistInit();
    bookmarksDialog()->show();
    bookmarksDialog()->raise();
    bookmarksDialog()->activateWindow();
}

void addBookmark(struct browser_window *bw)
{
    ensureHotlistInit();
    hotlist_add_url(browser_window_access_url(bw));
}

void finalizeHotlist()
{
    if (bookmarksDialog() != nullptr) {
        hotlist_fini();
        bookmarksDialog() = nullptr;
    }
}

void showCookies(QWidget *parent)
{
    static QDialog *dialog = nullptr;
    if (dialog == nullptr) {
        CoreWindowWidget *widget = nullptr;
        dialog = panelDialog(parent, QObject::tr("Cookies"), &widget,
            [](int x, int y, const struct rect *clip, const struct redraw_context *ctx) {
                cookie_manager_redraw(x, y, const_cast<struct rect *>(clip),
                    const_cast<struct redraw_context *>(ctx));
            },
            [](uint32_t key) { return cookie_manager_keypress(key); },
            [](browser_mouse_state mouse, int x, int y) {
                cookie_manager_mouse_action(mouse, x, y);
            });
        cookie_manager_init(static_cast<void *>(widget->handle()));
    }
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

} // namespace besra
