#pragma once

#include <iostream> // TODO: Remove this include once I change the error log output to use Qt's log system.
#include <map>

#include "chapters.hpp"
#include "util.hpp"

// Exclusive namespace just in case.
namespace omm {
	// Alias.
	using StringVector = std::vector<std::string>;

	// Enum for choosing which list of chapters to target.
	enum class ChapterList : int
	{
		liked,
		loved
	};

	// The class that manages an entry.
	class Entry {
		private:
		// The map containing most of the elements of this entry.
		std::map<std::string, std::string> details;
		// The vector containing the list of liked chapters.
		Chapters likedChapters;
		// The vector containing the list of loved chapters.
		Chapters lovedChapters;

		public:
		// Default constructor to initialize the necessary elements.
		Entry(): details(), likedChapters(), lovedChapters() {
			// Fill the details map with the default elements.
			StringVector keys{"Title", "Original Title", "Franchise/Series", "Franchise/Series Order", "Author", "Year", "Type",
					"Language", "Rating", "Progress", "Notes"};
			for(auto &&key: keys) {
				details[std::move(key)].clear();
			}
		}

		// Overload the subscript operator that accesses the underlying map object.
		auto &operator[](std::string const &key) {
			return details[key];
		}

		// Overload of at() that accesses the underlying map object.
		auto const &at(std::string const &key) const {
			return details.at(key);
		}

		// Get the number of elements in the underlying map object.
		auto size() const noexcept {
			return details.size();
		}

		// Add the given chapter to the specified list of chapters.
		void add_chapter(std::string &&chapter, ChapterList cl) {
			cl == ChapterList::loved ? lovedChapters.add(std::move(chapter)) : likedChapters.add(std::move(chapter));
		}

		// Remove the given chapter from the specified list of chapters.
		void delete_chapter(std::string &&chapter, ChapterList cl) {
			cl == ChapterList::loved ? lovedChapters.remove(std::move(chapter)) : likedChapters.remove(std::move(chapter));
		}

		// Fix any reversed ranges of chapters, sort the chapters, and merge any overlapping ranges of chapters.
		void organize_chapters() {
			likedChapters.organize();
			lovedChapters.organize();
		}

		// Getter for the underlying vector object for the liked chapters of this entry.
		Chapters &get_likedChapters() {
			return likedChapters;
		}

		// Getter for the underlying vector object for the loved chapters of this entry.
		Chapters &get_lovedChapters() {
			return lovedChapters;
		}

		// Serialize this entry in JSON format and save it to the given string.
		std::string &to_string_append(std::string &s) {
			// Serialize the elements as key-value pairs.
			s.append("\t\t{");
			for(auto di = details.cbegin(); di != details.cend(); di++) {
				if(di != details.cbegin()) {
					s.append(1, ',');
				}
				s.append("\n\t\t\t\"").append(di->first).append("\": \"").append(di->second).append(1, '"');
			}

			// Serialize the liked and loved chapters as an array.
			s.append(",\n\t\t\t\"Liked Chapters\": [");
			likedChapters.to_string_append(s).append("],\n\t\t\t\"Loved Chapters\": [");
			lovedChapters.to_string_append(s).append("]\n\t\t}");

			return s;
		}
	};

	// Overload of the less-than operator that uses natural ordering to compare the title and type of two entries.
	bool operator<(Entry const &l, Entry const &r) {
		std::string const tkey("title"), tykey("type");
		try {
			return l.at(tkey) != r.at(tkey) ? natural_compare(l.at(tkey), r.at(tkey)) :
												natural_compare(l.at(tykey), r.at(tykey));
		}
		catch(std::exception const &e) {
			std::cout << "This exception should not occur: " << e.what() << '\n';
		}

		return false;
	}

	// Overload of the equal-to operator that checks the title, type, author, and year for equality of two entries.
	bool operator==(Entry const &l, Entry const &r) {
		try {
			StringVector const keys{"title", "type", "author", "year"};
			for(auto const &key: keys) {
				if(l.at(key) != r.at(key)) {
					return false;
				}
			}
		}
		catch(std::exception const &e) {
			std::cout << "This exception should not occur: " << e.what() << '\n';
		}

		return true;
	}
} // namespace omm
