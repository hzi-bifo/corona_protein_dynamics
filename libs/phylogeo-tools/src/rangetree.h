#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <cassert>

const int INF = 1<<30;

struct SegmentAll {
	int n;
	std::vector<double> t;

	void build() {  // build the tree
		for (int i = n - 1; i > 0; --i) t[i] = std::min(t[i<<1], t[i<<1|1]);
	}

	void modify(int p, double value) {  // set value at position p
		//std::cerr << " Uy " << p << " " << value << std::endl;
		p += n;
		t[p] = std::min(t[p], value);
		for (; p > 1; p >>= 1) t[p>>1] = std::min(t[p], t[p^1]);
	}

	double query(int l, int r) {  // sum on interval [l, r)
		double res = INF;
		for (l += n, r += n; l < r; l >>= 1, r >>= 1) {
			if (l&1) res = std::min(res, t[l++]);
			if (r&1) res = std::min(res, t[--r]);
		}
		//std::cerr << "  qy " << l << " " << r << " = " << res << " " << print(std::cerr) << std::endl;
		return res;
	}

	std::ostream& print(std::ostream& os) {
		os << "   +[";
		for (int i=0; i<n; i++)
			os << t[i+n] << " ";
		return os << "]";
	}

	SegmentAll(int n=0) : n(n), t(2*n, INF) {
	}

};

struct Point {
	int x, y;
	Point(int x, int y) : x(x), y(y) {}
};

std::ostream& operator<<(std::ostream& os, const Point& p) {
	return os << p.x <<"," << p.y;
}

struct Segment {
	int n;
	std::vector<double> t;
	std::vector<int> y;

	//void build() {  // build the tree
	//	for (int i = n - 1; i > 0; --i) t[i] = std::min(t[i<<1], t[i<<1|1]);
	//}

	size_t y_lb(int v) {
		std::vector<int>::iterator i = std::lower_bound(y.begin(), y.end(), v);
		assert(i == y.end() || v <= *i);
		assert(i != y.end() || y.size() == 0 || y.back() < v);
		assert(i == y.begin() || *(i-1) < v);
		return i - y.begin();
	}

	size_t y_index(int v) {
		size_t lb = y_lb(v);
		if (!(lb < y.size() && y[lb] == v)) {
			std::cerr << "  y_index " << v << " " << y << " " << lb << std::endl;
		}
		assert(lb < y.size() && y[lb] == v);
		return lb;
	}

	void modify(int p_val, double value) {  // set value at position p
		//std::cerr << " Uy " << p << " " << value << std::endl;
		int p = y_index(p_val);
		p += n;
		t[p] = std::min(t[p], value);
		for (; p > 1; p >>= 1) t[p>>1] = std::min(t[p], t[p^1]);
	}

	double query(int l_val, int r_val) {  // sum on interval [l, r)
		double res = INF;
		int l = y_lb(l_val), r = y_lb(r_val);
		for (l += n, r += n; l < r; l >>= 1, r >>= 1) {
			if (l&1) res = std::min(res, t[l++]);
			if (r&1) res = std::min(res, t[--r]);
		}
		//std::cerr << "  qy " << l << " " << r << " = " << res << " " << print(std::cerr) << std::endl;
		return res;
	}

	std::ostream& print(std::ostream& os) {
		os << "   +[";
		for (int i=0; i<n; i++)
			os << t[i+n] << " ";
		return os << "]";
	}

	Segment() : n(0), t(0, INF) {
	}

	Segment(const std::vector<int>& s1, const std::vector<int>& s2) {
	//y does not contain duplicate values, although it should work with duplicate values
		y = s1;
		y.insert(y.begin(), s2.begin(), s2.end());
		sort(y.begin(), y.end());
		n = y.size();
		t = std::vector<double>(2*n, INF);
	}

	Segment(const std::vector<Point>::iterator& b, const std::vector<Point>::iterator& e) {
	//y does not contain duplicate values, although it should work with duplicate values
		for (auto i=b; i != e; i++)
			y.push_back(i->y);
		sort(y.begin(), y.end());
		n = y.size();
		t = std::vector<double>(2*n, INF);
	}

};

struct RangeTree {
	std::vector<Segment> y_segments;
	int n, m;

	void modify(int x, int y, double val) {
		//std::cerr << "U " << x << "," << y << " " << val << std::endl;
		y_segments[x += n].modify(y, val);
		//std::cerr <<"    " << x << " " << y_segments[x].print(std::cerr) << std::endl;
		for (; x > 1; x >>= 1) {
			y_segments[x >> 1].modify(y, val);
			//std::cerr <<"    " << (x>>1) << " " << y_segments[x>>1].print(std::cerr) << std::endl;
		}
	}

	double query(int xl, int yl, int xr, int yr) {  // min on interval [l, r)
		if (xl >= xr or yl >= yr)
			return INF;
		//std::cerr << "Q " << xl << "," << yl << " " << xr << "," << yr << std::endl;
		double res = INF;
		for (xl += n, xr += n; xl < xr; xl >>= 1, xr >>= 1) {
			//std::cerr << " q " << xl << "," << xr << std::endl;
			if (xl&1) res = std::min(res, y_segments[xl++].query(yl, yr));
			if (xr&1) res = std::min(res, y_segments[--xr].query(yl, yr));
		}
		return res;
	}

	RangeTree() {}

/*
	//TODO: remove this function!
	RangeTree(int n, int m, const std::vector<Point>& points_) : n(n), m(m) {
		std::vector<Point> points = points_;

		sort(points.begin(), points.end(), [](const Point& a, const Point& b) { return a.x < b.x; });
		assert(points.begin()->x >= 0 && points.back().x < n);
		std::vector<int> b(2*n, -1), e(2*n, -1);
		y_segments = std::vector<Segment>(2*n);
		for (int i = 2*n - 1; i > 0; --i) {
			if (i >= n) {
				b[i] = i - n;
				e[i] = i - n + 1;
			} else {
				b[i] = b[i<<1];
				e[i] = e[i<<1|1];
				//t[i] = std::min(t[i<<1], t[i<<1|1]);
			}
			y_segments[i] = Segment(points.begin()+b[i], points.begin()+e[i]);
		}
	}
	*/

	RangeTree(int n, int m, std::vector<Point>& points) : n(n), m(m) {
		sort(points.begin(), points.end(), [](const Point& a, const Point& b) { return a.x < b.x; });
		assert(points.begin()->x >= 0 && points.back().x < n);
		y_segments = std::vector<Segment>(2*n);
		for (int i = 2*n - 1; i > 0; --i) {
			if (i >= n) {
				std::vector<Point>::iterator it = lower_bound(points.begin(), points.end(), i-n, [](const Point& p, int v) {return p.x < v;});
				std::vector<int> y;
				for (; it != points.end() && it->x == i-n; it++)
					y.push_back(it->y);
				//y_segments[i] = Segment(points.begin()+ i - n, points.begin()+i - n + 1);
				y_segments[i] = Segment(y, std::vector<int>{});
			} else {
				y_segments[i] = Segment(y_segments[i<<1].y, y_segments[i<<1|1].y);
				//t[i] = std::min(t[i<<1], t[i<<1|1]);
			}
			//std::cerr << "seg " << i << " " << b[i] << " " << e[i] << std::endl;
			//y_segments[i] = Segment(points.begin()+b[i], points.begin()+e[i]);
			//std::cerr << "seg " << i << " " << y_segments[i].y << std::endl;
		}
	}
};

