#ifndef __IAC_H__
#define __IAC_H__

#include <vector>

using namespace std;

template <typename T>
struct Tuple
{
	Tuple() { low = high = 0; freq = cumf = 0; prob = cumm = 0; }
	Tuple(T l, T h)
	{
		low = l;
		high = h;
		freq = 0;
		prob = 0;
		cumm = 0;
		cumf = 0;
	}
	Tuple(T l, T h, unsigned int f, unsigned int c) {
		low = l; high = h; freq = f; cumf = c; prob = 1; cumm = 1;
	}

	T low;
	T high;
	unsigned int freq;
	unsigned int cumf;
	double prob;
	double cumm;
};

template <typename T>
class IAC
{
public:
	IAC();
	virtual ~IAC();

	void Load(vector<T>& buffer);
	double AvgBits(); // Average number of bits to store each element in the vector
	double Entropy();
	size_t MinSize(); // Average bits X size of the vector
	size_t Size() { return m_Buffer.size(); }
	void Show();
	void Debug();
	vector<Tuple<T>> GetDistrib() { return m_Distribution; }

protected:
	void Distribute();

private:
	vector<T> m_Buffer;
	vector<T> m_Output;
	vector<Tuple<T>> m_Distribution;
};

#endif /* __IAC_H__ */
