# WiseTagger #
Simple picture tagging tool

[![Build Status](https://travis-ci.org/0xb8/WiseTagger.svg?branch=master)](https://travis-ci.org/0xb8/WiseTagger)
[![Build Status](https://ci.appveyor.com/api/projects/status/h7kpn21xadcxsab1?svg=true)](https://ci.appveyor.com/project/catgirl/wisetagger)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://wolfgirl.org/software/wisetagger/documentation)
[![GitHub Releases](https://img.shields.io/github/v/release/0xb8/WiseTagger.svg)](https://github.com/0xb8/WiseTagger/releases)


## Features ##
* Tag autocomplete
* Tag implication
* Tag replacement and removal
* Filesystem-based tag file selection        
* Picture drag-and-drop support
* Picture reverse-search (using iqdb.org) with proxy support
* Custom commands support
* Almost instant switching between previous and next pictures (experimental, disabled by default)
* Crossplatform, using Qt5 Framework

### Tag File Syntax ###
Each tag must be placed on separate line and must contain only letters, numbers and limited punctuation (not including quotes, commas, dash and equal signs).
All whitespace is ignored.

If disallowed character is found, it will be ignored with the rest of the line. This behavior can be used to make comments, e.g. `# comment`.

```
# These tags will be presented in autocomplete sugestions as needed, with their relative order preserved.
maid
nekomimi
ponytail
pout
smile
sad

# Tag 'tagme' will be automatically removed.
# If you decide to keep this tag, you can type it again and it will stay.
# This tag will NOT be suggested in autocomplete.
-tagme

# tag replacement: 'megane' will be automatically replaced with 'glasses'.
# If you decide to keep original tag, you can type it again and it will stay.
# Tag 'glasses' will be suggested in autocomplete.
glasses = megane

# tag implication: 'nekomimi' will be automatically added when 'catgirl' is added.
# If you decide to remove implied tag, erase it and it will not be added again.
# Tag 'catgirl' will be suggested in autocomplete, but 'nekomimi' will NOT. Add it on other line if needed.
catgirl : nekomimi

# replace and add tags simultaneously. Order does not matter.
catgirl = animal_ears : nekomimi

# tag lists (comma-separated) may also be used.
some_tag = first_replaced_tag, second_replaced_tag : first_implied_tag, second_implied_tag

# all these tags will be removed
- one, two, three
```

**Note that tags will be presented in autocomplete suggestions in the same order they are in tags file!** Use external tools to sort tags.txt file if needed, e.g.:

```
cat original.tags.txt | sort | uniq > sorted.tags.txt
```

### Imageboard tag compaction ###
Some imageboard tags may be replaced with their shorter or longer versions.

For example, if *Options - Replace imageboard tags* is enabled, and `yande.re 12345` or `Konachan.com - 67890` tags are present, they will be replaced with `yandere_12345` and `konachan_67890` respectively.

Similarly, if *Options - Restore imageboard tags* is enabled, `konachan_67890` will be turned back to `Konachan.com - 67890`.

### Tag file selection ###
To use tag autocompletion place *Normal Tag File* or *Override Tag File* in the directory with your pictures, or in any of its parent directories.

* Normal Tag File - Files with suffix `.tags.txt`, e.g. `my.tags.txt`
* Override Tag File - Files with suffix `.tags!.txt`, e.g. `my.tags!.txt`

The difference between them is that when *Override Tag File* is found in some directory, the search stops. With *Normal Tag File*, the search continues until all candidate directories have been checked.

Tag file prefix can be omitted, leaving only corresponding suffix as file name, e.g. `.tags.txt`. Such files are treated as hidden in UNIX-like systems.

When multiple tag files are found, their contents will be combined. If this behavior is not desired in some particular directory, place *Override Tag File* there.

#### Tag file reloading ####

WiseTagger will automatically reload tag file(s) if it has been modified. You can also reload tag file manually with *Navigation - Reload Tag File* menu command.

You can open current set of tag files in default text editor using *Navigation - Edit Tag File* menu command.

#### New tags detection ####

WiseTagger will detect tags that are added by user but are not present in any of the tag files.

After such tag had been used a few times, WiseTagger will display notification message with copyable list of new tags.
