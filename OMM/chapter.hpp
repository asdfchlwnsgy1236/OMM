#pragma once

#include <QJsonArray>
#include <QString>
#include <cstdlib>
#include <exception>
#include <string>
#include <utility>
#include <vector>

// Exclusive namespace for the OMM.
namespace omm {
	using IntVector = std::vector<int>;

	enum class ChapterState : int { Valid = 0, Reversed, MultiSection, DifferentDepth, ConversionFailure };

	// The class that manages a chapter.
	class Chapter {
		private:
		// The vector containing the components of the left end of the range of chapters or the single chapter.
		IntVector l;
		// The vector containing the components of the right end of the range of chapters or the single chapter (equal to l).
		IntVector r;

		public:
		// Constructor that takes a chapter in string form and converts it for use; empty upon failure.
		explicit Chapter(QString const &chapter): l(), r() {
			std::string const cs(chapter.toStdString());
			// Index for the range symbol.
			auto ri = cs.find('~'), npos = std::string::npos;

			try {
				// The given chapter is not a range.
				if(ri == npos) {
					// Fill the chapter components by parsing through each number token.
					l.push_back(std::stoi(cs));
					for(auto si = cs.find('.'); si != npos; si = cs.find('.', si + 1)) {
						l.push_back(std::atoi(cs.data() + si + 1));
					}
					r = l;
				}
				// The given chapter is a range.
				else {
					// Fill the left chapter components by parsing through each number token.
					l.push_back(std::stoi(cs));
					for(auto si = cs.find('.'); si != npos && si < ri; si = cs.find('.', si + 1)) {
						l.push_back(std::atoi(cs.data() + si + 1));
					}

					// Fill the right chapter components by continuing the parse after the range symbol.
					r.push_back(std::stoi(cs.data() + ri + 1));
					for(auto si = cs.find('.', ri + 1); si != npos; si = cs.find('.', si + 1)) {
						r.push_back(std::atoi(cs.data() + si + 1));
					}
				}
			}
			catch(std::exception const &) {
				// The given string is not a chapter or range of chapters, so reset this chapter object to indicate failure.
				l.clear(), r.clear();
			}

			// If not valid for a minor reason, attempt a correction;
			// otherwise, reset this chapter object to indicate failure.
			switch(verify()) {
				case ChapterState::Valid:
					break;
				case ChapterState::Reversed:
					l.swap(r);
					break;
				default:
					l.clear(), r.clear();
					break;
			}
		}

		// Copy constructor.
		Chapter(IntVector const &_l, IntVector const &_r): l(_l), r(_r) {}

		// Move constructor.
		Chapter(IntVector &&_l, IntVector &&_r): l(std::move(_l)), r(std::move(_r)) {}

		IntVector &get_l() {
			return l;
		}

		IntVector &get_r() {
			return r;
		}

		IntVector const &get_l() const {
			return l;
		}

		IntVector const &get_r() const {
			return r;
		}

		// Checks if the chapter is valid.
		// Returns a ChapterState enum value depending on the result:
		// Valid is self-explanatory,
		// Reversed is self-explanatory,
		// MultiSection is when the range is not limited to the last component,
		// DifferentDepth is when the number of components on both sides are different, and
		// ConversionFailure is when the initial conversion from a string failed.
		ChapterState verify() const {
			if(l.empty()) {
				return ChapterState::ConversionFailure;
			}

			if(l.size() != r.size()) {
				return ChapterState::DifferentDepth;
			}

			for(IntVector::size_type a = 0; a < l.size() - 1; ++a) {
				if(l[a] != r[a]) {
					return ChapterState::MultiSection;
				}
			}

			if(l.back() > r.back()) {
				return ChapterState::Reversed;
			}

			return ChapterState::Valid;
		}

		// Returns true if the two given chapters overlap, false otherwise.
		bool does_overlap(Chapter const &other) const {
			return l <= other.get_r() && other.get_l() <= r;
		}

		// Serialize this chapter in JSON format.
		void to_json(QJsonArray &json) const {
			// Separate each component with a period and the range with a tilde, if applicable.
			QString s;
			for(IntVector::size_type a = 0; a < l.size(); ++a) {
				if(a) {
					s.append(u'.');
				}
				s.append(QString::number(l[a]));
			}
			if(l != r) {
				s.append(u" ~ "_qs);
				for(IntVector::size_type a = 0; a < r.size(); ++a) {
					if(a) {
						s.append(u'.');
					}
					s.append(QString::number(r[a]));
				}
			}
			json.append(s);
		}
	};

	bool operator<(Chapter const &l, Chapter const &r);

	bool operator==(Chapter const &l, Chapter const &r);
} // namespace omm
