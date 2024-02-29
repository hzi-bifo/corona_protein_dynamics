#include "tree.h"
#include "state.h"
#include <boost/program_options.hpp>
//#include "rangetree.h"
#include <vector>
#include <numeric>
#include <regex>

namespace po = boost::program_options;

array<string, StateInOut::size> StateInOut::names = {"uk", "nonuk"};

int month_since(const string& date, int y = 1900, int m = 1, int d = 1) {
	tm date_tm = {0};
	if (!strptime(date.c_str(), "%Y-%m-%d", &date_tm)) {
		throw exception();
	}
	return (date_tm.tm_year + 1900 - y) * 12 + (date_tm.tm_mon- m - 1);
}       

int days_since(const string& date, int y = 1900, int m = 1, int d = 1) {
	tm then = {0};
	then.tm_year = y - 1900;
	then.tm_mon = m - 1;
	then.tm_mday = d;
	time_t then_secs = mktime(&then);

	tm date_tm = {0};
	if (!strptime(date.c_str(), "%Y-%m-%d", &date_tm)) {
		throw exception();
	}
	//cerr << "D " << date_tm.tm_year << " " << date_tm.tm_mon << " " << date_tm.tm_mday << " " << date_tm.tm_hour << " " << endl;
	//cerr << "D " << then.tm_year << " " << then.tm_mon << " " << then.tm_mday << " " << then.tm_hour << " " << endl;
	time_t t = mktime(&date_tm);  // t is now your desired time_t

	double days = difftime(t, then_secs);
	//cerr << "days_since_1900 " << date << " " << days << endl;
	return ((long)days) / 24 / 60 / 60;
}       

bool operator<(const StateInOut& a, const StateInOut& b) {
	return a.type < b.type;
}

struct Data;
typedef Node<StateInOut, Data> INode;

struct Data {
	bool sampled;
	int dfs_num;
	int date;
	Data() : sampled(false), dfs_num(-1), date(-1) {
	}
};

ostream& operator<<(ostream& os, const Data& d) {
	//os << "(" << d.dfs_vis <<"/" << d.dfs_fin << " " << d.depth << " ";
	//for (auto& p : d.parents)
	//	os << p->label << " ";
	//os << d.back_bone << " " << d.sampled << ")";
	return os;
}

struct Bucket {
	int epiweek;
	string country;

	Bucket(int epiweek=0, string country = "") : epiweek(epiweek), country(country) {
	}

	bool operator<(const Bucket& b) const {
		if (epiweek != b.epiweek) return epiweek < b.epiweek;
		return country < b.country;
	}
};

struct INodeStarCmpEarly {
	bool operator()(const INode* a, const INode* b) const {
		if (a->data.date != b->data.date) return a->data.date < b->data.date;
		return a->data.dfs_num < b->data.dfs_num;
	}
};
struct INodeStarCmpLate {
	bool operator()(const INode* a, const INode* b) const {
		if (a->data.date != b->data.date) return a->data.date > b->data.date;
		return a->data.dfs_num < b->data.dfs_num;
	}
};

struct TreeUnsampledPrinter {
	ofstream& fo;
	const set<string>& special_parents;
	string active_parent;

	TreeUnsampledPrinter(ofstream& fo, const set<string>& special_parents) : fo(fo), special_parents(special_parents), active_parent("NA") {
	}

	void visit(const INode& n) {
		if (special_parents.find(n.label) != special_parents.end())
			active_parent = n.label;
		if (n.isLeaf() && !n.data.sampled)
			fo << n.label << "\t" << active_parent << endl;
	}


	void finish(const INode& n) {
		if (special_parents.find(n.label) != special_parents.end())
			active_parent = "NA";
	}
};

const int BUCKET_TIME_INTERVAL_WEEK = 1, BUCKET_TIME_INTERVAL_MONTH = 2;
const int SAMPLIG_METHOD_IN_OUT_THRESHOLD = 1, SAMPLIG_METHOD_IN_OUT_RATIO = 2, SAMPLIG_METHOD_CASE = 3;
//we assume branch_length represets dissimilarity
struct Sampler {
	const map<string, Metadata>& metadata;
	int bucket_time_interval;
	int sampling_method;
	int bucket_sample_size_in, bucket_sample_size_out, bucket_size_in_ratio, bucket_size_out_ratio;
	int removed_internal_count, removed_sample_count;

	size_t special_count = 0;
	set<string> special_nodes;

	size_t keep_count = 0, keep_max = 0;
	set<string> keep_nodes;

	bool debug_print_sampling_stat = false;

	map<pair<string, int>, int> case_count;

	Sampler(const map<string, Metadata>& metadata, int bucket_time_interval = BUCKET_TIME_INTERVAL_WEEK, int sampling_method = SAMPLIG_METHOD_IN_OUT_THRESHOLD, int bucket_sample_size_in = 0, int bucket_sample_size_out=0, int bucket_size_in_ratio = 1, int bucket_size_out_ratio = 1) : metadata(metadata), bucket_time_interval(bucket_time_interval), sampling_method(sampling_method), bucket_sample_size_in(bucket_sample_size_in), bucket_sample_size_out(bucket_sample_size_out), bucket_size_in_ratio(bucket_size_in_ratio), bucket_size_out_ratio(bucket_size_out_ratio), removed_internal_count(0), removed_sample_count(0) {
	}

	map<Bucket, vector<INode*>> buckets;

	int bucket_time_index(string date) {
		if (bucket_time_interval == BUCKET_TIME_INTERVAL_WEEK)
			return days_since(date) / 7;
		else if (bucket_time_interval == BUCKET_TIME_INTERVAL_MONTH)
			return month_since(date);
		else
			throw runtime_error("invalid time interval");
	}

	Bucket calc_bucket(const INode& n) {
		auto m = metadata.find(n.label);
		assert(m != metadata.end());
		vector<string> loc_split = split(m->second.location, '/');
		int ti = bucket_time_index(m->second.date);
		return Bucket(ti, trim(loc_split[1]));
		//return Bucket(days_since(m->second.date) / 7, n.location);
	}

	void put_to_bucket(INode& n, int& dfs_num) {
		n.data.dfs_num = dfs_num++;
		if (n.isLeaf() && metadata.find(n.label) != metadata.end()) {
			auto m = metadata.find(n.label);
			regex re("[0-9]{4}-[0-9]{2}-[0-9]{2}");
			smatch rem;
			if (regex_search(m->second.date, rem, re)) { 
				buckets[calc_bucket(n)].push_back(&n);
				n.data.date = days_since(m->second.date);
			}
		}
		for (auto & c : n.children) {
			put_to_bucket(c, dfs_num);
		}
	}

	// main entry
	INode run(INode& n) {
		int dfs_num = 0;
		put_to_bucket(n, dfs_num);
		assert(sampling_method == SAMPLIG_METHOD_IN_OUT_THRESHOLD || sampling_method == SAMPLIG_METHOD_IN_OUT_RATIO || sampling_method == SAMPLIG_METHOD_CASE);
		if (sampling_method == SAMPLIG_METHOD_IN_OUT_THRESHOLD || sampling_method == SAMPLIG_METHOD_CASE) {
			sample(); // bucket_sample_size_in, bucket_sample_size_out);
		} else if (sampling_method == SAMPLIG_METHOD_IN_OUT_RATIO) {
			sample_in_vs_all_out(bucket_sample_size_in, bucket_size_in_ratio, bucket_size_out_ratio);
		}
		cerr << "Sample from buckets" << endl;
		print_sampling_stat();
		if (special_count > 0)
			sample_special(n);
		cerr << "Sample special buckets fixed" << endl;
		print_sampling_stat();
		//we add samples from keep_nodes to have about keep_count samples
		if (keep_count > 0) {
			sample_keep(n);
		}
		cerr << "Sample keeps fixed" << endl;
		dfs_num = 0;
		put_to_bucket(n, dfs_num); // only for printing debug info
		print_sampling_stat();
		return sample(n).first;
	}

	void sample_keep(INode& n) {
		if (keep_nodes.find(n.label) != keep_nodes.end()) {
			int dfs_num = 0;
			buckets.clear();
			put_to_bucket(n, dfs_num);
			size_t sampled_size = 0;
			vector<INode*> all;
			vector<INode*> sampled;
			for (auto & b : buckets) {
				all.insert(all.end(), b.second.begin(), b.second.end());
				for (auto const & inp : b.second) {
					if (inp->data.sampled) {
						sampled_size++;
						sampled.push_back(&*inp);
					}
				}
			}
			size_t sampled_size_old = sampled_size;
			if (sampled_size < keep_count) {
				random_shuffle(all.begin(), all.end());
				for (size_t i=0; i < all.size() && sampled_size < keep_count; i++) {
					if (all[i]->data.sampled == false) {
						all[i]->data.sampled = true;
						sampled_size++;
					}
				}
				cerr << "K+ " << n.label << " " << sampled_size << " (" << sampled_size_old << ")" << endl;
			} else if (sampled_size > keep_max && keep_max > 0) {
				random_shuffle(sampled.begin(), sampled.end());
				for (auto i = sampled.begin() + keep_max; i != sampled.end(); i++) {
					(*i)->data.sampled = false;
					sampled_size--;
				}
				cerr << "K- " << n.label << " " << sampled_size << " (" << sampled_size_old << ")" << endl;
			} else {
				cerr << "k " << n.label << " " << sampled_size << " keep-count:(" << keep_count << ")" << endl;
			}
		}
		for (auto & c : n.children)
			sample_keep(c);
	}

	template<typename T, typename C>
	void set_add_and_keep(set<T,C>& a, set<T,C>& b, size_t k) {
		a.insert(b.begin(), b.end());
		if (a.size() > k) {
			//next could be implemented faster, here since k is small, it should not make any problem
			a.erase(std::next(a.begin(), k), a.end());
		}
	}

	pair<set<INode*, INodeStarCmpEarly>, set<INode*, INodeStarCmpLate>> sample_special(INode& n) {
		pair<set<INode*, INodeStarCmpEarly>, set<INode*, INodeStarCmpLate>> r;
		if (n.isLeaf()) {
			r.first.insert(&n);
			r.second.insert(&n);
			//cerr << "L " << n.label << endl;
		}
		for (auto & c : n.children) {
			pair<set<INode*, INodeStarCmpEarly>, set<INode*, INodeStarCmpLate>> c_r = sample_special(c);
			set_add_and_keep(r.first, c_r.first, special_count);
			set_add_and_keep(r.second, c_r.second, special_count);
		}
		if (special_nodes.find(n.label) != special_nodes.end()) {
			cerr << "Sp " << n.label << " " << n.height << " " << endl;
			for (auto & l : r.first) {
				//cerr << " early " << l->label << " " << l->data.date << " " << l->data.dfs_num << " " << l->location << endl;
				l->data.sampled = true;
			}
			for (auto & l : r.second) {
				//cerr << " late " << l->label << " " << l->data.date << " " << l->data.dfs_num << " " << l->location << endl;
				l->data.sampled = true;
			}
		}
		return r;
	}

	int samplingSizeOfBucket(const Bucket& b, const map<int,int>& time_case_count) const {
		if (sampling_method == SAMPLIG_METHOD_IN_OUT_THRESHOLD) {
			StateInOut locationInOut = b.country == StateInOut::names[0] ? StateInOut::IN : StateInOut::OUT ;
			return locationInOut == StateInOut::IN ? bucket_sample_size_in : bucket_sample_size_out;
		} else if (sampling_method == SAMPLIG_METHOD_IN_OUT_RATIO) {
			throw runtime_error("invalid sampling method here");
		} else if (sampling_method == SAMPLIG_METHOD_CASE) {
			if (case_count.find(make_pair(b.country, b.epiweek)) != case_count.end()) {
				int tc = time_case_count.find(b.epiweek)->second;
				if (tc == 0) 
					return 0;
				int cnt = case_count.find(make_pair(b.country, b.epiweek))->second * bucket_sample_size_in / tc;
				return cnt;
			} else {
				return 0;
			}
		} else {
			throw runtime_error("invalid sampling method here");
		}
	}

	//sample from each bucket, it does not count already sampled ones
	void sample() { // int sample_size_in, int sample_size_out) {
		cerr << "sampling from " << buckets.size() << " buckets" << endl;
		//useful for sampling_method == CASE
		map<int, int> time_case_count;
		for (auto & b : case_count) {
			time_case_count[b.first.second] += b.second;
		}
		
		for (auto & b : buckets) {
			if (b.first.country == "") continue;
			random_shuffle(b.second.begin(), b.second.end());
			//int size = b.first.country == StateInOut::IN ? bucket_sample_size_in : bucket_sample_size_out;
			int size = samplingSizeOfBucket(b.first, time_case_count);
			//int size_0 = size;
			for (int i=0; size > 0 && i < (int) b.second.size(); i++) {
				if (b.second[i]->data.sampled == false) {
					b.second[i]->data.sampled = true;
					size--;
				}
			}
			//cerr << "  b " << b.first.country << " " << b.first.epiweek << " s=" << (size_0 - size) << " -- ";
		}
		//cerr << endl;

		//debug info
		map<int, map<string, int>> bucket_time;
		for (auto & b : buckets) {
			bucket_time[b.first.epiweek][b.first.country] = b.second.size();
		}
		for (auto & bt : bucket_time) {
			cerr << "B " << bt.first << " " << time_case_count[bt.first] << ": ";
			for (auto & cc : bt.second) {
				if (cc.second > 0 && case_count[make_pair(cc.first, bt.first)] > 0) {
					int size = samplingSizeOfBucket(Bucket(bt.first, cc.first), time_case_count);
					cerr << cc.first << " " << cc.second << "/" << case_count[make_pair(cc.first, bt.first)] << ":" << size << " ";
				}
			}
			cerr << endl;
		}

	}

	//sample from each bucket, it does not count already sampled ones
	// sample_size_in: number from in-country in each epiweek, in each step sample_pick_in_ratio from inside and sample_pick_out_ratio from outside is picking until either 
	//    sample_size_in from inside is reached or there is no other sample from inside or outside to be picked
	void sample_in_vs_all_out(int sample_size_in, int sample_pick_in_ratio, int sample_pick_out_ratio) {
		//epiweek_buckets[epiweek].first = vector of pointers to buckets items from in-state, and .second from out-state
		map<int, pair<vector<map<Bucket, vector<INode*>>::iterator>,vector<map<Bucket, vector<INode*>>::iterator>>> epiweek_buckets;

		for (auto b = buckets.begin(); b!=buckets.end(); b++) {
			random_shuffle(b->second.begin(), b->second.end());
			StateInOut locationInOut = b->first.country == StateInOut::names[0] ? StateInOut::IN : StateInOut::OUT ;
			if (locationInOut == StateInOut::IN)
				epiweek_buckets[b->first.epiweek].first.push_back(b);
			else
				epiweek_buckets[b->first.epiweek].second.push_back(b);
		}
		for (auto & eb : epiweek_buckets) {
			cerr << "B " << eb.first << " " << endl;
			vector<vector<map<Bucket, vector<INode*>>::iterator>> buckets_in_out{eb.second.first, eb.second.second};
			//buckets_in_out_indices[in] = (j,i) j'th element of vector[i] (0'th element of all vectors, ...)
			vector<set<pair<int,int>>> buckets_in_out_indices;
			for (auto &v : buckets_in_out) {
				buckets_in_out_indices.push_back(set<pair<int,int>>());
				//for each epiweek different order of countries
				random_shuffle(v.begin(), v.end());
				int i=0;
				for (auto &bp: v) {
					for (size_t j=0; j<bp->second.size(); j++)
						buckets_in_out_indices.back().insert(make_pair(j, i));
					i++;
				}
			}
			//random_shuffle(buckets_in.begin(), buckets_in.end());
			//random_shuffle(buckets_out.begin(), buckets_out.end());
			set<pair<int,int>>::iterator in_index = buckets_in_out_indices[0].begin(), out_index = buckets_in_out_indices[1].begin();
			int item = 0, item_out=0;
			for (; item < sample_size_in && in_index != buckets_in_out_indices[0].end() && out_index != buckets_in_out_indices[1].end(); ) {
				for (int i=0; i<sample_pick_in_ratio && in_index != buckets_in_out_indices[0].end(); in_index++) {
					INode* n = buckets_in_out[0][in_index->second]->second[in_index->first];
					//cerr << "  pick_in " << in_index->second << " " << in_index->first << " " << n->label << endl;
					if (n->data.sampled == false) {
						n->data.sampled = true;
						item++;
						i++;
					}
				}
				for (int i=0; i<sample_pick_out_ratio && out_index != buckets_in_out_indices[1].end(); out_index++) {
					INode* n = buckets_in_out[1][out_index->second]->second[out_index->first];
					//cerr << "  pick_out " << out_index->second << " " << out_index->first << " " << n->label << endl;
					if (n->data.sampled == false) {
						n->data.sampled = true;
						item_out++;
						i++;
					}
				}
			}
			cerr << "  size=" << item << " " << item_out << " # " << buckets_in_out_indices[0].size() << " " << buckets_in_out_indices[1].size() << endl;
		}
	}


	//rebuild the tree based on n.data.sampled for all nodes n
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
		return make_pair(r, r.children.size() > 0 || n.data.sampled == true);
	}

	int sample_in = 0, sample_out = 0, all_in=0, all_out=0;
	void write_sample_file(ostream& fo, const INode& n) {
		if (n.isLeaf()) {
			if (n.location == StateInOut::IN)
				sample_in++;
			else
				sample_out++;
			fo << n.label << endl;
		}
		if (n.location == StateInOut::IN)
			all_in++;
		else
			all_out++;
		for (auto const& c: n.children) {
			write_sample_file(fo, c);
		}
	}

	void write_sample_file(string fn, const INode& n) {
		sample_in = 0, sample_out = 0, all_in=0, all_out=0;
		ofstream fo(fn);
		write_sample_file(fo, n);
		cerr << "Samples in=" << sample_in << "/" << all_in << " out=" << sample_out << "/" << all_out << endl;
	}

	void load_case_count(string fname) {
		ifstream fi(fname);
		case_count.clear();
		int location_index = -1, case_index = -1, date_index = -1;
		for (string line; getline(fi, line); ) {
			vector<string> x = split(line, ',');
			if (location_index == -1) {
				auto i = find(x.begin(), x.end(), "location");
				assert(i != x.end());
				location_index = i - x.begin();
				i = find(x.begin(), x.end(), "new_cases");
				assert(i != x.end());
				case_index = i - x.begin();
				i = find(x.begin(), x.end(), "date");
				assert(i != x.end());
				date_index = i - x.begin();
				continue;
			}
			try {
				int c = x[case_index] == "" ? 0 : stoi(x[case_index]);
				case_count[make_pair(x[location_index], bucket_time_index(x[date_index]))] += c;
			} catch (const std::invalid_argument & e) {
				std::cout << e.what() << x[case_index] << " " << line << "\n";
				throw e;
			}
		}
	}


	void print_unsampled(string fn, const vector<string>& parents, INode& n) const {
		set<string> special_parents(parents.begin(), parents.end());
		ofstream fo(fn);
		TreeUnsampledPrinter tup(fo, special_parents);
		TreeDFSGeneral<INode, TreeUnsampledPrinter> dfs(tup);

		dfs.dfs(n);
	}

	void print_sampling_stat() {
		if (!debug_print_sampling_stat)
			return;
		map<pair<string,int>,pair<int,int>>counts;
		for (auto & b : buckets) {
			if (b.first.country == "") continue;
			//int size = b.first.country == StateInOut::IN ? bucket_sample_size_in : bucket_sample_size_out;
			//StateInOut locationInOut = b.first.country == StateInOut::names[0] ? StateInOut::IN : StateInOut::OUT ;
			//int size = locationInOut == StateInOut::IN ? sample_size_in : sample_size_out;
			int count = 0, count_no = 0;
			for (int i=0; i < (int) b.second.size(); i++) {
				if (b.second[i]->data.sampled == true) {
					count++;
				 } else {
					 count_no++;
				 }
			}
			//cerr << "  b " << b.first.country << " " << b.first.epiweek << " s=" << (count) << "/" << count_no << " -- " << ;
			counts[make_pair(b.first.country, b.first.epiweek)] = make_pair(count, count_no);
		}
		//cerr << endl;

		set<string> countries;
		set<int> epiweeks;
		for (auto &i : counts) {
			countries.insert(i.first.first);
			epiweeks.insert(i.first.second);
		}
		cerr << "EPIWEEK ";
		for (auto c : countries) {
			cerr << c << " ";
		}
		cerr << endl;
		for (auto e : epiweeks) {
			cerr << e << " ";
			int s=0, s_no=0;
			for (auto c : countries) {
				pair<int, int> cc = counts.find(make_pair(c, e)) != counts.end() ? counts[make_pair(c, e)] : make_pair(0, 0);
				cerr << cc.first << "/" << cc.second << " ";
				s += cc.first;
				s_no += cc.second;
			}
			cerr << " all:"  << s << "/" << s_no << " G:" << counts[make_pair("Germany", e)].first << "/" << counts[make_pair("Germany", e)].second;
			cerr << endl;

		}
		
	}
};

// Algorithm: 
//   puts samples to buckets, sample evenly from each.
//   for special internal nodes, it keeps some amount of earliest and oldest samples.

int main(int argc, char* argv[]) {

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "Run sankoff")
	    ("seed", po::value<int>(), "random seed")
	    ("in", po::value<string>(), "input tree")
	    ("out", po::value<string>(), "output tree")
	    ("bucket-size", po::value<vector<int>>()->multitoken()->default_value(vector<int>{100, 100}, "100 100"), "sample size from each bucket (in,out)")
	    ("special", po::value<vector<string>>()->multitoken()->default_value(vector<string>{""}, ""), "special attention internal nodes: e.g. 5 inode0 inode25, means 5 earliest and oldest samples of sub-tree of inode0 and inode25")
	    ("keep", po::value<vector<string>>()->multitoken()->default_value(vector<string>{""}, ""), "list of subtrees for which at least some samples to be kept")
	    ("metadata", po::value<string>(), "metadata file")
	    ("samples-out", po::value<string>(), "output file to write sample ids")
	    ("location_label,l", po::value<vector<string>>()->multitoken(), "location labels, e.g. Germany nonGermany")
	    ("print-annotation", po::value<bool>()->default_value(true), "print annotation")
	    ("print-internal-node-label", po::value<bool>()->default_value(true), "print internal node labels")
	    ("ilabel", po::value<bool>()->default_value(false), "override internal node labels")
	    ("unsampled", po::value<vector<string>>()->multitoken(), "output file containing unsampled samples which are annotated as InState annotated with the first parent from the set. file-name [LIST OF PARENTS TO WHICH SAMPLES ARE ANNOTATED]")
	    ("bucket-time-interval", po::value<string>()->default_value("week"), "which bucket time interval to use, week of month")
	    ("unbias-method", po::value<string>()->default_value("threshold"), "unbias method inside each epiweek, either threshold or ratio or case. If unbias-method is ratio, bucket-size should contain 3 values: bucket_size_in bucket_in_ratio bucket_out_ratio.")
	    ("case", po::value<string>(), "file containing number of cases. Useful when unbias-method is case.")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help")) {
		cout << "Usage: " << argv[0] << " [options] ...\n";
		cout << desc;
		return 0;
        }

	try {
		po::notify(vm);
	} catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	if (vm.count("seed")) {
		srand(vm["seed"].as<int>());
	}

	StateInOut::names = {vm["location_label"].as<vector<string>>()[0], vm["location_label"].as<vector<string>>()[1]};

	string tree_file_name = vm["in"].as<string>(),
		output = vm["out"].as<string>();
	INode phylo = load_tree<INode>(tree_file_name) ;
	ifstream metadata_file(vm["metadata"].as<string>());
	map<string, Metadata> metadata = load_map(metadata_file);

	assert(vm["bucket-time-interval"].as<string>() == "week" || vm["bucket-time-interval"].as<string>() == "month");
	if (vm["unbias-method"].as<string>() == "ratio")
		assert(vm["bucket-size"].as<vector<int>>().size() == 3);

	assert(vm["unbias-method"].as<string>() == "threshold" || vm["unbias-method"].as<string>() == "ratio" || vm["unbias-method"].as<string>() == "case");
	int sampling_method = 0;
	if (vm["unbias-method"].as<string>() == "threshold")
		sampling_method = SAMPLIG_METHOD_IN_OUT_THRESHOLD;
	else if (vm["unbias-method"].as<string>() == "ratio")
		sampling_method = SAMPLIG_METHOD_IN_OUT_RATIO;
	else if (vm["unbias-method"].as<string>() == "case")
		sampling_method = SAMPLIG_METHOD_CASE;

	int bucket_size_in = vm["bucket-size"].as<vector<int>>()[0], bucket_size_out = vm["bucket-size"].as<vector<int>>()[1],
	    bucket_size_ratio_in = vm["bucket-size"].as<vector<int>>()[1], bucket_size_ratio_out = vm["bucket-size"].as<vector<int>>().size() >= 2 ? vm["bucket-size"].as<vector<int>>()[2] : -1;
	Sampler sampler(metadata, vm["bucket-time-interval"].as<string>() == "week" ? BUCKET_TIME_INTERVAL_WEEK: BUCKET_TIME_INTERVAL_MONTH, sampling_method, bucket_size_in, bucket_size_out, bucket_size_ratio_in, bucket_size_ratio_out);
	vector<string> special = vm["special"].as<vector<string>>();
	if (special.size() >= 2) {
		sampler.special_count = stoi(special[0]);
		sampler.special_nodes = set<string>(special.begin() + 1, special.end());
	}

	vector<string> keep = vm["keep"].as<vector<string>>();
	if (keep.size() >= 3) {
		sampler.keep_count = stoi(keep[0]);
		sampler.keep_max = stoi(keep[1]);
		sampler.keep_nodes = set<string>(keep.begin()+2, keep.end());
	} else {
		if (keep.size() > 0) {
			cerr << "Invalid number for keep" << endl;
			return 1;
		}
	}

	if (vm.count("case")) {
		sampler.load_case_count(vm["case"].as<string>());
	}

	INode contracted_phylo = sampler.run(phylo);

	/*
	cerr << "buckets:" << endl;
	for (auto const& b : sampler.buckets) {
		cerr << "  " << b.first.epiweek << " " << b.first.country << "->" << b.second.size() << endl;
	}
	*/
	cerr << "removed " << sampler.removed_internal_count << " internal nodes and " << sampler.removed_sample_count << " samples " << endl;
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

	if (vm.count("samples-out") > 0) {
		sampler.write_sample_file(vm["samples-out"].as<string>(), contracted_phylo);
	}

	if (vm.count("unsampled") > 0) {
		vector<string> parents = vm["unsampled"].as<vector<string>>();
		parents.erase(parents.begin());
		sampler.print_unsampled(vm["unsampled"].as<vector<string>>()[0], parents, phylo);
	}

	cerr << "output saved on " << output<< " " << "[" << vm["print-annotation"].as<bool>() << " " << vm["print-internal-node-label"].as<bool>() << "]" << endl;
	return 0;
}
