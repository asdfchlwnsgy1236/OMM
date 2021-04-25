#pragma once

#include <exception>
#include <string>
#include <utility>
#include <vector>

// Exclusive namespace just in case.
namespace omm {
	// Alias.
	using IntVector = std::vector<int>;

	// The class that manages a chapter.
	class Chapter {
		private:
		// The vector containing the components of the left end of the range of chapters or the single chapter.
		IntVector l;
		// The vector containing the components of the right end of the range of chapters or the single chapter.
		IntVector r;

		public:
		// Constructor that takes a chapter in string form and converts it for use.
		// When construction fails, the Chapter object is empty.
		Chapter(std::string &&chapter): l(), r() {
			// Index for the range symbol.
			auto ri = chapter.find('~');

			try {
				// The given chapter is not a range.
				if(ri == std::string::npos) {
					// Fill the chapter components by parsing through each number token.
					l.push_back(std::stoi(chapter));
					for(auto si = chapter.find('.'); si != std::string::npos; si = chapter.find('.', si + 1)) {
						l.push_back(std::stoi(chapter.data() + si + 1));
					}
					r = l;
				}
				// The given chapter is a range.
				else {
					// Fill the left chapter components by parsing through each number token.
					l.push_back(std::stoi(chapter));
					for(auto si = chapter.find('.'); si != std::string::npos && si < ri; si = chapter.find('.', si + 1)) {
						l.push_back(std::stoi(chapter.data() + si + 1));
					}

					// Fill the right chapter components by continuing the parse after the range symbol.
					r.push_back(std::stoi(chapter.data() + ri + 1));
					for(auto si = chapter.find('.', ri + 1); si != std::string::npos; si = chapter.find('.', si + 1)) {
						r.push_back(std::stoi(chapter.data() + si + 1));
					}
				}
			}
			catch(std::exception const &) {
				// The given string is not a chapter or range of chapters, so reset this chapter object to indicate failure.
				l.clear(), r.clear();
			}

			// If it is a range with the two ends having different depths, it is malformed, so reset this chapter object to
			// indicate failure.
			if(!is_valid()) {
				l.clear(), r.clear();
			}
		}

		// Copy constructor that takes two vectors of integers containing the chapter components.
		Chapter(IntVector const &_l, IntVector const &_r): l(_l), r(_r) {}

		// Move constructor that takes two vectors of integers containing the chapter components.
		Chapter(IntVector &&_l, IntVector &&_r): l(std::move(_l)), r(std::move(_r)) {}

		// Getter for the left chapter.
		IntVector &get_l() {
			return l;
		}

		// Getter for the right chapter.
		IntVector &get_r() {
			return r;
		}

		// Const getter for the left chapter.
		IntVector const &get_l() const {
			return l;
		}

		// Const getter for the right chapter.
		IntVector const &get_r() const {
			return r;
		}

		// Returns true if the chapter is valid, false otherwise.
		// A chapter is valid if its left and right have the same number of components,
		// and each component in its left and right are the same except for the last component.
		bool is_valid() {
			if(l.size() != r.size()) {
				return false;
			}

			for(IntVector::size_type a = 0; a < l.size() - 1; a++) {
				if(l[a] != r[a]) {
					return false;
				}
			}

			return true;
		}

		// Serialize this chapter in JSON format and save it to the given string.
		std::string &to_string_append(std::string &s) {
			// Separate each component with periods and separate the range with a tilde, if applicable.
			s.append(std::to_string(l[0]));
			for(IntVector::size_type a = 1; a < l.size(); a++) {
				s.append(1, '.').append(std::to_string(l[a]));
			}
			if(l != r) {
				s.append(" ~ ").append(std::to_string(r[0]));
				for(IntVector::size_type a = 1; a < r.size(); a++) {
					s.append(1, '.').append(std::to_string(r[a]));
				}
			}

			return s;
		}
	};

	// Overload of the less-than operator that checks the components to compare two chapters.
	bool operator<(Chapter const &l, Chapter const &r) {
		return l.get_l() != r.get_l() ? l.get_l() < r.get_l() : l.get_r() < r.get_r();
	}

	// Overload of equal-to operator that checks the components to compare two chapters.
	bool operator==(Chapter const &l, Chapter const &r) {
		return l.get_l() == r.get_l() && l.get_r() == r.get_r();
	}
} // namespace omm
