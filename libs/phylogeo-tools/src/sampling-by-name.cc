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


//we assume branch_length represets dissimilarity
struct Sampler {
	set<string> sample_names;
	int removed_internal_count, removed_sample_count;
	Sampler(const set<string>& sample_names) : sample_names(sample_names), removed_internal_count(0), removed_sample_count(0) {
	}

	INode run(const INode& n) {
		return sample(n).first;
	}

	pair<INode, bool> sample(const INode& n) {
		//height, size should be recalculated
		INode r(n.label, n.branch_length, n.annotation, list<INode>(), n.location, 1, 1, n.isLeaf() ? 1 : 0);
		for (auto &c: n.children) {
			pair<INode, bool> c_c_ = sample(c);
			if (c_c_.second == false) {
				if (c.isLeaf())
					removed_sample_count++;
				else
					removed_internal_count++;
				continue;
			}
			INode& c_c = c_c_.first;
			if (c_c.children.size() == 1 && !c.isLeaf()) {
				//cerr << "node removed " << c.label << " " << c.branch_length << endl;
				removed_internal_count++;
				for (auto& cc: c_c.children) {
					cc.branch_length += c.branch_length;
					r.height = max(r.height, cc.height + 1);
					r.size += cc.size;
					//r.children.push_back(move(cc));
					r.sample_size += cc.sample_size;
					r.children.push_back(cc);
				}
			} else {
				//normal:
				r.height = max(r.height, c_c.height + 1);
				r.size += c_c.size;
				r.sample_size += c_c.sample_size;
				//r.children.push_back(move(c_c));
				r.children.push_back(c_c);
			}
		}
		return make_pair(r, r.children.size() > 0 || sample_names.find(n.label) != sample_names.end());
	}

};

set<string> load_sample_names(string fn) {
	ifstream fi(fn);
	set<string> sample_names;
	for (string line; getline(fi, line); )
		sample_names.insert(line);
	return sample_names;
}

int main(int argc, char* argv[]) {

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "Run sankoff")
	    ("in", po::value<string>(), "input tree")
	    ("out", po::value<string>(), "output tree")
	    ("samples-in", po::value<string>(), "output file to write sample ids")
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

	string tree_file_name = vm["in"].as<string>(),
		output = vm["out"].as<string>();
	INode phylo = load_tree<INode>(tree_file_name);

	set<string> sample_names = load_sample_names(vm["samples-in"].as<string>());

	Sampler sampler(sample_names);
	INode contracted_phylo = sampler.run(phylo);

	cerr << "removed " << sampler.removed_internal_count << " internal nodes and " << sampler.removed_sample_count << " samples with short branch lenghts" << endl;
	cerr << "  h=" << phylo.height << ">" << contracted_phylo.height << " s=" << phylo.size << ">" << contracted_phylo.size << " ss=" << phylo.sample_size << ">" << contracted_phylo.sample_size << endl;

	if (vm["ilabel"].as<bool>() == true) {
		InternalNodeLabeler<INode> internalNodeLabeler;
		internalNodeLabeler.run(contracted_phylo);
		cerr << "internal nodes relabeled" << endl;
	}
	
	ofstream fo(output);
	NodePrinterGeneral<INode> np(vm["print-annotation"].as<bool>(), vm["print-internal-node-label"].as<bool>());
	//np.print(fo, phylo) << ";" << endl;
	np.print(fo, contracted_phylo) << ";" << endl;

	cerr << "output saved on " << output<< " " << "[" << vm["print-annotation"].as<bool>() << " " << vm["print-internal-node-label"].as<bool>() << "]" << endl;
	return 0;
}
