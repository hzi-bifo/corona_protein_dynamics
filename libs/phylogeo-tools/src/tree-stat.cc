#include "tree.h"
#include "state.h"
#include <boost/program_options.hpp>
#include "rangetree.h"
#include <vector>
#include <numeric>

namespace po = boost::program_options;

array<string, StateInOut::size> StateInOut::names = {"uk", "nonuk"};

struct INodeData {
	int in_count;
	int poc_count, poc_in_count;
	INodeData(int in_count=0, int poc_count=0, int poc_in_count=0): in_count(in_count), poc_count(poc_count), poc_in_count(poc_in_count) {
	}

	INodeData operator+(const INodeData& b) const {
		INodeData r = *this;
		r += b;
		return r;
	}

	INodeData& operator+=(const INodeData& b) {
		in_count += b.in_count;
		poc_count += b.poc_count;
		poc_in_count += b.poc_in_count;
		return *this;
	}
};

typedef Node<StateInOut, INodeData> INode;

struct Stat {
	int small_size;
	map<string, string> id_to_pangolin;
	string pangolin_of_concern;

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

	INodeData dfs1(INode& n) {
		if (n.isLeaf()) {
			return n.data = INodeData(n.location == StateInOut::IN ? 1 : 0, 
				id_to_pangolin.find(n.label) != id_to_pangolin.end() && id_to_pangolin[n.label] == pangolin_of_concern ? 1 : 0,
				n.location == StateInOut::IN && id_to_pangolin.find(n.label) != id_to_pangolin.end() && id_to_pangolin[n.label] == pangolin_of_concern ? 1 : 0
				);
		}
		INodeData state_sample_count;
		for (auto & c : n.children)
			state_sample_count += dfs1(c);
		return n.data = state_sample_count;
	}

	void dfs(const INode& n, int d, int h) {
		if (d < 0) return;
		//if (n.size >= 2*n.sample_size && n.size < 10) {
		//	cerr << n.print(cerr) << endl;
		//}

		INodeData state_sample_size_large;
		vector<int> cs;
		for (auto const& c : n.children)
			if (c.sample_size < small_size) {
				cs.push_back(c.sample_size);
				state_sample_size_large += c.data;
			}
		cerr << "" << mul(' ', h) << "s=" << n.size << " ss=" << n.sample_size << " h=" << n.height << " c=" << n.children.size() << " in=" << n.data.in_count;
		if (pangolin_of_concern != "")
			cerr << " poc=" << n.data.poc_count << "(" << n.data.poc_in_count << ")";
		cerr << " [#" << cs.size() << " sum=" << accumulate(cs.begin(), cs.end(), 0) << " max=" << accumulate(cs.begin(), cs.end(), 0, [](int a, int b) {return max(a, b);}) << " in=" << state_sample_size_large.in_count;
		if (pangolin_of_concern != "")
			cerr << " poc=" << state_sample_size_large.poc_count << "(" << state_sample_size_large.poc_in_count << ")";
		cerr << "] " << n.label << endl;
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
	    ("metadata", po::value<string>(), "metadata file, useful if poc is available")
	    ("poc", po::value<string>(), "pangolin of concern. Number of samples with this pangolin is also reported as output.")
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

	if (vm.count("poc") && vm.count("metadata")) {
		map<string, Metadata> metadata = load_map(vm["metadata"].as<string>());
		cerr << "metadata loaded " << metadata.size() << endl;

		map<string, string> id_to_pangolin;
		for (auto const & m : metadata) {
			id_to_pangolin[m.second.id] = m.second.pangolin;
		}
		stat.id_to_pangolin = id_to_pangolin;
		stat.pangolin_of_concern = vm["poc"].as<string>();
	}

	stat.run(phylo, vm["depth"].as<int>());

	return 0;
}
