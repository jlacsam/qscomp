#include <iostream>
#include <filesystem>
#include <bits/stdc++.h>
#include "prep.h"

Prep::Prep()
{

}

Prep::~Prep()
{

}

void Prep::Load(string filename)
{
	std::filesystem::path p(filename);
	size_t sz = std::filesystem::file_size(p);

	vector<unsigned char> buffer(sz,0);
	
	FILE* f = std::fopen(filename.c_str(), "rb");
	if (fread(&buffer[0], sizeof(unsigned char), sz, f) < sz) {
		cerr << "Unable to read the entire file.\n";
	}

	m_Prepped = buffer;
}

// Replace repeating characters of length 5 or more with XXXXN
void Prep::RLE(vector<unsigned char>& buffer)
{
	vector<unsigned char> rle;
	rle.reserve(buffer.size());
	
	unsigned char max = 255, min = 4;
	size_t c = 0;
	unsigned char p = 0;

	auto push_back = [&]() {
		do {
			if (c > (max+min)) {
				for (int i = 0; i < min; ++i) rle.push_back(p);
				rle.push_back(max);
				c -= max;
			} else if (c > min) {
				for (int i = 0; i < min; ++i) rle.push_back(p);
				c -= min;
				rle.push_back(c);
				c = 0;
			} else {
				for (size_t i = 0; i < c; ++i) rle.push_back(p);
				c = 0;
			}
		} while (c > 0);
	};

	for (unsigned char b : buffer) {
		if (b == p) c++;
		else {
			push_back();
			p = b;
			c = 1;
		}
	}

	push_back();
	m_Prepped = rle;
}

void Prep::Debug()
{
	cout << "m_Prepped: { ";
	for (unsigned char p : m_Prepped) {
		if (p == '\n') cout << "\\n";
		else if (p >= ' ') cout << p;
		else cout << "(" << (int)p << ")";
	}
	cout << " }\n";
}

// swap characters to increase sortedness
void Prep::Swap()
{
	vector<unsigned char> table(256);
	unsigned char index(0);
	generate(table.begin(), table.end(), [&]{ return index++; });

	table[')'] = '[';
	table['['] = ')';
	table['>'] = '{';
	table['{'] = '>';
	table['.'] = '|';
	table['|'] = '.';
	table['?'] = '~';
	table['~'] = '?';

	table['a'] = 'b';
	table['b'] = 'v';
	table['c'] = 'd';
	table['d'] = 'm';
	table['e'] = 'g';
	table['f'] = 's';
	table['g'] = 'j';
	table['h'] = 't';
	table['i'] = 'l';
	table['j'] = 'a';
	table['k'] = 'e';
	table['l'] = 'i';
	table['m'] = 'o';
	table['n'] = 'u';
	table['o'] = 'f';
	table['p'] = 'n';
	table['q'] = 'p';
	table['r'] = 'q';
	table['s'] = 'r';
	table['t'] = 'h';
	table['u'] = 'k';
	table['v'] = 'c';
	table['w'] = 'w';
	table['x'] = 'x';
	table['y'] = 'y';
	table['z'] = 'z';

	for (size_t i = 0; i < m_Prepped.size(); ++i) {
		m_Prepped.at(i) = table.at(m_Prepped.at(i));
	}
}

// reduce repeating whitespaces to 1
void Prep::Trim()
{
	vector<unsigned char> v;
	v.reserve(m_Prepped.size());

	unsigned char p = m_Prepped.at(0);
	v.push_back(p);
	for (size_t i = 1; i < m_Prepped.size(); ++i) {
		unsigned char c = m_Prepped.at(i);
		if (isspace(p) && isspace(c)) {
			// do nothing
		} else {
			v.push_back(c);
			p = c;
		}
	}

	m_Prepped = v;
}
