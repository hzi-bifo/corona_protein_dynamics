#include <iostream>
#include <ostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <regex>
#include <list>
using namespace std;

template<typename T>
ostream& operator<<(ostream& os, const vector<T>& v) {
	os << "[";
	for (auto const &i: v)
		os << i << " ";
	return os << "]";
}

template<typename T, size_t L>
ostream& operator<<(ostream& os, const array<T, L>& v) {
	os << "[";
	for (auto const &i: v)
		os << i << " ";
	return os << "]";
}

template<typename T, typename S>
ostream& operator<<(ostream& os, const map<T,S>& v) {
	os << "[";
	for (auto const &i: v)
		os << i.first <<":" << i.second << " ";
	return os << "]";
}

bool endswith(const string& s, const string& e) {
	if (s.size() < e.size()) return false;
	for (size_t i = s.size() - e.size(), j=0; j < e.size(); i++, j++) {
		if (s[i] != e[j]) return false;
	}
	return true;
}

bool iequals(const string& a, const string& b) {
	for (size_t i = 0, j = 0; i < a.size() || j < b.size(); ) {
		if (i < a.size() && isspace(a[i])) {
			i++;
		} else if (j < b.size() && isspace(b[j])) {
			j++;
		} else if (i < a.size() && j < b.size() && tolower(a[i]) == tolower(b[j])) {
			i++;
			j++;
		} else {
			return false;
		}
	}
	return true;
}

std::string trim(const std::string& str,
                 const std::string& whitespace = " \t\n\r")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    string r = str.substr(strBegin, strRange);
    //cerr << "trim " << r << endl;
    return r;
}

vector<string> split(const string& l, char splitter = '\t') {
	vector<string> x;
	std::istringstream iss(l);
	for (string token; getline(iss, token, splitter); ) {
		x.push_back(token);
	}
	return x;
}


istream& seek_spaces(istream& is, char space = ' ') {
	while (is.peek() == space)
		is.get();
	return is;
}

struct Metadata {
	string id, name, date, location, location_add, pangolin;
	Metadata(string _id = "", string _name = "", string _date = "", string _location = "", string _location_add = "", string pangolin ="") :
		id(_id), name(_name), date(_date), location(_location), location_add(_location_add), pangolin(pangolin) {}
};

ostream& operator<<(ostream& os, const Metadata& m) {
	return os << "(" << m.id << "," << m.name << "," << m.date << "," << m.location << "," << m.location_add << ")";
}


//TODO: size should be corrected
typedef array<array<float, 2>, 2> cost_type;
template<typename STATE, typename DATA=int>
struct Node {
	typedef STATE StateType;
	typedef DATA DataType;
	string label;
	float branch_length;
	string annotation;
	list<Node<STATE,DATA>> children;
	STATE location;
	int size, height, sample_size;
	int edge_label;//used in jplace

	array<double, STATE::size> sankoff_value;
	DATA data;

	Node(string _label="", float _branch_length=0, string _annotation="", 
		const list<Node<STATE,DATA>>& _children=list<Node<STATE,DATA>>(), 
		STATE _location=STATE::unknown, int _size=1, int _height=1, int _sample_size=0, int edge_label = -1) :
		label(_label), branch_length(_branch_length), annotation(_annotation), children(_children),
		location(_location), size(_size), height(_height), sample_size(_sample_size), edge_label(edge_label) {
	}

	void sankoff(const cost_type& cost) {
		if (children.size() == 0) {
			if (location == STATE::unknown) {
				cerr << "Invalid location for tip node " << label << endl;
			}
			assert (location != STATE::unknown);
			sankoff_value[location] = 0;
			sankoff_value[1-location] = 1e10;

			//if (label == "EPI_ISL_413489") {
			//	cerr << " e " << sankoff_value[location] << endl;
			//}
		} else {
			for (auto&n : children) {
				n.sankoff(cost);
			}
			//GERMANY, NON-GERMANY
			for (int l = 0; l<STATE::size; l++) {
				double v = 0;
				for (auto&n : children) {
					double v_with_child = 1e10;
					for (int l2=0; l2<STATE::size; l2++)
						v_with_child = min(v_with_child, cost[l][l2] + n.sankoff_value[l2]);
					v += v_with_child;
				}
				sankoff_value[l] = v;
			}
		}
	}

	void sankoff2(int l, const cost_type& cost) {
		assert(l != -2);
		if (l == -1) {
			int min_l = 0; //default is NON_GERMANY
			for (int l = 0; l<STATE::size; l++)
				if (sankoff_value[l] < sankoff_value[min_l])
					min_l = l;
			sankoff2(min_l, cost);
		} else {
			if (location != STATE::unknown && location != (STATE)l) {
				cerr << "Location label is incompatible " << label << " " << location << " " << l << " " << (STATE)l << endl;
			}
			assert(location == STATE::unknown || location == (STATE)l);
			//location = static_cast<STATE>(l);
			location = (STATE)l;
			for (auto&n : children) {
				//double v_with_child = 1e10;
				int min_l2 = 1;
				for (int l2=0; l2<STATE::size; l2++) {
					float cost_new = cost[l][l2] + n.sankoff_value[l2], 
						cost_old = cost[l][min_l2] + n.sankoff_value[min_l2];
					//if (cost[l][l2] + n.sankoff_value[l2] < cost[l][min_l2] + n.sankoff_value[min_l2])
					if (cost_new < cost_old || (cost_new == cost_old && l2 == l))
						min_l2 = l2;
					//v_with_child = min(v_with_child, cost[l][l2] + n.sankoff_value[l2]);
				}
				n.sankoff2(min_l2, cost);
			}
		}
	}

	void build_tree(istream& is) {
		//cerr << "BT " << (char) is.peek() << endl;
		if (is.peek() == '(') {
			do {
				is.get(); // either first ( or next ,s
				children.push_back(Node<STATE,DATA>());
				children.back().build_tree(is);
			} while (is.peek() == ',');
			assert(is.get() == ')');
		}
		label = read_name(is);
		annotation = read_annotation(is);
		branch_length = read_branch_length(is);
		edge_label = read_edge_label(is);
		//
		//if (label == "EPI_ISL_1142103") {
		//	cerr << "Load " << label << " " << branch_length << " " << annotation << endl;
		//}

		sample_size = isLeaf() ? 1 : 0;
		size = height = 1;
		for (auto &i : children) {
			size += i.size;
			sample_size += i.sample_size;
			height = max(height, i.height + 1);
		}
	}

	string read_name(istream& is) {
		string r;
		int c = is.get();
		while (c != ':' && c != ',' && c != '(' && c != ')' && c != '[' && c != ']' && c != ';' && !is.eof()) {
			r += (char) c;
			c = is.get();
		}
		is.putback(c);
		//cerr << " name: " << r << endl;
		return r;
	}

	int read_edge_label(istream& is) {
		if (is.peek() == '{') {
			assert(is.get() == '{');
			int e;
			is >> e;
			assert(is.get() == '}');
			return e;
		}
		return -1;
	}

	float read_branch_length(istream& is) {
		if (is.peek() == ':') {
			assert(is.get() == ':');
			float bl;
			is >> bl;
			//cerr << " bl: " << bl << endl;
			return bl;
		}
		return 0;
	}

	string read_annotation(istream& is) {
		string rr;
		if (is.peek() == '[') {
			char open_brace = is.get();
			if (is.peek() == '&') {
				string r;
				assert(is.get() == '&');
				while (is.peek() != ']')
					r += (char) is.get();
				assert(is.get() == ']');

				//cerr << "annotation: " << label << " : " << r << endl;
				istringstream r_iss(r);
				for (string nv, n, v; getline(r_iss, nv, ','); ) {
					istringstream nv_iss(nv);
					getline(nv_iss, n, '=');
					if (n == "location") {
						nv_iss >> location;
						//if (location == location.unknown)
						//	cerr << "R loc " << label << " " << location << endl;
					} else {
						if (rr.size() > 0) rr += ",";
						rr += nv;
					}
				}
				
			} else {
				is.putback(open_brace);
			}
		}
		return rr;
	}

	//TODO: remove
	//ostream& print_node_info(ostream&os, bool force_print_location) const {
	//	if (label != "") {
	//		os << label;
	//	}
	//	if (annotation != "" || (location != STATE::unknown && force_print_location) /*|| sankoff_value[0] > -1*/) {
	//		os << "[&";
	//		bool empty = true;
	//		if (annotation != "") {
	//			os << annotation;
	//			empty = false;
	//		}
	//		if (location != STATE::unknown && force_print_location) {
	//			if (!empty)
	//				os << ",";
	//			os << "location=" << location;
	//			empty = false;
	//		}
	//		//os << "," << "s=" << sankoff_value[0] << "|" << sankoff_value[1];
	//		os << "]";
	//	}
	//	return os << ":" << branch_length;
	//}

	//ostream& print(ostream& os, bool force_print_location = true) const {
	//	if (children.size() > 0) {
	//		os << "(";
	//		bool first = true;
	//		for (auto &n : children) {
	//			if (!first)
	//				os << ",";
	//			first = false;
	//			n.print(os, force_print_location);
	//		}
	//		os << ")";
	//	}
	//	print_node_info(os, force_print_location);
	//	return os;
	//}

	vector<string> leafLabels() const {
		vector<string> r;
		if (isLeaf()) {
			r.push_back(label);
			return r;
		}
		for (auto const& n : children) {
			vector<string> rr = n.leafLabels();
			r.insert(r.end(), rr.begin(), rr.end());
		}
		return r;
	}

	void set_tip_location(const map<string, STATE>& isolate_matrix) {
		for (auto &n: children)
			n.set_tip_location(isolate_matrix);
		if (!(children.size() > 0 || isolate_matrix.find(label) != isolate_matrix.end())) {
			cerr << "W TIP with no location " << label << " " << endl;
			location = STATE::def;
			return;
		}
		assert(children.size() > 0 || isolate_matrix.find(label) != isolate_matrix.end());
		if (isolate_matrix.find(label) != isolate_matrix.end())
			location = isolate_matrix.find(label)->second;
	}

//	void split_and_print(string prefix, int& index) const {
//		//cerr << "SP " << index << "..." << size << " " << height << endl;
//		bool all_children_non_germany = location == NON_GERMANY; // for root
//		for (const auto& n: children)
//			all_children_non_germany &= n.location == NON_GERMANY;
//		if (all_children_non_germany) {
//			for (const auto& n: children)
//				n.split_and_print(prefix, index);
//		} else {
//			//we should check children.size() > 1 to not do the sample for newly created noded
//			if (size > 20000 && children.size() > 1) {
//				cerr << "G->NG,NG splitted" << endl;
//				for (const auto& n: children)
//					if (n.location == NON_GERMANY)
//						n.split_and_print(prefix, index);
//					else {
//						Node nroot;
//						nroot.location = NON_GERMANY;
//						nroot.children.push_back(n);
//						nroot.size = n.size + 1;
//						nroot.sample_size = n.sample_size;
//						nroot.height = n.height;
//						nroot.split_and_print(prefix, index);
//						nroot.branch_length = branch_length;
//						nroot.label = label;
//						nroot.annotation = annotation;
//					}
//			} else {
//				index++;
//				ofstream fo(prefix + to_string(index) + ".tree");
//				fo << "#NEXUS" << endl;
//				fo << "BEGIN TREES;" << endl << ";" << endl;
//				fo << "tree STATE_0 = ";
//				print(fo, true) << ";" << endl;
//				fo << "END;" << endl;
//
//				cerr << "N " << index << " " << size << "(" << sample_size << ")" << " " << height << " " << label << " [";
//				for (auto const& n:children)
//					cerr << n.size << ":" << n.location << ",";
//				cerr << "]" <<endl;
//			}
//		}
//	}


	bool isValidLeaf(const map<string, Metadata>& id_to_metadata) const {
		bool b = (id_to_metadata.find(label) != id_to_metadata.end()) &&
			//regex_match(id_to_metadata.find(label)->second.date, regex("\\d\\d\\d\\d-\\d\\d-\\d\\d"));
			id_to_metadata.find(label)->second.date.size() == 10;
		//cerr << "L " << label << " " << b << endl;
		return b;
	}

	bool isLeaf() const {
		return children.size() == 0;
	}

	//check validity for printing. Now it checks validity of the date
	// if returns false, itself should be removed
	bool remove_invalid_children(const map<string, Metadata>& id_to_metadata, int& removed_count) {
		//cerr << "?E" << label << height << " ";
		if (isLeaf())
			return isValidLeaf(id_to_metadata);
		bool keepAtLeastOne = false;
		vector<bool> keep;
		for (auto &n: children) {
			bool k = (n.remove_invalid_children(id_to_metadata, removed_count));
			keepAtLeastOne |= k;
			keep.push_back(k);
		}
		if (!keepAtLeastOne) 
			//remove me
			return false;
		size = height = 1;
		/*
		int n = 0;
		for (size_t i =0, j=0; i<children.size(); i++) {
			if (!keep[i]) {
				//vector<string> rr = children[i].leafLabels();
				//cerr << "REM ";
				//children[i].print(cerr) << endl;
				//cerr << rr << endl;
				removed_count += children[i].size;

				//if (find(rr.begin(), rr.end(), "EPI_ISL_860761") != rr.end()) {
				//	cerr << "REMOVING THE CHILD IS FOUND! " << rr << " " << children[i].isLeaf() << " " << children[i].isValidLeaf(id_to_metadata) << " " << children[i].label << " " << id_to_metadata.find(label)->second.date.size() << endl;
				//}
			} else {
				size += children[i].size;
				height = max(height, children[i].height + 1);
				if (j != i)
					children[j] = children[i];
				j++;
				n++;
			}
		}
		*/
		int keepIndex = 0;
		//TODO: to be tested
		for (auto i = children.begin(); i != children.end(); keepIndex++) {
			if (!keep[keepIndex]) { 
				removed_count += i->size;
				i = children.erase(i);
			} else {
				size += i->size;
				height = max(height, i->height + 1);
				i++;
			}
		}
		//children.resize(n);
		return true;
	}	

	void name_internal_nodes(int& index) {
		if (!isLeaf()) 
			label = "inode" + to_string(index++);
		for (auto &n : children)
			n.name_internal_nodes(index);
	}

	template<typename T>
	void apply(T& f) {
		f(*this);
		for (auto & c : children) {
			c.apply(f);
		}
	}

	template<typename T>
	void apply_with_info(T& f, Node& parent) {
		f(*this, parent);
		for (auto & c : children) {
			c.apply_with_info(f, *this);
		}
	}

	void update_stat() {
		size = 1;
		height = 1;
		sample_size = isLeaf();
		for (const auto& c : children) {
			size += c.size;
			height = max(height, c.height + 1);
			sample_size += c.sample_size;
		}
	}
};


string get_name(string s) {
	vector<string> x;
	std::istringstream iss(s);
	for (string token; getline(iss, token, '/'); ) {
		x.push_back(token);
	}
	if (x.size() == 4) {
		x.erase(x.begin());
	}
	string r = "";
	for (auto i : x) {
		r += i;
	}
	return r;
}

template<typename NODE>
NODE load_tree(istream& fi) {
	//string l;
	//getline(fi, l);
	//istringstream is(l);
	NODE n;
	n.build_tree(fi);
	cerr << "tree with " << n.size << " nodes and " << n.height << " height loaded " << endl;
	return n;
}

template<typename NODE>
struct node_rename_by_map {
	const map<string, string>& label_map;

	node_rename_by_map(const map<string, string>& _label_map) : label_map(_label_map) {}

	void operator()(NODE& n) {
		if (n.isLeaf()) {
			if(label_map.find(n.label) == label_map.end()) {
				cerr << "Label " << n.label << " not present in map" << endl;
			}
			assert(label_map.find(n.label) != label_map.end());
			//cerr << " L " << n.label << " -> " << label_map.find(n.label)->second << endl;
			n.label = label_map.find(n.label)->second;
		}
	}
};

template<typename NODE>
NODE load_nexus_tree(ifstream& fi) {
	//cerr << "  nexus: begin " << endl;
	map<string, string> id_to_name;
	for (string s; getline(fi, s); ) {
		//cerr << "  nexus: read " << s.substr(0, 40) << endl;
		if (iequals(s, "begin trees;")) {
			fi >> s;
			//cerr << " after beg tree " << s << endl;
			if (iequals(trim(s), "translate")) {
				while (trim(s) != ";") {
					vector<string> row = split(trim(s), '\t');
					if (row.size() != 2)
						row = split(trim(s), ' ');
					if (row.size() == 2) {
						string name = trim(row[1]);
						if (name.size() > 0 && name[name.size()-1] == ',')
							name = name.substr(0, name.size()-1);
						id_to_name[row[0]] = name;
					}
					getline(fi, s);
				}
				fi >> s;
			}
			if (iequals(s, "tree")) {
				for (int c, i=0; (c=fi.get()) != '='; i++) {
					//cerr << "C " << (char) c << endl;
					assert(i < 50);
				}
				seek_spaces(fi);
				if (fi.peek() == '[') {
					while (fi.get() != ']')
						;
				}
				seek_spaces(fi);
				assert(fi.peek() == '(');
				break;
			} else {
				assert(1 != 1);
			}
		} else {
			//before begin trees
			//cerr << "LN bt != " << s.substr(0, 15) << endl;
		}
	}
	NODE n = load_tree<NODE>(fi);
	if (id_to_name.size() > 0) {
		node_rename_by_map<NODE> renamer(id_to_name);
		n.apply(renamer);
	}
	return n;
}

template<typename NODE>
NODE load_tree(string fn) {
	//cerr << "loading " << fn << " ..." << endl;
	ifstream fi(fn);
	NODE n = endswith(fn, ".nexus") ? load_nexus_tree<NODE>(fi) : load_tree<NODE>(fi);
	return n;
}

int indexOf(const vector<string> v, const std::initializer_list<string>& keys) {
	for (auto const &k : keys) {
		if ( find(v.begin(), v.end(), k) != v.end())
			return find(v.begin(), v.end(), k) - v.begin();
	}
	cerr << "Warning: not found indexOf "  << v << " " << vector<string>(keys.begin(), keys.end()) << endl;
	//assert(1 != 1);
	return -1;
}

map<string, Metadata> load_map(istream& fi) {
	map<string, Metadata> ret;
	int name_index = -1, id_index = -1, date_index = -1, location_index = -1, collection_date_index = -1, location_addition_index = -1, pangolin_index=-1;
	char separator = 0;
	for (string l; getline(fi, l); ) {
		//cerr << "M " << l << endl;
		if (separator == 0) {
			if (l.find('\t') != string::npos) 
				separator = '\t';
			else if (l.find(',') != string::npos) 
				separator = ',';
			else 
				assert(1 != 1);
		}

		vector<string> x = split(trim(l), separator);
		if (name_index == -1) {
			//cerr << "metadata first line " << l << endl;
			name_index = indexOf(x, {"Virus name", "Virus.name", "virus_name", "sequence_name"});
			id_index = indexOf(x, {"Accession ID", "Accession.ID", "accession_id", "sequence_name"});
			date_index = indexOf(x, {"Collection date", "Collection.date", "collection_date", "sample_date"});
			location_index = indexOf(x, {"Location", "country"});
			location_addition_index = indexOf(x, {"Additional.location.information", "Additional location information"});
			//collection_date_index = indexOf(x, {"Submission date"});
			collection_date_index = indexOf(x, {"Collection date", "Collection.date", "sample_date"});
			pangolin_index = indexOf(x, {"Pango lineage", "Pango.lineage"});
			//cerr << "metadata first line " << name_index << " " << id_index << " " << date_index << " " << location_index << " " << location_addition_index << " " << collection_date_index << endl;
		} else {
			//cerr << "META " << x[id_index] << " " << x[name_index] << " " << x[date_index] << " " << x[location_index] << endl;
			//if (x[id_index] == "EPI_ISL_860761") {
			//	cerr << "META " << x[id_index] << " " << x[name_index] << " " << x[date_index] << " " << x[location_index] << " " << x[collection_date_index] << endl;
			//}
			ret[x[id_index]] = Metadata(x[id_index], x[name_index], x[date_index].size() != 10 ? x[collection_date_index] : x[date_index], x[location_index], location_addition_index != -1 ? x[location_addition_index] : "", pangolin_index != -1 ? x[pangolin_index] : "");
		}
	}
	return ret;
}

map<string, Metadata> load_map(string fn) {
	ifstream fi(fn);
	return load_map(fi);
}


bool startsWith(string s, string w) {
	//cerr << "SW " << s << " " << w << endl;
	return s.find(w) == 0;
}

template<typename NODE>
struct NodePrinterAbstractClass {
	bool force_print_location;
	bool print_internal_node_labels, allow_print_annotation;

	NodePrinterAbstractClass(bool force_print_location, bool print_internal_node_labels, bool allow_print_annotation = true) : 
		force_print_location(force_print_location), print_internal_node_labels(print_internal_node_labels), allow_print_annotation(allow_print_annotation) {
	}

	ostream& print_node_info(ostream&os, const NODE& n) const {
		if (n.label != "" && (print_internal_node_labels || (n.isLeaf()))) {
			os << n.label;
		}
		if (allow_print_annotation && (n.annotation != "" || (n.location != NODE::StateType::unknown && force_print_location) /*|| sankoff_value[0] > -1*/)) {
			os << "[&";
			bool empty = true;
			if (n.annotation != "") {
				os << n.annotation;
				empty = false;
			}
			if (n.location != NODE::StateType::unknown && force_print_location) {
				if (!empty)
					os << ",";
				os << "location=" << n.location;
				empty = false;
			}
			//os << "," << "s=" << sankoff_value[0] << "|" << sankoff_value[1];
			os << "]";
		}
		return os << ":" << n.branch_length;
	}

};

template<typename NODE>
struct NodePrinter : public NodePrinterAbstractClass<NODE> {
	string prefix;
	int index;
	typename NODE::StateType include_state;

	NodePrinter(string _prefix = "", int _index = 0, typename NODE::StateType _include_state = NODE::StateType::unknown) : 
		NodePrinterAbstractClass<NODE>(true, true),
		prefix(_prefix), index(_index), include_state(_include_state) {}

	void print_lineage(const NODE& t, ofstream& os) {
		//if (t.children.size() > 0 && t.children[0].label == "EPI_ISL_960949") {
		//	cerr << "Parent of . ";
		//	for (auto const& n : t.children)
		//		n.print_node_info(cerr, true);
		//	cerr << endl;
		//}
		//if (t.label == "EPI_ISL_860761") {
		//	cerr << "PRINTING " << "EPI_ISL_860761 &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&77 " << endl;
		//}
		if (t.children.size() > 0) {
			os << "(";
			int printed_node_count = 0;
			for (auto &n : t.children) {
				if (n.location == include_state) {
					if (printed_node_count > 0)
						os << ",";
					print_lineage(n, os);
					printed_node_count ++;
				} else {
					find_and_print_lineage(n);
				}
			}
			if (printed_node_count < 2 /*&& all_printed_are_leaf */) {
				//cerr << "Adding " << printed_node_count << " " ;
				for (auto &n : t.children) {
					if (n.location != include_state) {
						if (printed_node_count > 0)
							os << ",";
						this->print_node_info(os, n);
						//n.print_node_info(os, true);
						printed_node_count ++;
					}
				}
				//cerr << " " << printed_node_count << endl;
			}
			while (printed_node_count < 2) {
				NODE n;
				n.location = NODE::StateType::def;
				n.size = 1;
				n.sample_size = 0;
				n.height = 1;
				n.branch_length = 0;
				n.label = "NON_GERMANY_LABEL_" + random_string();
				n.annotation = "height=0";
				if (printed_node_count > 0)
					os << ",";
				this->print_node_info(os, n);
				//n.print_node_info(os, true);
				printed_node_count ++;
				//cerr << " + " << printed_node_count << " " << n.label << " " << t.label << endl;
				//this->print_node_info(cerr, n);
			}

			os << ")";
		}

		//t.print_node_info(os, true);
		this->print_node_info(os, t);
	}

	string random_string() {
		std::string tmp_s;
		static const char alphanum[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";

		//srand( (unsigned) time(NULL));
		int len = 10;

		tmp_s.reserve(len);

		for (int i = 0; i < len; ++i) 
			tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];


		return tmp_s;
	}

	void find_and_print_lineage(const NODE& t) {
		//assert(t.location != include_state && t.location != STATE::unknown);
		if (!(t.location != NODE::StateType::unknown)) {
			cerr << "E: " << t.location << " " << t.location.names[0] << " " << t.location.names[1] << endl;
		}
		assert(t.location != NODE::StateType::unknown);
		//for the case that root is include_state. The result is a bit wired.
		if (t.location == include_state) {
			cerr << "Warning: a tree with root to be included" << endl;
			NODE nroot;
			nroot.location = NODE::StateType::def;
			nroot.children.push_back(t);
			nroot.size = t.size + 1;
			nroot.sample_size = t.sample_size;
			nroot.height = t.height + 1;
			//nroot.branch_length = t.branch_length; //It seems that UK paper scripts always assume root.branch_lenght = 0
			nroot.branch_length = 0;
			nroot.label = "";
			nroot.annotation = "";

			init_file(nroot);
		} else {
			for (const auto& v : t.children) {
				if (v.location == include_state) {
							NODE nroot;
							nroot.location = t.location;
							nroot.children.push_back(v);
							nroot.size = v.size + 1;
							nroot.sample_size = v.sample_size;
							nroot.height = v.height + 1;
							//nroot.branch_length = t.branch_length; //It seems that UK paper scripts always assume root.branch_lenght = 0
							nroot.branch_length = 0;
							nroot.label = t.label;
							nroot.annotation = t.annotation;

					init_file(nroot);
				} else 
					find_and_print_lineage(v);
			}
		}
	}

	void init_file(const NODE& t) {
		index++;
		string fn = prefix + to_string(index) + ".tree";
		if (prefix.find("?") != string::npos)
			fn = prefix.substr(0, prefix.find("?")) + to_string(index) + prefix.substr(prefix.find("?")+1);
		//cerr << "F: " << fn << " " << prefix << endl;
		ofstream fo(fn);
		fo << "#NEXUS" << endl;
		fo << "BEGIN TREES;" << endl;
		fo << "tree STATE_0 = ";
		print_lineage(t, fo);
		fo << ";" << endl << "END;" << endl;

		/*
		cerr << "N " << index << " " << t.size << "(" << t.sample_size << ")" << " " << t.height << " " << t.label << " [";
		for (auto const& n:t.children)
			cerr << n.size << ":" << n.location << ",";
		cerr << "]" <<endl;
		*/
	}
};

template<typename NODE>
struct NodePrinterGeneral : public NodePrinterAbstractClass<NODE> {

	NodePrinterGeneral(bool _force_print_location = true, bool _print_internal_node_labels = true, bool allow_print_annotation = true) : 
		NodePrinterAbstractClass<NODE>(_force_print_location, _print_internal_node_labels, allow_print_annotation) {}

	ostream& print(ostream& os, const NODE& n) const {
		if (n.children.size() > 0) {
			os << "(";
			bool first = true;
			for (auto &c : n.children) {
				if (!first)
					os << ",";
				first = false;
				print(os, c);
			}
			os << ")";
		}
		this->print_node_info(os, n);
		return os;
	}

};

template<typename NODE>
struct SamplePrinter {
	int printed_count = 0;
	SamplePrinter() {}

	void run(const NODE& n, string fn) {
		//cerr << "SamplePrinter::run() " << fn << " " << n.label << endl;
		ofstream fo(fn);
		//cerr << "SamplePrinter::run() file created " << endl;
		dfs(n, fo);
	}

	void dfs(const NODE& n, ofstream& fo) {
		//cerr << "sample printer " << n.label << endl;
		if (n.isLeaf()) {
			printed_count++;
			fo << n.label << endl;
		}
		for (auto const& c : n.children) {
			dfs(c, fo);
		}
	}
};

template<typename NODE>
struct InternalNodeLabeler {
	int cnt;
	InternalNodeLabeler() : cnt(0) {}
	void run(NODE& n) {
		cnt=0;
		label(n);
	}

	void label(NODE&n) {
		if (!n.isLeaf()) {
			n.label = "inode" + to_string(cnt++);
		}
		for (auto &c : n.children) {
			label(c);
		}
	}
};

template<typename NODE, typename SampleIncluder>
struct SubtreeExtractorOverSamples {

	int removed_internal_count, removed_sample_count;
	SampleIncluder sampleIncluder;

	SubtreeExtractorOverSamples(const SampleIncluder& sampleIncluder = SampleIncluder()) : removed_internal_count(0), removed_sample_count(0), 
		sampleIncluder(sampleIncluder) {
	}
	
	NODE run(const NODE& n) {
		NODE r = sample(n).first;
		if (r.children.size() == 1) 
			return r.children.back();
		return r;
	}

	//rebuild subtree basd on sampled/non-sampled, each internal node should have at least two children.
	pair<NODE, bool> sample(const NODE& n) {
		//height, size should be recalculated
		NODE r(n.label, n.branch_length, n.annotation, list<NODE>(), n.location, 1, 1, n.isLeaf() ? 1 : 0);
		for (auto &c: n.children) {
			pair<NODE, bool> c_c_ = sample(c);
			if (c_c_.second == false) {
				if (c.isLeaf())
					removed_sample_count++;
				else
					removed_internal_count++;
				continue;
			}
			NODE& c_c = c_c_.first;
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
		//return make_pair(r, r.children.size() > 0 || sample_names.find(n.label) != sample_names.end());
		return make_pair(r, r.children.size() > 0 || sampleIncluder(n));
	}
};

template<typename NODE>
struct SingleChildInternalNodeRemover {

	int removed_internal_count;

	SingleChildInternalNodeRemover() : removed_internal_count(0) {
	}
	
	NODE run(const NODE& n) {
		removed_internal_count = 0;
		return dfs(n);
	}

	NODE dfs(const NODE& n) {
		NODE r(n.label, n.branch_length, n.annotation, list<NODE>(), n.location, 1, 1, n.isLeaf() ? 1 : 0);
		for (auto &c: n.children) {
			NODE c_c = dfs(c);
			//normal:
			r.height = max(r.height, c_c.height + 1);
			r.size += c_c.size;
			r.sample_size += c_c.sample_size;
			//r.children.push_back(move(c_c));
			r.children.push_back(c_c);
		}
		if (r.children.size() == 1) {
			removed_internal_count++;
			r.children.begin()->branch_length += r.branch_length;
			//r = std::move(*r.children.begin());
			//TODO: we should make it faster!
			//cerr << r.children.begin()->label << endl;
			NODE tmp = *r.children.begin();
			r = tmp;
		}
		return r;
	}
};

template<typename NODE, typename DFS_ACTION>
struct TreeDFSGeneral {

	DFS_ACTION action;
	TreeDFSGeneral(DFS_ACTION action) : action(action) {}

	void dfs(NODE& n) {
		action.visit(n);

		for (auto & c : n.children) {
			dfs(c);
		}
		action.finish(n);
	}
};


template<typename NODE>
struct NodePrinterNexus {

	vector<tuple<int, string, bool>> index_name;
	int current_index;

	NodePrinterNexus() {}

	void print_node_info(ostream& os, const NODE& n) {
		if (n.label == "EPI_ISL_753872") 
			cerr << "going to pring EPI_ISL_753872 ++ " << endl;

		if (n.label != "") { //  && n.isLeaf()
			int l = current_index++;
			index_name.push_back(make_tuple(l, n.label, n.isLeaf()));
			os << l;
		}
		if (n.annotation != "") {
			os << "[&";
			bool empty = true;
			if (n.annotation != "") {
				os << n.annotation;
				empty = false;
			}
			if (n.location != NODE::StateType::unknown) {
				if (!empty)
					os << ",";
				os << "location=" << n.location;
				empty = false;
			}
			//os << "," << "s=" << sankoff_value[0] << "|" << sankoff_value[1];
			os << "]";
		}
		os << ":" << n.branch_length;
	}

	ostream& print(ostream& os, const NODE& n) {
			if (n.label == "EPI_ISL_753872") 
				cerr << "going to pring EPI_ISL_753872 " << endl;

		if (n.children.size() > 0) {
			os << "(";
			bool first = true;
			for (auto &c : n.children) {
				if (!first)
					os << ",";
				first = false;
				print(os, c);
			}
			os << ")";
		}
			if (n.label == "EPI_ISL_753872") 
				cerr << "going to pring EPI_ISL_753872 + " << endl;
		print_node_info(os, n);
		return os;
	}

	void run(ostream& os, const NODE& n) {
		index_name.clear();
		current_index = 1;
		print(os, n);
	}

};


template<typename NODE>
void save_nexus_tree(ostream& os, const NODE& n, bool print_internal_taxa = false) {
	//cerr << "  nexus: begin " << endl;
	ostringstream tree_oss;
	NodePrinterNexus<NODE> npn;
	npn.run(tree_oss, n);

	int ntaxa = 0;
	for (auto & iname : npn.index_name)
		if (get<2>(iname) || print_internal_taxa)
			ntaxa++;

	os << "#NEXUS" << endl << endl
	   << "Begin taxa;" << endl
	   << "\tDimensions ntax=" << ntaxa << ";" << endl
	   << "\tTaxlabels" << endl;
	for (auto & iname : npn.index_name) {
		if (get<2>(iname) || print_internal_taxa)
			os << "\t\t" << get<1>(iname) << endl;
	}
	os << "\t\t;" << endl << "End;" << endl << endl
	   << "Begin trees;" << endl
	   << "\tTranslate";
	bool first = true;
	for (auto & iname : npn.index_name) {
		if (!first)
			os << ",";
		if (get<2>(iname) || print_internal_taxa)
			os << endl
			   << "\t\t" << get<0>(iname) << " " << get<1>(iname);
		first = false;
	}
	os << endl << "\t;" << endl;

	os << "tree TREE1 = [&R] " 
	   << tree_oss.str() << ";" << endl
	   << "End;" << endl;
}

