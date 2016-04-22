/* Copyright © 2014 cat <cat@wolfgirl.org>
 * This program is free software. It comes without any warranty, to the extent
 * permitted by applicable law. You can redistribute it and/or modify it under
 * the terms of the Do What The Fuck You Want To Public License, Version 2, as
 * published by Sam Hocevar. See http://www.wtfpl.net/ for more details.
 */

#ifndef WINDOW_H
#define WINDOW_H

#include "reverse_search.h"
#include "tagger.h"
#include <vector>
#include <QMainWindow>
#include <QAction>
#include <QString>
#include <QMenu>
#include <QStatusBar>

#ifdef Q_OS_WIN32
#include <QtWinExtras>
#endif

class Window : public QMainWindow {
	Q_OBJECT
public:
	explicit Window(QWidget *_parent = nullptr);
	~Window() override = default;

public slots:
	void showUploadProgress(qint64 bytesSent, qint64 bytesTotal);
	void hideUploadProgress();
	void updateCurrentDirectory();
	void updateWindowTitle();
	void updateWindowTitleProgress(int progress);
	void updateStatusBarText();
	void updateImageboardPostURL(QString url);

protected:
	bool eventFilter(QObject*, QEvent *e) override;
	void closeEvent(QCloseEvent *e) override;
	void showEvent(QShowEvent* e) override;

private slots:
	void fileOpenDialog();
	void directoryOpenDialog();
	void about();
	void help();

private:
	void createActions();
	void createMenus();
	void updateMenus();
	void parseCommandLineArguments();

	void loadWindowSettings();
	void saveWindowSettings();
	void loadWindowStyles();

	static constexpr const char* MainWindowTitle = "%1%2 [%3x%4] (%5)  –  " TARGET_PRODUCT " v%6";
	static constexpr const char* MainWindowTitleEmpty = TARGET_PRODUCT " v%1";
	static constexpr const char* MainWindowTitleProgress = "%7%  –  %1%2 [%3x%4] (%5)  –  " TARGET_PRODUCT " v%6";

	Tagger m_tagger;
	ReverseSearch m_reverse_search;

	QString m_last_directory;
	QString m_post_url;

	QAction a_open_file;
	QAction a_open_dir;
	QAction a_delete_file;
	QAction a_open_post;
	QAction a_iqdb_search;
	QAction a_exit;
	QAction a_next_file;
	QAction a_prev_file;
	QAction a_save_file;
	QAction a_save_next;
	QAction a_save_prev;
	QAction a_save_session;
	QAction a_load_session;
	QAction a_open_loc;
	QAction a_reload_tags;
	QAction a_ib_replace;
	QAction a_ib_restore;
	QAction a_view_statusbar;
	QAction a_view_fullscreen;
	QAction a_view_menu;
	QAction a_view_input;
	QAction a_about;
	QAction a_about_qt;
	QAction a_help;
	QAction a_stats;

	QMenu menu_file;
	QMenu menu_navigation;
	QMenu menu_view;
	QMenu menu_options;
	QMenu menu_options_language;
	QMenu menu_options_style;
	QMenu menu_help;

	QStatusBar m_statusbar;
	QLabel     m_statusbar_info_label;

#ifdef Q_OS_WIN32
	QWinTaskbarButton m_win_taskbar_button;
#endif
};
#endif // WINDOW_H
