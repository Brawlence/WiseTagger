/* Copyright © 2014 cat <cat@wolfgirl.org>
 * This program is free software. It comes without any warranty, to the extent
 * permitted by applicable law. You can redistribute it and/or modify it under
 * the terms of the Do What The Fuck You Want To Public License, Version 2, as
 * published by Sam Hocevar. See http://www.wtfpl.net/ for more details.
 */

#include "tagger.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>
#include <algorithm>
#include <cmath>
#include "global_enums.h"
#include "util/size.h"
#include "util/misc.h"
#include "util/open_graphical_shell.h"
#include "window.h"

namespace logging_category {
	Q_LOGGING_CATEGORY(tagger, "Tagger")
}
#define pdbg qCDebug(logging_category::tagger)
#define pwarn qCWarning(logging_category::tagger)

Tagger::Tagger(QWidget *_parent) :
	QWidget(_parent)
{
	installEventFilter(_parent);
	m_picture.installEventFilter(_parent);
	m_file_queue.setNameFilter(util::supported_image_formats_namefilter());

	m_main_layout.setMargin(0);
	m_main_layout.setSpacing(0);

	m_tag_input_layout.addWidget(&m_input);

	m_separator.setFrameStyle(QFrame::HLine | QFrame::Sunken);
	m_main_layout.addWidget(&m_picture);
	m_main_layout.addWidget(&m_separator);
	m_main_layout.addLayout(&m_tag_input_layout);
	setLayout(&m_main_layout);
	setAcceptDrops(true);

	setObjectName(QStringLiteral("Tagger"));
	m_input.setObjectName(QStringLiteral("Input"));
	m_separator.setObjectName(QStringLiteral("Separator"));

	connect(&m_input, &TagInput::textEdited,     this, &Tagger::tagsEdited);
	connect(this,     &Tagger::fileRenamed, &m_statistics, &TaggerStatistics::fileRenamed);
	connect(this,     &Tagger::fileOpened, [this](const auto& file)
	{
		m_statistics.fileOpened(file, m_picture.mediaSize());
	});
	updateSettings();
}

void Tagger::clear()
{
	m_file_queue.clear();
	m_picture.clear();
	m_input.clear();
}

bool Tagger::open(const QString& filename)
{
	if(rename() == RenameStatus::Cancelled)
		return false;

	bool res     = openDir(filename);
	if(!res) res = openSession(filename);
	if(!res) res = openFile(filename);
	return res;
}

bool Tagger::openFile(const QString &filename)
{
	if(rename() == RenameStatus::Cancelled)
		return false;

	const QFileInfo fi(filename);
	if(!fi.isReadable() || !fi.isFile() || !fi.isAbsolute()) {
		return false;
	}
	m_file_queue.clear();
	m_file_queue.push(fi.absolutePath());
	m_file_queue.sort();
	m_file_queue.select(m_file_queue.find(fi.absoluteFilePath()));
	return loadCurrentFile();
}

bool Tagger::openDir(const QString &dir)
{
	if(rename() == RenameStatus::Cancelled)
		return false;

	const QFileInfo fi(dir);
	if(!fi.isReadable() || !fi.isDir() || !fi.isAbsolute()) {
		return false;
	}
	m_file_queue.clear();
	m_file_queue.push(dir);
	m_file_queue.sort();
	m_file_queue.select(0u);
	return loadCurrentFile();
}

bool Tagger::openSession(const QString& sfile)
{
	if(rename() == RenameStatus::Cancelled)
		return false;

	// NOTE: to prevent error message when opening normal file or directory
	if(!FileQueue::checkSessionFileSuffix(sfile)) return false;

	if(!m_file_queue.loadFromFile(sfile)) {
		QMessageBox::critical(this,
			tr("Load Session Failed"),
			tr("<p>Could not load session from <b>%1</b></p>").arg(sfile));
		return false;
	}

	bool res = loadCurrentFile();
	if(res) {
		emit sessionOpened(sfile);
	}
	return res;
}

void Tagger::nextFile(RenameOptions options)
{
	if(rename(options) == RenameStatus::Cancelled)
		return;

	m_file_queue.forward();
	loadCurrentFile();
}

void Tagger::prevFile(RenameOptions options)
{
	if(rename(options) == RenameStatus::Cancelled)
		return;

	m_file_queue.backward();
	loadCurrentFile();
}

bool Tagger::openFileInQueue(size_t index)
{
	m_file_queue.select(index);
	return loadCurrentFile();
}

void Tagger::deleteCurrentFile()
{
	QMessageBox delete_msgbox(QMessageBox::Question, tr("Delete file?"),
		tr("<h3 style=\"font-weight: normal;\">Are you sure you want to delete <u>%1</u> permanently?</h3>"
		   "<dd><dl>File type: %2</dl>"
		   "<dl>File size: %3</dl>"
		   "<dl>Dimensions: %4 x %5</dl>"
		   "<dl>Modified: %6</dl></dd>"
		   "<p><em>This action cannot be undone!</em></p>").arg(
			currentFileName(),
			currentFileType(),
			util::size::printable(mediaFileSize()),
			QString::number(mediaDimensions().width()),
			QString::number(mediaDimensions().height()),
			QFileInfo(currentFile()).lastModified().toString(tr("yyyy-MM-dd hh:mm:ss", "modified date"))),
		QMessageBox::Save|QMessageBox::Cancel);
	delete_msgbox.setButtonText(QMessageBox::Save, tr("Delete"));

	const auto reply = delete_msgbox.exec();

	if(reply == QMessageBox::Save) {
		if(!m_file_queue.deleteCurrentFile()) {
			QMessageBox::warning(this,
				tr("Could not delete file"),
				tr("<p>Could not delete current file.</p>"
				   "<p>This can happen when you don\'t have write permissions to that file, "
				   "or when file has been renamed or removed by another application.</p>"
				   "<p>Next file will be opened instead.</p>"));
		}
		loadCurrentFile();
	}
}

//------------------------------------------------------------------------------

QString Tagger::currentFile() const
{
	return m_file_queue.current();
}

QString Tagger::currentDir() const
{
	return QFileInfo(m_file_queue.current()).absolutePath();
}

QString Tagger::currentFileName() const
{
	return QFileInfo(m_file_queue.current()).fileName();
}

QString Tagger::currentFileType() const
{
	QFileInfo fi(m_file_queue.current());
	return fi.suffix().toUpper();
}

bool Tagger::fileModified() const
{
	if(m_file_queue.empty()) // NOTE: to avoid FileQueue::current() returning invalid reference.
		return false;

	return m_input.text() != QFileInfo(m_file_queue.current()).completeBaseName();
}

bool Tagger::isEmpty() const
{
	return m_file_queue.empty();
}

QSize Tagger::mediaDimensions() const
{
	return m_picture.mediaSize();
}

size_t Tagger::mediaFileSize() const
{
	return QFileInfo(m_file_queue.current()).size();
}

QString Tagger::postURL() const
{
	return m_input.postURL();
}

FileQueue& Tagger::queue()
{
	return m_file_queue;
}

TaggerStatistics &Tagger::statistics()
{
	return m_statistics;
}

//------------------------------------------------------------------------------

void Tagger::updateSettings()
{
	QSettings s;
	auto view_mode = s.value(QStringLiteral("window/view-mode")).value<ViewMode>();
	if(view_mode == ViewMode::Minimal) {
		m_tag_input_layout.setMargin(0);
		m_separator.hide();
	}
	if(view_mode == ViewMode::Normal) {
		m_tag_input_layout.setMargin(m_tag_input_layout_margin);
		m_separator.show();
	}
	m_input.updateSettings();
}

void Tagger::setInputVisible(bool visible)
{
	m_input.setVisible(visible);

	QSettings s;
	auto view_mode = s.value(QStringLiteral("window/view-mode")).value<ViewMode>();
	if(view_mode == ViewMode::Minimal)
		return; // NOTE: no need to update following properties because they've been taken care of already

	m_separator.setVisible(visible);

	if(visible) {
		m_tag_input_layout.setMargin(m_tag_input_layout_margin);
	} else {
		m_tag_input_layout.setMargin(0);
	}
}

//------------------------------------------------------------------------------

void Tagger::reloadTags()
{
	findTagsFiles(true);
}


void Tagger::reloadTagsContents()
{
	m_input.loadTagFiles(m_current_tag_files);
	m_input.clearTagState();
}

void Tagger::openTagFilesInEditor()
{
	for(const auto& file : m_current_tag_files) {
		QDesktopServices::openUrl(QUrl::fromLocalFile(file));
	}
}

void Tagger::openTagFilesInShell()
{
	for(const auto& file : m_current_tag_files) {
		util::open_file_in_gui_shell(file);
	}
}

void Tagger::findTagsFiles(bool force)
{
	if(m_file_queue.empty())
		return;

	const auto c = currentDir();
	if(c == m_previous_dir && !force)
		return;

	m_previous_dir = c;
	QFileInfo fi(m_file_queue.current());

	const QSettings settings;
	const auto tagsfile = settings.value(QStringLiteral("tags/normal"),
					     QStringLiteral("*.tags.txt")).toString();

	const auto override = settings.value(QStringLiteral("tags/override"),
					     QStringLiteral("*.tags!.txt")).toString();

	QString parent_dir, errordirs;

	const int path_capacity = 256;
	const int errors_capacity = 4 * path_capacity;

	parent_dir.reserve(path_capacity);
	errordirs.reserve(errors_capacity);

	errordirs += QStringLiteral("<li>");

	int max_height = 10;
	while(max_height --> 0) {
		parent_dir = fi.absolutePath();
		errordirs += parent_dir;
		errordirs += QStringLiteral("</li><li>");

		auto cd = fi.dir();
		auto list_override = cd.entryInfoList({override});
		if(!list_override.isEmpty()) {
			for(const auto& f : list_override) {
				m_current_tag_files.push_back(f.absoluteFilePath());
				pdbg << "found override:" << f.fileName();
			}
			break;
		}

		auto list_tags = cd.entryInfoList({tagsfile});
		for(const auto& f : list_tags) {
			m_current_tag_files.push_back(f.absoluteFilePath());
			pdbg << "found tag file:" << f.fileName();
		}

		// support old files
		if(list_override.isEmpty() && list_tags.isEmpty()) {
			auto legacy_override = parent_dir, legacy_tags = parent_dir;
			legacy_override += QChar('/');
			legacy_tags += QChar('/');
			std::copy(override.begin() + 2, override.end(), std::back_inserter(legacy_override));
			std::copy(tagsfile.begin() + 2, tagsfile.end(), std::back_inserter(legacy_tags));

			if(QFile::exists(legacy_override)) {
				m_current_tag_files.push_back(legacy_override);
				pdbg << "found legacy override:" << legacy_override;
				break;
			}
			if(QFile::exists(legacy_tags)) {
				m_current_tag_files.push_back(legacy_tags);
				pdbg << "found legacy tags:" << legacy_tags;
			}
		}

		if(fi.absoluteDir().isRoot())
			break;

		fi.setFile(parent_dir);
	}

	m_current_tag_files.removeDuplicates();

	if(m_current_tag_files.isEmpty()) {
		auto last_resort = qApp->applicationDirPath();
		last_resort += QStringLiteral("/tags.txt");

		errordirs += qApp->applicationDirPath();
		errordirs += QStringLiteral("</li>");

		if(QFile::exists(last_resort)) {
			m_current_tag_files.push_back(last_resort);
		} else {
			QMessageBox mbox;
			mbox.setText(tr("<h3>Could not locate suitable tag file</h3>"));
			mbox.setInformativeText(tr(
				"<p>You can still browse and rename files, but tag autocomplete will not work.</p>"
				"<hr>WiseTagger will look for <em>tag files</em> in directory of the currently opened file "
				"and in directories directly above it."

				"<p>Tag files we looked for:"
				"<dd><dl>Appending tag file: <b>%1</b></dl>"
				"<dl>Overriding tag file: <b>%2</b></dl></dd></p>"
				"<p>Directories where we looked for them, in search order:"
				"<ol>%3</ol></p>"
				"<p><a href=\"https://bitbucket.org/catgirl/wisetagger/overview\">"
				"Appending and overriding tag files documentation"
				"</a></p>").arg(tagsfile, override, errordirs));
			mbox.setIcon(QMessageBox::Warning);
			mbox.exec();
		}
	}

	std::reverse(m_current_tag_files.begin(), m_current_tag_files.end());
	m_fs_watcher = std::make_unique<QFileSystemWatcher>(nullptr);
	m_fs_watcher->addPaths(m_current_tag_files);
	connect(m_fs_watcher.get(), &QFileSystemWatcher::fileChanged, [this](const auto& f)
	{
		if(!QFile::exists(f)) {
			pdbg << "removing deleted file" << f <<  "from tag file set";
			this->m_current_tag_files.removeOne(f);
		}
		this->reloadTagsContents();
		pdbg << "reloaded tags due to changed file:" << f;
		emit this->tagFileChanged();
	});
	reloadTagsContents();
}

void Tagger::updateNewTagsCounts()
{
	const auto new_tags = m_input.getAddedTags(true);
	bool emitting = false;

	for(const auto& t : new_tags) {
		auto it = m_new_tag_counts.find(t);
		if(it != m_new_tag_counts.end()) {
			++it->second;
			if(it->second == 5)
				emitting = true;

		} else {
			m_new_tag_counts.insert(std::make_pair(t, 1u));
		}
		++m_overall_new_tag_counts;
	}

	if(m_overall_new_tag_counts >= std::min(8.0, 2*std::log2(m_file_queue.size()))) {
		emitting = true;
	}

	if(emitting) {
		QStringList tags;
		for(const auto& t : m_new_tag_counts) {
			tags.push_back(t.first);
		}
		std::sort(tags.begin(), tags.end(), [this](const QString&a, const QString&b) {
			const unsigned numa = m_new_tag_counts[a];
			const unsigned numb = m_new_tag_counts[b];
			if(numa == numb)
				return a < b;
			return numa > numb;
		});
		emit newTagsAdded(tags);
		m_overall_new_tag_counts = 0;
	}
}

//------------------------------------------------------------------------------

bool Tagger::loadFile(size_t index, bool silent)
{
	const auto filename = m_file_queue.select(index);
	if(filename.size() == 0)
		return false;

	const QFileInfo f(filename);
	const QDir fd(f.absolutePath());

	if(!fd.exists()) {
		if(!silent) {
			QMessageBox::critical(this,
			tr("Error opening file"),
			tr("<p>Directory <b>%1</b> does not exist anymore.</p>"
			   "<p>File from another directory will be opened instead.</p>")
				.arg(fd.absolutePath()));
		}
		return false;
	}

	if(!f.exists()) {
		if(!silent) {
			QMessageBox::critical(this,
			tr("Error opening file"),
			tr("<p>File <b>%1</b> does not exist anymore.</p>"
			   "<p>Next file will be opened instead.</p>")
				.arg(f.fileName()));
		}
		return false;
	}

	if(!m_picture.loadMedia(f.absoluteFilePath())) {
		QMessageBox::critical(this,
			tr("Error opening media"),
			tr("<p>Could not open <b>%1</b></p>"
			   "<p>File format is not supported or file corrupted.</p>")
				.arg(f.fileName()));
		return false;
	}

	m_input.setText(f.completeBaseName());
	findTagsFiles(false);
	m_picture.setFocus();
	return true;
}

/* just load picture into tagger */
bool Tagger::loadCurrentFile()
{
	bool silent = false;
	while(!loadFile(m_file_queue.currentIndex(), silent) && !m_file_queue.empty()) {
		pdbg << "erasing invalid file from queue:" << m_file_queue.current();
		m_file_queue.eraseCurrent();
		silent = true;
	}

	if(m_file_queue.empty()) {
		clear();
		return false;
	}
	emit fileOpened(m_file_queue.current());
	findTagsFiles();
	return true;
}

Tagger::RenameStatus Tagger::rename(RenameOptions options)
{
	if(!fileModified())
		return RenameStatus::NotModified;

	const QFileInfo file(m_file_queue.current());
	QString new_file_path;

	m_input.fixTags();

	/* Make new file path from input text */
	new_file_path += m_input.text();
	new_file_path += QChar('.');
	new_file_path += file.suffix();
	new_file_path = QFileInfo(QDir(file.canonicalPath()), new_file_path).filePath();

	if(new_file_path == m_file_queue.current() || m_input.text().isEmpty())
		return RenameStatus::Failed;

	/* Check for possible filename conflict */
	if(QFileInfo::exists(new_file_path)) {
		QMessageBox::critical(this,
			tr("Cannot rename file"),
			tr("<p>Cannot rename file <b>%1</b></p>"
			   "<p>File with this name already exists in <b>%2</b></p>"
			   "<p>Please change some of your tags.</p>")
				.arg(file.fileName(), file.canonicalPath()));
		return RenameStatus::Cancelled;
	}

	/* Show save dialog */
	QMessageBox renameMessageBox(QMessageBox::Question,
		tr("Rename file?"),
		tr("Rename <b>%1</b>?").arg(file.completeBaseName()),
		QMessageBox::Save|QMessageBox::Discard);

	renameMessageBox.addButton(QMessageBox::Cancel);
	renameMessageBox.setButtonText(QMessageBox::Save, tr("Rename"));
	renameMessageBox.setButtonText(QMessageBox::Discard, tr("Discard"));

	int reply;
	if(options.testFlag(RenameOption::ForceRename) || (reply = renameMessageBox.exec()) == QMessageBox::Save ) {
		if(!m_file_queue.renameCurrentFile(new_file_path)) {
			QMessageBox::warning(this,
				tr("Could not rename file"),
				tr("<p>Could not rename <b>%1</b></p>"
				   "<p>File may have been renamed or removed by another application.</p>").arg(file.fileName()));
			return RenameStatus::Failed;
		}
		QSettings settings;
		if(settings.value(QStringLiteral("track-added-tags"), true).toBool()) {
			updateNewTagsCounts();
		}
		emit fileRenamed(m_input.text());
		return RenameStatus::Renamed;
	}

	if(reply == QMessageBox::Cancel) {
		return RenameStatus::Cancelled;
	}

	if(reply == QMessageBox::Discard) {
		// NOTE: restore to initial state to prevent multiple rename dialogs
		m_input.setText(file.completeBaseName());
		return RenameStatus::NotModified;
	}

	return RenameStatus::Failed;
}
