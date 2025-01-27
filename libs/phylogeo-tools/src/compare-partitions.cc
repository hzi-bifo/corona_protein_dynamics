#include "tree.h"
#include <boost/assign/list_of.hpp>
#include <boost/program_options.hpp>
#include <boost/version.hpp>


int main(int argc, char* argv[]) {

	namespace po = boost::program_options;
	po::options_description desc("Allowed options");

	desc.add_options()
		("set", po::value<vector<string>>()->multitoken(), "set")
			    ;

	po::parsed_options parsed_options = po::command_line_parser(argc, argv)
		.options(desc)
		.run();

	std::vector<std::vector<std::string>> sets;
	for (const po::option& o : parsed_options.options) {
		if (o.string_key == "set")
			sets.push_back(o.value);
	}


	po::variables_map vm;
	po::store(parsed_options, vm);
	//po::notify(vm);

	for (auto const& vv : sets) {
		cerr << "SET " << vv << endl;
	}


	return 0;
}
