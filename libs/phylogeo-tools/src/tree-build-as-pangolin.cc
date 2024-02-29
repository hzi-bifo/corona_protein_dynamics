#include "tree.h"
#include "state.h"
#include <boost/program_options.hpp>
#include <tr1/unordered_map>

namespace po = boost::program_options;

array<string, StateInOut::size> StateInOut::names = {"UK", "nonUK"};

struct Data;
typedef Node<StateInOut, Data> INode;

struct Data {
	tr1::unordered_map<string, INode*> lineage_child_index;
};


struct Builder {

	const map<string, Metadata>& metadata;
	const set<string>& ids_valid;
	set<string> added_accession_id, added_name;

	Builder(const map<string, Metadata>& metadata, const set<string>& ids_valid) : metadata(metadata), ids_valid(ids_valid) {
	}

	bool is_in_valid(const Metadata& m) const {
		string name = m.name;
		//if (name.rfind("hCoV-19/", 0) == 0) 
		//	name = name.substr(8);
		return ids_valid.find(name) != ids_valid.end();
	}

	INode build() {
		added_accession_id.clear();
		added_name.clear();
		int invalid_location_count = 0,
			invalid_pandolin_count = 0,
			invalid_date_count = 0,
			dup_id_count = 0,
			dup_name_count = 0;
		INode root;
		for (const auto& m : metadata) {
			if (is_in_valid(m.second)) {
				//check date, check location, check pango
				// check not duplicated
 				if (m.second.location.find('/') == string::npos) {
					invalid_location_count++;
					continue;
				}
				if (m.second.pangolin == "") {
					invalid_pandolin_count++;
					continue;
				}
				static const std::regex re{"^[0-9]{4}-[0-9]{2}-[0-9]{2}$"};
				if (!std::regex_match(m.second.date, re)) { 
					invalid_date_count++;
					continue;
				}
				if (added_accession_id.find(m.second.id) != added_accession_id.end()) {
					dup_id_count++;
					continue;
				}
				if (added_name.find(m.second.name) != added_name.end()) {
					dup_name_count++;
					continue;
				}


				added_accession_id.insert(m.second.id);
				added_name.insert(m.second.name);

				add_to_tree(root, m.second, 0);
			}
		}
		update_stat_rec(root);
		cerr << "Not added metadata : " << 
			"invalid_location_count " << invalid_location_count << " " << 
				"invalid_pandolin_count " << invalid_pandolin_count << " " << 
				"invalid_date_count " << invalid_date_count << " " << 
				"dup_id_count " << dup_id_count << " " << 
				"dup_name_count " << dup_name_count << endl;
		return root;
	}

	void update_stat_rec(INode& n) {
		for (auto & c : n.children)
			update_stat_rec(c);
		n.update_stat();
	}

	void add_to_tree(INode& n, const Metadata& m, size_t pangolin_p) {
		//cerr << "A " << m.id << " " << m.pangolin << " " << n.label << " " << pangolin_p << endl;
		if (m.pangolin.size() == 0 or pangolin_p == string::npos) {
			INode c(m.id, 1);
			c.sample_size = 1;
			n.children.push_back(c);
		} else {
			assert(pangolin_p < m.pangolin.size());
			size_t nep = m.pangolin.find('.', pangolin_p);
			string npango = m.pangolin.substr(0, nep);
			//cerr << "  " << npango << " " << nep << endl;
			if (n.data.lineage_child_index.find(npango) == n.data.lineage_child_index.end()) {
				INode c(npango, 1);
				//n.data.lineage_child_index[npango] = n.children.size();
				n.children.push_back(c);
				n.data.lineage_child_index[npango] = &n.children.back();
				//cerr << "   new child created " << c.label << endl;
			}


			//int ci = n.data.lineage_child_index.find(npango)->second;
			INode* ci = n.data.lineage_child_index.find(npango)->second;
			//cerr << "   A " << n.children[ci].label << endl;
			add_to_tree(*ci, m, nep != string::npos ? nep + 1 : string::npos);
		}
	}

};

set<string> load_fasta_ids(istream& fi) {
	vector<string> ret;
	const int line_size = 1 << 10;
	char line[line_size];
	for (; fi.getline(line, line_size) > 0; ) {
	//for (string line; getline(fi, line); ) {
		if (line[0] != 0 && line[0] == '>') {
			char* divider_pos = strchr(line, '|');
			if (divider_pos != 0)
				*divider_pos = 0;
			ret.push_back(line + 1);
			if (ret.size() % 10000 == 0) {
				cerr << "\r\033[Fload_fasta_ids " << ret.size() << " " << ret.back() << endl << flush;
			}
		}
	}
	return set<string>(ret.begin(), ret.end());
}


int main(int argc, char* argv[]) {

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "Run sankoff")
	    ("metadata", po::value<string>(), "metadata file")
	    ("out", po::value<string>(), "output tree")
	    ("seqs", po::value<string>(), "file containing cleaned sequences, the id would be used")
	    ("seqs-stdin", "Run sankoff")
	    ("location_label,l", po::value<vector<string>>()->multitoken(), "location labels, e.g. Germany nonGermany")
	    ("print-annotation", po::value<bool>()->default_value(true), "print annotation")
	    ("print-internal-node-label", po::value<bool>()->default_value(true), "print internal node labels")
	    ("ilabel", po::value<bool>()->default_value(false), "override internal node labels")
	;

	po::parsed_options parsed_options = po::command_line_parser(argc, argv)
		.options(desc)
		.run();

	po::variables_map vm;
	po::store(parsed_options, vm);

	StateInOut::names = {vm["location_label"].as<vector<string>>()[0], vm["location_label"].as<vector<string>>()[1]};

	string annotation = vm["metadata"].as<string>(),
		output = vm["out"].as<string>();
	map<string, Metadata> metadata = load_map(annotation);
	cerr << "metadata loaded " << metadata.size() << endl;

	ifstream seqs_ifstream(vm.count("seqs") > 0 ? vm["seqs"].as<string>() : "");
	set<string> ids_valid = load_fasta_ids(vm.count("seqs-stdin") > 0 ? cin : seqs_ifstream);
	cerr << "fasta ids loaded " << ids_valid.size() << " ids [" << *(ids_valid.begin()) << " ..." << endl;

	Builder builder(metadata, ids_valid);
	INode phylo = builder.build();

	if (vm["ilabel"].as<bool>() == true) {
		InternalNodeLabeler<INode> internalNodeLabeler;
		internalNodeLabeler.run(phylo);
		cerr << "internal nodes relabeled" << endl;
	}

	ofstream fo(output);
	NodePrinterGeneral<INode> np(vm["print-annotation"].as<bool>(), vm["print-internal-node-label"].as<bool>());
	np.print(fo, phylo) << ";" << endl;

	cerr << "output saved on " << output<< " " << "[" << vm["print-annotation"].as<bool>() << " " << vm["print-internal-node-label"].as<bool>() << "]" << endl;
	cerr << "  s=" << phylo.size << " ss=" << phylo.sample_size << " h=" << phylo.height << endl;
	return 0;
}
