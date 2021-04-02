#include <algorithm>
#include <cctype>
#include <chrono>
// TODO: Remove the line below if I end up not using std::atoi.
//#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

using namespace std::string_literals;

namespace omm{
	// Forward declaration of classes.
	class Counts;
	class Chapter;
	class Entry;
	class Entries;
	class Save;

	// Aliases
	using Chapters = std::vector<Chapter>;
	using IntVector = std::vector<int>;
	using StringVector = std::vector<std::string>;
	using EntryVector = std::vector<Entry>;

	// Forward declaration of functions.
	bool is_number(std::string const &s, std::string::size_type const index = 0);
	bool natural_compare(std::string const &l, std::string const &r);
	std::string trim(std::string const &s);

	// The class that manages a group of counts of entries separated using an element as the standard.
	class Counts{
		private:
		// The name of this group of counts.
		std::string name;
		// The map containing the counts as key-value pairs.
		std::map<std::string, int> counts;

		public:
		// Custom constructor that takes a string as the name for this group of counts.
		Counts(std::string &&_name): name(std::move(_name)){
			// Empty.
		}

		// Overload of the subscript operator that accesses the underlying map object.
		int &operator[](std::string const &key){
			return counts[key];
		}

		// Wrapper for counts.begin() for iteration.
		auto begin(){
			return counts.begin();
		}

		// Wrapper for counts.end() for iteration.
		auto end(){
			return counts.end();
		}

		// Serialize this group of counts in JSON format and save it to the given string.
		std::string &to_string_append(std::string &s){
			// Serialize the name as the key.
			s.append("\t\"").append(name).append("\": {");

			// Serialize the counts as key-value pairs.
			for(auto ci = counts.cbegin(); ci != counts.cend(); ci++){
				if(ci != counts.cbegin()){
					s.append(1, ',');
				}
				s.append("\n\t\t\"").append(ci->first).append("\": \"").append(std::to_string(ci->second)).append(1, '"');
			}
			s.append("\n\t}");

			return s;
		}
	};

	// The class that manages a chapter.
	class Chapter{
		private:
		// The vector containing the components of the left end of the range of chapters or the single chapter.
		IntVector l;
		// The vector containing the components of the right end of the range of chapters or the single chapter.
		IntVector r;

		public:
		// Constructor that takes a chapter in string form and converts it for use.
		// When construction fails, the Chapter object is empty.
		Chapter(std::string &&chapter): l(), r(){
			// Index for the range symbol.
			auto ri = chapter.find('~');

			// TODO: Change the stoi to atoi if stoi turns out to be too slow.
			try{
				// The given chapter is not a range.
				if(ri == std::string::npos){
					// Fill the chapter components by parsing through each number token.
					l.push_back(std::stoi(chapter));
					for(auto si = chapter.find('.'); si != std::string::npos; si = chapter.find('.', si + 1)){
						l.push_back(std::stoi(chapter.data() + si + 1));
					}
					r = l;
				}
				// The given chapter is a range.
				else{
					// Fill the left chapter components by parsing through each number token.
					l.push_back(std::stoi(chapter));
					for(auto si = chapter.find('.'); si != std::string::npos && si < ri; si = chapter.find('.', si + 1)){
						l.push_back(std::stoi(chapter.data() + si + 1));
					}

					// Fill the right chapter components by continuing the parse after the range symbol.
					r.push_back(std::stoi(chapter.data() + ri + 1));
					for(auto si = chapter.find('.', ri + 1); si != std::string::npos; si = chapter.find('.', si + 1)){
						r.push_back(std::stoi(chapter.data() + si + 1));
					}
				}
			}
			catch(std::exception const &){
				// The given string is not a chapter or range of chapters, so reset this chapter object to indicate failure.
				l.clear(), r.clear();
			}

			// If it is a range with the two ends having different depths, it is malformed, so reset this chapter object to indicate failure.
			if(!is_valid()){
				l.clear(), r.clear();
			}
		}

		// Copy constructor that takes two vectors of integers containing the chapter components.
		Chapter(IntVector const &_l, IntVector const &_r): l(_l), r(_r){
			// Empty.
		}

		// Move constructor that takes two vectors of integers containing the chapter components.
		Chapter(IntVector &&_l, IntVector &&_r): l(std::move(_l)), r(std::move(_r)){
			// Empty.
		}

		// Returns true if the two given chapters overlap, false otherwise.
		static bool does_overlap(Chapter &c1, Chapter &c2){
			return c1.get_l() <= c2.get_r() && c1.get_r() >= c2.get_l();
		}

		// Merges the two given overlapping chapters into the first and erases the second if in the list.
		static void merge_chapters(Chapters &chapters, Chapter &c1, Chapter &c2){
			c1.get_l() = std::move(c1.get_l() > c2.get_l() ? c2.get_l() : c1.get_l());
			c1.get_r() = std::move(c1.get_r() < c2.get_r() ? c2.get_r() : c1.get_r());
			if(auto c2i = std::find(chapters.cbegin(), chapters.cend(), c2); c2i != chapters.cend()){
				chapters.erase(c2i);
			}
		}

		// Splits the given chapter from the given list of chapters based on the given
		// pivot chapter located in the given chapter; the pivot chapter is removed.
		static void split_chapters(Chapters &chapters, Chapter &chapter, Chapter &pivot){
			// If the leftmost chapter is the pivot chapter....
			if(chapter.get_l() == pivot.get_l()){
				// .... and if the chapter is a single chapter, then remove it.
				if(chapter.get_r() == pivot.get_l()){
					chapters.erase(std::find(chapters.cbegin(), chapters.cend(), chapter));
				}
				else{
					// Otherwise, remove the leftmost chapter.
					chapter.get_l().back()++;
				}
			}
			else if(chapter.get_r() == pivot.get_l()){
				// If the rightmost chapter is the pivot chapter, then remove the rightmost chapter.
				chapter.get_r().back()--;
			}
			else{
				// Otherwise, split the range of chapters, and remove the pivot chapter.
				pivot.get_l().back()--;
				chapters.insert(std::find(chapters.cbegin(), chapters.cend(), chapter),
					std::move(Chapter(chapter.get_l(), pivot.get_l())));
				pivot.get_l().back() += 2;
				chapter.get_l() = std::move(pivot.get_l());
			}
		}

		// Getter for the left chapter.
		IntVector &get_l(){
			return l;
		}

		// Getter for the right chapter.
		IntVector &get_r(){
			return r;
		}

		// Const getter for the left chapter.
		IntVector const &get_l() const{
			return l;
		}

		// Const getter for the right chapter.
		IntVector const &get_r() const{
			return r;
		}

		// Returns true if the chapter is valid, false otherwise.
		// A chapter is valid if its left and right have the same number of components,
		// and each component in its left and right are the same except for the last component.
		bool is_valid(){
			if(l.size() != r.size()){
				return false;
			}

			for(IntVector::size_type a = 0; a < l.size() - 1; a++){
				if(l[a] != r[a]){
					return false;
				}
			}

			return true;
		}

		// Serialize this chapter in JSON format and save it to the given string.
		std::string &to_string_append(std::string &s){
			// Separate each component with periods and separate the range with a tilde, if applicable.
			s.append(std::to_string(l[0]));
			for(IntVector::size_type a = 1; a < l.size(); a++){
				s.append(1, '.').append(std::to_string(l[a]));
			}
			if(l != r){
				s.append(" ~ ").append(std::to_string(r[0]));
				for(IntVector::size_type a = 1; a < r.size(); a++){
					s.append(1, '.').append(std::to_string(r[a]));
				}
			}

			return s;
		}
	};

	// Overload of the less-than operator that checks the components to compare two chapters.
	bool operator<(Chapter const &l, Chapter const &r){
		return l.get_l() != r.get_l() ? l.get_l() < r.get_l() : l.get_r() < r.get_r();
	}

	// Overload of equal-to operator that checks the components to compare two chapters.
	bool operator==(Chapter const &l, Chapter const &r){
		return l.get_l() == r.get_l() && l.get_r() == r.get_r();
	}

	// The class that manages an entry.
	class Entry{
		private:
		// The map containing most of the elements of this entry.
		std::map<std::string, std::string> details;
		// The vector containing the list of liked chapters.
		Chapters likedChapters;
		// The vector containing the list of loved chapters.
		Chapters lovedChapters;

		// Helper function for to_string_append() that appends the given chapters in serialized form to the given string.
		static std::string &to_string_append_chapters(std::string &s, Chapters &chapters){
			for(Chapters::size_type a = 0; a < chapters.size(); a++){
				if(a > 0){
					s.append(", ");
				}
				s.append(1, '"');
				chapters[a].to_string_append(s).append(1, '"');
			}

			return s;
		}

		public:
		// Default constructor to initialize the necessary elements.
		Entry(): details(), likedChapters(), lovedChapters(){
			// Fill the details map with the default elements.
			StringVector keys{"Title", "Original Title", "Franchise/Series", "Franchise/Series Order",
				"Author", "Year", "Type", "Language", "Rating", "Progress", "Notes"};
			for(auto &&key : keys){
				details[std::move(key)].clear();
			}
		}

		// Wrapper for the subscript operator that accesses the underlying map object.
		auto &operator[](std::string const &key){
			return details[key];
		}

		// Wrapper for at() that accesses the underlying map object.
		auto const &at(std::string const &key) const{
			return details.at(key);
		}

		// Get the number of elements in the details.
		auto size() const noexcept{
			return details.size();
		}

		// Add the given chapter to the list of liked or loved chapters depending on the value of isLoved.
		void add_chapter(std::string &chapter, bool isLoved){
			// Convert the given chapter into a convenient form.
			Chapter toadd(std::move(chapter));
			// Get the target list of chapters.
			Chapters &chapters = isLoved ? lovedChapters : likedChapters;

			// If the given chapter overlaps with an existing chapter, then merge them.
			for(auto &cc : chapters){
				if(Chapter::does_overlap(cc, toadd)){
					Chapter::merge_chapters(chapters, cc, toadd);

					return;
				}
			}

			// Otherwise, simply add the given chapter.
			chapters.push_back(std::move(toadd));
		}

		// Delete the given liked chapter.
		void delete_chapter(std::string &chapter, bool isLoved){
			// Convert the given chapter into a convenient form.
			Chapter todelete(std::move(chapter));
			// Get the target list of chapters.
			Chapters &chapters = isLoved ? lovedChapters : likedChapters;

			// If the given chapter exists, then delete it.
			for(auto &cc : chapters){
				if(Chapter::does_overlap(cc, todelete)){
					Chapter::split_chapters(chapters, cc, todelete);

					return;
				}
			}
		}

		// Fix any reversed ranges of chapters, sort the chapters, and merge any overlapping ranges of chapters.
		void organize_chapters(){
			// Handle both liked and loved chapters without duplicating code.
			std::vector<Chapters *> chaptervectors{&likedChapters, &lovedChapters};
			for(auto &chapters : chaptervectors){
				// Fix any reversed ranges of chapters.
				for(auto &chapter : *chapters){
					if(chapter.get_l() > chapter.get_r()){
						chapter.get_l().swap(chapter.get_r());
					}
				}

				// If the list of chapters has more than one element....
				if(chapters->size() > 1){
					// .... then sort the list of chapters....
					std::sort(chapters->begin(), chapters->end());

					// .... and merge any overlapping ranges of chapters.
					for(Chapters::size_type a = 0; a < chapters->size() - 1; a++){
						if(Chapter::does_overlap((*chapters)[a], (*chapters)[a + 1])){
							Chapter::merge_chapters((*chapters), (*chapters)[a], (*chapters)[a + 1]);
						}
					}
				}
			}
		}

		// Getter for the underlying vector object for the liked chapters of this entry.
		Chapters &get_likedChapters(){
			return likedChapters;
		}

		// Getter for the underlying vector object for the loved chapters of this entry.
		Chapters &get_lovedChapters(){
			return lovedChapters;
		}

		// Serialize this entry in JSON format and save it to the given string.
		std::string &to_string_append(std::string &s){
			// Serialize the elements as key-value pairs.
			s.append("\t\t{");
			for(auto di = details.cbegin(); di != details.cend(); di++){
				if(di != details.cbegin()){
					s.append(1, ',');
				}
				s.append("\n\t\t\t\"").append(di->first).append("\": \"").append(di->second).append(1, '"');
			}

			// Serialize the liked and loved chapters as an array.
			s.append(",\n\t\t\t\"Liked Chapters\": [");
			to_string_append_chapters(s, likedChapters).append("],\n\t\t\t\"Loved Chapters\": [");
			to_string_append_chapters(s, lovedChapters).append("]\n\t\t}");

			return s;
		}
	};

	// Overload of the less-than operator that uses natural ordering to compare the title and type of two entries.
	bool operator<(Entry const &l, Entry const &r){
		std::string const tkey("title"), tykey("type");
		try{
			return l.at(tkey) != r.at(tkey) ? natural_compare(l.at(tkey), r.at(tkey)) :
				natural_compare(l.at(tykey), r.at(tykey));
		}
		catch(std::exception const &e){
			std::cout << "This exception should not occur: " << e.what() << '\n';
		}

		return false;
	}

	// Overload of the equal-to operator that checks the title, type, author, and year for equality of two entries.
	bool operator==(Entry const &l, Entry const &r){
		try{
			StringVector const keys{"title", "type", "author", "year"};
			for(auto const &key : keys){
				if(l.at(key) != r.at(key)){
					return false;
				}
			}
		}
		catch(std::exception const &e){
			std::cout << "This exception should not occur: " << e.what() << '\n';
		}

		return true;
	}

	// The class that manages a list of entries.
	class Entries{
		private:
		// The vector containing the list of entries.
		EntryVector entries;

		public:
		// Overload of the subscript operator that accesses the underlying vector object.
		Entry &operator[](EntryVector::size_type const index){
			return entries[index];
		}

		// Wrapper for entries.begin() for iteration.
		auto begin(){
			return entries.begin();
		}

		// Wrapper for entries.end() for iteration.
		auto end(){
			return entries.end();
		}

		// Get the number of entries in the list.
		auto size() const noexcept{
			return entries.size();
		}

		// Add the given entry to the list of entries; the given entry contains no data after this.
		void add_entry(Entry &&entry){
			entries.push_back(std::move(entry));
		}

		// Duplicate the given entry and insert the duplicate right after the given entry.
		void duplicate_entry(Entry &entry){
			auto const entryIt = std::find(entries.cbegin(), entries.cend(), entry);
			if(entryIt != entries.cend()){
				entries.insert(entryIt + 1, entry);
			}
		}

		// Deletes the given entry from the list of entries.
		void delete_entry(Entry &entry){
			auto const entryIt = std::find(entries.cbegin(), entries.cend(), entry);
			if(entryIt != entries.cend()){
				entries.erase(entryIt);
			}
		}

		// Sort the list of entries according to the specifications.
		// Specifically, group the entries by franchise/series, order the entries within each franchise/series by
		// story order, title, then type, order the groups of franchise/series by franchise/series name, and then
		// have the rest of the entries come after ordered by title, then type, and finally,
		// organize the liked and loved chapters of each entry.
		void sort(){
			// Partition the list of entries into those that are members of a franchise or series, and those that are not.
			std::string fskey("franchiseSeries"), fsokey("franchiseSeriesOrder");
			auto partIter = std::partition(entries.begin(), entries.end(), [&](Entry &entry){
				return !entry[fskey].empty();
			});

			// Sort the entries that are members of a franchise or series separately first.
			std::sort(entries.begin(), partIter, [&](Entry &l, Entry &r){
				return l[fskey] != r[fskey] ? natural_compare(l[fskey], r[fskey]) :
					l[fsokey] != r[fsokey] ? std::stoi(l[fsokey]) < std::stoi(r[fsokey]) : l < r;
			});

			// Sort the rest of the entries separately after.
			std::sort(partIter, entries.end());

			// Organize the liked and loved chapters of each entry.
			for(auto &entry : entries){
				entry.organize_chapters();
			}
		}

		// Serialize this list of entries in JSON format and save it to the given string.
		std::string &to_string_append(std::string &s){
			// Serialize the name as the key.
			s.append("\t\"entries\": [");

			// Serialize the list of entries using their serialization function.
			for(EntryVector::size_type a = 0; a < entries.size(); a++){
				if(a > 0){
					s.append(1, ',');
				}
				s.append(1, '\n');
				entries[a].to_string_append(s);
			}
			if(!entries.empty()){
				s.append("\n\t");
			}
			s.append(1, ']');

			return s;
		}
	};

	// The class that manages a save of the OMM.
	class Save{
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
		Save(): id("OMM_"), countTotal(0), countByType("countByType"), countByLanguage("countByLanguage"),
			countByProgress("countByProgress"), entries(){
			// Initialize id.
			auto tn = std::chrono::system_clock::now().time_since_epoch();
			struct std::tm tm{};
			auto t = std::chrono::duration_cast<std::chrono::seconds>(tn).count();
			char ts[128]{};
			if(localtime_s(&tm, &t) == 0 && std::strftime(ts, sizeof(ts), "%F_%T_%Z_%z_%u_%V", &tm) > 0){
				// Format: [date]_[time]_[time zone]_[offset from UTC]_[day of the week]_[week of the year]
				// 24-hour format date and time with preceding zeros and milliseconds,
				// day of the week 1 ~ 7, week of the year 1 ~ 53.
				id.append(ts);
				std::string millitime =
					std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(tn).count() % 1000);
				millitime.insert(0, 1, '.');
				if(millitime.size() < 4){
					millitime.insert(1, 4 - millitime.size(), '0');
				}
				id.insert(id.find_last_of(':') + 3, millitime);
			}
			else{
				id.append(std::to_string(std::chrono::system_clock::now().time_since_epoch().count() / 1000000));
			}

			// Initialize the counts by filling them with the default elements.
			StringVector typekeys{"Manga", "Anime", "Light Novel", "Web Novel", "Visual Novel", "Anime Film", "OVA"},
				languagekeys{"Japanese", "Korean", "Chinese", "English"},
				progresskeys{"Not Started", "In Progress", "Finished"};
			for(auto &&key : typekeys){
				countByType[std::move(key)] = 0;
			}
			for(auto &&key : languagekeys){
				countByLanguage[std::move(key)] = 0;
			}
			for(auto &&key : progresskeys){
				countByProgress[std::move(key)] = 0;
			}
		}

		// Recalculate all counts.
		void re_count(){
			// Reset all counts.
			for(auto &count : countByType){
				count.second = 0;
			}
			for(auto &count : countByLanguage){
				count.second = 0;
			}
			for(auto &count : countByProgress){
				count.second = 0;
			}

			// Redo all counts.
			countTotal = entries.size();
			for(auto const &entry : entries){
				// Increment the corresponding type or unspecified if not applicable.
				if(entry.at("type").empty()){
					countByType["unspecified"]++;
				}
				else{
					countByType[entry.at("type")]++;
				}

				// Increment the corresponding language or unspecified if not applicable.
				if(entry.at("language").empty()){
					countByLanguage["unspecified"]++;
				}
				else{
					countByLanguage[entry.at("language")]++;
				}

				// Increment the corresponding progress.
				if(entry.at("progress").empty()){
					countByProgress["notStarted"]++;
				}
				else if(entry.at("progress") == "Finished"){
					countByProgress["finished"]++;
				}
				else{
					countByProgress["inProgress"]++;
				}
			}
		}

		// Refresh the counts and entries.
		void refresh(){
			// Recalculate the counts and re-sort the entries.
			re_count();
			entries.sort();
		}

		// Serialize this save in JSON format and append it to the given string.
		std::string &to_string_append(std::string &s){
			// Build the final serialized string for the save.
			s.append("{\n\t\"_id\": \"").append(id).append("\",\n\t\"countTotal\": \"")
				.append(std::to_string(countTotal)).append("\",\n");
			countByType.to_string_append(s).append(",\n");
			countByLanguage.to_string_append(s).append(",\n");
			countByProgress.to_string_append(s).append(",\n");
			entries.to_string_append(s).append("\n}");

			return s;
		}

		// Deserialize the given save; returns true if successful, false otherwise.
		bool from_string(std::stringstream &ss){
			// Temporary strings used for reading keys and values from the serialized save.
			std::string sk, sv;

			// Begin deserialization.
			while(true){
				// If the end of the save has not been reached, get a key.
				ss.ignore(std::numeric_limits<std::streamsize>::max(), '"');
				if(!ss.good()){
					break;
				}
				std::getline(ss, sk, '"');

				// Get the value differently based on the key.
				if(sk == "_id" || sk == "countTotal"){
					// If the key is "_id" or "countTotal", simply get the value after it.
					ss.ignore(std::numeric_limits<std::streamsize>::max(), '"');
					std::getline(ss, sv, '"');
					if(is_number(sv)){
						countTotal = std::stoi(sv);
					}
					else{
						id = std::move(sv);

						// If the id is invalid, abort deserialization.
						if(id.compare(0, 3, "OMM") != 0){
							return false;
						}
					}
				}
				else if(sk.compare(0, 7, "countBy") == 0){
					// If the count group is empty, then move on.
					ss.ignore(std::numeric_limits<std::streamsize>::max(), '{');
					if(ss.seekg(3, ss.cur).peek() == '}'){
						continue;
					}

					// If the key starts with "countBy", then loop to get the key-value pairs after it.
					while(true){
						// Get a key-value pair.
						ss.ignore(std::numeric_limits<std::streamsize>::max(), '"');
						std::getline(ss, sk, '"');
						ss.ignore(std::numeric_limits<std::streamsize>::max(), '"');
						std::getline(ss, sv, '"');

						// Save to the appropriate count group.
						if(sk == "countByType"){
							countByType[sk] = std::stoi(sv);
						}
						else if(sk == "countByLanguage"){
							countByLanguage[sk] = std::stoi(sv);
						}
						else if(sk == "countByProgress"){
							countByProgress[sk] = std::stoi(sv);
						}
						else{
							// If the key is invalid, abort deserialization.
							return false;
						}

						// If this key-value pair is the last of this count group, stop this loop.
						if(ss.get() != ','){
							break;
						}
					}
				}
				else if(sk == "entries"){
					// If there are no entries, then move on.
					ss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
					if(ss.seekg(3, ss.cur).peek() == ']'){
						continue;
					}

					// If the key is "entries", then loop to get the entry elements after it.
					while(true){
						// Rebuild an entry from the serialized save.
						Entry entry;
						while(true){
							// Get a key.
							ss.ignore(std::numeric_limits<std::streamsize>::max(), '"');
							std::getline(ss, sk, '"');

							// Get the value differently based on the key.
							if(sk.compare(6, 8, "Chapters") == 0){
								// If there are no chapters, then move on.
								ss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
								if(ss.seekg(1, ss.cur).peek() == ']'){
									continue;
								}

								// If the key ends with "Chapters", loop to get the list of values after it.
								while(true){
									// Get a value.
									ss.ignore(std::numeric_limits<std::streamsize>::max(), '"');
									std::getline(ss, sv, '"');

									// Add to the appropriate chapters list.
									if(sk == "Liked Chapters"){
										entry.add_chapter(sv, false);
									}
									else if(sk == "Loved Chapters"){
										entry.add_chapter(sv, true);
									}
									else{
										// If the key is invalid, abort deserialization.
										return false;
									}

									// If this value is the last of this chapters list, stop this loop.
									if(ss.get() != ','){
										break;
									}
								}
							}
							else{
								// Otherwise, get the value after it while taking care of the possibility of double quotes.
								ss.ignore(std::numeric_limits<std::streamsize>::max(), '"');
								std::getline(ss, sv);
								sv.erase(sv.find_last_of('"'));
								entry[sk] = std::move(sv);
							}

							// If this key-value pair is the last of this entry, stop this loop.
							if(ss.get() != ','){
								break;
							}
						}

						// Add the rebuilt entry to the list of entries.
						entries.add_entry(std::move(entry));

						// If this entry is the last of the list of entries, stop this loop.
						ss.ignore(std::numeric_limits<std::streamsize>::max(), '}');
						if(ss.get() != ','){
							break;
						}
					}
				}
				else{
					// If the key is invalid, abort deserialization.
					return false;
				}
			}

			// Deserialization ended successfully.
			return true;
		}

		// TODO: Write wrapper functions for the ones in the contained classes as necessary.
				// Wrapper for entries.add_entry() to make it available for use in the save.
		void add_entry(Entry &&entry){
			entries.add_entry(std::move(entry));
		}
	};

	// Returns true if the given string has a number at the given index, false otherwise.
	bool is_number(std::string const &s, std::string::size_type const index){
		return std::isdigit(s[index]) != 0;
	}

	// Returns true if the left string comes before the right string according to natural ordering, false otherwise.
	bool natural_compare(std::string const &l, std::string const &r){
		// If either is empty, comparison is trivial.
		if(l.empty() || r.empty()){
			return l < r;
		}

		// The number digit characters used for the comparison.
		std::string digits("0123456789");
		// The variables used to compare the strings by token without making copies.
		std::string::size_type li = 0, ri = 0, lc = is_number(l) ? l.find_first_not_of(digits) : l.find_first_of(digits),
			rc = is_number(r) ? r.find_first_not_of(digits) : r.find_first_of(digits), oli, ori;

		// Begin tokenization and comparison.
		while(true){
			// The result of lexicographical comparison of the current pair of tokens.
			int cmpRes = l.compare(li, lc, r, ri, rc);

			// If either side has run out of tokens, compare lexicographically.
			if(li == l.size() || ri == r.size()){
				return cmpRes < 0;
			}

			// If the two tokens are lexicographically different....
			if(cmpRes != 0){
				// .... and both are numbers of different values, then compare as numbers.
				if(is_number(l, li) && is_number(r, ri) && std::stoi(l.data() + li) != std::stoi(r.data() + ri)){
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
	std::string trim(std::string const &s){
		std::string ws(" \f\n\r\t\v");
		auto li = s.find_first_not_of(ws), ri = s.find_last_not_of(ws);

		return li == std::string::npos ? ""s : s.substr(li, ri - li + 1);
	}
}

// Main function to test the classes and functions I write.
int main(){
	auto start = std::chrono::steady_clock::now();
	omm::Save save;
	save.add_entry(omm::Entry());
	std::string s;
	save.to_string_append(s);
	std::cout << s << '\n';
	auto end = std::chrono::steady_clock::now();
	std::cout << "time to serialize a save (seconds): " << std::chrono::duration<double>(end - start).count() << '\n' <<
		"size of serialized save (bytes): " << s.size() << '\n';

	return 0;
}
