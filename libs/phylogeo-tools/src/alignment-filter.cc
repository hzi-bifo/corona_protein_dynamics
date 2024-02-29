#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <iostream>
#include <fstream>
#include <string>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/lzma.hpp>
#include <vector>
#include <set>
#include "fasta.h"
#include "tree.h"
namespace io = boost::iostreams;
using namespace std;

int main(int argc, char* argv[]) {
	//cerr << "main" << endl;
	ifstream file(argv[1], ios_base::in | ios_base::binary);
	cerr << "file opened " << argv[1] << endl;
	io::filtering_streambuf<io::input> in;
	//in.push(gzip_decompressor());
	in.push(io::lzma_decompressor());
	in.push(file);
	std::istream incoming(&in);

	istream& file_alignment = endswith(argv[1],".xz") ? incoming : file; 

	vector<string> sample_ids;
	ifstream fi(argv[2]);
	for (string line; getline(fi, line); ) {
		if (line != "") {
			sample_ids.push_back(line);
			//cerr << "id=(" << line << ")" << endl;
		}
	}
	cerr << "sample_ids loaded " << sample_ids.size() << endl;

	set<string> samples(sample_ids.begin(), sample_ids.end()), samples_found;
	FastaLoaderSequenceFilter filter(cout, samples);
	FastaLoader<FastaLoaderSequenceFilter> fastaLoader(filter);
	fastaLoader.load(file_alignment);

	cerr << "saved " << fastaLoader.filter.samples_found.size() << " samples " << "dup: " << fastaLoader.filter.dup_count << endl;
	int samples_notfount_count = 0;
	for (auto const& s:samples) {
		if (fastaLoader.filter.samples_found.find(s) == fastaLoader.filter.samples_found.end()) {
			if (samples_notfount_count < 10)
				cerr << "W " << s << " ";
			samples_notfount_count++;
		}
	}
	if (samples_notfount_count > 0) {
		cerr << "not found " << samples_notfount_count << " samples" << endl;
	}

	return 0;
}
