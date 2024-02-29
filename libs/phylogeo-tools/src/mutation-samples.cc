#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

int main(int argc, char* argv[]) {
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "generate set of mutation files")
	    ("in", po::value<string>(), "input file containig sample<tab>mutation")
	    ("out", po::value<string>(), "output folder")
	    ("min", po::value<int>()->default_value(0), "minimum value to store")
	;

	po::parsed_options parsed_options = po::command_line_parser(argc, argv)
		.options(desc)
		.run();

	po::variables_map vm;
	po::store(parsed_options, vm);

	ifstream fi(vm["in"].as<string>());
	map<string, vector<string>> mutation;
	for (string line, sample, mut; getline(fi, line); ) {
		istringstream is(line.c_str());
		is >> sample >> mut;
		mutation[mut].push_back(sample);
	}
	string folder = vm["out"].as<string>();
	//if (folder.size() == 0 || folder[folder.size() - 1] != '/')
	//	folder += '/';
	size_t mutation_min_size = vm["min"].as<int>();
	for (auto const & m : mutation) {
		if (m.second.size() < mutation_min_size) continue;
		ofstream fo(folder + m.first + ".txt");
		//bool first = true;
		for (auto const s : m.second) {
			//if (!first) fo << ",";
			fo << s << endl;
			//first = false;
		}
		//fo << endl;
	}
	return 0;
}
