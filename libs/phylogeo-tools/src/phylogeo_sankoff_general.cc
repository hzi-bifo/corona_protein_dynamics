#include "tree.h"
//#include "state.h"
#include <boost/program_options.hpp>
namespace po = boost::program_options;

class StateInOut {
public:
	enum Type {
		UNKNOWN=-1, IN=0, OUT=1
	};

	Type type;
	StateInOut() = default;
	constexpr StateInOut(Type _type) : type(_type) { }
	constexpr StateInOut(int _type) : type(static_cast<Type>(_type)) { }
	operator  Type() const { return type; }
	constexpr bool operator == (StateInOut error) const { return type == error.type; }
	constexpr bool operator != (StateInOut error) const { return type != error.type; }    
	constexpr bool operator == (Type errorType) const { return type == errorType; }
	constexpr bool operator != (Type errorType) const { return type != errorType; }
	static const StateInOut unknown;
	static const StateInOut def;
	static const int size = 2;

	//IN, OUT
	static array<string, 2> names;
};

const StateInOut StateInOut::unknown = StateInOut::Type::UNKNOWN;
const StateInOut StateInOut::def = StateInOut::Type::OUT;
ostream& operator<<(ostream& os, StateInOut s) {
	return os << ((s.type == StateInOut::Type::IN) ? StateInOut::names[0] : (s.type == StateInOut::Type::UNKNOWN ? "U" : StateInOut::names[1])) ;
}

istream& operator>>(istream& is, StateInOut& st) {
	string s;
	is >> s;
	if (s == StateInOut::names[0])
		st.type = StateInOut::Type::IN;
	else if (s == StateInOut::names[1])
		st.type = StateInOut::Type::OUT;
	else {
		st.type = StateInOut::Type::OUT;
		cerr << "W Invalid state " << s << " " << StateInOut::names[0] << " " << StateInOut::names[1] << endl;
		//if (s != "unknown")
		//	cerr << "Invalid state " << s << " " << StateInOut::names[0] << " " << StateInOut::names[1] << endl;
		//assert(s == "unknown");
		//st.type = StateInOut::Type::UNKNOWN;
	}
	return is;
}


array<string, StateInOut::size> StateInOut::names = {"UK", "nonUK"};

typedef int Data;
typedef Node<StateInOut, Data> INode;

struct TreeInternalNodeLocationCleaner {
	void visit(INode & n) {
		if (!n.isLeaf())
			n.location = StateInOut::UNKNOWN;
	}

	void finish(INode& n) {
	}
};

struct TreeInternalNodeLabelCleaner {
	void visit(INode & n) {
		if (!n.isLeaf())
			n.label = "";
	}

	void finish(INode& n) {
	}
};

int main(int argc, char* argv[]) {

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "Run sankoff")
	    ("nosankoff", "Do not run sankoff, only do cleaning.")
	    ("merge", "Merge additional location to the location as its 4rd element")
	    ("save-nexus", "Save output in nexus format")
	    ("remove-internal-node-locations", "Remove locations of internal nodes")
	    ("remove-internal-node-label", "Remove label of internal nodes")
	    ("metadata", po::value<string>(), "metadata file")
	    ("in", po::value<string>(), "input tree")
	    ("inout-file", po::value<string>(), "input tree, list of files given in a file")
	    ("out", po::value<string>(), "output tree")
	    ("location_label", po::value<vector<string>>()->multitoken(), "location labels, e.g. Germany nonGermany")
	    ("cost", po::value<vector<float>>()->multitoken()->default_value(vector<float>{0, 1, 1, 0}, "0 1 1 0"), "cost function, in->in, in->out, out->in, out->out")
	    ("cond", po::value<vector<string>>()->multitoken(), "conditions e.g. --cond 2 == Germany 3 >= Dusseldorf --cond 2 == Germany 4 >= Dusseldorf")
	    ("print-annotation", po::value<bool>()->default_value(true), "print annotation")
	    ("set-tip-location", po::value<bool>()->default_value(true), "set tip location from metadata")
	    ("remove-invalid-children", po::value<bool>()->default_value(true), "remove invalid (not in metadata) children")
	    ("print-allow-annotation", po::value<bool>()->default_value(true), "allow printing annotations. If setted to false, annotations are removed.")
	    ("print-internal-node-label", po::value<bool>()->default_value(true), "print internal node labels")
	    ("ilabel", po::value<bool>()->default_value(false), "override internal node labels")
	    ("single-child", po::value<bool>()->default_value(true), "allow single-child internal nodes")
	    ("samples", po::value<string>(), "sample files")
	    ("nexus-print-internal-labels", po::value<bool>()->default_value(true), "print internal nodes' labels in nexus output")
	;

	// Just parse the options without storing them in a map.
	po::parsed_options parsed_options = po::command_line_parser(argc, argv)
		.options(desc)
		.run();
	//cerr << "options run" << endl;

	// Build list of multi-valued option instances. We iterate through
	// each command-line option, whether it is repeated or not. We
	// accumulate the values for our multi-valued option in a
	// container.
	std::vector<std::vector<std::string>> conditions;
	for (const po::option& o : parsed_options.options) {
		if (o.string_key == "cond")
			conditions.push_back(o.value);
	}

	//cerr << "cond created" << endl;

	// If we had other normal options, we would store them in a map
	// here. In this demo program it isn't really necessary because
	// we are only interested in our special multi-valued option.
	po::variables_map vm;
	po::store(parsed_options, vm);

	//cerr << "options stored " << vm["in"].as<string>() << endl;

	StateInOut::names = {vm["location_label"].as<vector<string>>()[0], vm["location_label"].as<vector<string>>()[1]};

	string annotation = vm["metadata"].as<string>();
	map<string, Metadata> id_to_name = load_map(annotation);
	cerr << "metadata loaded" << endl;

	//0:GERMANY, 1:NON_GERMANY
	//cost_type cost = {0,1,1,0};
	vector<float> cost_vector = vm["cost"].as<vector<float>>();
	cost_type cost = {cost_vector[0], cost_vector[1], cost_vector[2], cost_vector[3]};
	cerr << "cost: " << cost << endl;

	int in_location_count = 0;
	map<string, StateInOut> isolate_matrix;
	for (auto const &i: id_to_name) {
		vector<string> location = split(i.second.location, '/');
		if (vm.count("merge"))
			location.push_back(i.second.location_add);
		//cerr << "checking for location " << location << endl;
		bool in_location = false;
		for (auto & cond : conditions) {
			bool in_loc_cond = false;
			for (size_t i=0; i<cond.size(); i+=3) {
				size_t loc_index = stoi(cond[i])-1;
				if (loc_index >= location.size()) {
					in_loc_cond = false;
					break;
				}
				string l = trim(location[loc_index]);
				if (cond[i+1] == "==" || cond[i+1] == "eq") {
					in_loc_cond = l == cond[i+2];
				} else if (cond[i+1] == ">=" || cond[i+1] == "gt") {
					in_loc_cond = l.find(cond[i+2]) != string::npos;
				}
			}
			//if (trim(location[1]) == "Germany")
			//	cerr << "  cond " << cond << " " << in_loc_cond << endl;
			//if (in_loc_cond) {
			//	cerr << "matched location " << location << " -- " << cond << endl;
			//}
			in_location |= in_loc_cond;
		}
		if (in_location) {
			isolate_matrix[i.first] = StateInOut::Type::IN;
			//cerr << "matched location " << location << endl;
			in_location_count++;
		} else {
			isolate_matrix[i.first] = StateInOut::Type::OUT;
		}
	}

	cerr << "Location assigned " << in_location_count << " as in " << endl;

	//cerr << "isolate_matrix:" << isolate_matrix << endl;
	//cerr << "cost" << cost << endl;

	ifstream inout_file;
	for (int i=0; ; i++) {
		string	tree_file_name,
			output;
		if (vm.count("inout-file")) {
			if (i == 0) {
				inout_file.open(vm["inout-file"].as<string>());
			}
			if (!(inout_file >> tree_file_name >> output))
				break;
		} else {
			if (i > 0) break;
			tree_file_name = vm["in"].as<string>();
			output = vm["out"].as<string>();
		}
		INode phylo = load_tree<INode>(tree_file_name) ;

		//{
		//	NodePrinterGeneral<INode> np(vm["print-annotation"].as<bool>(), vm["print-internal-node-label"].as<bool>(), vm["print-allow-annotation"].as<bool>());
		//	np.print(cerr, phylo) << ";" << endl;
		//}

		if (vm["set-tip-location"].as<bool>())
			phylo.set_tip_location(isolate_matrix);

		//phylo.annotation= "location=Germany";
		if (vm.count("remove-internal-node-locations") > 0) {
			TreeInternalNodeLocationCleaner cleanerAction;
			TreeDFSGeneral<INode, TreeInternalNodeLocationCleaner> dfsCleaner(cleanerAction);
			dfsCleaner.dfs(phylo);
		}

		if (vm.count("nosankoff") == 0) {
			phylo.sankoff(cost);
			phylo.sankoff2(-1, cost);
			//phylo.print(cerr);
		}

		int removed_count = 0;
		//ofstream fo(splitted_tree_prefix + "1" + ".trees");
		if (vm["remove-invalid-children"].as<bool>())
			phylo.remove_invalid_children(id_to_name, removed_count);

		if (vm["ilabel"].as<bool>() == true) {
			InternalNodeLabeler<INode> internalNodeLabeler;
			internalNodeLabeler.run(phylo);
			cerr << "internal nodes relabeled" << endl;
		}
		if (vm.count("remove-internal-node-label") > 0) {
			TreeInternalNodeLabelCleaner cleanerAction;
			TreeDFSGeneral<INode, TreeInternalNodeLabelCleaner> dfsCleaner(cleanerAction);
			dfsCleaner.dfs(phylo);
		}

		if (vm["single-child"].as<bool>() == false) {
			SingleChildInternalNodeRemover<INode> singleChildInternalNodeRemover;
			phylo = singleChildInternalNodeRemover.run(phylo);
			cerr << "Single child internal nodes removed " << singleChildInternalNodeRemover.removed_internal_count << endl;
		}

		if (vm.count("samples") > 0) {
			SamplePrinter<INode> sp;
			sp.run(phylo, vm["samples"].as<string>());
			cerr << "samples printed in " << vm["samples"].as<string>() << " for " << sp.printed_count << " samples" << endl;
		}

		//{
		//	NodePrinterGeneral<INode> np(vm["print-annotation"].as<bool>(), vm["print-internal-node-label"].as<bool>(), vm["print-allow-annotation"].as<bool>());
		//	np.print(cerr, phylo) << ";" << endl;
		//}

		ofstream fo(output);
		if (vm.count("save-nexus") > 0) {
			save_nexus_tree(fo, phylo, vm["nexus-print-internal-labels"].as<bool>());
		} else {
			NodePrinterGeneral<INode> np(vm["print-annotation"].as<bool>(), vm["print-internal-node-label"].as<bool>(), vm["print-allow-annotation"].as<bool>());
			np.print(fo, phylo) << ";" << endl;
		}

		cerr << "done " << tree_file_name << " -> " << output<< " " << "[" << vm["print-annotation"].as<bool>() << " " << vm["print-internal-node-label"].as<bool>() << "]" << endl;

	}
	return 0;
}
