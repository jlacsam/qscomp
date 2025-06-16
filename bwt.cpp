#include <iostream>
#include <filesystem>
#include <bits/stdc++.h>
#include "bwt.h"

BWT::BWT()
{
	m_Filename = "";
	m_Front = m_Back = 0;
	m_Unsorted.clear();
	m_BWT.clear();
	m_Indexes.clear();
	m_Inverted.clear();
}

BWT::~BWT()
{

}

void BWT::Load(vector<unsigned char> &buffer)
{
	m_Unsorted = buffer;
}

void BWT::Load(string filename)
{
	m_Filename = filename;
   std::filesystem::path p(filename);
   size_t sz = std::filesystem::file_size(p);

   m_Unsorted.clear();
   m_Unsorted.resize(sz);
   FILE* f = std::fopen(filename.c_str(), "rb");
   if (fread(&m_Unsorted[0], sizeof(unsigned char), m_Unsorted.size(), f) < sz)
	{
		cerr << "Unable to read the entire file." << endl;
	}
}

void BWT::Transform()
{
	size_t sz = m_Unsorted.size();

	// determine the indexes if m_Unsorted were sorted
	vector<size_t> indexes(sz);
	size_t index(0);
	generate(indexes.begin(), indexes.end(), [&]{ return index++; });
	stable_sort(indexes.begin(), indexes.end(),
		[&](int i, int j)
		{
			if (m_Unsorted[i] == m_Unsorted[j])
			{
				for (size_t k = 1; k < sz; ++k)
				{
					int _i = (i + k + sz) % sz;
					int _j = (j + k + sz) % sz;
					int l = m_Unsorted.at(_i);
					int r = m_Unsorted.at(_j);
					if (l == r) continue;
					else return (l < r);
				}
				return false;
			}
			else return m_Unsorted[i] < m_Unsorted[j];
		});
	m_Indexes = indexes;

	// build m_BWT from the above indexes, find the zero index
	m_BWT.clear();
	m_BWT.reserve(sz);
	m_Front = m_Back = 0;
	index = 0;
	for (size_t t : indexes)
	{
		unsigned char c = m_Unsorted.at((t-1+sz)%sz);
		m_BWT.push_back(c);
		if (t == 0) m_Front = index;
		if (t == 1) m_Back = index;
		else index++;
	}
}

void BWT::Invert()
{
	// implementation is based from http://mattmahoney.net/dc/dce.html
	vector<unsigned char> bwt = m_BWT;

	size_t n = bwt.size();
	size_t p = m_Back;

	// collect cummulative counts of UnBWT
	vector<int> t(257,0);
	for (size_t i = 0; i < n; ++i)
		++t.at(bwt.at(i)+1);
	for (int i = 1; i < 257; ++i)
		t.at(i) += t.at(i-1);
	if (t.at(256)!=(int)n) cerr << "Something is not right." << endl;

	// build linked list
	vector<int> next(n,0);
	for (size_t i = 0; i < n; ++i)
		next.at(t.at(bwt.at(i))++) = i;
	if (t.at(255)!=(int)n) cerr << "Something is not right again." << endl;

	// traverse and output list
	m_Inverted.clear();
	m_Inverted.reserve(n);
	for (size_t i = 0; i < n; ++i)
	{
		m_Inverted.push_back(bwt.at(p));
		p = next.at(p);
	}
}

vector<unsigned char> BWT::Substring(unsigned char c)
{
	vector<unsigned char>v;

	size_t sz = m_Indexes.size();
	for (size_t t : m_Indexes) {
		if (m_Unsorted.at(t) == c)
			v.push_back(m_Unsorted.at((t-1+sz)%sz));
	}

	return v;
}

void BWT::Check()
{
	cerr << "CHECKS>>" << endl;

	// check "Inverted"
	if (m_Unsorted.size() == m_Inverted.size())
		cerr << "Unsorted and Inverted match in size." << endl;
	else
		cerr << "Unsorted and Inverted DONT match in size." << m_Unsorted.size()
			<< " vs " << m_Inverted.size() << endl;

	bool matched = true;
	for (size_t i = 0; i < m_Unsorted.size(); ++i)
	{
		if (m_Unsorted.at(i) != m_Inverted.at(i))
		{
			matched = false;
			break;
		}
	}

	if (matched)
		cerr << "Unsorted and Inverted match in content." << endl;
	else
		cerr << "Unsorted and Inverted DONT match in content." << endl;
}

void BWT::Stats()
{
	cerr << "Front/Back Row: " << m_Front << "/" << m_Back << endl;
}

void BWT::Debug()
{
	cout << "Unsorted (" << m_Unsorted.size() << "): { ";
	for (unsigned char c : m_Unsorted)
	{
		if (c == '\n') cout << "\\n";
		else cout << c;
	}
	cout << " }\n" << endl;

	cout << "Indexes (" << m_Indexes.size() << "): { ";
	for (size_t i : m_Indexes)
	{
		cout << i << ",";
	}
	cout << " }\n" << endl;

	cout << "BWT Front/Back: { " << m_Front << "," << m_Back << " }\n" << endl;

	cout << "BWT: { ";
	for (unsigned char c : m_BWT)
	{
		if (c == '\n') cout << "\\n";
		else cout << c;
	}
	cout << " }\n" << endl;

	cout << "Contexts of BWT: {\n";
	size_t sz = m_Unsorted.size();
	for (size_t t : m_Indexes)
	{
		cout << setw(7) << t << " : ";
		unsigned char b = m_Unsorted.at((t-1+sz)%sz);
		if (b == '\t') cout << "\\t" << ": ";
		else if (b == '\n') cout << "\\n" << ": ";
		else cout << b << " : ";
		for (size_t k = 0; k < 16; ++k) {
			unsigned char a = m_Unsorted.at((t+k+sz)%sz);
			if (a == '\n') cout << "\\n";
			else cout << a;
		}
		cout << endl;
	}
	cout << " }\n" << endl;

	cout << "Inverted: { ";
	for (unsigned char c : m_Inverted)
	{
		if (c == '\n') cout << "\\n";
		else cout << c;
	}
	cout << " } " << endl;

	unsigned char p = m_Unsorted.at((m_Indexes.at(0)+1+sz)%sz);
	int count = 0;
	cout << "Context Counts: {\n";
	for (size_t t : m_Indexes) {
		unsigned char c = m_Unsorted.at((t+1+sz)%sz);
		if (p == c) count++;
		else {
			if (m_Unsorted.at(t) == '\t') cout << "\\t";
			else if (m_Unsorted.at(t) == '\n') cout << "\\n";
			else cout << m_Unsorted.at(t);
			if (p == '\t') cout << "\\t";
			else if (p == '\n') cout << "\\n";
			else cout << p;
			cout << "," << count << endl;
			p = c;
			count = 1;
		}
	}
	cout << " } " << endl;
}

