#ifndef __MUTATION_H__
#define __MUTATION_H__

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

/**
 * Mutation datastructure: from -> loc -> to
 */
struct MutationMutMut {
    int loc; 
    char from, to;

    MutationMutMut(int loc, char from, char to) : loc(loc), from(from), to(to) {
    }

    MutationMutMut() : loc(-1), from(0), to(0) {
    }

    bool operator<(const MutationMutMut& o) const {
        if (loc != o.loc) return loc < o.loc;
        if (from != o.from) return from < o.from;
        return to < o.to;
    }

    bool no_Xdash(bool matters_X, bool matters_dash) const {
        return (!matters_X || (from != 'X' && to != 'X')) &&
            (!matters_dash || (from != '-' && to != '-'));
    }

    void save(std::ofstream& fo) const {
        SaveLoadManager::save_uint(loc, fo);
        SaveLoadManager::save_char(from, fo);
        SaveLoadManager::save_char(to, fo);
    }

    void load(std::ifstream& fo) {
        loc = SaveLoadManager::load_uint(fo);
        from = SaveLoadManager::load_char(fo);
        to = SaveLoadManager::load_char(fo);
    }

    bool operator==(const MutationMutMut& o) const {
        return loc == o.loc && from == o.from && to == o.to;
    }

    std::string to_string() const {
        return from + std::to_string(loc+1) + to;
    }


    friend std::istream& operator>>(std::istream& is, MutationMutMut& m) {
        std::string s;
        is >> s;
        if (!is)
            return is;
        if (s.size() < 3 || !((s[0] >= 'A' && s[0] <= 'Z') || s[0] == '-') || !((s[s.size()-1] >= 'A' && s[s.size()-1] <= 'Z') || s[s.size()-1] == '-')) {
            throw std::runtime_error("Invalid mutation '" + s + "'");
        }
        m.from = s[0];
        m.to = s[s.size()-1];
        std::string t = s.substr(1, s.size()-2);
        m.loc = std::atoi(t.c_str())-1;
        return is;
    }

    friend std::ostream& operator<<(std::ostream& os, const MutationMutMut& m) {
        return os << m.from << m.loc+1 << m.to;
    }

};

struct CharModel {
	int NA_COUNT;

	int na_to_int(char na) const {
		na = std::toupper(na);
		if (!__char_is_valid[(int)na]) {
			std::cerr << "Invalid char '" << na << "'=" << ((int) na) << std::endl;
			throw std::runtime_error("Invalid character '" + std::to_string((int)na) +"'");
		}
		// assert(__char_is_valid[(int)na] == true);
		return __char_to_int[(int)na];
	}

	bool na_is_valid(char na) const {
		na = std::toupper(na);
		return __char_is_valid[(int)na];
	}

	int na_to_int(std::string na) {
		return na_to_int(na[0]);
	}

	char int_to_char(int na) {
		assert(na >= 0 && na < NA_COUNT);
		return __int_to_char[na];
	}

	static const int MAX_NA_COUNT = 30;

	int cost[MAX_NA_COUNT][MAX_NA_COUNT];
	int __char_to_int[255];
	char __int_to_char[MAX_NA_COUNT];
	bool __char_is_valid[255];

	void load_cost(std::string fn) {
		std::ifstream f(fn);
		std::vector<std::string> names;
		for (size_t i=0; i<255; i++)
			__char_is_valid[i] = false;
		for (std::string line, v; std::getline(f, line); ) {
			std::istringstream is(line.c_str());
			if (names.size() == 0) {
				for (std::string s; is >> s; )
					names.push_back(s);
				NA_COUNT = names.size();
				for (size_t i=0; i<names.size(); i++) {
					__int_to_char[i] = names[i][0];
					__char_to_int[std::toupper(names[i][0])] = i;
					__char_is_valid[(int)std::toupper(names[i][0])] = true;
				}
			} else {
				is >> v;
				for (int vv, i=0; is >> vv; i++) {
					cost[na_to_int(v)][na_to_int(names[i])] = vv;
				}
			}
		}
	}


};

#endif
