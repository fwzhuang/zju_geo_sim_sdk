/*
	State Key Lab of CAD&CG Zhejiang Unv.

	Author: Jin Huang (hj@cad.zju.edu.cn)

	Copyright (c) 2004-2010 <Jin Huang>
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:
	1. Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.
	3. The name of the author may not be used to endorse or promote products
	derived from this software without specific prior written permission.
*/

#include <time.h>

#ifdef _MSC_VER
	// NOTE: to use vector as data containner in vc8 define _
	// debug version fails because the boundary check assert.
	#define _SECURE_SCL 0
	#define _ITERATOR_DEBUG_LEVEL 0
#endif

#include "../../include/matrix.h"
#include "../../include/itr_matrix.h"
#include "../../include/io.h"


#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <deque>

using namespace std;
using namespace zjucad::matrix;

template <typename T>
class const_pointer_iterator : public default_matrix_iterator<T>
{
public:
	typedef T value_type;
	typedef T const *const_iterator;
	typedef const T& const_reference;

	operator const_iterator(void) const {
		return ptr_;
	}

	const_pointer_iterator():ptr_(0){}
	const_pointer_iterator(T *ptr):ptr_(ptr){}
	const_pointer_iterator operator++(void) {
		++ptr_;
		return *this;
	}
	const_pointer_iterator operator +=(int n) {
		ptr_ += n;
		return *this;
	}
	const_pointer_iterator operator+(size_type n) const {
		return const_pointer_iterator(ptr_+n);
	}
	const_reference operator*() const {return *ptr_;}
	template <typename I1>
	bool operator != (const I1 &i) const {return ptr_ != i.ptr_;}
	template <typename I1>
	bool operator == (const I1 &i) const {return !this->operator!=(i);}
protected:
	T* ptr_;
};

template <typename T>
class pointer_iterator : public const_pointer_iterator<T>
{
public:
	typedef T* iterator;
	typedef T& reference;
	pointer_iterator(){}
	pointer_iterator(const pointer_iterator &i) {
	 	this->ptr_ = i.ptr_;
	}
	const pointer_iterator &operator = (const pointer_iterator<T> &i) {
		this->ptr_ = i.ptr_;
		return *this;
	}
	pointer_iterator(T *ptr):const_pointer_iterator<T>(ptr){}
	operator const_pointer_iterator<T>(void) {
		return const_pointer_iterator<T>(this->ptr_);
	}
	reference operator*() {return *this->ptr_;}
};

template <typename E>
void func(matrix_expression<E> &m)
{
	typename E::const_iterator ci = m().begin();
	++ci;
	typename E::value_type v = *ci;
	typename E::iterator i = m().begin();
	++i;
	if(*i != v)
		cout << "error in func" << endl;
	*i = 0;
	ci = i;
	ci = i+4;
	i = i+4;
}


void test_itr_matrix(void)
{
	cout << "test_itr_matrix" << endl;
	matrix<int> A = to<int>(rand<double>(5, 5)*10.0);
//	itr_matrix<pointer_iterator<int> > iA(A.size(1), A.size(2), &A[0]);
	itr_matrix<int *> iA(A.size(1), A.size(2), &A[0]);
	cout << iA;
	iA(colon(0, 2), colon(1,3)) = 0;
	cout << iA;
	func(iA);
	cout << iA;
	vector<int> v(iA.size());
	copy(iA.begin(), iA.end(), v.begin());
	itr_matrix<vector<int>::iterator> iv(iA.size(1), iA.size(2), v.begin());
	cout << endl;
	cout << iv << endl;
	iv(colon(0, 2), colon(1,3)) = 9;
	cout << iv << endl;
	func(iv);
	cout << iv << endl;
}

void build_in()
{
	cout << "build_in\n";
	cout << eye<int>(3);
	cout << ones<int>(3, 2);
	cout << zeros<int>(2, 3);
	cout << fixed << setprecision(4);
	cout << rand<double>(4, 6);
}

void raw_access()
{
	cout << "raw_access\n";
	matrix<int> A = to<int>(rand(3, 2)*10);
	zjucad::matrix::size_type len = A.size();	// row, col, and lenth(when viewed as vector)
	int *pA = A.data().begin();	// get the raw data
	cout << A;
	copy(pA, pA+len, ostream_iterator<int>(cout, " "));
	cout << endl;
}

void op()
{
	cout << "op\n";
	matrix<int> A = to<int>(rand(4)*10), B = to<int>(rand(4)*10);
	cout << A << B;
	cout << trans(A);	// transpose of A
	cout << (A<5) <<(A<=5) << (A>5) << !(A<5) << (A>=5)  << (A==5);	// logic operator
	cout << A+B << A+1 << A*2 << A-B << A*B;	// algebra operator
	cout << sum(A) << endl;	// others

	matrix<float> v1 = rand<float>(3, 1), v2 = rand<float>(3, 1);
	cout << v1 << v2 << cross(v1, v2) << cross(v1, v2)/norm(cross(v1, v2)) << dot(v1, v2) << endl;
}

void io()
{
	cout << "io\n";
	// const char *path = "test.bin";
	// matrix<float> A = rand<float>(4);
	// // write A into a file in binary format
	// ofstream ofs(path, ofstream::binary);
	// write(ofs, A);
	// ofs.close();

	// // readin a matrix from a file in binary format
	// matrix<float> B;
	// ifstream ifs(path, ifstream::binary);
	// read(ifs, B);
	// ifs.close();
	// cout << "read in matrix\n" << B;
}

template <typename R_type, typename C_type>
void test_index(const matrix<int> &A, const R_type &R, const C_type &C)
{
  matrix<int> B = A;
  B(R, C) = 0;
  const matrix<int> D = abs(B(R, C));
  if(max(D) != 0)
    cerr << "idx error." << endl;;
}

template <typename R_type>
bool test_row(const matrix<int> &A, const R_type &R)
{
	matrix<size_t> I(3);
	I(0)=0;I(1)=2;I(2)=3;
  test_index(A, R, 3);
  test_index(A, R, colon(0, 2));
  test_index(A, R, colon(0, 2, 4));
  test_index(A, R, colon());
  test_index(A, R, I);
}

template <typename I_type>
bool test_vec(const matrix<int> &A, const I_type &I)
{
  matrix<int> B = A;
  B(I) = 0;
  matrix<int> D = abs(B(I));
  if(max(D) != 0)
    cerr << "idx error." << endl;;
}

void index()
{
	cout << "index\n";
	matrix<int> A = ones<int>(6);
	matrix<size_t> I(3);
	I(0)=0;I(1)=2;I(2)=3;
  cout << A;
	// in matlab: A(:, 1)          A(3, :)          A(2:4, 1:3)		// matlab index starts from 1 instead of 0.
	//            0th col           2nd row    sub matrix: 1~3 row, 0~2 col
	cout << "testing the index methods (scalar, range, interleave, all, index)\n";

  // the following test is not complete.  We also need to check the
  // left part unchanged.

  //  test_row(A, 1);
  test_row(A, colon(1, 3));
  test_row(A, colon(1, 2, 5));
  test_row(A, colon());
  test_row(A, I);

  //  test_vec(A, 1);
  test_vec(A, colon(3, 6));
  test_vec(A, colon(3, 4, 18));
  test_vec(A, colon());
  test_vec(A, I);

 /*
	cout << "matlab blkdiag\n";
	cout << (blkdiag<int>(),A,A(colon(0,2),colon(1,2,3)),A).fetch();

	cout << "matlab cat\n";
	cout << (cat<int>(),A,A,A).fetch2();

	cout << "matlab circshift\n";
	matrix<int> B(3,3);
	B(0,0)=1;B(0,1)=2;B(0,2)=3;
	B(1,0)=4;B(1,1)=5;B(1,2)=6;
	B(2,0)=7;B(2,1)=8;B(2,2)=9;
	cout << B << circshift(B,1000,0) << circshift(B,1000,-100000);

	cout << "matlab diag\n";
	cout << diag_gen(A,1) << diag_gen(A,-1);
	cout << diag_get(diag_gen(A,1),1);

	//we don't implement func end.

	cout << "matlab flipdim\n";
	cout << A << flipdim(A,1) << flipdim(A,2);

	cout << "matlab horzcat\n";
	cout << (horzcat<int>(),A,A,A).fetch();

	cout << "matlab repmat\n";
	cout << repmat(A,2,3);

	cout << "matlab reshape\n";
	cout << reshape(A,2,8);

	cout << "matlab rot90\n";
	B=matrix<int> (3,4);
	B(0,0)=1;B(0,1)=2;B(0,2)=3;B(0,3)=4;
	B(1,0)=5;B(1,1)=6;B(1,2)=7;B(1,3)=8;
	B(2,0)=9;B(2,1)=10;B(2,2)=11;B(2,3)=12;
	cout << B << rot90(B) << rot90(B,2) << rot90(B,3);

	cout << "matlab vertcat\n";
	cout << A << (vertcat<int>(),A,A,A).fetch();
*/
}

void advanced()
{
	cout << "advanced\n";

	// elements can be complex object
	matrix<matrix<int> > blocked_matrix(3, 2);
	fill(blocked_matrix.begin(), blocked_matrix.end(), to<int>(rand<float>(3, 3)*10));
	cout << blocked_matrix << blocked_matrix(colon(), 1);

	// const char *path = "bm.bin";
	// ofstream ofs(path, ofstream::binary);
	// write(ofs, blocked_matrix);
	// ofs.close();

	// readin the blocked matrix from a file in binary format
	// matrix<matrix<int> > blocked_matrix2;
	// ifstream ifs(path, ifstream::binary);
	// read(ifs, blocked_matrix2);
	// ifs.close();
	// cout << "read in blocked matrix\n" << blocked_matrix2;

	// can customize raw memory container
	// row major memory format for C style array, use vector<int> as memory container
	cout << "customize raw memory layout\n";
	matrix<int, row_major, vector<int> > A = to<int>(rand<float>(2, 3)*10);
	cout << A;
	const vector<int> *pA = &A.data();
	copy(pA->begin(), pA->end(), ostream_iterator<int>(cout, " "));
	cout << endl;

	// bmp memory format for bmp image or coordinates style array, use deque as memory container
	matrix<int, bmp_order, deque<int> > B = A;
	cout << B;
	deque<int> *pB = &B.data();
	copy(pB->begin(), pB->end(), ostream_iterator<int>(cout, " "));
	cout << endl;
}

// extern "C" {
// #include <cblas.h>
// }

// static void c_mult(const matrix<double> &A, const matrix<double> &B, matrix<double> &D)
// {
// 	const int m = A.size(1), k = A.size(2), n = B.size(2);
// 	const double *pA = &A[0], *pB = &B[0];
// 	double *pD = &D[0];
// 	for(int ni = 0; ni < n; ++ni) {
// 		for(int mi = 0; mi < m; ++mi) {
// 			for(int ki = 0; ki < k; ++ki) {
// 				pD[mi+ni*m] += pA[mi+ki*m] * pB[ki+ni*k];
// 			}
// 		}
// 	}
// }

// void performance()
// {
// 	const int m = 400, k = 500, n = 600;
// 	matrix<double> A = rand<double>(m, k), B = rand<double>(k, n), C = rand<double>(m, n);
// 	matrix<double> D = C;

// 	cout << "test performance: C = A*B*2+C*3;" << endl;
// 	time_t start = clock();
// 	C = A*B*2+C*3;
// 	cout << "matrix: " << clock()-start << endl;

// 	// compare with cblas
// 	start = clock();
// 	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, D.size(1), D.size(2), A.size(2),
// 		2.0, A.data().begin(), A.size(1), B.data().begin(), B.size(1), 3.0, D.data().begin(), D.size(1));
// 	cout << "cblas: " << clock()-start << endl;

// 	//fill(C.begin(), C.end(), 0);
// 	//start = clock();
// 	//c_mult(A, B, C);
// 	//cout << "c_mult: " << clock()-start << endl;

// 	// same result
// 	cout << C(colon(0, 3), colon(0, 3))-D(colon(0, 3), colon(0, 3));
// }

void notice()
{
	cout << "notice\n";
	matrix<int> A = to<int>(rand<float>(2)*10), B = to<int>(rand<float>(2)*10);
	matrix<int> C = A;
	cout << "A: \n" << A << "B: \n" << B;
	cout << "wrong: A=A*B\n" << (A=A*B);
	A = C;
	cout << "right: A=temp(A*B)\n" << (A=temp(A*B));

	matrix<float> A1 = rand<float>(80), B1 = rand<float>(80), C1 = rand<float>(80), D1, E1;
	cout << "bad performance: D1=A1*B1*C1\n";
	time_t start = clock();
	D1 = A1*B1*C1;
	cout << "use time: " << clock()-start << endl;
	cout << "good performance: E1=temp(A1*B1)*C1\n";
	start = clock();
	E1 = temp(A1*B1)*C1;
	cout << "use time: " << clock()-start << endl;

  //  matrix<int> no_pass = 1; // will not pass
  matrix<int> Scalar;
  Scalar = 1;
  cout << "assign scalar: " << Scalar << endl;
}

template <typename E>
int f(E &m)
{
  cout << typeid(typename E::iterator).name() << endl;
  cout << typeid(typename E::const_iterator).name() << endl;
  const double *a = 0;
  //  typename E::iterator b = a;
  //  typename iterator_type<E>::iterator c = a;
}

int main(int argc, char* argv[])
{
  cout << typeid(matrix<double>).name() << endl;
  cout << typeid(const matrix<double>).name() << endl;
  const matrix<double> K = rand<double>(10, 10);
  const matrix<size_t> noncons = rand<double>(3, 1)*10;
  const matrix<double> free_K = K(1, noncons);
  cout << K(1, noncons);// compile pass
  f(K);
  matrix<double> b;
  f(b);

	build_in();
	raw_access();
	op();
//	io();
	index();
	advanced();
//	performance();
	notice();
	test_itr_matrix();
	return 0;
}
