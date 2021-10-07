#pragma once

#include <QCollator>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <iterator>
#include <utility>
#include <vector>

#include "entry.hpp"

// Exclusive namespace for the OMM.
namespace omm {
	// Alias.
	using EntryVector = std::vector<Entry>;

	// The class that manages a list of entries.
	class Entries {
		private:
		// The name of this list of entries.
		QString const name;
		// The vector containing the list of entries.
		EntryVector entries;
		// The QCollator object to inject into each entry.
		QCollator collator;

		public:
		// Default constructor that initializes the collator.
		Entries(): name(u"Entries"_qs), entries(), collator() {
			collator.setCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
			collator.setIgnorePunctuation(false);
			collator.setNumericMode(true);
		}

		// Overload of the subscript operator that accesses the underlying vector object.
		auto &operator[](EntryVector::size_type const index) {
			return entries[index];
		}

		// Wrapper for entries.begin() for easy iteration.
		auto begin() {
			return entries.begin();
		}

		// Wrapper for entries.end() for easy iteration.
		auto end() {
			return entries.end();
		}

		// Get the number of entries in the list.
		auto size() const noexcept {
			return entries.size();
		}

		// Create a new entry.
		Entry create_entry() {
			return Entry(collator);
		}

		// Add the given entry to the list of entries; the given entry contains no data after this.
		void add_entry(Entry &&entry) {
			entries.push_back(std::move(entry));
		}

		// Duplicate the given entry and insert the duplicate right after the given entry.
		void duplicate_entry(Entry const &entry) {
			auto const entryIt = std::find(entries.cbegin(), entries.cend(), entry);
			if(entryIt != entries.cend()) {
				entries.insert(std::next(entryIt), entry);
			}
		}

		// Deletes the given entry from the list of entries.
		void delete_entry(Entry &entry) {
			auto const entryIt = std::find(entries.cbegin(), entries.cend(), entry);
			if(entryIt != entries.cend()) {
				entries.erase(entryIt);
			}
		}

		// Sort the list of entries according to the specifications.
		// Specifically, group the entries by franchise/series, order the entries within each franchise/series by
		// story order, title, then type, order the groups of franchise/series by franchise/series name, and then
		// have the rest of the entries come after ordered by title, then type, and finally,
		// organize the liked and loved chapters of each entry.
		void sort() {
			// Partition the list of entries into those that are members of a franchise or series, and those that are not.
			QString const fskey(u"Franchise/Series"_qs), fsokey(u"Franchise/Series Order"_qs);
			auto partIter = std::partition(entries.begin(), entries.end(), [&](Entry &entry) {
				return !entry[fskey].isEmpty();
			});

			// Sort the entries that are members of a franchise or series separately first.
			std::sort(entries.begin(), partIter, [&](Entry &l, Entry &r) {
				return l[fskey] == r[fskey] ? l[fsokey] == r[fsokey] ? l < r : l[fsokey].toInt() < r[fsokey].toInt() :
												collator(l[fskey], r[fskey]);
			});

			// Sort the rest of the entries after.
			std::sort(partIter, entries.end());

			// Organize the liked and loved chapters of each entry.
			for(auto &entry: entries) {
				entry.organize_chapters();
			}
		}

		// Serialize this list of entries in JSON format.
		void to_json(QJsonObject &json) const {
			QJsonArray entriesArray;
			for(auto const &a: entries) {
				QJsonObject entryObject;
				a.to_json(entryObject);
				entriesArray.append(entryObject);
			}
			json[name] = entriesArray;
		}

		// Reconstruct this list of entries from JSON data.
		void from_json(const QJsonObject &json) {
			entries.clear();
			QJsonArray entriesArray = json[name].toArray();
			entries.reserve(entriesArray.size());
			for(auto const &a: entriesArray) {
				Entry entry(collator);
				entry.from_json(a.toObject());
				entries.push_back(std::move(entry));
			}
		}
	};
} // namespace omm
