//
//  A 4x4 matrix (C)+(W) 2001 Thorsten Jordan
//

#ifndef MATRIX4_H
#define MATRIX4_H

#ifdef WIN32
#undef min
#undef max
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning (disable:4786)
#endif
#include <GL/gl.h>

#include "vector3.h"

#include <vector>
#include <cmath>
#include <iostream>
using namespace std;

template<class D>
class matrix4t
{
protected:
	static const unsigned size = 4;	// routines are generic, so use constant here
	vector<D> values;

        void columnpivot(vector<unsigned>& p, unsigned offset);

public:

	matrix4t() : values(size*size, D(0.0)) {}

	// construct in C++ order
        matrix4t(D* v) : values(size*size) {
		for (unsigned i = 0; i < size*size; ++i)
			values[i] = v[i];
	}

	matrix4t(const matrix4t<D>& other) : values(other.values) {}

	~matrix4t() {}

	// constuct in C++ order	
	matrix4t(const D& e0, const D& e1, const D& e2, const D& e3,
		const D& e4, const D& e5, const D& e6, const D& e7,
		const D& e8, const D& e9, const D& e10, const D& e11,
		const D& e12, const D& e13, const D& e14, const D& e15) {
		values.reserve(16);
		values.push_back(e0); values.push_back(e1); values.push_back(e2); values.push_back(e3);
		values.push_back(e4); values.push_back(e5); values.push_back(e6); values.push_back(e7);
		values.push_back(e8); values.push_back(e9); values.push_back(e10); values.push_back(e11);
		values.push_back(e12); values.push_back(e13); values.push_back(e14); values.push_back(e15);
	}

	matrix4t<D>& operator= (const matrix4t<D>& other) { values = other.values; return *this; }

	static matrix4t<D> one(void) { matrix4t<D> r; for (unsigned i = 0; i < size; ++i) r.values[i+i*size] = D(1.0); return r; }

	matrix4t<D> operator- (const matrix4t<D>& other) const { matrix4t<D> r; for (unsigned i = 0; i < size*size; ++i) r.values[i] = values[i] - other.values[i]; return r; }

	matrix4t<D> operator+ (const matrix4t<D>& other) const { matrix4t<D> r; for (unsigned i = 0; i < size*size; ++i) r.values[i] = values[i] + other.values[i]; return r; }

	matrix4t<D> operator* (const matrix4t<D>& other) const {
		matrix4t<D> r;
		for (unsigned i = 0; i < size; ++i) {	// columns of "other"
			for (unsigned j = 0; j < size; ++j) {	// rows of "this"
				D tmp = D(0.0);
				for (unsigned k = 0; k < size; ++k)	// columns of "this"/rows of "other"
					tmp += values[k+j*size] * other.values[i+k*size];
				r.values[i+j*size] = tmp;
			}
		}
		return r;
	}

	matrix4t<D> operator- (void) const { matrix4t<D> r; for (unsigned i = 0; i < size*size; ++i) r.values[i] = -values[i]; return r; }

	// store in C++ order
	void to_array(D* v) const {	// store in OpenGL order
		for (unsigned i = 0; i < size*size; ++i)
			v[i] = values[i];
	}

	// no range tests for performance reasons
	D& elem(unsigned col, unsigned row) { return values[col + row * size]; }
	D& elem_at(unsigned col, unsigned row) { return values.at(col + row * size); }

	void print(void) const {
		for(unsigned y = 0; y < size; y++) {
			cout << "/ ";
			for(unsigned x = 0; x < size; x++) {
				cout << "\t" << values[y*size+x];
			}
			cout << "\t/\n";
		}
	}

	void swap_rows(unsigned r1, unsigned r2) {
		for (unsigned i = 0; i < size; ++i) {
			D tmp = values[i+r1*size];
			values[i+r1*size] = values[i+r2*size];
			values[i+r2*size] = tmp;
		}
	}

	void swap_columns(unsigned c1, unsigned c2) {
		for (unsigned i = 0; i < size; ++i) {
			D tmp = values[c1+i*size];
			values[c1+i*size] = values[c2+i*size];
			values[c2+i*size] = tmp;
		}
	}

	matrix4t<D> inverse(void) const;

	matrix4t<D> transpose(void) const {
		matrix4t<D> r;
		for (unsigned i = 0; i < size; ++i)
			for (unsigned j = 0; j < size; ++j)
				r.values[i+size*j] = values[j+size*i];
		return r;
	}

	void set_gl(GLenum pname) {		// GL_PROJECTION, GL_MODELVIEW, GL_TEXTURE
		GLdouble m[16];
		for (unsigned i = 0; i < 4; ++i)
			for (unsigned j = 0; j < 4; ++j)
				m[i+j*4] = GLdouble(r.values[j+i*4]);
		glMatrixMode(pname);
		glLoadMatrixd(m);
		glMatrixMode(GL_MODELVIEW);
	}

	static matrix4t<D> get_gl(GLenum pname) {	// GL_PROJECTION_MATRIX, GL_MODELVIEW_MATRIX, GL_TEXTURE_MATRIX
		GLdouble m[16];
		glGetDoublev(pname, m);
		matrix4t<D> r;
		for (unsigned i = 0; i < 4; ++i)
			for (unsigned j = 0; j < 4; ++j)
				r.values[j+i*4] = D(m[i+j*4]);
		return r;
	}
	
	vector3t<D> operator* (const vector3t<D>& v) const {
		D r[4];
		for (unsigned j = 0; j < 4; ++j) {	// rows of "this"
			r[j] = values[j*4+0] * v.x + values[j*4+1] * v.y + values[j*4+2] * v.z + values[j*4+3];
		}
		// divide x,y,z by w
		return vector3t<D>(r[0]/r[3], r[1]/r[3], r[2]/r[3]);
	}

	// get n-th row/col, ignores last value
	vector3t<D> row(unsigned i) const {
		return vector3t<D>(values[i*4], values[i*4+1], values[i*4+2]);
	}
	vector3t<D> column(unsigned i) const {
		return vector3t<D>(values[i], values[i+4], values[i+2*4]);
	}
};



template<class D>
void matrix4t<D>::columnpivot(vector<unsigned>& p, unsigned offset)
{
	// find largest entry
        D max = values[offset * size + offset];
        unsigned maxi = 0;

	for (unsigned i = 1; i < size-offset; i++) {
                double tmp = values[(offset+i) * size + offset];
                if (fabs(tmp) > fabs(max)) {
                        max = tmp;
			maxi = i;
		}
	}

	// swap rows, change p
	if (maxi != 0) {
		swap_rows(offset, offset+maxi);
		unsigned tmp = p[offset];
		p[offset] = p[offset+maxi];
		p[offset+maxi] = tmp;
	}
}



template<class D>
matrix4t<D> matrix4t<D>::inverse(void) const
{
	matrix4t<D> r(*this);
	unsigned i, j, k;

	// prepare row swap
	vector<unsigned> p(size);
	for (i = 0; i < size; i++)
		p[i] = i;

	// LR - distribution
	for (i = 0; i < size-1; i++) { // columns of L
                r.columnpivot(p, i);
		for (j = i+1; j < size; j++) { // rows of L
			r.values[j*size+i] /= r.values[i*size+i];
			for (k = i+1; k < size; k++) { // columns of R
				r.values[j*size+k] -= r.values[j*size+i] * r.values[i*size+k];
			}
		}
	}

	// invert R without using extra memory
	for (j = size; j > 0; ) {
		--j;
		r.values[j*size+j] = D(1.0)/r.values[j*size+j];
                for (i = j; i > 0; ) {
                	--i;
			D s = r.values[i*size+j] * r.values[j*size+j];
			for (k = i+1; k < j; k++) {
				s += r.values[i*size+k] * r.values[k*size+j];
			}
			r.values[i*size+j] = -s/r.values[i*size+i];
		}
	}

	// invert L without using extra memory
	for (j = size; j > 0; ) {
		--j;
                for (i = j; i > 0; ) {
                	--i;
			D s = r.values[j*size+i];
			for (k = i+1; k < j; k++) {
				s += r.values[k*size+i] * r.values[j*size+k];
			}
			r.values[j*size+i] = -s;
		}
	}

	// compute R^-1 * L^-1 without using extra memory
	for (i = 0; i < size; i++) { // columns of L^-1
		for (j = 0; j < size; j++) { // rows of R^-1
			unsigned z = (i > j) ? i : j;
			D s = 0;
			for (k = z; k < size; k++) { // rows of L^-1
				if (i == k)
					s += r.values[j*size+k];
				else
					s += r.values[j*size+k] * r.values[k*size+i];
			}
			r.values[j*size+i] = s;
		}
	}

	// column swap
	vector<D> h(size);
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++)
                        h[j] = r.values[i*size+j];
		for (j = 0; j < size; j++)
                        r.values[i*size+p[j]] = h[j];
	}

	return r;
}



typedef matrix4t<double> matrix4;
typedef matrix4t<float> matrix4f;
typedef matrix4t<int> matrix4i;

#endif
