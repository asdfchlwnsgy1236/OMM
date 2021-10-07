#pragma once

#include <QJsonObject>
#include <QString>
#include <map>
#include <utility>

// Exclusive namespace for the OMM.
namespace omm {
	// The class that manages a group of counts of entries separated using an element as the standard.
	class Counts {
		private:
		// The name of this group of counts.
		QString const name;
		// The map containing the counts as key-value pairs.
		std::map<QString, int> counts;

		public:
		// Constructor that takes a string as the name for this group of counts.
		explicit Counts(QString &&_name): name(std::move(_name)) {}

		// Overload of the subscript operator that accesses the underlying map object.
		auto &operator[](QString const &key) {
			return counts[key];
		}

		// Wrapper for counts.begin() for easy iteration.
		auto begin() noexcept {
			return counts.begin();
		}

		// Wrapper for counts.end() for easy iteration.
		auto end() noexcept {
			return counts.end();
		}

		// Wrapper for counts.cbegin() for easy iteration.
		auto cbegin() const noexcept {
			return counts.cbegin();
		}

		// Wrapper for counts.cend() for easy iteration.
		auto cend() const noexcept {
			return counts.cend();
		}

		// Serialize this group of counts in JSON format.
		void to_json(QJsonObject &json) const {
			QJsonObject countsObject;
			for(auto const &a: counts) {
				countsObject[a.first] = a.second;
			}
			json[name] = countsObject;
		}

		// Reconstruct this group of counts from JSON data.
		void from_json(const QJsonObject &json) {
			counts.clear();
			if(json.contains(name)) {
				QJsonObject countsObject = json[name].toObject();
				for(auto ci = countsObject.constBegin(); ci != countsObject.constEnd(); ++ci) {
					counts[ci.key()] = ci.value().toInt();
				}
			}
		}
	};
} // namespace omm
