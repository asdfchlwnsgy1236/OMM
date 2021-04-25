#pragma once

#include <chrono>
#include <ctime>
#include <limits>
#include <sstream>

#include "counts.hpp"
#include "entries.hpp"
#include "util.hpp"

// Exclusive namespace just in case.
namespace omm {
	// The class that manages a save of the OMM.
	class Save {
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
		Save():
				id("OMM_"), countTotal(0), countByType("Count by Type"), countByLanguage("Count by Language"),
				countByProgress("Count by Progress"), entries() {
			// Initialize id.
			auto tn = std::chrono::system_clock::now().time_since_epoch();
			struct std::tm tm {};
			auto t = std::chrono::duration_cast<std::chrono::seconds>(tn).count();
			char ts[128]{};
			if(localtime_s(&tm, &t) == 0 && std::strftime(ts, sizeof(ts), "%F_%T_%Z_%z_%u_%V", &tm) > 0) {
				// Format: [date]_[time]_[time zone]_[offset from UTC]_[day of the week]_[week of the year]
				// 24-hour format date and time with preceding zeros and milliseconds,
				// day of the week 1 ~ 7, week of the year 1 ~ 53.
				id.append(ts);
				std::string millitime =
						std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(tn).count() % 1000);
				millitime.insert(0, 1, '.');
				if(millitime.size() < 4) {
					millitime.insert(1, 4 - millitime.size(), '0');
				}
				id.insert(id.find_last_of(':') + 3, millitime);
			}
			else {
				id.append(std::to_string(std::chrono::system_clock::now().time_since_epoch().count() / 1000000));
			}

			// Initialize the counts by filling them with the default elements.
			StringVector typekeys{"Manga", "Anime", "Light Novel", "Web Novel", "Visual Novel", "Anime Film", "OVA"},
					languagekeys{"Japanese", "Korean", "Chinese", "English"},
					progresskeys{"Not Started", "In Progress", "Finished"};
			for(auto &&key: typekeys) {
				countByType[std::move(key)] = 0;
			}
			for(auto &&key: languagekeys) {
				countByLanguage[std::move(key)] = 0;
			}
			for(auto &&key: progresskeys) {
				countByProgress[std::move(key)] = 0;
			}
		}

		// Recalculate all counts.
		void re_count() {
			// Reset all counts.
			for(auto &count: countByType) {
				count.second = 0;
			}
			for(auto &count: countByLanguage) {
				count.second = 0;
			}
			for(auto &count: countByProgress) {
				count.second = 0;
			}

			// Redo all counts.
			countTotal = entries.size();
			for(auto const &entry: entries) {
				try {
					// Increment the corresponding type or unspecified if not applicable.
					if(entry.at("type").empty()) {
						countByType["unspecified"]++;
					}
					else {
						countByType[entry.at("type")]++;
					}

					// Increment the corresponding language or unspecified if not applicable.
					if(entry.at("language").empty()) {
						countByLanguage["unspecified"]++;
					}
					else {
						countByLanguage[entry.at("language")]++;
					}

					// Increment the corresponding progress.
					if(entry.at("progress").empty()) {
						countByProgress["notStarted"]++;
					}
					else if(entry.at("progress") == "Finished") {
						countByProgress["finished"]++;
					}
					else {
						countByProgress["inProgress"]++;
					}
				}
				catch(std::exception const &e) {
					std::cout << "This exception should not occur: " << e.what() << '\n';
				}
			}
		}

		// Refresh the counts and entries.
		void refresh() {
			// Recalculate the counts and re-sort the entries.
			re_count();
			entries.sort();
		}

		// Serialize this save in JSON format and append it to the given string.
		std::string &to_string_append(std::string &s) {
			// Build the final serialized string for the save.
			s.append("{\n\t\"_id\": \"")
					.append(id)
					.append("\",\n\t\"Total Count\": \"")
					.append(std::to_string(countTotal))
					.append("\",\n");
			countByType.to_string_append(s).append(",\n");
			countByLanguage.to_string_append(s).append(",\n");
			countByProgress.to_string_append(s).append(",\n");
			entries.to_string_append(s).append("\n}");

			return s;
		}

		// Deserialize the given save; returns true if successful, false otherwise.
		bool from_string(std::stringstream &ss) {
			// Temporary strings used for reading keys and values from the serialized save.
			std::string sk, sv;
			constexpr auto ssmax = std::numeric_limits<std::streamsize>::max();

			// Begin deserialization.
			while(true) {
				// If the end of the save has not been reached, get a key.
				ss.ignore(ssmax, '"');
				if(!ss.good()) {
					break;
				}
				std::getline(ss, sk, '"');

				// Get the value differently based on the key.
				if(sk == "_id" || sk == "countTotal") {
					// If the key is "_id" or "countTotal", simply get the value after it.
					ss.ignore(ssmax, '"');
					std::getline(ss, sv, '"');
					if(is_number(sv)) {
						countTotal = std::stoi(sv);
					}
					else {
						id = std::move(sv);

						// If the id is invalid, abort deserialization.
						if(id.compare(0, 3, "OMM") != 0) {
							return false;
						}
					}
				}
				else if(sk.compare(0, 7, "countBy") == 0) {
					// If the count group is empty, then move on.
					ss.ignore(ssmax, '{');
					if(ss.seekg(3, ss.cur).peek() == '}') {
						continue;
					}

					// If the key starts with "countBy", then loop to get the key-value pairs after it.
					while(true) {
						// Get a key-value pair.
						ss.ignore(ssmax, '"');
						std::getline(ss, sk, '"');
						ss.ignore(ssmax, '"');
						std::getline(ss, sv, '"');

						// Save to the appropriate count group.
						if(sk == "countByType") {
							countByType[sk] = std::stoi(sv);
						}
						else if(sk == "countByLanguage") {
							countByLanguage[sk] = std::stoi(sv);
						}
						else if(sk == "countByProgress") {
							countByProgress[sk] = std::stoi(sv);
						}
						else {
							// If the key is invalid, abort deserialization.
							return false;
						}

						// If this key-value pair is the last of this count group, stop this loop.
						if(ss.get() != ',') {
							break;
						}
					}
				}
				else if(sk == "entries") {
					// If there are no entries, then move on.
					ss.ignore(ssmax, '[');
					if(ss.seekg(3, ss.cur).peek() == ']') {
						continue;
					}

					// If the key is "entries", then loop to get the entry elements after it.
					while(true) {
						// Rebuild an entry from the serialized save.
						Entry entry;
						while(true) {
							// Get a key.
							ss.ignore(ssmax, '"');
							std::getline(ss, sk, '"');

							// Get the value differently based on the key.
							if(sk.compare(6, 8, "Chapters") == 0) {
								// If there are no chapters, then move on.
								ss.ignore(ssmax, '[');
								if(ss.seekg(1, ss.cur).peek() == ']') {
									continue;
								}

								// If the key ends with "Chapters", loop to get the list of values after it.
								while(true) {
									// Get a value.
									ss.ignore(ssmax, '"');
									std::getline(ss, sv, '"');

									// Add to the appropriate chapters list.
									if(sk == "Liked Chapters") {
										entry.add_chapter(std::move(sv), ChapterList::liked);
									}
									else if(sk == "Loved Chapters") {
										entry.add_chapter(std::move(sv), ChapterList::loved);
									}
									else {
										// If the key is invalid, abort deserialization.
										return false;
									}

									// If this value is the last of this chapters list, stop this loop.
									if(ss.get() != ',') {
										break;
									}
								}
							}
							else {
								// Otherwise, get the value after it while taking care of the possibility of double quotes.
								ss.ignore(ssmax, '"');
								std::getline(ss, sv);
								sv.erase(sv.find_last_of('"'));
								entry[sk] = std::move(sv);
							}

							// If this key-value pair is the last of this entry, stop this loop.
							if(ss.get() != ',') {
								break;
							}
						}

						// Add the rebuilt entry to the list of entries.
						entries.add_entry(std::move(entry));

						// If this entry is the last of the list of entries, stop this loop.
						ss.ignore(ssmax, '}');
						if(ss.get() != ',') {
							break;
						}
					}
				}
				else {
					// If the key is invalid, abort deserialization.
					return false;
				}
			}

			// Deserialization ended successfully.
			return true;
		}

		// Get/set the ID of this save.
		auto &gs_id() {
			return id;
		}

		// Get/set the total count of all entries in this save.
		auto &gs_countTotal() {
			return countTotal;
		}

		// Get/set the elements in countByType of this save.
		auto &gs_countByType(std::string const &key) {
			return countByType[key];
		}

		// Get/set the elements in countByLanguage of this save.
		auto &gs_countByLanguage(std::string const &key) {
			return countByLanguage[key];
		}

		// Get/set the elements in countByProgress of this save.
		auto &gs_countByProgress(std::string const &key) {
			return countByProgress[key];
		}

		// Wrapper for entries.add_entry().
		void add_entry(Entry &&entry) {
			entries.add_entry(std::move(entry));
		}
	};
} // namespace omm
