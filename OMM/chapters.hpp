#pragma once

#include <QJsonObject>
#include <QString>
#include <utility>
#include <vector>

#include "chapter.hpp"

// Exclusive namespace for the OMM.
namespace omm {
	// Alias.
	using ChapterVector = std::vector<Chapter>;

	// The class that manages a list of chapters.
	class Chapters {
		private:
		// The name of this list of chapters.
		QString const name;
		// The vector of chapters that this class manages.
		ChapterVector chapters;

		public:
		explicit Chapters(QString &&_name): name(std::move(_name)), chapters() {}

		// Merges the two given overlapping chapters into the first and erases the second from the list.
		void merge_chapters(Chapter &c1, Chapter &c2) {
			c1.get_l() = std::move(c1.get_l() > c2.get_l() ? c2.get_l() : c1.get_l());
			c1.get_r() = std::move(c1.get_r() < c2.get_r() ? c2.get_r() : c1.get_r());
			if(auto c2i = std::find(chapters.cbegin(), chapters.cend(), c2); c2i != chapters.cend()) {
				chapters.erase(c2i);
			}
		}

		// Splits the list based on the given pivot chapter located in the list;
		// the pivot chapter is removed.
		void split_chapters(Chapter &chapter, Chapter &pivot) {
			// If the leftmost chapter is the pivot chapter...
			if(chapter.get_l() == pivot.get_l()) {
				// ... and if the chapter is a single chapter, then remove it.
				if(chapter.get_r() == pivot.get_l()) {
					chapters.erase(std::find(chapters.cbegin(), chapters.cend(), chapter));
				}
				else {
					// Otherwise, remove the leftmost chapter.
					++chapter.get_l().back();
				}
			}
			else if(chapter.get_r() == pivot.get_l()) {
				// If the rightmost chapter is the pivot chapter, then remove the rightmost chapter.
				--chapter.get_r().back();
			}
			else {
				// Otherwise, split the range of chapters, and remove the pivot chapter.
				--pivot.get_l().back();
				chapters.insert(std::find(chapters.cbegin(), chapters.cend(), chapter),
						Chapter(chapter.get_l(), pivot.get_l()));
				pivot.get_l().back() += 2;
				chapter.get_l() = std::move(pivot.get_l());
			}
		}

		// Add the given chapter to the list.
		void add(QString const &chapter) {
			// Convert the given chapter into a convenient form.
			Chapter toAdd(chapter);

			// If the given chapter overlaps with an existing chapter, then merge them.
			for(auto &ac: chapters) {
				if(ac.does_overlap(toAdd)) {
					merge_chapters(ac, toAdd);

					return;
				}
			}

			// Otherwise, simply add the given chapter.
			chapters.push_back(std::move(toAdd));
		}

		// Remove the given chapter from the list.
		void remove(QString const &chapter) {
			// Convert the given chapter into a convenient form.
			Chapter toRemove(chapter);

			// If the given chapter exists, then remove it.
			for(auto &ac: chapters) {
				if(ac.does_overlap(toRemove)) {
					split_chapters(ac, toRemove);

					return;
				}
			}
		}

		// Fix any reversed ranges of chapters, sort the list, and merge any overlapping ranges of chapters.
		void organize() {
			// If the list of chapters has more than one element...
			if(chapters.size() > 1) {
				// ... then sort the list of chapters...
				std::sort(chapters.begin(), chapters.end());

				// ... and merge any overlapping ranges of chapters.
				for(ChapterVector::size_type a = chapters.size() - 1; a > 0; --a) {
					if(chapters[a - 1].does_overlap(chapters[a])) {
						merge_chapters(chapters[a - 1], chapters[a]);
					}
				}
			}
		}

		// Serialize this list of chapters in JSON format.
		void to_json(QJsonObject &json) const {
			QJsonArray chaptersArray;
			for(auto const &a: chapters) {
				a.to_json(chaptersArray);
			}
			json[name] = chaptersArray;
		}

		// Reconstruct this list of chapters from JSON data.
		void from_json(const QJsonObject &json) {
			chapters.clear();
			QJsonArray chaptersArray = json[name].toArray();
			chapters.reserve(chaptersArray.size());
			for(auto const &a: chaptersArray) {
				chapters.push_back(Chapter(a.toString()));
			}
		}
	};
} // namespace omm
