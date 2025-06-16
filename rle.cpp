#include <iostream>
#include <list>
#include <bits/stdc++.h>
#include "rle.h"

template class RLE<bool,unsigned char>;
template class RLE<bool,unsigned int>;
template class RLE<unsigned char,unsigned char>;
template class RLE<unsigned char,unsigned int>;
template class RLE<unsigned int,unsigned char>;
template class RLE<unsigned int,unsigned int>;

template <class T, class U>
RLE<T,U>::RLE()
{

}

template <class T, class U>
RLE<T,U>::~RLE()
{

}

template <class T, class U>
vector<U> RLE<T,U>::GetOdd(vector<T>& v)
{
	vector<U> odd;
	odd.reserve(v.size()/2);

	for (size_t i = 0; i < v.size(); ++i) {
		if (i % 2 == 1) odd.push_back((U)v.at(i));
	}

	return odd;
}

template <class T, class U>
vector<U> RLE<T,U>::GetEven(vector<T>& v)
{
	vector<U> even;
	even.reserve(v.size()/2);

	for (size_t i = 0; i < v.size(); ++i) {
		if (i % 2 == 0) even.push_back((U)v.at(i));
	}

	return even;
}

template <class T, class U>
void RLE<T,U>::MTF(vector<T>& v)
{
	vector<U> mtf;
	mtf.reserve(v.size());

	size_t sz = 1UL << (8 * sizeof(U));
	list<T> q(sz);
	iota(q.begin(),q.end(),0);
	for (T e : v) {
		unsigned int p = 0;
		typename list<T>::iterator it = q.begin();
		while (*it != e) {
			it++;
			p++;
		}

		mtf.push_back((U)p);
		if (it != q.begin()) {
			q.splice(q.begin(), q, it, std::next(it));
		}
	}

	m_MTF = mtf;
}

template <class T, class U>
void RLE<T,U>::Encode(vector<T>& v)
{
	m_RLE.clear();
	m_RLE.reserve(v.size());

	U max = (U) ((1UL << (8 * sizeof(U))) - 1);
	size_t c = 0;
	T p = 0;

	auto push_back = [&]() {
		do {
			if (c > max) {
				m_RLE.push_back(max);
				if (typeid(T) == typeid(bool))
					m_RLE.push_back(0);
				else
					m_RLE.push_back(p);
				c -= max;
			} else {
				m_RLE.push_back((U)c);
				if (typeid(T) != typeid(bool))
					m_RLE.push_back(p);
				c = 0;
			}
		} while (c > 0);
	};

	for (T b : v) {
		if (b == p) c++;
		else {
			push_back();
			p = b;
			c = 1;
		}
	}

	push_back();
}

// Run length encoding of the zeros only
template <class T, class U>
void RLE<T,U>::EncodeZ(vector<T>& v)
{
	m_RLE.clear();
	m_RLE.reserve(v.size());

	U max = (U) ((1UL << (8 * sizeof(U))) - 1);
	size_t c = 0;

	auto push_back = [&]() {
		do {
			if (c > max) {
				m_RLE.push_back(0);
				m_RLE.push_back(max);
				c -= max;
			} else {
				m_RLE.push_back(0);
				m_RLE.push_back((U)c);
				c = 0;
			}
		} while (c > 0);
	};

	for (T b : v) {
		if (b == 0) c++;
		else {
			if (c > 0) push_back();
			m_RLE.push_back(b);
			c = 0;
		}
	}

	if (c > 0) push_back();
}

template <class T, class U>
void RLE<T,U>::BRLE(vector<bool>& v)
{
	m_BRLE.clear();

	auto push_header = [&](auto flag, auto val, auto len) {
		unsigned char header = 0;
		int count = 1;

		len--;									// the minimum value of length is 1, we can reduce to zero

		if (flag) header |= 128;         // indicate a repeated pattern (0) or literal (1)
		if (val) header |= 64;           // indicate if the pattern ir zero or 1
		if (len > 31) header |= 32;      // indicate an additional length header
		header |= len % 32;
		m_BRLE.push_back(header);

		if (len > 31) {                  // 2^5-1
			header = (len > 4095) ? 128 : 0;
			header |= (len >> 5) % 128;
			m_BRLE.push_back(header);
			count++;
		}

		if (len > 4095) {                // 2^12-1
			header = (len > 524287) ? 128 : 0;
			header |= (len >> 12) % 128;
			m_BRLE.push_back(header);
			count++;
		}

		if (len > 524287) {              // 2^19-1
			header = (len > 67108864) ? 128 : 0;
			header |= (len >> 19) % 128;
			m_BRLE.push_back(header);
			count++;
		}

		if (len > 67108864) {            // 2^26-1, end of the line for unsigned int
			header = len >> 26;
			m_BRLE.push_back(header);
			count++;
		}
	};

	vector<bool> literals;
	literals.clear();

	auto push_literals = [&]() {
		unsigned char b = 0;
		int i = 0;
		int sz = 8 * sizeof(unsigned char);
		for (bool l : literals) {
			if (i > (sz-1) && i % sz == 0) {
				m_BRLE.push_back(b);
				b = 0;
			}
			if (l) b |= (unsigned char)(1UL << ((sz-1)-(i%sz)));
			i++;
		}
		m_BRLE.push_back(b);
	};

	bool prev = v.at(0);
	size_t rlc = 0;

	for (bool b : v) {
		if (prev == b) rlc++;
		else {
			if (rlc >= 8) { // makes sense if the runs are at least 8
				if (literals.size() > 0) {
					push_header(true,false,literals.size());
					push_literals();
               literals.clear();
				}
				push_header(false,prev,rlc);
			} else {
				for (size_t i = 0; i < rlc; ++i)
					literals.push_back(prev);
			}
			prev = b;
			rlc = 1;
		}
	}

	if (literals.size() > 0) {
		push_header(true,false,literals.size());
		push_literals();
	}

	push_header(false,prev,rlc);
}

// TO DO: the following is still buggy.
template <class T, class U>
void RLE<T,U>::UnBRLE(vector<T>& v, size_t sz)
{
	vector<bool> dec;
	if (sz > 0) dec.reserve(sz);
	dec.clear();
	T t = 0;

	for (size_t i = 0; i < v.size(); ++i) {
		t = v.at(i);
		bool b = (bool) (t & 64);
		bool ext = (bool) (t & 32);
		size_t len = (size_t) (t & 31);
		int shift = 5;
		while (ext) {
			++i;
			t = v.at(i);
			ext = (bool) (t & 128);
			len += ((t & 127) << shift);
			shift += 7;
		}
		len++;
		if (t & 128) { // literal
			size_t bitsz = 8 * sizeof(T);
			size_t len_bytes = len / bitsz;
			if (len % bitsz != 0) len_bytes++;
			for (size_t j = 0, k = 0; j < len_bytes && k < len; ++j) {
				++i;
				t = v.at(i);
				for (size_t n = 0; n < bitsz && k < len; ++n) {
					b = (t & (T)(1UL << (bitsz-1-n)));
					dec.push_back(b);
					k++;
				}
			}
		} else { // pattern
			for (size_t j = 0; j < len; ++j)
				dec.push_back(b);
		}
	}

	m_UnBRLE = dec;
}

template <class T, class U>
void RLE<T,U>::Debug()
{
	cout << "m_RLE(" << m_RLE.size() << "): { ";
	for (U u : m_RLE) {
		cout << (int)u << ",";
	}
	cout << " }\n";

	cout << "m_BRLE(" << m_BRLE.size() << "): { ";
	for (unsigned char c : m_BRLE) {
		cout << (int)c << ",";
	}
	cout << " }\n";

	cout << "m_MTF(" << m_MTF.size() << "): { ";
	for (U u : m_MTF) {
		cout << (int)u << ",";
	}
	cout << " }\n";
}
