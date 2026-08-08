#ifndef BESRA_QT6_DIALOGS_H
#define BESRA_QT6_DIALOGS_H

extern "C" {
struct browser_window;
}

class QWidget;

namespace besra {

void showAbout(QWidget *parent);
void showViewSource(QWidget *parent, struct browser_window *bw);
void showFindInPage(QWidget *parent, struct browser_window *bw);
void showPreferences(QWidget *parent);
void showHistory(QWidget *parent);
void showBookmarks(QWidget *parent);
void showCookies(QWidget *parent);
void showDownloads(QWidget *parent);
void showPrint(QWidget *parent, struct browser_window *bw);
void addBookmark(struct browser_window *bw);

/** Save the bookmarks file to disk if the hotlist was ever initialized
 * (i.e. the user opened the bookmarks panel or added a bookmark this
 * session). Safe to call unconditionally; a no-op otherwise. Call once,
 * before the application quits. */
void finalizeHotlist();

} // namespace besra

#endif
