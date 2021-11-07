/* Copyright © 2021 cat <cat@wolfgirl.org>
 * This program is free software. It comes without any warranty, to the extent
 * permitted by applicable law. You can redistribute it and/or modify it under
 * the terms of the Do What The Fuck You Want To Public License, Version 2, as
 * published by Sam Hocevar. See http://www.wtfpl.net/ for more details.
 */

#include "tag_file.h"
#include <QStandardPaths>


void util::find_tag_files_in_dir(QDir current_dir, const QString & tagsfile, const QString & override, std::vector<QDir> & search_dirs, QStringList & tags_files) {

	int max_height = 100; // NOTE: to avoid infinite loop with symlinks etc.
	do {
		search_dirs.push_back(current_dir);
	} while(max_height --> 0 && current_dir.cdUp());


	for(const auto& loc : QStandardPaths::standardLocations(QStandardPaths::AppDataLocation))
	{
		search_dirs.push_back(QDir{loc});
	}


	auto add_tag_files = [&tags_files](const QFileInfoList& list)
	{
		for(const auto& f : qAsConst(list)) {
			tags_files.push_back(f.absoluteFilePath());
		}
	};

	for(const auto& dir : qAsConst(search_dirs)) {
		if(!dir.exists()) continue;

		auto filter = QDir::Files | QDir::Hidden;
		auto sort_by = QDir::Name;
		auto list_override = dir.entryInfoList({override}, filter, sort_by);
		auto list_normal   = dir.entryInfoList({tagsfile}, filter, sort_by);

		add_tag_files(list_override);

		if(!list_override.isEmpty())
			break;

		add_tag_files(list_normal);

		// support old files
		if(list_override.isEmpty() && list_normal.isEmpty()) {

			auto legacy_override_list = dir.entryInfoList({override.mid(2)});
			auto legacy_normal_list   = dir.entryInfoList({tagsfile.mid(2)});

			add_tag_files(legacy_override_list);
			if(!legacy_override_list.isEmpty())
				break;

			add_tag_files(legacy_normal_list);
			if(!legacy_normal_list.isEmpty())
				break;
		}
	}

	tags_files.removeDuplicates();

	// search order is leaves -> root, but tag file data should be loaded in root -> leaves order
	std::reverse(tags_files.begin(), tags_files.end());
}
