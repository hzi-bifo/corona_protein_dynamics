#include "tree.h"
#include "state.h"
#include <boost/program_options.hpp>
namespace po = boost::program_options;

typedef int Data;
array<string, StateInOut::size> StateInOut::names = {"UK", "nonUK"};

typedef Node<StateInOut, Data> INode;

struct Contractor {
	double limit;
	int removed_count;
	bool contract_leaf_enabled;
	Contractor(double _limit = 0, bool contract_leaf_enabled = false) : limit(_limit), removed_count(0), contract_leaf_enabled(contract_leaf_enabled) {}


	INode contract(const INode& n) {
		//height, size should be recalculated
		INode r(n.label, n.branch_length <= limit && contract_leaf_enabled ? 0 : n.branch_length, n.annotation, list<INode>(), n.location, 1, 1, n.sample_size);
		for (auto &c: n.children) {
			INode c_c = contract(c);
			if (c.branch_length <= limit && !c.isLeaf()) {
				//cerr << "node removed " << c.label << " " << c.branch_length << endl;
				removed_count++;
				for (auto& cc: c_c.children) {
					//cc.branch_length += c.branch_length;
					cc.branch_length += c_c.branch_length;
					r.height = max(r.height, cc.height + 1);
					r.size += cc.size;
					//r.children.push_back(move(cc));
					r.children.push_back(cc);
				}
			} else {
				//normal:
				r.height = max(r.height, c_c.height + 1);
				r.size += c_c.size;
				//r.children.push_back(move(c_c));
				r.children.push_back(c_c);
			}
		}


		return r;
	}
};


int main(int argc, char* argv[]) {

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "Run sankoff")
	    ("short", po::value<double>(), "Branches of length less than or equal to this value are contracted")
	    ("in", po::value<string>(), "input tree")
	    ("out", po::value<string>(), "output tree")
	    ("location_label", po::value<vector<string>>()->multitoken(), "location labels, e.g. Germany nonGermany")
	    ("contract_leaf_enabled", po::value<bool>()->default_value(false), "replace branch length of short branch length leafs with 0")
	    ("print-annotation", po::value<bool>()->default_value(true), "print annotation")
	    ("print-internal-node-label", po::value<bool>()->default_value(true), "print internal node labels")
	    ("ilabel", po::value<bool>()->default_value(false), "override internal node labels")
	    ("single-child", po::value<bool>()->default_value(false), "allow single-child internal nodes")
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
	INode phylo = load_tree<INode>(tree_file_name) ;

	Contractor contractor(vm["short"].as<double>(), vm["contract_leaf_enabled"].as<bool>());
	INode contracted_phylo = contractor.contract(phylo);

	cerr << "removed " << contractor.removed_count << " internal nodes with short branch lenghts" << endl;
	cerr << "  h=" << phylo.height << ">" << contracted_phylo.height << " s=" << phylo.size << ">" << contracted_phylo.size << " ss=" << phylo.sample_size << ">" << contracted_phylo.sample_size << endl;

	if (vm["ilabel"].as<bool>() == true) {
		InternalNodeLabeler<INode> internalNodeLabeler;
		internalNodeLabeler.run(contracted_phylo);
		cerr << "internal nodes relabeled" << endl;
	}

	if (vm["single-child"].as<bool>() == false) {
		SingleChildInternalNodeRemover<INode> singleChildInternalNodeRemover;
		phylo = singleChildInternalNodeRemover.run(phylo);
		cerr << "Single child internal nodes removed " << singleChildInternalNodeRemover.removed_internal_count << endl;
	}

	ofstream fo(output);
	NodePrinterGeneral<INode> np(vm["print-annotation"].as<bool>(), vm["print-internal-node-label"].as<bool>());
	//np.print(fo, phylo) << ";" << endl;
	np.print(fo, contracted_phylo) << ";" << endl;

	cerr << "output saved on " << output<< " " << "[" << vm["print-annotation"].as<bool>() << " " << vm["print-internal-node-label"].as<bool>() << "]" << endl;
	return 0;
}
