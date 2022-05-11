#include "macro.h"
#include <iostream>

using namespace std;

Macro::Macro(const Macro& m_copy){
	width = m_copy.width;
	height = m_copy.height;
	_x1 = m_copy._x1;
	_y1 = m_copy._y1;
	_x2 = m_copy._x2;
	_y2 = m_copy._y2;
	_id = m_copy._id;
	_is_fixed = m_copy._is_fixed;

	macro_shape = m_copy.macro_shape;
	macro_type = m_copy.macro_type;
	macro_name = m_copy.macro_name;
}

Macro::Macro(double w, double h, double _x, double _y, bool is_f, int i) : width{w}, height{h}, _x1{_x}, _y1{_y}, _is_fixed{is_f}, _id{i}{
	_x2 = _x1 + width;
	_y2 = _y1 + height;
	macro_shape = "null";
	macro_type = 3;
	macro_name = "null";
}

Macro::Macro(string name, string shape, int type, double _x, double _y, bool is_f, int i) : macro_name{name}, macro_shape{shape}, macro_type{type}, width{-1}, height{-1}, _x1{_x}, _y1{_y}, _is_fixed{is_f}, _id{i}{
	_x2 = -1;
	_y2 = -1;
}

Macro::Macro(string name, double w, double h) : macro_name{name}, width{w}, height{h}{
	_x2 = -1;
	_y2 = -1;
	macro_shape = "null";
	macro_type = -1;
	_x1 = -1;
	_y1 = -1;
	_is_fixed = true;
	_id = -1;
}

void Macro::updateXY(pair<double, double> p){
	this->_x1 = p.first - width/2;
	this->_x2 = p.first + width/2;
	this->_y1 = p.second - height/2;
	this->_y2 = p.second + height/2;
}

void Macro::setWidthHeight(Macro m, int dbu_per_micron){
	width = m.w() * (double)dbu_per_micron;
	height = m.h() * (double)dbu_per_micron;
	_x2 = _x1 + width;
	_y2 = _y1 + height;
}