#include <istream>
#include <string>
#include <map>
#include <set>
#include <cassert>


template<typename FILTER>
struct FastaLoader {

	FILTER filter;
	FastaLoader(FILTER filter = FILTER()) : filter(filter) {
	}

	void load(std::istream& file_alignment) {
		filter.clear();
		std::string seq, id;
		for (std::string line; getline(file_alignment, line); ) {
			//cerr << "L " << line << endl;
			if (line.size() == 0) continue;
			if (line[0] == '>') {
				if (id != "") {
					filter.process(id, seq);
				}
				//id = split(line.substr(1), '|')[0];
				id = line.substr(1);
				id = filter.id(id);
				seq = "";
				//cerr << "new id (" << id << ") " << id.size() << endl;
			} else {
				seq += line;
			}
		}
		if (id != "") {
			filter.process(id, seq);
		}
	}

	void load(std::string fn) {
		std::ifstream fi(fn);
		load(fi);
	}
};

struct FastaLoaderSequenceFilter {
	int dup_count=0;
	std::set<std::string> samples_found;

	std::ostream& os;
	const std::set<std::string>& samples;

	FastaLoaderSequenceFilter(std::ostream& os, const std::set<std::string>& samples) : os(os), samples(samples) {
	}

	void clear() {
		dup_count = 0;
		samples_found.clear();
	}

	std::string id(std::string raw_id) {
		if (raw_id.find("|") != std::string::npos) 
			raw_id = raw_id.substr(0, raw_id.find("|"));
		return raw_id;
	}

	void process(std::string id, std::string seq) {
		if (samples.find(id) != samples.end()) {
			if (samples_found.find(id) != samples_found.end()) {
				//cerr << "dup " << id << endl;
				dup_count++;
			} else {
			//cerr << "S " << id << endl;
				os << ">" << id << std::endl;
				os << seq << std::endl;
				samples_found.insert(id);
			}
		}
	}
};

struct FastaLoaderSequenceRename {
	int dup_count=0;
	std::set<std::string> samples_found;

	std::ostream& os;
	const std::map<std::string, std::string>& name_to_id;

	FastaLoaderSequenceRename(std::ostream& os, const std::map<std::string, std::string>& name_to_id) : os(os), name_to_id(name_to_id) {
	}

	void clear() {
		dup_count = 0;
		samples_found.clear();
	}

	std::string id(std::string raw_id) {
		return raw_id;
	}

	void process(std::string id, std::string seq) {
		auto i = name_to_id.find(id);
		if (i == name_to_id.end()) {
			std::cerr << "name not in metadata " << id << std::endl;
		}
		assert(i != name_to_id.end());
		os << ">" << i->second << std::endl;
		os << seq << std::endl;
		samples_found.insert(id);
	}
};

