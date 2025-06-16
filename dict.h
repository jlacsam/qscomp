#ifndef __DICT_H__
#define __DICT_H__

#include <vector>
#include <cstdint>

using namespace std;

#define MIN_LEN 3
#define MAX_LEN 64
#define MAX_DST1 1048576
#define MAX_DST2 1048576
#define COST_DST1 1
#define COST_DST2 65
#define WORD_LEN 8
#define CODE_LEN 3
#define CODE_START 0x01
#define CODE_END 0x08
#define MAX_CODES 65536 // codes limited by the allowable prefix and sorted numbers					
#define MIN_COUNT 1 // replace string when it repeats this number of times

template <typename T>
struct Token
{
	Token() { length = distance = gap = bpos = fpos = 0; }
	Token(int l, int d, int g, int b, int f)
	{
		length = l;
		distance = d;
		gap = g;
		bpos = b;
		fpos = f;
	}

	int length;   // Length - 1 actually
	int distance; // Distance between tail of "front" and head of "back" - 1
	int gap;      // Number of literal bytes between this token and the previous one
	int bpos;     // Position/index of "back" pattern
	int fpos;     // Position/index of "front" pattern
	vector<T> pattern;
};

template <typename W>
class Word 
{
public:
	Word() { count = 0; code.clear(); word.clear(); refs.clear(); }
	Word(vector<W>& v, int s, int len, size_t c, size_t si) {
		word.clear();
		code.clear();
		refs.clear();
		count = c;
		index = s;
		sort_index = si;
		size_t sz = v.size();
		for (int i = 0; i < len; ++i) {
			int j = (s + i + sz) % sz;
			word.push_back(v.at(j));
		}
		refs.push_back(s);
	}
	void SetCode(W c0, W c1, W c2) {
		code.clear();
		code.push_back(c0);
		code.push_back(c1);
		code.push_back(c2);
	}
	void SetCode(vector<W> v) { code = v; }
	vector<W>& Code() { return code; }
	vector<W>& String() { return word; }
	size_t Count() { return count; }
	size_t Index() { return index; }
	size_t SortIndex() { return sort_index; }
	uint64_t Delta() { return delta; }
	void SetDelta(uint64_t d) { delta = d; }
	void SetDelta(vector<W>& a, vector<W>& b) {
		uint64_t x = vtoi(a), y = vtoi(b);
		delta = y - x; // y must be greater or equal to x, dictionary is sorted anyway	
	}
	void Link(size_t r) { refs.push_back(r); count++; }
	vector<size_t>& GetRefs() { return refs; }

	bool operator< (Word<W>& rhs) const {
		vector<W>& rw = rhs.String();
		if (word.size() == 0 && rw.size() == 0) return false;
		else if (word.size() == 0 && rw.size() > 0) return true;
		else if (word.at(0) == rw.at(0)) {
			size_t min = word.size() < rw.size() ? word.size() : rw.size();
			for (size_t i = 1; i < min; ++i) {
				if (word.at(i) == rw.at(i)) continue;
				else return word.at(i) < rw.at(i);
			}
			return word.size() < rw.size();
		} else { return word.at(0) < rw.at(0); }
	}

	bool operator<= (Word<W>& rhs) const {
		vector<W>& rw = rhs.String();
		if (word.size() == 0 && rw.size() >= 0) return true;
		else if (word.size() > 0 && rw.size() == 0) return false;
		else {
			size_t min = word.size() <= rw.size() ? word.size() : rw.size();
			for (size_t i = 0; i < min; ++i) {
				if (word.at(i) == rw.at(i)) continue;
				else return word.at(i) < rw.at(i);
			}
			return word.size() <= rw.size();
		}
	}

	bool operator== (Word<W>& rhs) const {
		vector<W>& rw = rhs.String();
		if (word.size() != rw.size()) return false;
		else if (word.size() == 0) return true;
		else {
			for (size_t i = 0; i < word.size(); ++i)
				if (word.at(i) != rw.at(i)) return false;
		}
		return true;
	}

	void operator++ (int) {
		count++;
	}

	static bool IsLess(vector<W>& l, vector<W>& r) {
		if (l.size() == 0 && r.size() == 0) return false;
		else if (l.size() == 0 && r.size() > 0) return true;
		else if (l.at(0) == r.at(0)) {
			size_t min = l.size() < r.size() ? l.size() : r.size();
			for (size_t i = 1; i < min; ++i) {
				if (l.at(i) == r.at(i)) continue;
				else return l.at(i) < r.at(i);
			}
			return l.size() < r.size();
		} else { return l.at(0) < r.at(0); }
	}

	static bool AreEqual(vector<W>& l, vector<W>& r) {
		if (l.size() != r.size()) return false;
		else if (l.size() == 0) return true;
		else {
			for (size_t i = 0; i < l.size(); ++i)
				if (l.at(i) != r.at(i)) return false;
		}
		return true;
	}

protected:
	uint64_t vtoi(vector<W>& v) {
		uint64_t u = 0;
		size_t sz = v.size();
		for (size_t i = 0; i < sz; ++i) { // no check for overflow as long as WORD_LEN <= 8
			u |= v.at(i) << (8 * (sz-i-1));
		}
		return u;
	}

private:
	vector<W> code;
	vector<W> word;
	vector<size_t> refs;
	uint64_t delta;
	int count;
	int index, sort_index;
};

template <typename T>
class DC
{
public:
	DC();
	virtual ~DC();

	void SetMinLen(int l) { m_MinLen = l; }
	void SetMaxLen(int l) { m_MaxLen = l; m_CostDst2 = l+1; }
	void SetMaxDist(int d) { m_MaxDst1 = m_MaxDst2 = d; }
	void SetMaxDst1(int d) { m_MaxDst1 = d; }
	void SetMaxDst2(int d) { m_MaxDst2 = d; }
	void SetMaxCost1(int c) { m_CostDst1 = c; }
	void SetMaxCost2(int c) { m_CostDst2 = c; }
	void SetWordLen(int l) {
		m_WordLen = l;
		if (m_CodeLen > l) {
			m_CodeLen = l;
			UpdateMaxCodes();
		}
	};
	void SetCodeLen(int l) {
		if (l <= m_WordLen) {
			m_CodeLen = l;
			UpdateMaxCodes();
		}
	};
	void SetMinCount(int c) { m_MinCount = c; }

	void Stats();
	void Write();
	void Load(string filename);
	void Load(vector<T>& buffer);
	void Compress(string fname);
	void Compress(vector<T>& buffer);
	void Build();
	void Build2();
	void Parse();
	void Replace();
	void Unreplace();
	void Debug();

	size_t Size() { return m_Buffer.size(); }
	vector<T>& GetBuffer() { return m_Buffer; }
	vector<T>& GetCoded() { return m_Coded; }
	vector<unsigned int>& GetLengths() { return m_Lengths; }
	vector<unsigned int>& GetGaps() { return m_Gaps; }
	vector<unsigned int>& GetDistances() { return m_Distances; }

protected:
	void RollUp();
	void Sieve();
	void Compress();
	bool TooFar(int f1, int f2, int b1, int b2) {
		int distance = b1 - f2 - 1;
		int savings = b2 - b1 + 1;
		int cost = distance < m_MaxDst1 ? m_CostDst1 :
			distance < m_MaxDst2 ? m_CostDst2 : m_MaxLen + 1;
		return cost > savings;
	};
	bool TooLong(int b1, int b2) { return (b2-b1+1) > m_MaxLen; }
	bool Identical(int f1, int f2, int b1, int b2) {
		if ((f2-f1) != (b2-b1)) return false;
		for (int i = f1, j = b1; i <= f2; i++, j++) {
			if (m_Buffer[i] != m_Buffer[j]) return false;
		}
		return true;
	};
	bool Equal(int i, int j, int len) {
		size_t sz = m_Buffer.size();
		for (int k = 0; k < len; ++k) {
			int _i = (i + k + sz) % sz;
			int _j = (j + k + sz) % sz;
			if (m_Buffer[_i] != m_Buffer[_j]) return false;
		}
		return true;
	};
	bool Equal(vector<T>& w, int i, int len) {
		size_t sz = m_Buffer.size();
		for (int k = 0; k < len; ++k) {
			int _i = (i + k + sz) % sz;
			if (m_Buffer[_i] != w[k]) return false;
		}
		return true;
	};
	void StorePattern(Token<T> &t, int b1_, int b2_);
	void MinMax(vector<unsigned int>& buffer);
	void UpdateMaxCodes() {
		m_MaxCodes = CODE_END - CODE_START + 1;
		for (int i = 0; i < m_CodeLen; ++i) m_MaxCodes *= m_MaxCodes;
	};

private:
	int m_MinLen;
	int m_MaxLen;
	int m_MaxDst1;
	int m_MaxDst2;
	int m_CostDst1;
	int m_CostDst2;
	int m_CodeLen, m_WordLen, m_MaxCodes, m_MinCount;
	string m_Filename;

	// Main vectors used by the compression algorithm
	vector<T> m_Buffer;						// Input buffer
	vector<T> m_Coded, m_Seed;				// Output buffers
	vector<size_t> m_Indexes;				// Indexes when buffer is sorted
	vector<unsigned int> m_Lengths;		// Lengths of repeating patterns
	vector<unsigned int> m_Gaps;			// Lengths of non-repeating patters
	vector<unsigned int> m_Distances;	// Distances of repeating patterns
	vector<Token<T>> m_Tokens;
	vector<Word<T>> m_Dictionary;
};

#endif /* __DICT_H__ */
