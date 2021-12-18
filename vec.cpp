#include "vec.hpp"
#include <istream>

std::istream &operator>>(std::istream &is, int_vec2 &out)
{
	is >> out.x;
	is.ignore(1, 'x');
	is >> out.y;
	return is;
}

std::istream &operator>>(std::istream &is, vec2 &out)
{
	is >> out.x;
	is.ignore(1, 'x');
	is >> out.y;
	return is;
}

std::istream &operator>>(std::istream &is, vec3 &out)
{
	is >> out.r;
	is.ignore(1, 'x');
	is >> out.g;
	is.ignore(1, 'x');
	is >> out.b;
	return is;
}
