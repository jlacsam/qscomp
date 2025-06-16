#include <algorithm>
#include <cmath>
#include "iac.h"

// headers for debugging
#include <iostream>
#include <iomanip>
#include <bitset>
#include <typeinfo>

template struct Tuple<bool>;
template struct Tuple<unsigned char>;
template struct Tuple<unsigned short>;
template struct Tuple<unsigned int>;

template class IAC<bool>;
template class IAC<unsigned char>;
template class IAC<unsigned short>;
template class IAC<unsigned int>;

template <class T>
IAC<T>::IAC()
{
	m_Buffer.clear();
	m_Output.clear();
	m_Distribution.clear();
}

template <class T>
IAC<T>::~IAC()
{

}

template <class T>
void IAC<T>::Load(vector<T>& buffer)
{
	m_Buffer.clear();
	m_Buffer = buffer;
	Distribute();
}

template <class T>
void IAC<T>::Distribute()
{
	m_Distribution.clear();

	// This algorithm is designed for high values of t.
	for (T t : m_Buffer) {
		if (m_Distribution.size() == 0) {
			Tuple<T> x(t,t+1,1,1);
			m_Distribution.push_back(x);
			continue;
		}

		// binary search
		size_t l = 0;
		size_t r = m_Distribution.size() - 1;
		size_t m = (r+l)/2;
		T low = m_Distribution.front().low;
		T high = m_Distribution.back().low;
		T pivot = m_Distribution.at(m).low;
		while (low < t && t < high) {
			if (t <= pivot) {
				r = m;
			} else {
				l = m;
			}
			m = (r+l)/2;
			low = m_Distribution.at(l).low;
			high = m_Distribution.at(r).low;
			pivot = m_Distribution.at(m).low;
			if (m == l || m == r) break;
		}

		// either t is new (before l or after r), or not new (same as l or same as r)
		if (t < low) {
			typename vector<Tuple<T>>::iterator it = m_Distribution.begin() + l;
			Tuple<T> x(t,t+1,1,1);
			m_Distribution.insert(it,x);
		} else if (t == low ) {
			m_Distribution.at(l).freq++;
		} else if (low < t && t < high) {
			typename vector<Tuple<T>>::iterator it = m_Distribution.begin() + r;
			Tuple<T> x(t,t+1,1,1);
			m_Distribution.insert(it,x);
		} else if (t == high) {
			m_Distribution.at(r).freq++;
		} else if (t > high) {
			Tuple<T> x(t,t+1,1,1);
			m_Distribution.push_back(x);
		}
	}

	unsigned int tf = 0;
	double total = 0;
	for (size_t i = 0; i < m_Distribution.size(); ++i) {
		Tuple<T> t = m_Distribution.at(i);
		double x = (double) t.freq / ((double) m_Buffer.size());
		tf += t.freq;
		total += x;
		m_Distribution.at(i).cumf = tf;
		m_Distribution.at(i).prob = x;
		m_Distribution.at(i).cumm = total;
	}
}

template <class T>
double IAC<T>::AvgBits()
{
	double avg = 0;
	for (Tuple<T> t : m_Distribution) {
		unsigned char i = (unsigned char) t.low;
		double bits = i < 2 ? 1 : i < 4 ? 2 : i < 8 ? 3 : i < 16 ? 4 : i < 32 ? 5 :
			i < 64 ? 6 : i < 128 ? 7 : 8;
		avg += bits * t.prob;
	}

	return avg;
}

template <class T>
double IAC<T>::Entropy()
{
	double entropy = 0;
	for (Tuple<T> t : m_Distribution) {
		double p = t.prob;
		entropy += p > 0 ? -1.0 * p * log2(p) : 0;
	}

	return entropy;
}

template <class T>
size_t IAC<T>::MinSize()
{
	return (size_t) (((double)m_Buffer.size()) * AvgBits() / 8.0);
}

template <class T>
void IAC<T>::Show()
{
	size_t sz = m_Buffer.size();
	cerr << "Buffer Size   : " << sz << endl;
	double avg = AvgBits();
	cerr << "AvgBits/Total : " << avg << "/" << (size_t)(avg*(double)sz/8.0) << endl;
	double ent = Entropy();
	cerr << "Entropy/Total : " << ent << "/" << (size_t)(ent*(double)sz/8.0) << endl;
}

template <class T>
void IAC<T>::Debug()
{
	size_t sz = m_Buffer.size();
	cout << "Buffer Size   : " << sz << endl;
	double avg = AvgBits();
	cout << "AvgBits/Total : " << avg << "/" << (size_t)(avg*(double)sz/8.0) << endl;
	double ent = Entropy();
	cout << "Entropy/Total : " << ent << "/" << (size_t)(ent*(double)sz/8.0) << endl;

	cout << "IAC.m_Buffer: { ";
	if (typeid(T) == typeid(bool)) {
		for (T c : m_Buffer) {
			std::bitset<8> x(c);
			cout << x << " ";
		}
	} else {
		for (T c : m_Buffer) {
			cout << (int)c << ",";
		}
	}
	cout << " }\n";
	
	cout << "IAC.m_Distribution:\n";
	cout << std::fixed << setprecision(8);
	for (Tuple t : m_Distribution) {
		if (t.freq > 0)
         cout << "    " << setw(3) << (int)t.low << ":" << setw(7) << t.freq << ","
            << setw(7) << t.cumf << ", "
            << setw(10) << t.prob << ", "
            << setw(10) << t.cumm << endl;
	}
}
