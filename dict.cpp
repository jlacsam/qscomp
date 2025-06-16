#include <filesystem>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <bits/stdc++.h>
#include "iac.h"
#include "dict.h"

template struct Token<bool>;
template struct Token<unsigned char>;

template class Word<bool>;
template class Word<unsigned char>;

template class DC<bool>;
template class DC<unsigned char>;

template <class T>
DC<T>::DC()
{
	m_MinLen = MIN_LEN;
	m_MaxLen = MAX_LEN;
	m_MaxDst1 = MAX_DST1;
	m_MaxDst2 = MAX_DST2;
	m_CostDst1 = COST_DST1;
	m_CostDst2 = COST_DST2;
	m_WordLen = WORD_LEN;
	m_CodeLen = CODE_LEN;
	m_MaxCodes = MAX_CODES;
	m_MinCount = MIN_COUNT;

	m_Seed.clear();
	m_Lengths.clear();
	m_Distances.clear();
	m_Tokens.clear();
}

template <class T>
DC<T>::~DC()
{

}

template <class T>
void DC<T>::Load(string filename)
{
	m_Filename = filename;
	std::filesystem::path p(filename);
	int sz = std::filesystem::file_size(p);
	vector<unsigned char> buffer(sz,0);

	std::FILE* f = std::fopen(filename.c_str(), "rb");
	sz = std::fread(&buffer[0], sizeof(unsigned char), sz, f);

	m_Buffer.clear();
	if (typeid(T) == typeid(unsigned char)) {
		m_Buffer.reserve(sz);
		for (unsigned char b : buffer) m_Buffer.push_back(b);
	} else if (typeid(T) == typeid(bool)) {
		m_Buffer.reserve(8 * sz);
		for (unsigned char c : buffer) {
			for (int i = 0; i < 8; ++i) {
				bool b = (bool)(c & (1 << (7-i)));
				m_Buffer.push_back(b);
			}
		}
	}
}

template <class T>
void DC<T>::Load(vector<T>& buffer)
{
	m_Filename = string("none");
	m_Buffer.clear();
	m_Buffer = buffer;
}

template <class T>
void DC<T>::StorePattern(Token<T> &t, int b1_, int b2_)
{
	t.pattern.clear();
	for (int i = b1_; i <= b2_; ++i) {
		t.pattern.push_back(m_Buffer.at(i));
	}
}

template <class T>
void DC<T>::RollUp()
{
	int l = m_Buffer.size();
	int b1 = l - m_MinLen;
	int b2 = l - 1;
	int b0 = l; // marker for the previous token

	cerr << "PARAMS:(" << m_MinLen << "," << m_MaxLen << "," << m_MaxDst1 << "," << m_MaxDst2 
		<< "," << m_CostDst1 << "," << m_CostDst2 << ")" << endl;

	while (b1 >= m_MinLen) {
		cerr << '\r' << "Rolling up " << b1 << "/" << l << "     " << std::flush;

		int f2 = b1 - 1;
		int f1 = f2 - (b2 - b1);
		bool found = false;
		int f2_ = -1, b1_ = -1, b2_ = -1;

		while (f1 >= 0) {
			if (Identical(f1,f2,b1,b2)) {
				found = true;

				// remember the indexes of the matching substrings
				f2_ = f2;
				b1_ = b1;
				b2_ = b2;

				// increase the length of the substrings, avoid overlap
				b1 -= 1;
				if (b1 == f2) {
					f2 -= 1;
					f1 -= 2;
				} else {
					f1 -= 1;
				}
			} else { // slide to the next substring
				f1 -= 1;
				f2 -= 1;
			}

			// check if the distance is too far such that cost is greater than savings
			if (TooFar(f1,f2,b1,b2)) break;
			if (TooLong(b1,b2)) break;
		}

		if (found) {
			// create a token: length, distance, gap, position of back, position of front
			Token<T> t(b2_-b1_+1, b1_-f2_-1, b0-b2_-1, b1_, f2_);
			StorePattern(t,b1_,b2_);
			typename std::vector<Token<T>>::iterator it = m_Tokens.begin();
			m_Tokens.insert(it,t);

			b0 = b1_;
			b2 = b1_ - 1;
			b1 = b1_ - m_MinLen;
		} else {
			b2 -= 1;
			b1 -= 1;
		}
	}

	// create the first token
	Token<T> t(0,0,b0,0,0);
	typename std::vector<Token<T>>::iterator it = m_Tokens.begin();
	m_Tokens.insert(it,t);

	cerr << endl << std::flush;
}

// gather non-redundant bytes from the input buffer
template <class T>
void DC<T>::Sieve()
{
	m_Seed.clear();
	m_Lengths.clear();
	m_Gaps.clear();
	m_Distances.clear();

	int offset = 0;
	typename std::vector<Token<T>>::iterator it = m_Tokens.begin();
	for ( ; it != m_Tokens.end(); ++it) {
		m_Lengths.push_back(it->length); // pushing back an int into a uchar vector
		m_Gaps.push_back(it->gap); // same here
		m_Distances.push_back(it->distance);

		// skip redundant bytes
		if (it->length > 0) {
			offset += it->length;
		}

		// insert non-redundant bytes
		if (it->gap > 0) {
			for (int i = 0; i < it->gap; ++i) {
				m_Seed.push_back(m_Buffer.at(offset));
				offset++;
			}
		}
	}
}

template <class T>
void DC<T>::Stats()
{
	float ratio = ((float)m_Seed.size())/((float)m_Buffer.size());
	cerr << "File:        " << m_Filename << endl;
	cerr << "File size:   " << m_Buffer.size() << endl;
	cerr << "Seed size:   " << m_Seed.size() << endl;
	cerr << "Ratio:       " << setprecision(6) << ratio << endl;
   cerr << "Tokens:      " << m_Tokens.size() << endl;

	IAC<unsigned int> l,d,g;
	l.Load(m_Lengths);
	cerr << "Lengths size/entropy:  " << l.Size() << "/" << l.Entropy() << endl;
	d.Load(m_Distances);
	cerr << "Distances size/entropy:" << d.Size() << "/" << d.Entropy() << endl;
	g.Load(m_Gaps);
	cerr << "Gaps size/entropy:     " << g.Size() << "/" << g.Entropy() << endl;

	int total = m_Seed.size() + l.MinSize() + d.MinSize() + g.MinSize();
	cerr << "TOTAL SIZE:  " << total << endl;

	cerr << "Lengths   "; MinMax(m_Lengths);
	cerr << "Distances "; MinMax(m_Distances);
	cerr << "Gaps      "; MinMax(m_Gaps);
}

template <class T>
void DC<T>::Write()
{
	std::string filename(m_Filename);

	// Write m_Seed to file
	ofstream	out1(filename+".sd", ios::out | ios::binary);
	if (!out1) {
		cerr << "Cannot open " << filename << ".sd for writing.\n";
		return;
	}

	size_t sz = m_Seed.size();
	vector<unsigned char> seed;
	if (typeid(T) == typeid(unsigned char)) {
		seed.reserve(sz);
		for (T t : m_Seed) seed.push_back(t);
	} else {
		seed.reserve(sz/8);
		unsigned char c = 0;
		int i = 7;
		for (T t : m_Seed) {
			if (t) c |= (1 << i);
			--i;
			if (i < 0) {
				seed.push_back(c);
				c = 0;
				i = 7;
			}
		}
		seed.push_back(c);
	}

	out1.write((char *)&seed[0], seed.size());
	out1.close();
	if (!out1.good()) {
		cerr << "Error occurred while writing " << filename << ".sd" << endl;
		return;
	}

#ifdef DEBUG1
	// Write m_Distances to file
	ofstream	out2(filename+".dst", ios::out | ios::binary);
	if (!out2) {
		cerr << "Cannot open " << fname << ".sd for writing.\n";
		return;
	}

	out2.write((char *)&m_Distances[0], sizeof(int) * m_Distances.size());
	out2.close();
	if (!out2.good()) {
		cerr << "Error occurred while writing " << fname << ".dst" << endl;
		return;
	}
#endif
}

template <class T>
void DC<T>::Compress()
{
	m_Seed.clear();
	m_Lengths.clear();
	m_Distances.clear();
	m_Tokens.clear();
	RollUp();
	Sieve();
}

template <class T>
void DC<T>::Compress(string filename)
{
	Load(filename);
	Compress();
}

template <class T>
void DC<T>::Compress(vector<T>& buffer)
{
	Load(buffer);
	Compress();
}

template <class T>
void DC<T>::MinMax(vector<unsigned int>& buffer)
{
	unsigned int min = 0xFFFFFFFF;
	unsigned int max = 0;
	double avg = 0;
	for (unsigned int b : buffer) {
		if (b < min) min = b;
		if (b > max) max = b;
		avg += (double) b / (double) buffer.size();
	}

	cerr << "Min/Max/Avg: " << min << "/" << max << "/" << setprecision(6) << avg << endl;
}

template <class T>
void DC<T>::Build()
{
	size_t sz = m_Buffer.size();

	// determine the indexs if m_Buffer were sorted
	m_Indexes.resize(sz);
	size_t index(0);
	generate(m_Indexes.begin(), m_Indexes.end(), [&]{ return index++; });
	stable_sort(m_Indexes.begin(), m_Indexes.end(),
		[&](int i, int j) {
			if (m_Buffer.at(i) == m_Buffer.at(j)) {
				for (size_t k = 1; k < sz; ++k) {
					int _i = (i + k + sz) % sz;
					int _j = (j + k + sz) % sz;
					int l = m_Buffer.at(_i);
					int r = m_Buffer.at(_j);
					if (l == r) continue;
					else return (l < r);
				}
				return false;
			}
			else return m_Buffer.at(i) < m_Buffer.at(j);
		});

	// determine repeating patterns of length WORD_LEN
	unsigned int total = 0;
	for (size_t i = 0; i < m_Indexes.size()-1; ) {
		unsigned int count = 1;
		size_t j = i + 1;
		size_t len = m_WordLen;
		size_t x = m_Indexes.at(i);
		do {
			size_t y = m_Indexes.at(j);
			if (Equal(x,y,len)) {
				count++;
				j++;
				if (j >= sz) break;
			} else {
				if (count >= (unsigned int) m_MinCount) {
					Word<T> w(m_Buffer,x,len,count,i);
					m_Dictionary.push_back(w);
					total += count;
				}
				break;
			}
		} while (true);
		i = j;
	}

	// break if there is nothing to process further
	if (m_Dictionary.size() == 0)
		return;

	// sort the dictionary by index
	stable_sort(m_Dictionary.begin(), m_Dictionary.end(),
		[&](Word<T> l, Word<T> r) { return l.Index() < r.Index(); });

	// remove overlapping indexes
	for (size_t i = m_Dictionary.size()-1; i > 0; --i) {
		Word<T> f = m_Dictionary.at(i-1);
		Word<T> b = m_Dictionary.at(i);
		if (f.Index() + m_WordLen >= b.Index()) {
			// b should have less count than f because of the sort
			m_Dictionary.erase(m_Dictionary.begin()+i);
		}
	}

	// sort the dictionary by count, then truncate
	if (m_Dictionary.size() > (size_t)m_MaxCodes) {
		stable_sort(m_Dictionary.begin(), m_Dictionary.end(),
			[&](Word<T> l, Word<T> r) { return l.Count() < r.Count(); });
		m_Dictionary.erase(m_Dictionary.begin()+m_MaxCodes,m_Dictionary.end());
	}

	// sort the dictionary by word
	stable_sort(m_Dictionary.begin(), m_Dictionary.end(),
		[&](Word<T> l, Word<T> r) {
			vector<T>& lw = l.String();
			vector<T>& rw = r.String();
			if (lw.at(0) == rw.at(0)) {
				for (int k = 1; k < m_WordLen; ++k) {
					if (lw.at(k) == rw.at(k)) continue;
					else return lw.at(k) < rw.at(k);
				}
				return false;
			} else return lw.at(0) < rw.at(0); 
		});

	// get deltas between each word
	m_Dictionary.at(0).SetDelta(0);
	for (size_t i = 1; i < m_Dictionary.size(); ++i) {
		Word<T> w0 = m_Dictionary.at(i-1);
		Word<T> w1 = m_Dictionary.at(i);
		m_Dictionary.at(i).SetDelta(w0.String(),w1.String());
	}

	// assign codes, MAX_CODES can be derived from the following lines
	vector<T> code(m_CodeLen,(T)CODE_START);
	for (size_t i = 0; i < m_Dictionary.size(); ++i) {
		m_Dictionary.at(i).SetCode(code);
		size_t j = m_CodeLen - 1;
		code.at(j) = (T)((unsigned char)code.at(j)+1);
		for ( ; j > 1; --j) { // don't touch the prefix
			if (code.at(j) == 0xFF) {
				code.at(j-1) = (T)((unsigned char)code.at(j-1)+1);
				code.at(j) = CODE_START;
			}
		}
	}
}

// build a dictionary based on every string of length WORD_LEN
template <class T>
void DC<T>::Build2()
{
	size_t sz = m_Buffer.size();

	m_Indexes.resize(sz);
	size_t index(0);
	generate(m_Indexes.begin(), m_Indexes.end(), [&]{ return index++; });

	// parse buffer
	for (size_t i = 0; i < sz / m_WordLen; ++i) {
		size_t j = i * m_WordLen;
		Word<T> w(m_Buffer,j,m_WordLen,1,i);

		if (m_Dictionary.size() == 0) {
			m_Dictionary.push_back(w);
		} else {

			// binary search
			size_t l = 0;
			size_t r = m_Dictionary.size() - 1;
			size_t m = (l+r)/2;
			Word<T> wl = m_Dictionary.at(l);
			Word<T> wr = m_Dictionary.at(r);
			Word<T> pivot = m_Dictionary.at(m);
			while (wl < w && w < wr) {
				if (w <= pivot) {
					r = m;
				} else {
					l = m;
				}
				m = (l+r)/2;
				wl = m_Dictionary.at(l);
				wr = m_Dictionary.at(r);
				pivot = m_Dictionary.at(m);
				if (m == l || m == r) break;
			}

			// insert or add new word in dictionary
			if (w < wl) {
				m_Dictionary.insert(m_Dictionary.begin()+l,w);
			} else if (w == wl) {
				m_Dictionary.at(l).Link(j);
			} else if (wl < w && w < wr) {
				m_Dictionary.insert(m_Dictionary.begin()+r,w);
			} else if (w == wr) {
				m_Dictionary.at(r).Link(j);
			} else if (wr < w) {
				m_Dictionary.push_back(w);
			}
		}
	}

	// sort dictionary by word
	stable_sort(m_Dictionary.begin(), m_Dictionary.end(),
		[&](Word<T> l, Word<T> r) { return l < r; });

	// get deltas between each word
	m_Dictionary.at(0).SetDelta(0);
	for (size_t i = 1; i < m_Dictionary.size(); ++i) {
		Word<T> w0 = m_Dictionary.at(i-1);
		Word<T> w1 = m_Dictionary.at(i);
		m_Dictionary.at(i).SetDelta(w0.String(),w1.String());
	}

	// assign codes to each word
	vector<T> code(m_CodeLen,(T)CODE_START);
	for (size_t i = 0; i < m_Dictionary.size(); ++i) {
		m_Dictionary.at(i).SetCode(code);
		size_t j = m_CodeLen - 1;
		code.at(j) = (T)((unsigned char)code.at(j)+1);
		for ( ; j > 0; --j) {
			if (code.at(j) > CODE_END) {
				code.at(j-1) = (T)((unsigned char)code.at(j-1)+1);
				code.at(j) = CODE_START;
			}
		}
	}
}

// extract words delimited by non-alphanumerics from the buffer
template <class T>
void DC<T>::Parse()
{
	m_Dictionary.clear();
	size_t sz = m_Buffer.size();

	// parse buffer
	int len = 0;
	for (size_t i = 0; i <= sz; ++i) {
		unsigned char c = i < sz ? (unsigned char)m_Buffer.at(i) : 0;
		if (isalpha(c) || isdigit(c)) {
			len++;
		} else {
			if (len > 1) { // must be at least two characters
				Word<T> w(m_Buffer,i-len,len,1,i-len);

				if (m_Dictionary.size() == 0) {
					m_Dictionary.push_back(w);
					continue;
				}

				// check if this word is already in the dictionary
				size_t l = 0;
				size_t r = m_Dictionary.size() - 1;
				size_t m = (l+r)/2;
				Word<T> wl = m_Dictionary.at(l);
				Word<T> wr = m_Dictionary.at(r);
				Word<T> pivot = m_Dictionary.at(m);
				while (wl < w && w < wr) {
					if (w <= pivot) {
						r = m;
					} else {
						l = m;
					}
					m = (l+r)/2;
					wl = m_Dictionary.at(l);
					wr = m_Dictionary.at(r);
					pivot = m_Dictionary.at(m);
					if (m == l || m == r) break;
				}

				// insert or add new word in the dictionary
				if (w < wl) {
					m_Dictionary.insert(m_Dictionary.begin()+l,w);
				} else if (w == wl) {
					m_Dictionary.at(l).Link(i-len);
				} else if (wl < w && w < wr) {
					m_Dictionary.insert(m_Dictionary.begin()+r,w);
				} else if (w == wr) {
					m_Dictionary.at(r).Link(i-len);
				} else if (wr < w) {
					m_Dictionary.push_back(w);
				}
			}
			len = 0;
		}
	}

	// assign codes to each word, code is the sorted version of the string
	for (size_t i = 0; i < m_Dictionary.size(); ++i) {
		vector<T> w = m_Dictionary.at(i).String();
		stable_sort(w.begin(),w.end());
		m_Dictionary.at(i).SetCode(w);
	}

	// sort dictionary by code
	stable_sort(m_Dictionary.begin(), m_Dictionary.end(),
		[&](Word<T> l, Word<T> r) { return Word<T>::IsLess(l.Code(),r.Code()); });

	// erase words with repeated codes
	Word<T> w1 = m_Dictionary.at(m_Dictionary.size()-1);
	for (int i = m_Dictionary.size()-2; i >= 0; --i) {
		Word<T> w0 = m_Dictionary.at(i);
		if (Word<T>::AreEqual(w0.Code(),w1.Code())) {
			if (w0.Count() < w1.Count()) {
				m_Dictionary.erase(m_Dictionary.begin()+i);
			} else {
				m_Dictionary.erase(m_Dictionary.begin()+i+1);
				w1 = w0;
			}
		} else {
			w1 = w0;
		}
	}

	// sort dictionary by word
	stable_sort(m_Dictionary.begin(), m_Dictionary.end(),
		[&](Word<T> l, Word<T>r) { return l < r; });

	// get deltas between each word
	m_Dictionary.at(0).SetDelta(0);
	for (size_t i = 1; i < m_Dictionary.size(); ++i) {
		Word<T> w0 = m_Dictionary.at(i-1);
		Word<T> w1 = m_Dictionary.at(i);
		m_Dictionary.at(i).SetDelta(w0.String(),w1.String());
	}
}

// replace words in the buffer with codes
template <class T>
void DC<T>::Replace()
{
	size_t sz = m_Buffer.size();
	vector<T> v = m_Buffer;

	// replace words with codes, followed by zeros temporarily
	for (Word<T> w : m_Dictionary) {
		vector<T>& code = w.Code();
		vector<T>& word = w.String();
		for (size_t r : w.GetRefs()) {
			for (size_t i = 0; i < word.size(); ++i) {
				v.at((r+i+sz)%sz) = i < code.size() ? code.at(i) : 0;
			}
		} 
	}

	// remove all zeros following the codes
	m_Coded.clear();
	m_Coded.reserve(sz);
	for (T t : v) {
		if (t > 0) m_Coded.push_back(t);
	} 
}

template <class T>
void DC<T>::Debug()
{
	cout << "m_Buffer: { " << endl;
	for (T t : m_Buffer) {
		if (typeid(t) == typeid(bool)) cout << (t ? "1" : "0");
		else cout << t;
	}
	cout << " }\n";

	cout << "m_Tokens (index,length,distance,gap,bpos,fpos): {\n";
	for (int i = 0; i < (int)m_Tokens.size(); ++i) {
		Token<T> t = m_Tokens[i];
		cout << i << "," << t.length << "," << t.distance << "," << t.gap << ",";
		cout << t.bpos << "," << t.fpos << ",";
		for (T p : t.pattern) {
			if (typeid(p) == typeid(bool)) cout << (p ? "1" : "0");
			else if ((int)p > 126) cout << "~";
			else cout << p;
		}
		cout << endl;
	}
	cout << "}\n";

	cout << "m_Lengths: {\n";
	for (unsigned int i : m_Lengths) {
		cout << i << ",";
	}
	cout << " }\n";

	cout << "m_Distances: {\n";
	for (unsigned int i : m_Lengths) {
		cout << i << ",";
	}
	cout << " }\n";

	cout << "m_Gaps: {\n";
	for (unsigned int i : m_Lengths) {
		cout << i << ",";
	}
	cout << " }\n";

	size_t total = 0;
	for (Word w : m_Dictionary)
		total += w.Count();

	cerr << "m_Dictionary(" << m_Dictionary.size() << "," << total  <<"): {\n";
	cout << "m_Dictionary(" << m_Dictionary.size() << "," << total  <<"): {\n";
	cout << "Count : Index  : Sort   : Delta    : Code     : Word\n";
	for (Word w : m_Dictionary) {
		cout << setw(5) << w.Count() << " : " << setw(6) << w.Index() << " : "
			<< setw(6) << w.SortIndex() << " : " << setw(8) << w.Delta() << " : ";
		for (unsigned char c : w.Code())	cout << (int)c;
		cout << " : ";
		for (unsigned char c : w.String())
			if (c == '\n') cout << "\\n";
			else cout << c;
		cout << endl;
	}
	cout << " }\n";

	cerr << "m_Coded(" << m_Coded.size() << "): {\n";
	cout << "m_Coded(" << m_Coded.size() << "): {\n";
	for (T t : m_Coded) {
		unsigned char c = (unsigned char) t;
		if (c == '\t') cout << "\\t";
		else if (c == '\n') cout << "\\n";
		else if (c >= ' ') cout << c;
		else cout << (int)c;
	}
	cout << " }\n";
}

