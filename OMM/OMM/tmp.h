#pragma once
#ifndef _TMP_
#define _TMP_

#include <map>
#include <string>
#include <vector>

namespace omm{
	// Forward declaration of classes.
	class Counts;
	class Chapter;
	class Entry;
	class Entries;
	class Save;

	// Aliases.
	using Chapters = std::vector<Chapter>;
	using IntVector = std::vector<int>;
	using StringVector = std::vector<std::string>;
	using EntryVector = std::vector<Entry>;

	// Overload of the less-than operator that checks the components to compare two chapters.
	bool operator<(Chapter const &l, Chapter const &r);

	// Overload of equal-to operator that checks the components to compare two chapters.
	bool operator==(Chapter const &l, Chapter const &r);

	// Overload of the less-than operator that uses natural ordering to compare the title and type of two entries.
	bool operator<(Entry const &l, Entry const &r);

	// Overload of the equal-to operator that checks the title, type, author, and year for equality of two entries.
	bool operator==(Entry const &l, Entry const &r);

	// Returns true if the given string has a number at the given index, false otherwise.
	bool is_number(std::string const &s, std::string::size_type const index = 0);

	// Returns true if the left string comes before the right string according to natural ordering, false otherwise.
	bool natural_compare(std::string const &l, std::string const &r);

	// Returns a copy of the given string with leading and trailing whitespace removed.
	std::string trim(std::string const &s);

	// The class that manages a group of counts of entries separated using an element as the standard.
	class Counts{
		private:
		// The name of this group of counts.
		std::string name;
		// The map containing the counts as key-value pairs.
		std::map<std::string, int> counts;

		public:
		// Custom constructor that takes a string as the name for this group of counts.
		Counts(std::string &&_name);

		// Overload of the subscript operator that accesses the underlying map object.
		int &operator[](std::string const &key);

		// Wrapper for counts.begin() for iteration.
		auto begin();

		// Wrapper for counts.end() for iteration.
		auto end();

		// Serialize this group of counts in JSON format and save it to the given string.
		std::string &to_string_append(std::string &s);
	};

	// The class that manages a chapter.
	class Chapter{
		private:
		// The vector containing the components of the left end of the range of chapters or the single chapter.
		IntVector l;
		// The vector containing the components of the right end of the range of chapters or the single chapter.
		IntVector r;

		public:
		// Constructor that takes a chapter in string form and converts it for use.
		// When construction fails, the Chapter object is empty.
		Chapter(std::string &&chapter);

		// Copy constructor that takes two vectors of integers containing the chapter components.
		Chapter(IntVector const &_l, IntVector const &_r);

		// Move constructor that takes two vectors of integers containing the chapter components.
		Chapter(IntVector &&_l, IntVector &&_r);

		// Returns true if the two given chapters overlap, false otherwise.
		static bool does_overlap(Chapter &c1, Chapter &c2);

		// Merges the two given overlapping chapters into the first and erases the second if in the list.
		static void merge_chapters(Chapters &chapters, Chapter &c1, Chapter &c2);

		// Splits the given chapter from the given list of chapters based on the given
		// pivot chapter located in the given chapter; the pivot chapter is removed.
		static void split_chapters(Chapters &chapters, Chapter &chapter, Chapter &pivot);

		// Getter for the left chapter.
		IntVector &get_l();

		// Getter for the right chapter.
		IntVector &get_r();

		// Const getter for the left chapter.
		IntVector const &get_l() const;

		// Const getter for the right chapter.
		IntVector const &get_r() const;

		// Returns true if the chapter is valid, false otherwise.
		// A chapter is valid if its left and right have the same number of components,
		// and each component in its left and right are the same except for the last component.
		bool is_valid();

		// Serialize this chapter in JSON format and save it to the given string.
		std::string &to_string_append(std::string &s);
	};

	// The class that manages an entry.
	class Entry{
		private:
		// The map containing most of the elements of this entry.
		std::map<std::string, std::string> details;
		// The vector containing the list of liked chapters.
		Chapters likedChapters;
		// The vector containing the list of loved chapters.
		Chapters lovedChapters;

		// Helper function for to_string_append() that appends the given chapters in serialized form to the given string.
		static std::string &to_string_append_chapters(std::string &s, Chapters &chapters);

		public:
		// Default constructor to initialize the necessary elements.
		Entry();

		// Overload the subscript operator that accesses the underlying map object.
		auto &operator[](std::string const &key);

		// Overload of at() that accesses the underlying map object.
		auto const &at(std::string const &key) const;

		// Get the number of elements in the details.
		auto size() const noexcept;

		// Add the given chapter to the list of liked or loved chapters.
		void add_chapter(std::string &chapter, bool isLoved);

		// Delete the given chapter from the list of liked or loved chapters.
		void delete_chapter(std::string &chapter, bool isLoved);

		// Fix any reversed ranges of chapters, sort the chapters, and merge any overlapping ranges of chapters.
		void organize_chapters();

		// Getter for the underlying vector object for the liked chapters of this entry.
		Chapters &get_likedChapters();

		// Getter for the underlying vector object for the loved chapters of this entry.
		Chapters &get_lovedChapters();

		// Serialize this entry in JSON format and save it to the given string.
		std::string &to_string_append(std::string &s);
	};

	// The class that manages a list of entries.
	class Entries{
		private:
		// The vector containing the list of entries.
		EntryVector entries;

		public:
		// Overload of the subscript operator that accesses the underlying vector object.
		Entry &operator[](EntryVector::size_type const index);

		// Wrapper for entries.begin() for iteration.
		auto begin();

		// Wrapper for entries.end() for iteration.
		auto end();

		// Get the number of entries in the list.
		auto size() const noexcept;

		// Add the given entry to the list of entries; the given entry contains no data after this.
		void add_entry(Entry &&entry);

		// Duplicate the given entry and insert the duplicate right after the given entry.
		void duplicate_entry(Entry &entry);

		// Deletes the given entry from the list of entries.
		void delete_entry(Entry &entry);

		// Sort the list of entries according to the specifications.
		// Specifically, group the entries by franchise/series, order the entries within each franchise/series by
		// story order, title, then type, order the groups of franchise/series by franchise/series name, and then
		// have the rest of the entries come after ordered by title, then type, and finally,
		// organize the liked and loved chapters of each entry.
		void sort();

		// Serialize this list of entries in JSON format and save it to the given string.
		std::string &to_string_append(std::string &s);
	};

	// The class that manages a save of the OMM.
	class Save{
		private:
		// The id used to identify this save as one for the OMM.
		std::string id;
		// The count of all entries stored in this save.
		EntryVector::size_type countTotal;
		// The group containing the counts of entries stored in this save separated by type.
		Counts countByType;
		// The group containing the counts of entries stored in this save separated by language.
		Counts countByLanguage;
		// The group containing the counts of entries stored in this save separated by progress.
		Counts countByProgress;
		// The collection of all entries stored in this save.
		Entries entries;

		public:
		// Default constructor; initializes id using the current time and the counts with the default elements.
		Save();

		// Recalculate all counts.
		void re_count();

		// Refresh the counts and entries.
		void refresh();

		// Serialize this save in JSON format and append it to the given string.
		std::string &to_string_append(std::string &s);

		// Deserialize the given save; returns true if successful, false otherwise.
		bool from_string(std::stringstream &ss);

		// Wrapper for entries.add_entry().
		void add_entry(Entry &&entry);
	};
}

#endif // !_TMP_
