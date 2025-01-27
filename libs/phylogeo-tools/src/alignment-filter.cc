#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <codecvt>
#include <ranges>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include "fasta.h"
#include <regex>
#include "argagg.hpp"
#include "tarxz-reader.h"


using namespace std;

// template<typename T, typename S>
// ostream& operator<<(ostream& os, const pair<T,S>& p) {
//         return os << "[" << p.first << "," << p.second << "]";
// }

// template<typename T>
// ostream& operator<<(ostream& os, const vector<T>& v) {
// 	os << "[";
// 	for (auto const &i: v)
// 		os << i << " ";
// 	return os << "]";
// }

// template<typename T, size_t L>
// ostream& operator<<(ostream& os, const array<T, L>& v) {
// 	os << "[";
// 	for (auto const &i: v)
// 		os << i << " ";
// 	return os << "]";
// }

// template<typename T, typename S>
// ostream& operator<<(ostream& os, const map<T,S>& v) {
// 	os << "[";
// 	for (auto const &i: v)
// 		os << i.first <<":" << i.second << " ";
// 	return os << "]";
// }


// vector<string> split(const string& l, char splitter = '\t') {
// 	vector<string> x;
// 	std::istringstream iss(l);
// 	for (string token; getline(iss, token, splitter); ) {
// 		x.push_back(token);
// 	}
// 	return x;
// }

// std::string trim(const std::string& str,
//                  const std::string& whitespace = " \t\n\r")
// {
//     const auto strBegin = str.find_first_not_of(whitespace);
//     if (strBegin == std::string::npos)
//         return ""; // no content

//     const auto strEnd = str.find_last_not_of(whitespace);
//     const auto strRange = strEnd - strBegin + 1;

//     string r = str.substr(strBegin, strRange);
//     //cerr << "trim " << r << endl;
//     return r;
// }

// int indexOf(const vector<string> v, const std::initializer_list<string>& keys) {
// 	for (auto const &k : keys) {
// 		if ( std::find(v.begin(), v.end(), k) != v.end())
// 			return find(v.begin(), v.end(), k) - v.begin();
// 	}
// 	cerr << "Warning: not found indexOf "  << v << " " << vector<string>(keys.begin(), keys.end()) << endl;
// 	//assert(1 != 1);
// 	return -1;
// }

#include "metadata.h"

const int SELECTION_BY_ID = 1;
const int SELECTION_BY_NAME = 2;

const int OUTPUT_SEQUENCE_HEADER_SOURCE_SEQUENCE_FILE = 1;
const int OUTPUT_SEQUENCE_HEADER_SOURCE_METADATA_FILE = 2;


struct FastaLoaderSequenceFilterIdIndex {
	int dup_count=0;
	std::set<std::string> samples_found;

	std::ostream& os;
	const std::set<std::string>& samples;

	int id_index;
	int selection_by;
	int output_sequence_header_source;
	const map<string, Metadata2>& metadata_by_id;

	// {0} = epi, {1} = name, {2} = date
	bool progress;
	string format_template;

	int64_t file_size = -1;

	FastaLoaderSequenceFilterIdIndex(std::ostream& os, 
		const std::set<std::string>& samples, 
		int id_index, 
		int selection_by,
		int output_sequence_header_source,
		const map<string, Metadata2>& metadata_by_id,
		bool progress = false, 
		const string& format_template = "{0}") : os(os), samples(samples), id_index(id_index), 
			selection_by(selection_by),
			output_sequence_header_source(output_sequence_header_source),
			metadata_by_id(metadata_by_id),
			progress(progress), format_template(format_template) {
	}

	void clear() {
		dup_count = 0;
		samples_found.clear();
		assert(id_index == -1);
	}


	std::string id_printable(const std::string& raw_id) const {
		if (output_sequence_header_source == OUTPUT_SEQUENCE_HEADER_SOURCE_SEQUENCE_FILE) {
			vector<string> x = split(raw_id, '|');
			string epi = "", name = "", date = "";
			std::regex year_regex("^([0-9]{4})");
			for (const auto& xx : x) {
				if (xx.starts_with("EPI_ISL_")) {
					epi = xx;
				}
				if (std::sregex_iterator(xx.begin(), xx.end(), year_regex) != std::sregex_iterator()) {
					date = xx;
				}
				if (xx.starts_with("hCoV-19/")) {
					vector<string> xxx = split(xx, '/');
					if (xxx.size() >= 3) {
						name = xxx[xxx.size()-3] + '/' + xxx[xxx.size()-2] + '/' + xxx[xxx.size()-1];
					}
				}
			}

			// if (name == "" || epi == "" || date == "") {
			// 	cerr << "E id-format '" << raw_id << "' " << name << " " << epi << " " << date << endl;
			// }

			return std::vformat(format_template, std::make_format_args(epi, name, date));

			// return name + "|" + epi + "|" + date;
		} else if (output_sequence_header_source == OUTPUT_SEQUENCE_HEADER_SOURCE_METADATA_FILE) {
			string id_ = id(raw_id);
			auto metadata_it = metadata_by_id.find(id_);
			if (metadata_it == metadata_by_id.end()) {
				cerr << "ID not found in metadata information: '" << id_ << "'" << std::endl;
				return raw_id;
			}
			string epi = metadata_it->second.id,
				name = metadata_it->second.name,
				date = metadata_it->second.date;
			if (name.starts_with("hCoV-19/")) {
				vector<string> xxx = split(name, '/');
				if (xxx.size() >= 3) {
					name = xxx[xxx.size()-3] + '/' + xxx[xxx.size()-2] + '/' + xxx[xxx.size()-1];
				}
			}
			return std::vformat(format_template, std::make_format_args(epi, name, date));
		} else {
			std::cerr << "Invalid output_sequence_header_source = " << output_sequence_header_source << std::endl;
			exit(1);
		}
	}

	std::string id(const std::string& raw_id) const {
		// const char* search_template = "EPI_ISL_";
		// const int template_length = 8;
		// string ret;
		// int match_size = 0;
		// for (const char *l = raw_id.c_str(), *c = raw_id.c_str(); ; c++) {
		// 	if (*c == 0 || *c == '|') {
		// 		if (match_size == template_length) {
		// 			string ret = string(l, c);
		// 			return ret;
		// 		}
		// 		l=c+1;
		// 		match_size = 0;
		// 		if (*c == 0) {
		// 			break;
		// 		} else {
		// 			continue;
		// 		}
		// 	}
		// 	if (match_size == (c-l) && (c-l) < template_length && *c == search_template[c-l]) {
		// 		match_size++;
		// 	}
		// }
		// return raw_id;
		vector<string> x = split(raw_id, '|');
		// if (raw_id.find("EPI_ISL_1001837") != string::npos) {
		// 	cerr << "D raw_id Found! " << x << " " << id_index << endl;
		// }
		if (id_index == -1) {
			for (const auto& xx : x) {
				if (selection_by == SELECTION_BY_ID && xx.starts_with("EPI_ISL_")) {
					// if (xx == "EPI_ISL_1001837") {
					// 	cerr << "D Found! " << x << " " << xx << endl;
					// }
					return xx;
				}
				if (selection_by == SELECTION_BY_NAME && xx.starts_with("hCoV-19/")) {
					return xx;
				}
			}
			if (!raw_id.starts_with("EPI_ISL_")) {
				cerr << "W invalid id: " << raw_id << endl; 
			}
			return raw_id;
		}
		if ((int)x.size() <= id_index) {
			cerr << "W " << raw_id << " short length " << id_index << endl;
			return raw_id;
		}
		return x[id_index];

		//id = x[id_index];
		//
		//if (raw_id.find("|") != std::string::npos) 
		//	raw_id = raw_id.substr(0, raw_id.find("|"));
		//return raw_id;
	}

	int count =0, accept_count=0;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	string elapsed_time_format(const std::chrono::steady_clock::duration& elapsed) const {
		auto elapsed_time = std::chrono::hh_mm_ss(elapsed);
		string ret = "";
		bool h = false, m = false;
		if (elapsed_time.hours().count() > 0) {
			ret += to_string(elapsed_time.hours().count()) + "h";
			h = true;
		}
		if (h || elapsed_time.minutes().count() > 0) {
			ret += to_string(elapsed_time.minutes().count()) + "m";
			m = true;
		}
		if (!h && (m || elapsed_time.seconds().count() > 0)) {
			ret += to_string(elapsed_time.seconds().count()) + "s";
			// s = true;
		}
		return ret;
	}

	void show_progress(const string& id, int64_t loc) const {
		int percentage = file_size > 0 ? (loc * 100 + file_size/2) / file_size : -1;
		const std::string blocks[] = {" ", "▏", "▎", "▍", "▌", "▋", "▊", "▉", "█"};
		stringstream os;
		os << accept_count << "/" << count << 
			"'" << id << "':";

		const size_t first_part_size = 20, second_part_size=40;
		
		cerr << os.str().substr(0, first_part_size);
		for (size_t i=os.str().size() ; i<first_part_size; i++)
			cerr << " ";
		
		std::string bar = "";
		if (file_size > 0) {
			size_t filled_length = (loc*second_part_size + file_size/2/8)/file_size;
			for (size_t i=0; i<filled_length; i++)
				bar += blocks[8];
			if (filled_length < second_part_size) {
				bar += blocks[((loc*second_part_size * 8 + file_size/2)/file_size) % 8] + 
					std::string(second_part_size - filled_length - 1, ' ');
			}
		} else {
			bar = std::string(second_part_size, ' ');
		}
		cerr << "[" << bar <<  "]";
		cerr << " ";

		cerr << (percentage >= 0 ? to_string(percentage) + "%" : "");

		// cerr << "(" << std::chrono::duration_cast<std::chrono::seconds> (std::chrono::steady_clock::now() - begin).count() << "s)";

		cerr << "(";
		cerr << elapsed_time_format(std::chrono::steady_clock::now() - begin);
		
		if (percentage >= 1) {
			cerr << " ETA:";
			cerr << elapsed_time_format((100-percentage) * (std::chrono::steady_clock::now() - begin) / percentage);
		}
		
		cerr << ")";
		// cerr << loc << "/" << file_size;
		cerr << "\r";
	}

	void process(const std::string& id, const std::string& seq, const std::string& raw_id, std::istream& is) {
		process(id, seq, raw_id);
		if (progress && count % 100000 == 0) {
			show_progress(id, is.tellg());
		}
	}

	void process(const std::string& id, const std::string& seq, const std::string& raw_id, TarXZReader& is) {
		process(id, seq, raw_id);
		if (progress && count % 100000 == 0) {
			show_progress(id, is.current_file_offset);
		}
	}


	void process(const std::string& id, const std::string& seq, const std::string& raw_id) {
		if (count == 0) {
			begin = std::chrono::steady_clock::now();
		}
		count++;
		// cerr << "}'" << id << "'" << seq.size() << endl;
		if (samples.contains(id)) {
		// if (false) {
			if (samples_found.contains(id)) {
				//cerr << "dup " << id << endl;
				dup_count++;
			} else {
			//cerr << "S " << id << endl;
				accept_count++;
				os << ">" << id_printable(raw_id) << std::endl;
				os << seq << std::endl;
				samples_found.insert(id);
			}
		}
	}
};

// resolve<boost::iostreams::detail::chain_client<boost::iostreams::chain<boost::iostreams::input, char, std::char_traits<char>, std::allocator<char> > >::mode, boost::iostreams::detail::chain_client<boost::iostreams::chain<boost::iostreams::input, char, std::char_traits<char>, std::allocator<char> > >::char_type>(std::basic_istream<wchar_t>&) {

// }


// struct Metadata2 {
// 	string id, name, date, location, location_add, pangolin, aasub;
// 	Metadata2(string _id = "", string _name = "", string _date = "", string _location = "", string _location_add = "", string pangolin ="", string aasub="") :
// 		id(_id), name(_name), date(_date), location(_location), location_add(_location_add), pangolin(pangolin), aasub(aasub) {}

// 	string country() const {
// 		vector<string> lparts = split(location, '/');
// 		if (lparts.size() >= 2)
// 			return trim(lparts[1]);
// 		return "";
// 	}
// };

// struct MetadataReader {
//     string fn;
//     ifstream file;

//     MetadataReader(string fn) : fn(fn), file(fn) {
//     }
    
//     int name_index = -1, id_index = -1, date_index = -1, location_index = -1, collection_date_index = -1, location_addition_index = -1, pangolin_index=-1, aasub_index = -1;
//     string line = "";
//     Metadata2 metadata;
//     vector<string> metadata_line_split, metadata_header_line_split;
//     bool next() {
//         for (; line != "" || getline(file, line); ) {
//             while (line.back() == '\r' || line.back() == '\n') 
//                 line.pop_back();
//             //cerr << "line read " << line << endl;
//             if (line.size() == 0) {
//                 line = "";
//                 continue;
//             }
//             char separator = 0;
//             if (separator == 0) {
//                 if (line.find('\t') != string::npos) 
//                     separator = '\t';
//                 else if (line.find(',') != string::npos) 
//                     separator = ',';
//             }
//             //vector<string> x = split(trim(line), separator);
//             vector<string> x = split(line, separator);
//             //cerr << "line split " << x << endl;
//             if (id_index == -1) {
//                 //cerr << "reading header " << x << endl;
//                 name_index = indexOf(x, {"Virus name", "Virus.name", "virus_name", "sequence_name", "strain"});
//                 id_index = indexOf(x, {"Accession ID", "Accession.ID", "accession_id", "accession", "sequence_name"});
//                 date_index = indexOf(x, {"Collection date", "Collection.date", "collection_date", "date"});
//                 location_index = indexOf(x, {"Location", "country"});
//                 location_addition_index = indexOf(x, {"Additional.location.information", "Additional location information", "city"});
//                 //collection_date_index = indexOf(x, {"Submission date"});
//                 collection_date_index = indexOf(x, {"Collection date", "Collection.date", "collection_date", "date"});
//                 pangolin_index = indexOf(x, {"Pango lineage", "Pango.lineage", "lineage"});
//                 // aasub_index = indexOf(x, {"AA Substitutions"});
//                 line = "";
//                 //cerr << "header loaded " << x << endl;
//                 metadata_header_line_split = x;
//                 continue;
//             } else {
//                 if (id_index >= (int)x.size() ) {
//                     cerr << "x short " << x.size() << " id=" << id_index << endl;
//                 }
//                 if (name_index >= (int)x.size() ) {
//                     cerr << "x short " << x.size() << " name=" << name_index << endl;
//                 }
//                 if (date_index >= (int)x.size() ) {
//                     cerr << "x short " << x.size() << " date_index=" << date_index << endl;
//                 }
//                 if (collection_date_index >= (int)x.size() ) {
//                     cerr << "x short " << x.size() << " collection_date_index=" << collection_date_index << endl;
//                 }
//                 if (location_index >= (int)x.size() ) {
//                     cerr << "x short " << x.size() << " location_index=" << location_index << endl;
//                 }
//                 if (location_addition_index >= (int)x.size() ) {
//                     cerr << "x short " << x.size() << " location_addition_index=" << location_addition_index << " " << x << endl;
//                 }
//                 if (pangolin_index >= (int)x.size() ) {
//                     cerr << "x short " << x.size() << " pangolin_index=" << pangolin_index << endl;
//                 }
//                 // if (aasub_index >= (int)x.size() ) {
//                 //     cerr << "x short " << x.size() << " aasub_index=" << aasub_index << " " << x << endl;
//                 //     exit(0);
//                 // }

// 		/*
// 		for (const auto &y:x) {
// 			cerr << "|" << y;
// 		}
// 		cerr << "|" << endl;
// 		*/


//                 metadata_line_split = x;
//                 metadata = Metadata2(x[id_index], x[name_index], x[date_index].size() != 10 ? x[collection_date_index] : x[date_index], x[location_index], location_addition_index != -1 ? x[location_addition_index] : "", pangolin_index != -1 ? x[pangolin_index] : "", "");
//                 line = "";

//                 // if (line.find("EPI_ISL_10046212") != string::npos) {
//                 //     cerr << "MR EPI_ISL_10046212 found in line " << line << endl;
//                 // }

//                 // if (x[id_index] == "EPI_ISL_10046212") {
//                 //     cerr << "MR EPI_ISL_10046212:" << x << " x[id_index]=" << x[id_index] << " metadata.id=" << metadata.id << "|" << endl;
//                 // }

//                 return true;
//             }
//         }
//         return false;
//     }
// };



int main(int argc, char* argv[]) {
	//cerr << "main" << endl;
	// if (argc != 3) {
	// 	cerr << "We need two inputs: sequence file and the metadata file" << endl;
	// 	return 1;
	// }
	// TCLAP::CmdLine cmd("Filter fasta sequence or ziped fasta file.", ' ', "0.9");

	// TCLAP::ValueArg<string> fasta_file("a", "aln", "The fasta file", true, "", "string");
	// cmd.add(fasta_file);

	// TCLAP::ValueArg<string> metadata_file("m", "metadata", "The metadata file containing the ids to be printed", false, "", "string");
	// cmd.add(metadata_file);

	// TCLAP::ValueArg<string> ids_file("i", "ids", "A file containing the ids to be printed. Either this or the metadata option should be provided", false, "", "string");
	// cmd.add(ids_file);

	// cmd.parse( argc, argv );

	argagg::parser argparser {{
		{ "help", {"-h", "--help"},
			"shows this help message", 0},
		{ "metadata", {"-m", "--metadata"},
			"The metadata file containing the ids to be printed.", 1},
		{ "ids", {"-i", "--ids"},
			"A file containing the ids to be printed. Either this or the metadata option should be provided.", 1},
		{ "by", {"-b", "--by"},
			"Select by. Options: 'id' which is EPI_ISL ID (default) or 'name' which is Virus name.", 1},
		{ "aln", {"-a", "--aln"},
			"The fasta file. If fasta file is compressed (specified with -z), the path inside the compressed file should be provided after the file path separated by a colon. For example sequence-file.tar.xz:folder/seq01.fasta.", 1},
		{ "zip", {"-z", "--zip"},
			"The fasta file is compressed.", 0},
		{ "progress", {"-p", "--progress"},
			"Show the progress bar.", 0},
		{ "format", {"-f", "--format"},
			"Format of the sequence descriptions of the output fasta file. {0}=accession id, {1}=epi name, {2}=date", 1},
		{ "desc", {"-d", "--desc"},
			"Source of information for sequence description information, i.e. the part appears after '>'. Options: 'seq' from sequence file (default) or 'metadata' from metadata file.", 1},
		{ "out", {"-o", "--out"},
			"Output file.", 1},
	}};

	argagg::parser_results args;

	try {
		args = argparser.parse(argc, argv);
	} catch (const std::exception& e) {
		std::cerr << "E: " << e.what() << '\n';
		return EXIT_FAILURE;
	}

	if (args["help"]) {
		std::cerr << argparser;
		return EXIT_SUCCESS;
	}

	string metadata_file = "", 
		fasta_file = "",
		ids_file = "", 
        out_file = "";

	if (args["metadata"]) {
		metadata_file = args["metadata"].as<string>();
	}

	if (args["aln"]) {
		fasta_file = args["aln"].as<string>();
	}

	if (args["ids"]) {
		ids_file = args["ids"].as<string>();
	}

	string id_format = "{0}";
	if (args["format"]) {
		id_format = args["format"].as<string>();
	}

	bool progress = false;
	if (args["progress"]) {
		progress = true;
	}

	int selection_by = SELECTION_BY_ID;
	if (args["by"]) {
		string selection_by_string = args["by"].as<string>();
		if (selection_by_string == "id") {
			selection_by = SELECTION_BY_ID;
		} else if (selection_by_string == "name") {
			selection_by = SELECTION_BY_NAME;
		} else {
			std::cerr << "Invalid selection by argument: '" << selection_by_string << "'" << std::endl;
			exit(1);
		}
	}

	int output_sequence_header_source = OUTPUT_SEQUENCE_HEADER_SOURCE_SEQUENCE_FILE;
	if (args["desc"]) {
		string oheader_string = args["desc"].as<string>();
		if (oheader_string == "seq") {
			output_sequence_header_source = OUTPUT_SEQUENCE_HEADER_SOURCE_SEQUENCE_FILE;
		} else if (oheader_string == "metadata") {
			output_sequence_header_source = OUTPUT_SEQUENCE_HEADER_SOURCE_METADATA_FILE;
		} else {
			std::cerr << "Invalid output sequence header argument: '" << oheader_string << "'" << std::endl;
			exit(1);
		}
	}

	if (args["out"]) {
		out_file = args["out"].as<string>();
	}

	assert((metadata_file != "") + (ids_file != "") == 1);

	// wifstream file(argv[1], ios_base::in | ios_base::binary);
	// int id_index = (argc > 3 ? std::atoi(argv[3])-1 : 0);
	int id_index = -1;
	// cerr << "sequence file: '" << fasta_file << "' metadata file: '" << metadata_file << "' ids file: '" <<  ids_file << "'" << endl;
	// io::filtering_streambuf<io::input> in;
	// //in.push(gzip_decompressor());
	// in.push(io::lzma_decompressor());
	// in.push(file);
	// std::istream incoming(&in);
	// std::wistream my_wistream_reinterpreter(incoming);
	// reinterpret_as_wistream< std::istream > my_wistream_reinterpreter(incoming);

	// wistream& file_alignment = endswith(argv[1],".xz") ? my_wistream_reinterpreter : file; 
	// if (string(argv[1]).ends_with(".xz")) {
	// 	cerr << "We don't support xz file " << argv[1] << endl;
	// }
	// assert(!string(argv[1]).ends_with(".xz"));
	// wistream& file_alignment = file; 

	vector<string> sample_ids;
	map<string, Metadata2> metadata_by_id;

	if (metadata_file != "") {
        int w_count = 0;
		cerr << "Loading IDs from metadata file: " << metadata_file << endl;
		for (MetadataReader mr(metadata_file); mr.next(); ) {

			if (selection_by == SELECTION_BY_ID) {
				sample_ids.push_back(mr.metadata.id);
				metadata_by_id[mr.metadata.id] = mr.metadata;
			} else if (selection_by == SELECTION_BY_NAME) {
				sample_ids.push_back(mr.metadata.name);
				metadata_by_id[mr.metadata.name] = mr.metadata;
			}
			if (!mr.metadata.id.starts_with("EPI_ISL_")) {
                if (w_count <= 10) {
    				cerr << "W invalid id in metadata: " << mr.metadata.id << endl;
                    if (w_count == 10) {
                        cerr << "W invalid id in metadata: limit exceeds " << endl;
                    }
                }
                w_count++;
			}
		}
        if (w_count > 10) {
            cerr << "W invalid id in metadata count " << w_count << endl;
        }
	}

	if (ids_file != "") {
        int w_count = 0;
		cerr << "Loading IDs from id file: " << ids_file << endl;
		ifstream fi(ids_file);
		for (string line; getline(fi, line); ) {
			if (line != "") {
				if (!line.starts_with("EPI_ISL_")) {
                    if (w_count <= 10) {
    					cerr << "W invalid id in ids: " << line << endl;
                        if (w_count == 10) {
    					    cerr << "W invalid id in ids limit exceeds " << endl;
                        }
                    }
                    w_count++;
				}
				sample_ids.push_back(line);
				//cerr << "id=(" << line << ")" << endl;
			}
		}
        if (w_count > 10) {
            cerr << "W invalid id in ids count " << w_count << endl;
        }
	}

	set<string> samples(sample_ids.begin(), sample_ids.end()), samples_found;

	cerr << "sample_ids loaded " << sample_ids.size() << " id-index=" << id_index << " [";
	for (int i=0; i<(int)sample_ids.size() && i <= 5; i++) {
		cerr << "'" << sample_ids[i] << "' ";
	}
	cerr << " ... ]" << endl;

    ofstream fo;
    if (out_file != "") {
        fo.open(out_file);
    }
    ostream& of = (out_file != "") ? fo : cout;

	FastaLoaderSequenceFilterIdIndex filter(of, samples, id_index, selection_by, output_sequence_header_source, metadata_by_id, progress, id_format);
	FastaLoader<FastaLoaderSequenceFilterIdIndex> fastaLoader(filter);
	if (args["zip"]) {
		vector<string> x = split_string(fasta_file, ':');
		assert(x.size() == 2);
		cerr << "Loading sequences from compressed fasta file: '" << x[0] << "' inside path: '" << x[1] << "'" << endl;
		TarXZReader file_alignment;
		file_alignment.open(x[0]);
		file_alignment.seekFile(x[1]);
		fastaLoader.filter.file_size = file_alignment.current_file_size;
		// cerr << "filter.file_size=" << filter.file_size << endl;
		fastaLoader.load(file_alignment);
		// cerr << "filter.file_size=" << filter.file_size << endl;
		file_alignment.close();
	} else {
		cerr << "Loading sequences from raw fasta file: '" << fasta_file << "'" << endl;
		auto size = std::filesystem::file_size(fasta_file);
		fastaLoader.filter.file_size = size;
		ifstream file_alignment(fasta_file);
		fastaLoader.load(file_alignment);
	}
	cerr << "Loading done." << endl;

	cerr << "Saved " << fastaLoader.filter.samples_found.size() << " samples " << endl;
	cerr << "  duplicate samples in fasta file: " << fastaLoader.filter.dup_count << endl;
	cerr << "  not found samples in fasta file: ";
	int samples_notfount_count = 0;
	for (auto const& s:samples) {
		if (fastaLoader.filter.samples_found.find(s) == fastaLoader.filter.samples_found.end()) {
			if (samples_notfount_count < 10)
				cerr << s << " ";
			samples_notfount_count++;
		}
	}
	if (samples_notfount_count > 0) {
		cerr << " (" << samples_notfount_count << " samples)" << endl;
	}
	cerr << endl;
	
	return 0;
}
