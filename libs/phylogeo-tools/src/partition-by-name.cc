#include "tree.h"
#include "state.h"
#include <boost/program_options.hpp>
#include "rangetree.h"
#include <vector>
#include <numeric>

namespace po = boost::program_options;

string replace_all(string src, string from, string to) {
	while(src.find(from) != string::npos)
		src.replace(src.find(from), from.length(), to);
	return src;
}

array<string, StateInOut::size> StateInOut::names = {"uk", "nonuk"};

typedef int Data;
typedef Node<StateInOut, Data> INode;

//we assume branch_length represets dissimilarity
struct SubPartitionByName {
	set<string> sub_partitions;
	string tree_filename_template, sample_filename_template;
	bool print_location, print_label;
	SubPartitionByName(const vector<string>& sub_partitions, string tree_filename_template, string sample_filename_template, bool print_location = true, bool print_label = true) : 
		sub_partitions(sub_partitions.begin(), sub_partitions.end()),
		tree_filename_template(tree_filename_template), 
		sample_filename_template(sample_filename_template),
		print_location(print_location), 
		print_label(print_label) {
	}

	void run(const INode& n) {
		if (sub_partitions.find(n.label) != sub_partitions.end()) {
			string fo_name = replace_all(tree_filename_template, "?", n.label);
			ofstream fo(fo_name);
			NodePrinterGeneral<INode> np(print_location, print_label);
			//np.print(fo, phylo) << ";" << endl;
			np.print(fo, n) << ";" << endl;
			cerr << "sub-tree saved " << n.label << " to " << fo_name << " ss=" << n.sample_size << endl;

			string fo_sample_name = replace_all(sample_filename_template, "?", n.label);
			write_sample_file(fo_sample_name, n);
			cerr << "sub-tree samples saved " << n.label << " to " << fo_sample_name << endl;
		} else {
			for (auto &c: n.children) {
				run(c);
			}
		}
	}

	void write_sample_file_dfs(ostream& fo, const INode& n, int& all_in, int& all_out) {
		if (n.isLeaf()) {
			if (n.location == StateInOut::IN)
				all_in++;
			else
				all_out++;
			fo << n.label << endl;
		}
		for (auto const& c : n.children) {
			write_sample_file_dfs(fo, c, all_in, all_out);
		}
	}

	void write_sample_file(string fn, const INode& n) {
		int all_in=0, all_out=0;
		ofstream fo(fn);
		write_sample_file_dfs(fo, n, all_in, all_out);
		cerr << "Samples in=" << all_in << " out=" << all_out << endl;
	}

};


// Algorithm: extracts sub-trees based on names. 
//   For each (internal) node with name in 'par', creates a tree file with template in trees and sample file 
//   with template of samples.
int main(int argc, char* argv[]) {

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "Run sankoff")
	    ("in", po::value<string>(), "input tree")
	    ("par", po::value<vector<string>>()->multitoken(), "name of partitions")
	    ("samples", po::value<string>(), "samples file template")
	    ("trees", po::value<string>(), "trees file template")
	    ("location_label,l", po::value<vector<string>>()->multitoken(), "location labels, e.g. Germany nonGermany")
	    ("print-annotation", po::value<bool>()->default_value(true), "print annotation")
	    ("print-internal-node-label", po::value<bool>()->default_value(true), "print internal node labels")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);

	try {
		po::notify(vm);
	} catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	StateInOut::names = {vm["location_label"].as<vector<string>>()[0], vm["location_label"].as<vector<string>>()[1]};

	string tree_file_name = vm["in"].as<string>();
	INode phylo = load_tree<INode>(tree_file_name) ;

	SubPartitionByName subPartitionByName(vm["par"].as<vector<string>>(), vm["trees"].as<string>(), vm["samples"].as<string>(), vm["print-annotation"].as<bool>(), vm["print-internal-node-label"].as<bool>());
	subPartitionByName.run(phylo);

	return 0;
}
