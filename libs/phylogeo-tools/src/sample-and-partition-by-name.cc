#include "tree.h"
#include "state.h"
#include <boost/program_options.hpp>
#include "rangetree.h"
#include <vector>
#include <numeric>

namespace po = boost::program_options;

array<string, StateInOut::size> StateInOut::names = {"uk", "nonuk"};

struct Data;
typedef Node<StateInOut, Data> INode;

struct Data {
	int dfs_vis, dfs_fin;
	double depth;
	vector<INode*> parents;
	bool back_bone;
	bool sampled;
	Data(int dfs_vis=-1, int dfs_fin=-1, double depth=-1) : dfs_vis(dfs_vis), dfs_fin(dfs_fin), depth(depth), back_bone(false), sampled(false) {
	}
};

ostream& operator<<(ostream& os, const Data& d) {
	os << "(" << d.dfs_vis <<"/" << d.dfs_fin << " " << d.depth << " ";
	for (auto& p : d.parents)
		os << p->label << " ";
	os << d.back_bone << " " << d.sampled << ")";
	return os;
}


struct SampleInSampleSetChecker {
	const set<string>& sample_names;
	SampleInSampleSetChecker(const set<string>& sample_names = set<string>()) : sample_names(sample_names) {}

	bool operator()(const INode& n) const {
		return sample_names.find(n.label) != sample_names.end();
	}
};

//we assume branch_length represets dissimilarity
struct Sampler {
	set<string> sample_names;
	SubtreeExtractorOverSamples<INode, SampleInSampleSetChecker> subtreeExtractor;

	Sampler(const set<string>& sample_names) : sample_names(sample_names), 
		subtreeExtractor(sample_names) {
	}

	//returns the first node with all the samples as its sub-tree
	INode run(const INode& n) {
		//INode r = sample(n).first;
		INode r = subtreeExtractor.run(n);
		assert(sample_names.size() == (size_t)r.sample_size);
		return r;
	}

	int removed_internal_count() const {
		return subtreeExtractor.removed_internal_count;
	}

	int removed_sample_count() const {
		return subtreeExtractor.removed_sample_count;
	}

};

set<string> load_sample_names(string fn) {
	ifstream fi(fn);
	set<string> sample_names;
	for (string line; getline(fi, line); )
		sample_names.insert(line);
	return sample_names;
}

string replace_all(string src, string from, string to) {
	while(src.find(from) != string::npos)
		src.replace(src.find(from), from.length(), to);
	return src;
}


// Algorithm: Given a sample file and a tree, sub-trees with specified samples are extracted as sub-trees.
int main(int argc, char* argv[]) {

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "Run sankoff")
	    ("in", po::value<string>(), "input tree")
	    ("samples-in", po::value<string>(), "")
	    ("par", po::value<vector<string>>()->multitoken(), "name of partitions")
	    ("samples", po::value<string>(), "samples file template")
	    ("trees", po::value<string>(), "trees file template")
	    ("location_label,l", po::value<vector<string>>()->multitoken(), "location labels, e.g. Germany nonGermany")
	    ("print-annotation", po::value<bool>()->default_value(true), "print annotation")
	    ("print-internal-node-label", po::value<bool>()->default_value(true), "print internal node labels")
	    ("ilabel", po::value<bool>()->default_value(false), "override internal node labels")
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
	for (string partition : vm["par"].as<vector<string>>()) {
		set<string> sample_names = load_sample_names(replace_all(vm["samples-in"].as<string>(), "?", partition));

		Sampler sampler(sample_names);
		INode contracted_phylo = sampler.run(phylo);

		cerr << "removed " << sampler.removed_internal_count() << " internal nodes and " << sampler.removed_sample_count() << " samples with short branch lenghts" << endl;
		cerr << "  h=" << phylo.height << ">" << contracted_phylo.height << " s=" << phylo.size << ">" << contracted_phylo.size << " ss=" << phylo.sample_size << ">" << contracted_phylo.sample_size << endl;

		if (vm["ilabel"].as<bool>() == true) {
			InternalNodeLabeler<INode> internalNodeLabeler;
			internalNodeLabeler.run(contracted_phylo);
			cerr << "internal nodes relabeled" << endl;
		}
		
		string output = replace_all(vm["trees"].as<string>(), "?", partition);
		ofstream fo(output);
		NodePrinterGeneral<INode> np(vm["print-annotation"].as<bool>(), vm["print-internal-node-label"].as<bool>());
		//np.print(fo, phylo) << ";" << endl;
		np.print(fo, contracted_phylo) << ";" << endl;

		cerr << "output saved on " << output<< " " << "[" << vm["print-annotation"].as<bool>() << " " << vm["print-internal-node-label"].as<bool>() << "]" << contracted_phylo.label << endl;
	}


	return 0;
}
