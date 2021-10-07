#pragma once

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QtDebug>
#include <chrono>
#include <ctime>
#include <limits>
#include <sstream>
#include <string>

#include "counts.hpp"
#include "entries.hpp"

// Exclusive namespace for the OMM.
namespace omm {
	// The class that manages a save of the OMM.
	class Save {
		private:
		// The id used to identify this save as one for the OMM.
		QString id;
		// The count of all entries stored in this save.
		EntryVector::size_type countTotal;
		// The group containing the counts of entries stored in this save separated by type.
		Counts countsByType;
		// The group containing the counts of entries stored in this save separated by language.
		Counts countsByLanguage;
		// The group containing the counts of entries stored in this save separated by progress.
		Counts countsByProgress;
		// The collection of all entries stored in this save.
		Entries entries;

		public:
		// Default constructor that initializes id using the current time and the other elements to their default states.
		Save():
				id(u"OMM_"_qs), countTotal(0), countsByType(u"Counts by Type"_qs), countsByLanguage(u"Counts by Language"_qs),
				countsByProgress(u"Counts by Progress"_qs), entries() {
			// Initialize id.
			auto tn = std::chrono::system_clock::now().time_since_epoch();
			struct std::tm tm {};
			auto ts = std::chrono::duration_cast<std::chrono::seconds>(tn).count(),
				 tms = std::chrono::duration_cast<std::chrono::milliseconds>(tn).count();
			char tstr[128]{};
			if(localtime_s(&tm, &ts) == 0 && std::strftime(tstr, sizeof(tstr), "%F_%T_%Z_%z_%u_%V", &tm) > 0) {
				// Format: [date]_[time]_[time zone]_[offset from UTC]_[day of the week]_[week of the year]
				// 24-hour format date and time with preceding zeros and milliseconds,
				// day of the week 1 ~ 7, week of the year 1 ~ 53.
				id.append(tstr);
				std::string millitime(std::to_string(tms % 1000));
				millitime.insert(0, 1, '.');
				if(millitime.size() < 4) {
					millitime.insert(1, 4 - millitime.size(), '0');
				}
				id.insert(id.indexOf(':') + 3, QString::fromStdString(millitime));
			}
			else {
				id.append(QString::number(tms));
			}

			// Initialize the counts by filling them with the default elements.
			StringVector typekeys{u"Manga"_qs, u"Anime"_qs, u"Light Novel"_qs, u"Web Novel"_qs, u"Visual Novel"_qs,
					u"Anime Film"_qs, u"OVA"_qs},
					languagekeys{u"Japanese"_qs, u"Korean"_qs, u"Chinese"_qs, u"English"_qs},
					progresskeys{u"Not Started"_qs, u"In Progress"_qs, u"Finished"_qs};
			for(auto &&key: typekeys) {
				countsByType[std::move(key)] = 0;
			}
			for(auto &&key: languagekeys) {
				countsByLanguage[std::move(key)] = 0;
			}
			for(auto &&key: progresskeys) {
				countsByProgress[std::move(key)] = 0;
			}
		}

		// Recalculate all counts.
		void re_count() {
			// Reset all counts.
			for(auto &count: countsByType) {
				count.second = 0;
			}
			for(auto &count: countsByLanguage) {
				count.second = 0;
			}
			for(auto &count: countsByProgress) {
				count.second = 0;
			}

			// Redo all counts.
			countTotal = entries.size();
			for(auto const &entry: entries) {
				try {
					// Increment the corresponding type or unspecified if not applicable.
					if(entry.at(u"Type"_qs).isEmpty()) {
						++countsByType[u"Unspecified"_qs];
					}
					else {
						++countsByType[entry.at(u"Type"_qs)];
					}

					// Increment the corresponding language or unspecified if not applicable.
					if(entry.at(u"Language"_qs).isEmpty()) {
						++countsByLanguage[u"Unspecified"_qs];
					}
					else {
						++countsByLanguage[entry.at(u"Language"_qs)];
					}

					// Increment the corresponding progress.
					if(entry.at(u"Progress"_qs).isEmpty()) {
						++countsByProgress[u"Not Started"_qs];
					}
					else if(entry.at(u"Progress"_qs) == u"Finished"_qs) {
						++countsByProgress[u"Finished"_qs];
					}
					else {
						++countsByProgress[u"In Progress"_qs];
					}
				}
				catch(std::exception const &e) {
					qDebug() << u"This exception should not occur:"_qs << e.what();
				}
			}
		}

		// Refresh the counts and entries.
		void refresh() {
			// Recalculate the counts and re-sort the entries.
			re_count();
			entries.sort();
		}

		// Getter/setter for the ID of this save.
		auto &gs_id() {
			return id;
		}

		// Getter/setter for the total count of all entries in this save.
		auto &gs_countTotal() {
			return countTotal;
		}

		// Getter/setter for the elements in countByType of this save.
		auto &gs_countByType(QString const &key) {
			return countsByType[key];
		}

		// Getter/setter for the elements in countByLanguage of this save.
		auto &gs_countByLanguage(QString const &key) {
			return countsByLanguage[key];
		}

		// Getter/setter for the elements in countByProgress of this save.
		auto &gs_countByProgress(QString const &key) {
			return countsByProgress[key];
		}

		// Wrapper for entries.add_entry().
		void add_entry(Entry &&entry) {
			entries.add_entry(std::move(entry));
		}

		// Serialize this save in JSON format.
		void to_json(QJsonObject &json) const {
			json[u"_ID"_qs] = id;
			json[u"Count Total"_qs] = static_cast<qint64>(countTotal);
			countsByType.to_json(json);
			countsByLanguage.to_json(json);
			countsByProgress.to_json(json);
			entries.to_json(json);
		}

		// Reconstruct this save from JSON data.
		void from_json(const QJsonObject &json) {
			id = json[u"_ID"_qs].toString();
			countTotal = static_cast<EntryVector::size_type>(json[u"Count Total"_qs].toInteger());
			countsByType.from_json(json);
			countsByLanguage.from_json(json);
			countsByProgress.from_json(json);
			entries.from_json(json);
		}

		// Save this save to a file.
		bool save() {
			QFile file(u"omm.json"_qs);

			if(!file.open(QIODevice::WriteOnly)) {
				qWarning() << u"Could not open save file."_qs;

				return false;
			}

			QJsonObject saveObject;
			to_json(saveObject);
			file.write(QJsonDocument(saveObject).toJson());

			return true;
		}

		// Load a save from a file.
		bool load() {
			QFile file(u"omm.json"_qs);

			if(!file.open(QIODevice::ReadOnly)) {
				qWarning() << u"Could not open save file."_qs;

				return false;
			}

			QByteArray data = file.readAll();
			QJsonDocument loadDocument(QJsonDocument::fromJson(data));
			from_json(loadDocument.object());

			return true;
		}
	};
} // namespace omm
