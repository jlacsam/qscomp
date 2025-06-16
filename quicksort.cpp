#include <iostream>
#include <bits/stdc++.h>
#include "quicksort.h"

template class Quicksort<unsigned char>;
template class Quicksort<unsigned short>;
template class Quicksort<unsigned int>;

template <class T>
Quicksort<T>::Quicksort()
{
	m_MaxSteps = 0;
	m_ActualSteps = 0;
}

template <class T>
Quicksort<T>::Quicksort(vector<T> &buffer)
{
	m_Unsorted = buffer;
	m_MaxSteps = 0;
	m_ActualSteps = 0;
}

template <class T>
Quicksort<T>::~Quicksort()
{

}

template <class T>
void Quicksort<T>::Load(vector<T> &buffer)
{
	m_Unsorted.clear();
	m_Sorted.clear();
	m_Distribution.clear();
	m_Flips.clear();
	m_Deltas.clear();

	m_Unsorted = buffer;
	m_MaxSteps = 0;
	m_ActualSteps = 0;
}

template <class T>
void Quicksort<T>::Load(std::string filename)
{
	m_Filename = filename;
	std::filesystem::path p(filename);
	size_t sz = std::filesystem::file_size(p);

	m_Unsorted.clear();
	m_Unsorted.resize(sz/sizeof(T));

	m_Sorted.clear();
	m_Distribution.clear();
	m_Flips.clear();
	m_Deltas.clear();

	m_MaxSteps = 0;
	m_ActualSteps = 0;

	std::FILE* f = std::fopen(filename.c_str(), "rb");
	sz = std::fread(&m_Unsorted[0], sizeof(T), m_Unsorted.size(), f);
}

template <class T>
void Quicksort<T>::Distribute()
{
	m_Distribution.clear();
	vector<T> v = m_Unsorted;

	T max = 0;
	for (T val : v) if (val > max) max = val;
	cerr << "WARNING: Max value is " << (int)max << endl << std::flush;

	for (size_t i = 0; i <= (size_t) max; ++i)
	{
		Tuple<T> t(i,i);
		m_Distribution.push_back(t);
	}

	for (T val : v)
	{
		m_Distribution[val].freq++;
	}

	double total = 0;
	double entropy = 0;
	int totalf = 0;
	for (size_t i = 0; i < m_Distribution.size(); ++i)
	{
		Tuple<T> &t = m_Distribution[i];
		double x = ((double) t.freq) / ((double) v.size());
		total += x;
		totalf += t.freq;
		t.prob = x;
		t.cumm = total;
		t.cumf = totalf;
		entropy += x > 0 ? -1.0 * x * log2(x) : 0;
	}

	cerr << "ENTROPY: " << entropy << endl;

	m_MaxSteps = 0;
	size_t sz = v.size();
	do
	{
		m_MaxSteps++;
		sz = sz >> 1;
	} while (sz > 0);
	if (m_MaxSteps < 8) m_MaxSteps = 8; 
}

template <class T>
T Quicksort<T>::GetPivot(int partitions, int segment)
{
	if (sizeof(T) == 1)
	{
		return (256 >> partitions) * (2 * segment + 1) - 1;
	}
	else
	{
		// TO DO: Optimize this - do a binary search
		double psize = 1.0 / ((double) (1 << (partitions-1)));
		double boundary = psize * (0.5 + (double)segment);

		T index = 0;
		for (size_t j = 0; j < m_Distribution.size(); ++j)
		{
			if (m_Distribution[j].cumm <= boundary)
			{
				index = j;
			}
			else
			{
				break;
			}
		}
		return index;
	}
}

template <class T>
size_t Quicksort<T>::GetBoundary(T pivot)
{
	int index = pivot < m_Distribution.size() ? pivot : m_Distribution.size()-1;
	return (size_t)m_Distribution.at(index).cumf;
}

template <class T>
bool Quicksort<T>::Sorted(vector<vector<T>> &v)
{
	bool sorted = true;
	for (size_t i = 0; i < v.size(); ++i)
	{
		if (v[i].size() == 0)
			continue;

		for (size_t j = 0; j < v[i].size() - 1; ++j)
		{
			if (v[i][j] > v[i][j+1])
			{
				sorted = false;
				cerr << "NOT SORTED YET!" << endl;
				break;
			}
		}
		if (!sorted) break;
	}

	return sorted;
}

template <class T>
bool Quicksort<T>::Sorted(vector<T> &v)
{
	bool sorted = true;
	for (size_t i = 0; i < v.size() - 1; ++i)
	{
		if (v[i] > v[i+1])
		{
			sorted = false;
			break;
		}
	}

	return sorted;
}

template <class T>
void Quicksort<T>::ToFlips(vector<vector<vector<bool>>> &v)
{
	m_Flips.clear();
	m_Flips.reserve(m_MaxSteps * m_Unsorted.size());

	for (vector<vector<bool>> x : v) {
		for (vector<bool> y : x) {
			for (bool b : y) {
				m_Flips.push_back(b);
			}
		}
	}
}

template <class T>
vector<unsigned char> Quicksort<T>::GetFlipsInBytes()
{
	vector<unsigned char> v;
	v.reserve(m_Flips.size()/8);

	unsigned char b = 0;
	int j = 0;
	for (bool f : m_Flips) {
		if (j > 7 && j % 8 == 0) {
			v.push_back(b);
			b = 0;
		}
		if (f) b |= 1 << (7 - (j % 8));
		j++;
	}
	v.push_back(b);

	return v;
}

template <class T>
void Quicksort<T>::ToSorted(vector<vector<T>> &v)
{
	m_Sorted.clear();
	for (size_t i = 0; i < v.size(); ++i)
	{
		for (size_t j = 0; j < v[i].size(); ++j)
		{
			m_Sorted.push_back(v[i][j]);
		}
	}
}

template <class T>
void Quicksort<T>::Prepare()
{
	cerr << "Prepping frequency distribution table ..." << endl;
	Distribute();
}

template <class T>
void Quicksort<T>::Presort()
{
	m_Runs.clear();
	m_Reverses.clear();

	vector<vector<T>> runs;
	vector<T> run;
	vector<bool> reverses;

	T t0 = m_Unsorted.at(0);
	run.push_back(t0);
	bool ascending = true;

	for (size_t i = 1; i < m_Unsorted.size(); ++i) {
		T t1 = m_Unsorted.at(i);
		if (run.size() == 1) {
			ascending = (t0 <= t1);
			run.push_back(t1);
		} else if ((t0 <= t1) && ascending) {
			run.push_back(t1);
		} else if ((t0 >= t1) && !ascending) {
			run.push_back(t1);
		} else {
			runs.push_back(run);
			reverses.push_back(!ascending);
			run.clear();
			run.push_back(t1);
		}
		t0 = t1;
	}

	if (run.size() > 0) {
		runs.push_back(run);
		reverses.push_back(!ascending);
	}

	m_Reverses = reverses;
	for (vector<T> v : runs) {
		m_Runs.push_back(v.size());
	}

	m_Presorted.clear();
	m_Presorted.reserve(m_Unsorted.size());
	for (size_t i = 0; i < runs.size(); ++i) {
		vector<T>& v = runs.at(i);
		if (reverses.at(i)) {
			for (int j = v.size()-1; j >= 0; --j) {
				m_Presorted.push_back(v.at(j));
			}
		} else {
			for (size_t j = 0; j < v.size(); ++j) {
				m_Presorted.push_back(v.at(j));
			}
		}
	}

	cerr << "Presort runs: " << m_Runs.size() << "/" << 
		(double)m_Unsorted.size() / (double)m_Runs.size() << endl;
}

template <class T>
void Quicksort<T>::Sort()
{
	vector<vector<T>> src, dst; 				// M partitions by N elements
	vector<vector<vector<bool>>> flips; 	// S steps by M partitions by N elements
	m_ActualSteps = 0;

	src.clear();
	dst.clear();
	src.resize(1); 								// start with 1 partition with N elements
	src[0] = m_Unsorted; 

	flips.clear();
	flips.reserve(m_MaxSteps);
	int flip_count, skip_count;

	for (int i = 1; i <= m_MaxSteps; ++i)
	{
		// size of partition at step i
		size_t src_sz = src[i-1].size(); 	// number of elements at each partition of the source
		size_t dst_sz = src_sz / 2;			// destination will have half that

		// keep track of each step, partition and element of src
		flips.push_back(vector<vector<bool>>());
		flips[i-1].reserve(src.size());
		flip_count = 0;
		skip_count = 0;

		// container for sorted items, one for the left another for the right
		dst.reserve(2*src.size());
#ifdef DEBUG
		cout << "STEP " << i << ": ";
#endif
		// perform sort
		for (size_t j = 0; j < src.size(); ++j)			// for each src partition
		{
			cerr << '\r' << "Sorting " << i << "/" << m_MaxSteps;
			cerr << " partition: " << j << "/" << src.size()-1 << "/" << src[j].size() << std::flush;

			flips[i-1].push_back(vector<bool>());
			flips[i-1][j].reserve(src[j].size());

			// container for sorted items, one for the left another for the right
			dst.push_back(vector<T>());
			dst.push_back(vector<T>());
			dst[2*j].reserve(src[j].size()/2);
			dst[2*j+1].reserve(src[j].size()/2);

			// if the partition has zero elements, skip it
			if (src[j].size() == 0)
			{
				skip_count++;
				continue;
			}

			// if this partition is already sorted, just copy it over
			if (Sorted(src[j]))
			{
				dst[2*j] = src[j];
				vector<bool> z(src[j].size(),false);
				flips[i-1][j] = z;
				continue;
			}

			// if not, perform the actual sorting
			T pivot = GetPivot(i,j);
			size_t boundary = sizeof(T) == 1 ? GetBoundary(pivot) : dst_sz;
#ifdef DEBUG
			cout << "(" << (int)pivot << "," << boundary << "),";
#endif

			for (size_t k = 0; k < src[j].size(); ++k)	// for each element in each src partition
			{
				if (src[j][k] <= pivot)							// if the value is less than the boundary
				{														// at this step and partition
					if (k <= boundary)							// and if the element is within the boundary
					{
						// stay left
						flips[i-1][j].push_back(false);
						dst[2*j].push_back(src[j][k]);
					}
					else
					{
						// flip left
						flips[i-1][j].push_back(true);
						dst[2*j].push_back(src[j][k]);
						flip_count++;
					}
				}
				else
				{
					if (k <= boundary)
					{
						// flip right
						flips[i-1][j].push_back(true);
						dst[2*j+1].push_back(src[j][k]);
						flip_count++;
					}
					else
					{
						// stay right
						flips[i-1][j].push_back(false);
						dst[2*j+1].push_back(src[j][k]);
					}
				}
			}
		}

#ifdef DEBUG
		cout << endl;
#endif
		cerr << " flips: " << flip_count << " skips: " << skip_count;
		cerr << "/" << src.size() << "    ";

#ifdef DEBUG
		// DUMP dst1
		cout << "STEP " << i << ": { ";
		for (size_t j = 0; j < dst.size(); ++j)
		{
			cout << "{";
			for (size_t k = 0; k < dst[j].size(); ++k)
			{
				cout << dst[j][k];
				//if (k < dst[j].size()-1) cout << ",";
			}
			cout << "}";
			if (j % 2 == 0) cout << "|";
			else if (j < dst.size()-1) cout << " , ";
		}
		cout << " } " << endl;
#endif
		// transfer dst back to src
		src.clear();
		src = dst;
		for (int j = 0; j < (1 << i); ++j) dst[j].clear();
		dst.clear();

		// increment actual steps
		m_ActualSteps += 1;

		// test if the vector is already sorted
		if (Sorted(src)) break;
	}

	cerr << endl << std::flush;

	// transfer flips to m_Flips, transfer src to m_Sorted
	ToFlips(flips);
	ToSorted(src);
}

template <class T>
void Quicksort<T>::Unsort()
{

}

template <class T>
void Quicksort<T>::Delta()
{
	// store differences between values of the sorted sequence
	m_Deltas.clear();
	m_Deltas.push_back(m_Sorted[0]);
	for (size_t i = 1; i < m_Sorted.size(); ++i) {
		m_Deltas.push_back(m_Sorted[i] - m_Sorted[i-1]);
	}
}

template <class T>
void Quicksort<T>::Write(string filename)
{
	ofstream out(filename, ios::out | ios::binary);
	if (!out) {
		cerr << "Cannot open " << filename << " for writing.\n";
		return;
	}

	vector<unsigned char> flips = GetFlipsInBytes();
	out.write((char*)&flips[0], flips.size());
	out.close();
	if (!out.good()) {
		cerr << "Error occurred while writing " << filename << endl;
		return;
	}
}

template <class T>
void Quicksort<T>::Debug()
{
	cout << "Size     : " << m_Unsorted.size() << endl;
	cout << "Flips Sz : " << m_Flips.size() << endl;
	cout << "Max Steps: " << m_MaxSteps << endl;
	cout << "Act Steps: " << m_ActualSteps << endl;

	cout << "m_Unsorted : { ";
	for (T t : m_Unsorted) {
		cout << (char)t;
	}
	cout << " }" << endl;

	cout << "m_Distribution:" << endl;
	cout << std::fixed << setprecision(8);
	for (Tuple t : m_Distribution) {
		if (t.freq > 0)
			cout << "    " << setw(3) << t.low << ":" << setw(7) << t.freq << "," 
				<< setw(7) << t.cumf << ", " 
				<< setw(10) << t.prob << ", "
				<< setw(10) << t.cumm << endl;
	}

	cout << "m_Deltas: { ";
	for (T t : m_Deltas) {
		cout << (int)t << ",";
	}
	cout << " }\n";

	cout << "m_Sorted: { ";
	for (T e : m_Sorted) {
		cout << e;
	}
	cout << " }\n";

	cout << "m_Flips: (" << m_Flips.size() << ")";
	int i = 0, j = 1;
	for (bool b : m_Flips) {
		if ((i % m_Sorted.size()) == 0) {
			cout << "\nSTEP " << j << ": ";
			j++;
		}
		if (i > 1 && i % 8 == 0) cout << " ";
		cout << (b ? "1" : "0");
		i++;
	}
	cout << endl;

	cout << "m_Runs (" << m_Runs.size() << "): { ";
	for (unsigned int r : m_Runs) {
		cout << r << ",";
	}
	cout << " }\n";

	cout << "m_Reverses (" << m_Reverses.size() << "): {";
	for (bool b : m_Reverses) {
		cout << (b ? "1" : "0");
	}
	cout << " }\n";

	cout << "m_Presorted : { ";
	for (T t : m_Presorted) {
		cout << (char)t;
	}
	cout << " }" << endl;
}
