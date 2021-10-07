#pragma once

#include <QCollator>
#include <QJsonObject>
#include <QString>
#include <map>
#include <utility>
#include <vector>

#include "chapters.hpp"

// Exclusive namespace for the OMM.
namespace omm {
	// Alias.
	using StringVector = std::vector<QString>;

	// Enum for choosing which list of chapters to target.
	enum class ChapterList : int { liked, loved };

	// The class that manages an entry.
	class Entry {
		private:
		// The map containing most of the elements of this entry.
		std::map<QString, QString> details;
		// The vector containing the list of liked chapters.
		Chapters likedChapters;
		// The vector containing the list of loved chapters.
		Chapters lovedChapters;
		// Pointer to the QCollator to use for comparison.
		QCollator const &collator;

		public:
		// Forbid constructing an entry without a collator.
		Entry() = delete;

		// Constructor that initializes the necessary elements to their default states, and sets the collator.
		Entry(QCollator const &_collator):
				details(), likedChapters(u"Liked Chapters"_qs), lovedChapters(u"Loved Chapters"_qs), collator(_collator) {
			// Fill the details map with the default elements.
			for(auto const &key: {u"Title"_qs, u"Original Title"_qs, u"Franchise/Series"_qs, u"Franchise/Series Order"_qs,
						u"Author"_qs, u"Year"_qs, u"Type"_qs, u"Language"_qs, u"Rating"_qs, u"Progress"_qs, u"Notes"_qs}) {
				details[key].clear();
			}
		}

		// Wrapper for accessing the underlying map object.
		auto &operator[](QString const &key) {
			return details[key];
		}

		// Wrapper for accessing the underlying map object (const).
		auto const &at(QString const &key) const {
			return details.at(key);
		}

		// Wrapper for the size of the underlying map object.
		auto size() const noexcept {
			return details.size();
		}

		// Add the given chapter to the specified list of chapters.
		void add_chapter(QString const &chapter, ChapterList cl) {
			cl == ChapterList::liked ? likedChapters.add(chapter) : lovedChapters.add(chapter);
		}

		// Remove the given chapter from the specified list of chapters.
		void delete_chapter(QString const &chapter, ChapterList cl) {
			cl == ChapterList::liked ? likedChapters.remove(chapter) : lovedChapters.remove(chapter);
		}

		// Fix any reversed ranges of chapters, sort the chapters, and merge any overlapping ranges of chapters.
		void organize_chapters() {
			likedChapters.organize();
			lovedChapters.organize();
		}

		// Getter for the liked chapters that allows modification.
		Chapters &get_likedChapters() {
			return likedChapters;
		}

		// Getter for the loved chapters that allows modification.
		Chapters &get_lovedChapters() {
			return lovedChapters;
		}

		// Comparison function for QStrings used when comparing entries.
		bool less(QString const &l, QString const &r) const {
			return collator(l, r);
		}

		// Serialize this entry in JSON format.
		void to_json(QJsonObject &json) const {
			for(auto const &a: details) {
				json[a.first] = a.second;
			}
			likedChapters.to_json(json);
			lovedChapters.to_json(json);
		}

		// Reconstruct this entry from JSON data.
		void from_json(const QJsonObject &json) {
			details.clear();
			for(auto ci = json.constBegin(); ci != json.constEnd(); ++ci) {
				if(ci.value().isString()) {
					details[ci.key()] = ci.value().toString();
				}
			}
			likedChapters.from_json(json);
			lovedChapters.from_json(json);
		}
	};

	// Uses title, type, author, and year for comparison.
	bool operator<(Entry const &l, Entry const &r);

	// Uses title, type, author, and year for comparison.
	bool operator==(Entry const &l, Entry const &r);
} // namespace omm
