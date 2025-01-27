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

SRC=src
BIN=build
#ALL: $(join $(addsuffix $(BIN), $(dir $(SOURCE)))
ALL: $(BIN) $(addprefix $(BIN)/, $(ALL))
INCLUDE=-I$(CONDA_PREFIX)/include -L$(CONDA_PREFIX)/lib



$(BIN)/phylogeo_sankoff_general: $(SRC)/phylogeo_sankoff_general.cc $(SRC)/tree.h $(SRC)/state.h
	g++ $(SRC)/phylogeo_sankoff_general.cc -o $@ -O2 -std=c++20 -Wall -lboost_program_options $(INCLUDE) -llzma -larchive

$(BIN)/contract_short_branch: $(SRC)/contract_short_branch.cc $(SRC)/tree.h $(SRC)/state.h
	g++ $(SRC)/contract_short_branch.cc -o $@ -O2 -std=c++20 -Wall -lboost_program_options $(INCLUDE)

$(BIN)/phylogeo_sankoff_general_dna: $(SRC)/phylogeo_sankoff_general_dna.cc $(SRC)/tree.h $(SRC)/state.h
	g++ $(SRC)/phylogeo_sankoff_general_dna.cc -o $@ -O2 -std=c++20 -Wall -lboost_program_options $(INCLUDE)
	#g++ $(SRC)/phylogeo_sankoff_general_dna.cc -o $@ -O2 -std=c++20 -Wall -lboost_program_options -lboost_iostreams -I$(CONDA_PREFIX)/include -Wl,-rpath-link=$(CONDA_PREFIX)/lib -L$(CONDA_PREFIX)/lib

$(BIN)/mutation-samples: $(SRC)/mutation-samples.cc
	g++ $(SRC)/mutation-samples.cc -o $@ -O2 -std=c++20 -Wall -lboost_program_options $(INCLUDE)

$(BIN)/tree-stat: $(SRC)/tree-stat.cc $(SRC)/tree.h
	g++ -o $@ $(SRC)/tree-stat.cc -O0 -Wall -std=c++20 -lboost_program_options $(INCLUDE)

$(BIN)/alignment-filter: $(SRC)/alignment-filter.cc $(SRC)/fasta.h $(SRC)/metadata.h $(SRC)/tarxz-reader.h
	g++ -o $@ $(SRC)/alignment-filter.cc -O6 -Wall -std=c++20 -llzma -larchive $(INCLUDE)

$(BIN)/partition-by-name: $(SRC)/partition-by-name.cc $(SRC)/tree.h
	g++ -o $@ $(SRC)/partition-by-name.cc -O0 -Wall -std=c++20 -lboost_iostreams -lboost_program_options  $(INCLUDE)

$(BIN)/sampling-by-name: $(SRC)/sampling-by-name.cc $(SRC)/tree.h
	g++ -o $@ $(SRC)/sampling-by-name.cc -O2 -Wall -std=c++20 -lboost_iostreams -lboost_program_options $(INCLUDE)

$(BIN)/sample-and-partition-by-name: $(SRC)/sample-and-partition-by-name.cc $(SRC)/tree.h
	g++ -o $@ $(SRC)/sample-and-partition-by-name.cc -O2 -Wall -std=c++20 -lboost_iostreams -lboost_program_options  $(INCLUDE)

$(BIN)/sample-evenly: $(SRC)/sample-evenly.cc $(SRC)/tree.h $(SRC)/rangetree.h
	g++ -o $@ $(SRC)/sample-evenly.cc -O2 -Wall -std=c++20 -lboost_program_options  $(INCLUDE)

$(BIN)/tree-build-as-pangolin: $(SRC)/tree-build-as-pangolin.cc $(SRC)/tree.h
	g++ -o $@ $(SRC)/tree-build-as-pangolin.cc -O2 -Wall -std=c++20 -lboost_program_options $(INCLUDE)

$(BIN)/rename-alignment-ids: $(SRC)/rename-alignment-ids.cc $(SRC)/tree.h
	g++ -o $@ $(SRC)/rename-alignment-ids.cc -O2 -Wall -std=c++20 $(INCLUDE) -llzma -larchive

$(BIN)/compare-partitions: $(SRC)/compare-partitions.cc $(SRC)/tree.h
	g++ -o $@ $(SRC)/compare-partitions.cc -std=c++20 -O2 -lboost_program_options   $(INCLUDE)

$(BIN)/split_tree_general: $(SRC)/split_tree_general.cc $(SRC)/tree.h $(SRC)/state.h
	g++ -o $@ $(SRC)/split_tree_general.cc -std=c++20 -O2 -lboost_program_options  $(INCLUDE)

$(BIN)/test: $(SRC)/test.cc
	g++ -o $@ $(SRC)/test.cc -std=c++20

$(BIN):
	mkdir bin/

clean:
	rm -rf $(ALL)
