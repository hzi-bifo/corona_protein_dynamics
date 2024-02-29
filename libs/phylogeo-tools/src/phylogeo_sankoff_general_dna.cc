#define BOOST_BIND_GLOBAL_PLACEHOLDERS
//#include <boost/bind/bind.hpp>
//using namespace boost::placeholders;
#include "tree.h"
#include "state.h"
#include <boost/program_options.hpp>
//#include <boost/iostreams/filtering_streambuf.hpp>
//#include <boost/iostreams/copy.hpp>
//#include <boost/iostreams/filter/gzip.hpp>
//#include <boost/iostreams/filter/lzma.hpp>

//namespace io = boost::iostreams;
namespace po = boost::program_options;

array<string, StateInOut::size> StateInOut::names = {"UK", "nonUK"};

struct CharModel {
	int NA_COUNT;

	int na_to_int(char na) {
		na = toupper(na);
		if (!__char_is_valid[(int)na]) {
			cerr << "Invalid char '" << na << "'" << endl;
		}
		assert(__char_is_valid[(int)na] == true);
		return __char_to_int[(int)na];
		/*
		if (na == 'A') return 0;
		if (na == 'T') return 1;
		if (na == 'C') return 2;
		if (na == 'G') return 3;
		if (na == '-') return 4;
		if (na == 'N') return 5;
		return 5;
		*/
	}

	int na_to_int(string na) {
		return na_to_int(na[0]);
	}

	char int_to_char(int na) {
		assert(na < NA_COUNT);
		return __int_to_char[na];
		/*
		switch (na) {
			case 0: return 'A';
			case 1: return 'T';
			case 2: return 'C';
			case 3: return 'G';
			case 4: return '-';
			case 5: return 'N';
			default: return 'N';
		}
		assert(1 != 1);
		*/
	}

	static const int MAX_NA_COUNT = 30;

	int cost[MAX_NA_COUNT][MAX_NA_COUNT];
	int __char_to_int[255];
	char __int_to_char[MAX_NA_COUNT];
	bool __char_is_valid[255];

	void load_cost(string fn) {
		ifstream f(fn);
		vector<string> names;
		for (size_t i=0; i<255; i++)
			__char_is_valid[i] = false;
		for (string line, v; getline(f, line); ) {
			istringstream is(line.c_str());
			if (names.size() == 0) {
				for (string s; is >> s; )
					names.push_back(s);
				NA_COUNT = names.size();
				for (size_t i=0; i<names.size(); i++) {
					__int_to_char[i] = names[i][0];
					__char_to_int[toupper(names[i][0])] = i;
					__char_is_valid[(int)toupper(names[i][0])] = true;
				}
			} else {
				is >> v;
				for (int vv, i=0; is >> vv; i++) {
					cost[na_to_int(v)][na_to_int(names[i])] = vv;
				}
			}
		}

		/*
		cerr << "COST " << names << endl;
		for (auto n1 : names) {
			cerr << n1 << " ";
			for (auto n2:names) {
				cerr << cost[na_to_int(n1)][na_to_int(n2)] << " ";
			}
			cerr << endl;
		}
		*/
	}


};

struct Data {
	bool valid;
	int sankoff_value[CharModel::MAX_NA_COUNT];
	vector<char> seq;
	void load(string s) {
		seq = vector<char>(s.begin(), s.end());
	}
};
typedef Node<StateInOut, Data> INode;

struct Sankoff {
	map<string, INode*> label_node;
	int seq_len;
	CharModel char_model;
	bool omit_leaf_mutations = false;

	void load_cost(string fn) {
		char_model.load_cost(fn);
	}

	void build_map(INode& n) {
		//cerr << "build_map " << n.label << " " << label_node.size() << endl;
		n.data.valid = true;
		if (n.isLeaf()) {
			label_node[n.label] = &n;
		}
		for (auto & c : n.children) {
			build_map(c);
		}
	}

	void load_dna(string fn) {
		cerr << "loading sequences for " << label_node.size() << " nodes" << endl;
		assert(label_node.size() != 0);
		//for (auto & m : label_node)
		//	cerr << m.first << " ";
		//cerr << endl;

		//seq_len = 40000;
		//for (auto & i : label_node) {
		//	i.second->data.seq = vector<char>(seq_len, 'A');
		//}
		//return;

		seq_len = -1;
		/*
		ifstream file(fn, ios_base::in | ios_base::binary);
		cerr << "file opened " << fn << endl;
		io::filtering_streambuf<io::input> in;
		//in.push(gzip_decompressor());
		in.push(io::lzma_decompressor());
		in.push(file);
		std::istream incoming(&in);
		istream& file_alignment = endswith(fn,".xz") ? incoming : file; 
		*/
		ifstream file_alignment(fn);

		string seq, id;
		for (string line; getline(file_alignment, line); ) {
			//cerr << "L " << line << endl;
			if (line.size() == 0) continue;
			if (line[0] == '>') {
				if (id != "" && label_node.find(id) != label_node.end()) {
					//cerr << "S " << id << endl;
					//cout << ">" << id << endl;
					//cout << seq << endl;
					//samples_found.insert(id);
					label_node[id]->data.load(seq);
					if (seq_len == -1) seq_len = seq.size();
					assert(seq_len == (int) seq.size());
				}
				//id = split(line.substr(1), '|')[0];
				id = line.substr(1);
				seq = "";
				//cerr << "new id (" << id << ") " << id.size() << endl;
			} else {
				seq += line;
			}
		}
		if (id != "" && label_node.find(id) != label_node.end()) {
			//cout << ">" << id << endl;
			//cout << seq << endl;
			//samples_found.insert(id);
			label_node[id]->data.load(seq);
		}
		cerr << "dna loaded" << endl;

		for (auto & i : label_node) {
			if ((int)i.second->data.seq.size() != seq_len) {
				cerr << "E " << i.second->label << " no sequence data" << endl;
				i.second->data.valid = false;
			}
			//assert((int)i.second->data.seq.size() == seq_len);
			//for (auto & c : i.second->data.seq) {
			//	c = toupper(c);
			//	//if (c != 'A' && c != 'T' && c != 'C' && c != 'G' && c != '-' && c != 'N') {
			//	//	cerr << "invalid char " << i.second->label << " " << c << endl;
			//	//	assert( 1 != 1 );
			//	//}
			//}
		}
	}

	void print_alignment(string fn, const INode& n) {
		ofstream fo(fn);
		cerr << "print_alignment " << fn << endl;
		print_alignment(fo, n);
	}

	void print_alignment(ofstream& fo, const INode& n) {
		string s(n.data.seq.begin(), n.data.seq.end());
		fo << ">" << n.label << endl << s << endl;
		for (auto const& c : n.children)
			print_alignment(fo, c);
	}

	void print_mutation_dependency(string fn, const INode& n) {
		ofstream fo(fn);
		vector<string> mutations;
		int mutation_cnt = 0;
		print_mutation_dependency(fo, n, mutations, mutation_cnt);
	}

	void print_mutation_dependency(ostream& os, const INode& n, vector<string>& mutations, int& mutation_cnt) {
		//if (n.isLeaf()) {
		//	for (string m : mutations) {
		//		os << n.label << "\t" << m << endl;
		//	}
		//}
		for (auto const& c : n.children) {
			size_t sz = mutations.size();
			for (size_t i=0; i<c.data.seq.size(); i++) {
				if (n.data.seq[i] != c.data.seq[i]) {
					//mutations.push_back(mutation_cnt++);
					mutations.push_back(n.data.seq[i] + to_string(i+1) + c.data.seq[i] + "_" + to_string(mutation_cnt));
					mutation_cnt++;
				}
			}

			os << sz << " ";
			for (size_t i=0; i<mutations.size(); i++) {
				os << mutations[i] << " ";
			}
			os << endl;

			print_mutation_dependency(os, c, mutations, mutation_cnt);
			mutations.resize(sz);
		}
	}


	void print_mutation_samples(string fn, const INode& n) {
		ofstream fo(fn);
		vector<string> mutations;
		int mutation_cnt = 0;
		print_mutation_samples(fo, n, mutations, mutation_cnt);
	}

	void print_mutation_samples(ostream& os, const INode& n, vector<string>& mutations, int& mutation_cnt) {
		if (n.isLeaf()) {
			for (string m : mutations) {
				os << n.label << "\t" << m << endl;
			}
		}
		for (auto const& c : n.children) {
			size_t sz = mutations.size();
			if (!c.isLeaf() || !omit_leaf_mutations) {
				for (size_t i=0; i<c.data.seq.size(); i++) {
					if (n.data.seq[i] != c.data.seq[i]) {
						//mutations.push_back(mutation_cnt++);
						mutations.push_back(n.data.seq[i] + to_string(i+1) + c.data.seq[i] + "_" + to_string(mutation_cnt));
						mutation_cnt++;
					}
				}
			}
			print_mutation_samples(os, c, mutations, mutation_cnt);
			mutations.resize(sz);
		}
	}

	int loc;
	void sankoff1(INode& n) {
		if (n.children.size() == 0) {
			for (int l = 0; l < char_model.NA_COUNT; l++)
				n.data.sankoff_value[l] = 1<<25;
			n.data.sankoff_value[char_model.na_to_int(n.data.seq[loc])] = 0;
		} else {
			for (auto&c : n.children) {
				if (c.data.valid) {
					sankoff1(c);
				}
			}
			for (int l = 0; l<char_model.NA_COUNT; l++) {
				int v = 0;
				for (auto& c : n.children) {
					if (c.data.valid) {
						int v_with_child = 1<<25;
						for (int l2=0; l2<char_model.NA_COUNT; l2++)
							v_with_child = min(v_with_child, char_model.cost[l][l2] + c.data.sankoff_value[l2]);
						v += v_with_child;
					}
				}
				n.data.sankoff_value[l] = v;
			}
		}
		/*
		   cerr << "sankoff1 " << n.label << " ";
		for (int i=0; i<NA_COUNT; i++)
			cerr << n.data.sankoff_value[i] << " ";
		cerr << endl;
		*/
	}

	void sankoff2(INode& n, int l) {
		if (l == -1) {
			int min_l = 0; //default is NON_GERMANY
			for (int l = 0; l<char_model.NA_COUNT; l++)
				if (n.data.sankoff_value[l] < n.data.sankoff_value[min_l])
					min_l = l;
			sankoff2(n, min_l);
		} else {
			//if ((int)n.data.seq.size() != seq_len) {
			//	n.data.seq.resize(seq_len);
			//}
			n.data.seq[loc] = char_model.int_to_char(l);
			//cerr << "A " << n.label << " loc=" << loc << " =" << l << endl;
			for (auto&c : n.children) {
				if (c.data.valid) {
					//double v_with_child = 1e10;
					int min_l2 = l;
					for (int l2=0; l2<char_model.NA_COUNT; l2++) {
						float cost_new = char_model.cost[l][l2] + c.data.sankoff_value[l2], 
							cost_old = char_model.cost[l][min_l2] + c.data.sankoff_value[min_l2];
						//if (cost[l][l2] + n.sankoff_value[l2] < cost[l][min_l2] + n.sankoff_value[min_l2])
						if (cost_new < cost_old || (cost_new == cost_old && l2 == l))
							min_l2 = l2;
						//v_with_child = min(v_with_child, cost[l][l2] + n.sankoff_value[l2]);
					}
					sankoff2(c, min_l2);
				}
			}
		}
	}

	void assign_memory(INode& n) {
		if ((int)n.data.seq.size() != seq_len)
			n.data.seq.resize(seq_len, 0);
		for (auto& c: n.children)
			assign_memory(c);
	}

	void sankoff_pre(INode& n) {
		n.data.valid = true;
		for (auto& c: n.children) {
			sankoff_pre(c);
			n.data.valid |= c.data.valid;
		}
	}

	void sankoff(INode& n) {
		//cerr << "assigning memory " << endl;
		assign_memory(n);
		cerr << "sankoff " << n.label << " " << seq_len << endl;
		sankoff_pre(n);
		for (int i=0; i<seq_len; i++) {
			loc = i;
			sankoff1(n);
			sankoff2(n, -1);
			//cerr << "S loc=" << i << " ";
			//for (int j=0; j<NA_COUNT; j++)
			//	cerr << n.data.sankoff_value[j] << " ";
			//cerr << endl;

		}
	}
};

struct VMValue {
	void* value;
	template<typename T>
	T as() {
		return *static_cast<T*>(value);
	}
	VMValue(void* value=0) : value(value) {}
};

/*
struct VM {
	map<string, VMValue> options;

	VM(char* argv[]) {
		vector<string>* l = new vector<string>;
		//cerr << "VM" << endl;
		l->push_back(argv[1]);
		//cerr << "VM" << endl;
		l->push_back(argv[2]);
		//cerr << "VM" << endl;
		options["location_label"] = l;
		//cerr << "VM" << endl;
		cerr << "location_label" << *l << endl;
		options["in"] = new string(argv[3]);
		options["out"] = new string(argv[4]);
		options["dna"] = new string(argv[5]);
		options["cost"] = new string(argv[6]);
		options["out-dep"] = new string(argv[7]);
	}

	int count(string name) {
		return options.find(name) != options.end() ? 1 : 0;
	}

	VMValue operator[](string n) {
		return options[n];
	}


};

*/

int main(int argc, char* argv[]) {

	po::options_description desc("Allowed options");
	desc.add_options()
	    //("help", "Run sankoff")
	    ("tree", po::value<string>()->required(), "input tree")
	    ("out", po::value<string>()->required(), "output mutation-sample file")
	    ("aln", po::value<string>()->required(), "aligned sequence file")
	    ("cost", po::value<string>()->required(), "cost function file name")
	    ("print-alignment", po::value<string>(), "print alignment")
	    ("omit-leaf-mutations", "omit mutations happen at leaf nodes")
	;


	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);    


	//VM vm(argv);

	StateInOut::names = {"in", "out"};

	string tree_file_name = vm["tree"].as<string>(),
		output = vm["out"].as<string>();
	//map<string, Metadata> id_to_name = load_map(annotation);
	//cerr << "metadata loaded" << endl;

	INode phylo = load_tree<INode>(tree_file_name) ;

	Sankoff sankoff;
	//0:GERMANY, 1:NON_GERMANY
	//cost_type cost = {0,1,1,0};
	
	//vector<float> cost_vector = vm["cost"].as<vector<float>>();
	//cost_type cost = {cost_vector[0], cost_vector[1], cost_vector[2], cost_vector[3]};
	//cerr << "cost: " << cost << endl;
	sankoff.load_cost(vm["cost"].as<string>());

	if (vm.count("omit-leaf-mutations")) {
		sankoff.omit_leaf_mutations = true;
	}

	sankoff.build_map(phylo);
	cerr << "build_map done with " << sankoff.label_node.size() << " samples" << endl;
	sankoff.load_dna(vm["aln"].as<string>());

	sankoff.sankoff(phylo);
	if (vm.count("print-alignment")) {
		sankoff.print_alignment(vm["print-alignment"].as<string>(), phylo);
	}
	sankoff.print_mutation_samples(output, phylo);

	if (vm.count("out-dep") > 0) {
		sankoff.print_mutation_dependency(vm["out-dep"].as<string>(), phylo);
	}

	return 0;
}
