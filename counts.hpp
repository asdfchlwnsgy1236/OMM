#pragma once

#include <QJsonObject>
#include <map>
#include <string>
#include <utility>

// Exclusive namespace for the OMM.
namespace omm {
	// The class that manages a group of counts of entries separated using an element as the standard.
	class Counts {
		private:
		// The name of this group of counts.
		std::string name;
		// The map containing the counts as key-value pairs.
		std::map<std::string, int> counts;

		public:
		// Constructor that takes a string as the name for this group of counts.
		Counts(std::string &&_name): name(std::move(_name)) {}

		// Overload of the subscript operator that accesses the underlying map object.
		auto &operator[](std::string const &key) {
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

		// Serialize this group of counts in JSON format and save it to the given string.
		std::string &to_string_append(std::string &s) const {
			// Serialize the name as the key.
			s.append("\t\"").append(name).append("\": {");

			// Serialize the counts as key-value pairs.
			for(auto ci = counts.cbegin(); ci != counts.cend(); ++ci) {
				if(ci != counts.cbegin()) {
					s.append(1, ',');
				}
				s.append("\n\t\t\"").append(ci->first).append("\": \"").append(std::to_string(ci->second)).append(1, '"');
			}
			s.append("\n\t}");

			return s;
		}

		// Serialize this group of counts in JSON format using Qt.
		void to_json(QJsonObject &json) const {
			QJsonObject countsObject;
			for(auto const &a: counts) {
				countsObject[QString::fromStdString(a.first)] = a.second;
			}
			json[QString::fromStdString(name)] = countsObject;
		}

		// Reconstruct this group of counts from JSON data using Qt.
		void from_json(const QJsonObject &json) {
			counts.clear();
			QJsonObject countsObject = json[QString::fromStdString(name)].toObject();
			for(auto ci = countsObject.constBegin(); ci != countsObject.constEnd(); ++ci) {
				counts[ci.key().toStdString()] = ci.value().toInt();
			}
		}
	};
} // namespace omm
