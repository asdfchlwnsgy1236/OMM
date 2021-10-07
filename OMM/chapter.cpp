#include "chapter.hpp"

namespace omm {
	bool operator<(Chapter const &l, Chapter const &r) {
		return l.get_l() == r.get_l() ? l.get_r() < r.get_r() : l.get_l() < r.get_l();
	}

	bool operator==(Chapter const &l, Chapter const &r) {
		return l.get_l() == r.get_l() && l.get_r() == r.get_r();
	}
} // namespace omm
