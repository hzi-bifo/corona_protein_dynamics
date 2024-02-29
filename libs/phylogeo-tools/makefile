ALL= phylogeo_sankoff_general \
	contract_short_branch \
	phylogeo_sankoff_general_dna \
	mutation-samples \
	alignment-filter \
	partition-by-name \
	sampling-by-name \
	sample-and-partition-by-name \
	sample-evenly \
	tree-build-as-pangolin \
	rename-alignment-ids \
	compare-partitions \
	split_tree_general

CONDA_PREFIX=~/miniconda/envs/corona_sd_plots/
SRC=src/
BIN=build/
#ALL: $(join $(addsuffix $(BIN), $(dir $(SOURCE)))
ALL: $(BIN) $(addprefix $(BIN)/, $(ALL))



$(BIN)/phylogeo_sankoff_general: $(SRC)/phylogeo_sankoff_general.cc $(SRC)/tree.h $(SRC)/state.h
	g++ $(SRC)/phylogeo_sankoff_general.cc -o $@ -O2 -std=c++11 -Wall -lboost_program_options

$(BIN)/contract_short_branch: $(SRC)/contract_short_branch.cc $(SRC)/tree.h $(SRC)/state.h
	g++ $(SRC)/contract_short_branch.cc -o $@ -O2 -std=c++11 -Wall -lboost_program_options

$(BIN)/phylogeo_sankoff_general_dna: $(SRC)/phylogeo_sankoff_general_dna.cc $(SRC)/tree.h $(SRC)/state.h
	g++ $(SRC)/phylogeo_sankoff_general_dna.cc -o $@ -O2 -std=c++11 -Wall -lboost_program_options 
	#g++ $(SRC)/phylogeo_sankoff_general_dna.cc -o $@ -O2 -std=c++11 -Wall -lboost_program_options -lboost_iostreams -I$(CONDA_PREFIX)/include -Wl,-rpath-link=$(CONDA_PREFIX)/lib -L$(CONDA_PREFIX)/lib

$(BIN)/mutation-samples: $(SRC)/mutation-samples.cc
	g++ $(SRC)/mutation-samples.cc -o $@ -O2 -std=c++11 -Wall -lboost_program_options

$(BIN)/tree-stat: $(SRC)/tree-stat.cc $(SRC)/tree.h
	g++ -o $@ $(SRC)/tree-stat.cc -O0 -Wall -std=c++11 -lboost_program_options

$(BIN)/alignment-filter: $(SRC)/alignment-filter.cc $(SRC)/fasta.h
	g++ -o $@ $(SRC)/alignment-filter.cc -O3 -Wall -std=c++11 -lboost_iostreams  -I$(CONDA_PREFIX)/include -L$(CONDA_PREFIX)/lib -Wl,-rpath-link=$(CONDA_PREFIX)/lib

$(BIN)/partition-by-name: $(SRC)/partition-by-name.cc $(SRC)/tree.h
	g++ -o $@ $(SRC)/partition-by-name.cc -O0 -Wall -std=c++11 -lboost_iostreams  -I$(CONDA_PREFIX)/include -lboost_program_options

$(BIN)/sampling-by-name: $(SRC)/sampling-by-name.cc $(SRC)/tree.h
	g++ -o $@ $(SRC)/sampling-by-name.cc -O2 -Wall -std=c++11 -lboost_iostreams  -I$(CONDA_PREFIX)/include -lboost_program_options

$(BIN)/sample-and-partition-by-name: $(SRC)/sample-and-partition-by-name.cc $(SRC)/tree.h
	g++ -o $@ $(SRC)/sample-and-partition-by-name.cc -O2 -Wall -std=c++11 -lboost_iostreams  -I$(CONDA_PREFIX)/include -lboost_program_options

$(BIN)/sample-evenly: $(SRC)/sample-evenly.cc $(SRC)/tree.h $(SRC)/rangetree.h
	g++ -o $@ $(SRC)/sample-evenly.cc -O2 -Wall -std=c++11 -lboost_program_options

$(BIN)/tree-build-as-pangolin: $(SRC)/tree-build-as-pangolin.cc $(SRC)/tree.h
	g++ -o $@ $(SRC)/tree-build-as-pangolin.cc -O2 -Wall -std=c++11 -lboost_program_options

$(BIN)/rename-alignment-ids: $(SRC)/rename-alignment-ids.cc $(SRC)/tree.h
	g++ -o $@ $(SRC)/rename-alignment-ids.cc -O2 -Wall -std=c++11

$(BIN)/compare-partitions: $(SRC)/compare-partitions.cc $(SRC)/tree.h
	g++ -o $@ $(SRC)/compare-partitions.cc -std=c++11 -O2 -lboost_program_options 

$(BIN)/split_tree_general: $(SRC)/split_tree_general.cc $(SRC)/tree.h $(SRC)/state.h
	g++ -o $@ $(SRC)/split_tree_general.cc -std=c++11 -O2 -lboost_program_options 

$(BIN):
	mkdir build/

clean:
	rm $(ALL)
