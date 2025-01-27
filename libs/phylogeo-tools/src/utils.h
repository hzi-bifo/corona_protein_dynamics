#ifndef __UTILS_H__
#define __UTILS_H__
#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <chrono>

template<typename T, typename S>
std::ostream& operator<<(std::ostream& os, const std::pair<T,S>& p) {
        return os << "[" << p.first << "," << p.second << "]";
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
	os << "[";
	for (auto const &i: v)
		os << i << " ";
	return os << "]";
}

template<typename T, size_t L>
std::ostream& operator<<(std::ostream& os, const std::array<T, L>& v) {
	os << "[";
	for (auto const &i: v)
		os << i << " ";
	return os << "]";
}

template<typename T, typename S>
std::ostream& operator<<(std::ostream& os, const std::map<T,S>& v) {
	os << "[";
	for (auto const &i: v)
		os << i.first <<":" << i.second << " ";
	return os << "]";
}

bool endswith(const std::string& s, const std::string& e) {
	if (s.size() < e.size()) return false;
	for (size_t i = s.size() - e.size(), j=0; j < e.size(); i++, j++) {
		if (s[i] != e[j]) return false;
	}
	return true;
}

bool iequals(const std::string& a, const std::string& b) {
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

std::vector<std::string> split_string(const std::string& l, char splitter = '\t') {
	// cerr << "split_string: " << l << " ... " << endl;
	std::vector<std::string> x;
	for (size_t st = 0; st != std::string::npos; ) {
		size_t i = l.find(splitter, st);
		if (i == std::string::npos) {
			x.push_back(l.substr(st));
			st = i;
		} else {
			x.push_back(l.substr(st, i - st));
			st = i + 1;
		}
	}
	// cerr << "split_string: " << l << " " << x << endl;
	return x;
}


std::string replace_all(std::string text, const std::string& from, const std::string& to) {
    for (auto at = text.find(from, 0); at != std::string::npos;
        at = text.find(from, at + to.length())) {
        text.replace(at, from.length(), to);
    }
    return text;
}

template<typename T>
std::string string_join(const T& x, char splitter = '\t') {
	std::string r;
	bool is_first = true;
	for (const std::string& y : x) {
		if (!is_first)
			r += splitter;
		r += y;
		is_first = false;
	}
	return r;
}

std::string trim(const std::string& str,
                 const std::string& whitespace = " \t\n\r")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    std::string r = str.substr(strBegin, strRange);
    //cerr << "trim " << r << endl;
    return r;
}

std::vector<std::string> split(const std::string& l, char splitter = '\t') {
	std::vector<std::string> x;
	std::istringstream iss(l);
	for (std::string token; getline(iss, token, splitter); ) {
		x.push_back(token);
	}
	return x;
}

int indexOf(const std::vector<std::string>& v, const std::initializer_list<std::string>& keys) {
	for (auto const &k : keys) {
		if ( std::find(v.begin(), v.end(), k) != v.end())
			return std::find(v.begin(), v.end(), k) - v.begin();
	}
	std::cerr << "Warning: not found indexOf in '"  << v << "' k='" << std::vector<std::string>(keys.begin(), keys.end()) << "'" << std::endl;
	//assert(1 != 1);
	return -1;
}


struct SaveLoadManager {
    static int load_uint(std::istream& is) {
        uint64_t u;
        is.read((char*)&u, sizeof(u));
        return u;
    }
    static void save_uint(int i, std::ostream& is) {
        uint64_t u = i;
        is.write(reinterpret_cast<const char *>(&u), sizeof(u));
    }
    static char load_char(std::istream& is) {
        char u;
        is.read((char*)&u, sizeof(u));
        return u;
    }
    static void save_char(char c, std::ostream& is) {
        char u = c;
        is.write(reinterpret_cast<const char *>(&u), sizeof(u));
    }
    static char load_bool(std::istream& is) {
        bool u;
        is.read((char*)&u, sizeof(u));
        return u;
    }
    static void save_bool(bool c, std::ostream& is) {
        bool u = c;
        is.write(reinterpret_cast<const char *>(&u), sizeof(u));
    }
    static double load_double(std::istream& is) {
        double u;
        is.read((char*)&u, sizeof(u));
        return u;
    }
    static void save_double(double i, std::ostream& is) {
        double u = i;
        is.write(reinterpret_cast<const char *>(&u), sizeof(u));
    }

    static void save_uint_vector(const std::vector<int>& v, std::ostream& is) {
        save_uint(v.size(), is);
        for (int x : v)
            save_uint(x, is);
    }
    static void save_uint_vector_2d(const std::vector<std::vector<int>>& v, std::ostream& is) {
        save_uint(v.size(), is);
        for (const auto& x : v)
            save_uint_vector(x, is);
    }

    static void load_uint_vector(std::vector<int>& v, std::istream& is) {
        size_t v_l = load_uint(is);
        // assert(v.size() == vl);
        v.resize(v_l);
        for (size_t i=0; i<v_l; i++) {
            v[i] = load_uint(is);
        }
    }
    static void load_uint_vector_2d(std::vector<std::vector<int>>& v, std::istream& is) {
        size_t vl = load_uint(is);
        // assert(v.size() == vl);
        v.resize(vl);
        for (size_t i=0; i<vl; i++) {
            load_uint_vector(v[i], is);
        }
    }

    static void save_bool_vector(const std::vector<bool>& v, std::ostream& is) {
        save_uint(v.size(), is);
        for (int x : v)
            save_bool(x, is);
    }

    static void load_bool_vector(std::vector<bool>& v, std::istream& is) {
        size_t v_l = load_uint(is);
        // assert(v.size() == vl);
        v.resize(v_l);
        for (size_t i=0; i<v_l; i++) {
            v[i] = load_bool(is);
        }
    }

    template<typename T>
    static void save_map_T_uint(const std::map<T, int>& m, std::ofstream& fo) {
        save_uint(m.size(), fo);
        std::cerr << "save_map_T_uint l=" << m.size() << std::endl;
        for (const auto& x : m) {
            x.first.save(fo);
            save_uint(x.second, fo);
        }   
    }

    template<typename T>
    static void load_map_T_uint(std::map<T, int>& m, std::ifstream& fo) {
        size_t l = load_uint(fo);
        std::cerr << "load_map_T_uint l=" << l << std::endl;
        for (size_t i=0; i<l; i++) {
            T first;
            int second = load_uint(fo);
            first.load(fo);
            m[first] = second;
        }
    }

    template<typename T>
    static void save_set_T(const std::set<T>& m, std::ofstream& fo) {
        save_uint(m.size(), fo);
        for (const auto& x : m) {
            x.save(fo);
        }   
    }

    template<typename T>
    static void load_set_T(std::set<T>& m, std::ifstream& fo) {
        size_t l = load_uint(fo);
        for (size_t i=0; i<l; i++) {
            T first;
            first.load(fo);
            m.insert(first);
        }
    }


    template<typename T>
    static void save_vector_T(const std::vector<T>& m, std::ofstream& fo) {
        save_uint(m.size(), fo);
        for (const auto& x : m) {
            x.save(fo);
        }   
    }

    template<typename T>
    static void load_vector_T(std::vector<T>& m, std::ifstream& fo) {
        size_t l = load_uint(fo);
        m.resize(l);
        for (size_t i=0; i<l; i++) {
            m[i].load(fo);
        }
    }

    template<typename T>
    static void save_tuple_uint_double_T(const std::tuple<int, double, T>& t, std::ofstream& fo) {
        save_uint(get<0>(t), fo);
        save_double(get<1>(t), fo);
        get<2>(t).save(fo);
    }
    template<typename T>
    static void load_tuple_uint_double_T(std::tuple<int, double, T>& t, std::ifstream& fo) {
        get<0>(t) = load_uint(fo);
        get<1>(t) = load_double(fo);
        get<2>(t).load(fo);
    }

    template<typename T>
    static void save_tuple_uint_double_T_vector_2d(const std::vector<std::vector<std::tuple<int,double,T>>>& mut_mut_pvalues_, std::ofstream &fo) {
        save_uint(mut_mut_pvalues_.size(), fo);
        for (const auto& vt: mut_mut_pvalues_) {
            save_uint(vt.size(), fo);
            for (const auto& t : vt) {
                save_tuple_uint_double_T(t, fo);
            }
        }
    }
    template<typename T>
    static void load_tuple_uint_double_T_vector_2d(std::vector<std::vector<std::tuple<int,double,T>>>& mut_mut_pvalues_, std::ifstream &fo) {
        size_t l1 = load_uint(fo);
        mut_mut_pvalues_.resize(l1);

        for (size_t i=0; i<l1; i++) {
            size_t l2 = load_uint(fo);
            mut_mut_pvalues_[i].resize(l2);
            for (size_t j=0; j<l2; j++) {
                load_tuple_uint_double_T(mut_mut_pvalues_[i][j], fo);
            }
        }
    }
};

int date_string_to_days(const std::string& string_date) {
    std::vector<std::string> s = split_string(string_date, '-');
    if (s.size() != 3) return -1;
    int y = std::atoi(s[0].c_str()), m = std::atoi(s[1].c_str()), d = std::atoi(s[2].c_str());
    auto now = std::chrono::sys_days{std::chrono::year{y}/m/d},
	first_day = std::chrono::sys_days{std::chrono::year{1}/1/1};
    return (now - first_day).count();
}

std::string date_days_to_string(int date) {
	auto first_day = std::chrono::sys_days{std::chrono::year{1}/1/1} + std::chrono::days(date);
	std::string s = std::format("{:%Y-%m-%d}", first_day);
	return s;
}

float date_string_to_fractional(const std::string& string_date) {
    // source: https://stackoverflow.com/questions/55367898/fractional-day-of-the-year-computation-in-c14
    std::vector<std::string> s = split_string(string_date, '-');
    if (s.size() != 3) return -1;
    int y = std::atoi(s[0].c_str()), m = std::atoi(s[1].c_str()), d = std::atoi(s[2].c_str());
    auto now = std::chrono::sys_days{std::chrono::year{y}/m/d},
	now_year_beg = std::chrono::sys_days{std::chrono::year{y}/1/1},
	now_year_next = std::chrono::sys_days{std::chrono::year{y+1}/1/1};

    float days_frac = std::chrono::days(now - now_year_beg) * 1.0 / std::chrono::days(now_year_next - now_year_beg);
    return y + days_frac;
}

std::chrono::year_month_day date_fractional_to_ymd(float date_f) {
    int y = int(date_f);
	auto now_year_beg = std::chrono::sys_days{std::chrono::year{y}/1/1},
	    now_year_next = std::chrono::sys_days{std::chrono::year{y+1}/1/1};
    int y_d = int((date_f - y) * std::chrono::duration_cast<std::chrono::days>(now_year_next - now_year_beg).count());
    

    auto ymd = std::chrono::year_month_day( now_year_beg + std::chrono::days(y_d) );

    return ymd;
}


#endif
