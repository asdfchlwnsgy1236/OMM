#include "entry.hpp"

#include <QtDebug>

namespace omm {
	bool operator<(Entry const &l, Entry const &r) {
		StringVector const keys{u"Title"_qs, u"Type"_qs, u"Author"_qs, u"Year"_qs};
		try {
			for(auto const &key: keys) {
				if(l.at(key) != r.at(key)) {
					return l.less(l.at(key), r.at(key));
				}
			}
		}
		catch(std::exception const &e) {
			qDebug() << u"This exception should not occur (Entry less than):"_qs << e.what();
		}

		return false;
	}

	bool operator==(Entry const &l, Entry const &r) {
		StringVector const keys{u"Title"_qs, u"Type"_qs, u"Author"_qs, u"Year"_qs};
		try {
			for(auto const &key: keys) {
				if(l.at(key) != r.at(key)) {
					return false;
				}
			}
		}
		catch(std::exception const &e) {
			qDebug() << u"This exception should not occur (Entry equal to):"_qs << e.what();
		}

		return true;
	}
} // namespace omm
