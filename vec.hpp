#ifndef vec_hpp_INCLUDED
#define vec_hpp_INCLUDED
#include "glad/glad.h"
#include <istream>
#include <tclap/ArgTraits.h>

struct int_vec2 {
	int x;
	int y;
	
	typedef TCLAP::ValueLike ValueCategory; // TCLAP will not work without this. 

	friend std::istream &operator>>(std::istream &is, int_vec2 &out);

};

struct vec2 {
	GLfloat x;
	GLfloat y;
	
	typedef TCLAP::ValueLike ValueCategory; 
	friend std::istream &operator>>(std::istream &is, vec2 &out);
	
};

struct vec3 {
	GLfloat r;
	GLfloat g;
	GLfloat b;
	
	typedef TCLAP::ValueLike ValueCategory; 
	friend std::istream &operator>>(std::istream &is, vec3 &out);

};

#endif // vec_hpp_INCLUDED

