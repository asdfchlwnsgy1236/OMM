#pragma once

#include <algorithm>
#include <cctype>
#include <string>

using namespace std::string_literals;

// Exclusive namespace for the OMM.
namespace omm {
	// Returns true if the given string has a number at the given index, false otherwise.
	bool is_number(std::string const &s, std::string::size_type const index = 0) {
		return std::isdigit(s[index]) != 0;
	}

	// Returns true if the left string comes before the right string according to natural ordering, false otherwise.
	bool natural_compare(std::string const &l, std::string const &r) {
		// If either is empty, comparison is trivial.
		if(l.empty() || r.empty()) {
			return l < r;
		}

		// The number digit characters used for the comparison.
		std::string digits("0123456789");
		// The variables used to compare the strings by token without making copies.
		std::string::size_type li = 0, ri = 0, lc = is_number(l) ? l.find_first_not_of(digits) : l.find_first_of(digits),
							   rc = is_number(r) ? r.find_first_not_of(digits) : r.find_first_of(digits), oli, ori;

		// Begin tokenization and comparison.
		while(true) {
			// The result of lexicographical comparison of the current pair of tokens.
			int cmpRes = l.compare(li, lc, r, ri, rc);

			// If either side has run out of tokens, compare lexicographically.
			if(li == l.size() || ri == r.size()) {
				return cmpRes < 0;
			}

			// If the two tokens are lexicographically different...
			if(cmpRes != 0) {
				// ... and both are numbers of different values, then compare as numbers.
				if(is_number(l, li) && is_number(r, ri) && std::stoi(l.data() + li) != std::stoi(r.data() + ri)) {
					return std::stoi(l.data() + li) < std::stoi(r.data() + ri);
				}

				// Otherwise, compare lexicographically.
				return cmpRes < 0;
			}

			// The tokens are equal, so update the variables for the next pair of tokens.
			oli = li, ori = ri, li += std::min(lc, l.size() - li), ri += std::min(rc, r.size() - ri),
			lc = (is_number(l, oli) ? l.find_first_of(digits, li + 1) : l.find_first_not_of(digits, li + 1)) - li,
			rc = (is_number(r, ori) ? r.find_first_of(digits, ri + 1) : r.find_first_not_of(digits, ri + 1)) - ri;
		}
	}

	// Returns a copy of the given string with leading and trailing whitespace removed.
	std::string trim(std::string const &s) {
		std::string ws(" \f\n\r\t\v");
		auto li = s.find_first_not_of(ws), ri = s.find_last_not_of(ws);

		return li == std::string::npos ? ""s : s.substr(li, ri - li + 1);
	}
} // namespace omm
