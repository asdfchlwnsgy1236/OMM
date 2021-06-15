#pragma once

#include "entry.hpp"

// Exclusive namespace for the OMM.
namespace omm {
	// Alias.
	using EntryVector = std::vector<Entry>;

	// The class that manages a list of entries.
	class Entries {
		private:
		// The vector containing the list of entries.
		EntryVector entries;

		public:
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

		// Add the given entry to the list of entries; the given entry contains no data after this.
		void add_entry(Entry &&entry) {
			entries.push_back(std::move(entry));
		}

		// Duplicate the given entry and insert the duplicate right after the given entry.
		void duplicate_entry(Entry const &entry) {
			auto const entryIt = std::find(entries.cbegin(), entries.cend(), entry);
			if(entryIt != entries.cend()) {
				entries.insert(entryIt + 1, entry);
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
			std::string const fskey("franchiseSeries"), fsokey("franchiseSeriesOrder");
			auto partIter = std::partition(entries.begin(), entries.end(), [&](Entry &entry) {
				return !entry[fskey].empty();
			});

			// Sort the entries that are members of a franchise or series separately first.
			std::sort(entries.begin(), partIter, [&](Entry &l, Entry &r) {
				return l[fskey] != r[fskey]	   ? natural_compare(l[fskey], r[fskey]) :
						l[fsokey] != r[fsokey] ? std::stoi(l[fsokey]) < std::stoi(r[fsokey]) :
												   l < r;
			});

			// Sort the rest of the entries separately after.
			std::sort(partIter, entries.end());

			// Organize the liked and loved chapters of each entry.
			for(auto &entry: entries) {
				entry.organize_chapters();
			}
		}

		// Serialize this list of entries in JSON format and save it to the given string.
		std::string &to_string_append(std::string &s) const {
			// Serialize the name as the key.
			s.append("\t\"entries\": [");

			// Serialize the list of entries using their serialization function.
			for(EntryVector::size_type a = 0; a < entries.size(); a++) {
				if(a > 0) {
					s.append(1, ',');
				}
				s.append(1, '\n');
				entries[a].to_string_append(s);
			}
			if(!entries.empty()) {
				s.append("\n\t");
			}
			s.append(1, ']');

			return s;
		}

		// Serialize this list of entries in JSON format using Qt.
		void to_json(QJsonObject &json) const {
			QJsonArray entriesArray;
			for(auto const &a: entries) {
				QJsonObject entryObject;
				a.to_json(entryObject);
				entriesArray.append(entryObject);
			}
			json[QStringLiteral("Entries")] = entriesArray;
		}

		// Reconstruct this list of entries from JSON data using Qt.
		void from_json(const QJsonObject &json) {
			entries.clear();
			QJsonArray entriesArray = json[QStringLiteral("Entries")].toArray();
			entries.reserve(entriesArray.size());
			for(auto const &a: entriesArray) {
				Entry entry;
				entry.from_json(a.toObject());
				entries.push_back(entry);
			}
		}
	};
} // namespace omm
