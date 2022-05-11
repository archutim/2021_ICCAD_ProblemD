#ifndef _MACRO_H_
#define _MACRO_H_

#include <iostream>
#include <string>

using namespace std;

#define border 3
#define _fixed 2
#define placed 1

class Macro
{

private:
	double width, height;
	double _x1, _y1; // lower left coordinate
	double _x2, _y2; // upper right coordinate
	int _id;
	bool _is_fixed;

	string macro_shape;
	int macro_type;
	string macro_name;

public:
	Macro(const Macro& m_copy);
	Macro(double w, double h, double _x, double _y, bool is_f, int i);
	Macro(string name, string shape, int type, double _x, double _y, bool is_f, int i);
	Macro(string name, double w, double h);
	double x1() const { return _x1; }
	double x2() const { return _x2; }
	double y1() const { return _y1; }
	double y2() const { return _y2; }
	int id() const { return _id; }
	double w() const { return width; }
	double h() const { return height; }
	bool is_fixed() const { return _is_fixed; }
	double cx() const { return _x1 + width / 2; }
	double cy() const { return _y1 + height / 2; }
	int type() const { return macro_type; }
	string shape() const { return macro_shape; }
	string name() const { return macro_name; }

	void set_is_fixed(bool b) { _is_fixed = b; } // Dangerous!! Only for issue case.
	void updateXY(pair<double, double> p);
	void setWidthHeight(Macro m, int dbu_per_micron);
};

#endif
