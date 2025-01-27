#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include "tree.h"
#include "fasta.h"

int main(int argc, char* argv[]) {
	map<string, Metadata> metadata = load_map(argv[1]);

	cerr << "metadata loaded " << metadata.size() << endl;

	map<string, string> name_to_id;
	for (auto const & m : metadata) {
		string name = m.second.name;
		//if (name.rfind("hCoV-19/", 0) == 0) {
		//	name = name.substr(8);
		//}
		name_to_id[name] = m.second.id;
		//cerr << "X " << m.second.name << " " << m.second.id << endl;
	}

/*
	set<string> samples_found;
	string seq, id;
	for (string line; getline(cin, line); ) {
		//cerr << "L " << line << endl;
		if (line.size() == 0) continue;
		if (line[0] == '>') {
			//cerr << "S " << id << endl;
			if (id != "") {
				if (name_to_id.find(id) == name_to_id.end()) {
					cerr << "name not in metadata " << id << endl;
				}
				assert(name_to_id.find(id) != name_to_id.end());
				cout << ">" << name_to_id[id] << endl;
				cout << seq << endl;
				samples_found.insert(id);
			}
			//id = split(line.substr(1), '|')[0];
			id = line.substr(1);
			seq = "";
			//cerr << "new id (" << id << ") " << id.size() << endl;
		} else {
			seq += line;
		}
	}
	{
		assert(name_to_id.find(id) != name_to_id.end());
		cout << ">" << name_to_id[id] << endl;
		cout << seq << endl;
		samples_found.insert(id);
	}
*/
	FastaLoaderSequenceRename filter(cout, name_to_id);
	FastaLoader<FastaLoaderSequenceRename> fastaLoader(filter);
	fastaLoader.load(cin);

	cerr << "saved " << filter.samples_found.size() << " samples" << endl;
	int samples_notfount_count = 0;
	for (auto const& s:name_to_id) {
		if (filter.samples_found.find(s.first) == filter.samples_found.end()) {
			if (samples_notfount_count < 10)
				cerr << "W " << s.first << " ";
			samples_notfount_count++;
		}
	}
	if (samples_notfount_count > 0) {
		cerr << "not found " << samples_notfount_count << " samples" << endl;
	}


	return 0;
}
