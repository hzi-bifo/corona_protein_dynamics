#include "tree.h"
#include "state.h"
#include <boost/program_options.hpp>
namespace po = boost::program_options;

typedef int Data;
typedef Node<StateInOut,Data> INode;
array<string, StateInOut::size> StateInOut::names = {"UK", "nonUK"}; 

int main(int argc, char* argv[]) {
	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "Run sankoff")
	    ("metadata", po::value<string>(), "metadata file")
	    ("in", po::value<string>(), "input tree")
	    ("out", po::value<string>(), "output tree")
	    ("location_label,l", po::value<vector<string>>()->multitoken(), "location labels, e.g. Germany nonGermany")
	;
	po::parsed_options parsed_options = po::command_line_parser(argc, argv)
		.options(desc)
		.run();
	po::variables_map vm;
	po::store(parsed_options, vm);

	//cerr << vm["location_label"].as<vector<string>>() << endl;
	StateInOut::names = {vm["location_label"].as<vector<string>>()[0], vm["location_label"].as<vector<string>>()[1]};
	string tree = vm["in"].as<string>();
	string splitted_tree_prefix = vm["out"].as<string>();
	string annotation = vm["metadata"].as<string>();

	Node<StateInOut> phylo = load_tree<INode>(tree);

	map<string, Metadata> metadata = load_map(annotation);
	cerr << "metadata loaded" << endl;

//	mul_to mul_to_5(1.0/5.0);
//	phylo.apply(mul_to_5);

	int int_index = 0;
	phylo.name_internal_nodes(int_index);

	//phylo.split_and_print(splitted_tree_prefix, index);
	NodePrinter<INode> np(splitted_tree_prefix, 0, StateInOut(StateInOut::Type::IN));
	np.find_and_print_lineage(phylo);

	int removed_count = 0;
	

	cerr << "Saved " << np.index << " trees with prefix " << splitted_tree_prefix << " removed nodes: " << removed_count << endl;
	return 0;
}

