#include "tree.h"
#include "state.h"
#include <boost/program_options.hpp>
#include "rangetree.h"
#include <vector>
#include <numeric>

namespace po = boost::program_options;

array<string, StateInOut::size> StateInOut::names = {"uk", "nonuk"};

typedef Node<StateInOut, int> INode;

struct Stat {
	int small_size;
	Stat(int small_size=0) : small_size(small_size) {}

	string mul(char c, int m) {
		string r = "";
		for (int i=0; i<m; i++)
			r += c;
		return r;
	}

	void run(INode& n, int d) {
		dfs1(n);
		dfs(n, d, 0);
	}

	int dfs1(INode& n) {
		if (n.isLeaf())
			return n.data = (n.location == StateInOut::IN ? 1 : 0);
		int state_sample_count = 0;
		for (auto & c : n.children)
			state_sample_count += dfs1(c);
		return n.data = state_sample_count;
	}

	void dfs(const INode& n, int d, int h) {
		if (d < 0) return;
		//if (n.size >= 2*n.sample_size && n.size < 10) {
		//	cerr << n.print(cerr) << endl;
		//}

		int state_sample_size_large = 0;
		vector<int> cs;
		for (auto const& c : n.children)
			if (c.sample_size < small_size) {
				cs.push_back(c.sample_size);
				state_sample_size_large += c.data;
			}
		cerr << "" << mul(' ', h) << "s=" << n.size << " ss=" << n.sample_size << " h=" << n.height << " c=" << n.children.size() << " in=" << n.data;
		cerr << " [#" << cs.size() << " sum=" << accumulate(cs.begin(), cs.end(), 0) << " max=" << accumulate(cs.begin(), cs.end(), 0, [](int a, int b) {return max(a, b);}) << " in=" << state_sample_size_large << "] " << n.label << endl;
		for (auto const& c : n.children)
			if (c.sample_size > small_size)
				dfs(c, d-1, h+1);
	}
};

int main(int argc, char* argv[]) {
	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "Run statistics")
	    ("in", po::value<string>(), "input tree")
	    ("large", po::value<int>()->default_value(1000), "size of large sub-samples")
	    ("depth", po::value<int>()->default_value(10), "max depth")
	    ("location_label,l", po::value<vector<string>>()->multitoken(), "location labels, e.g. Germany nonGermany")
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

	Stat stat(vm["large"].as<int>());
	stat.run(phylo, vm["depth"].as<int>());

	return 0;
}
