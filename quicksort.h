#ifndef __QUICKSORT_H__
#define __QUICKSORT_H__

#include <vector>
#include "iac.h"

using namespace std;

template <typename T>
class Quicksort
{
public:
	Quicksort();
	Quicksort(vector<T> &buffer);
	virtual ~Quicksort();

	vector<T>& GetSorted() { return m_Sorted; }
	vector<T>& GetDeltas() { return m_Deltas; }
	vector<bool>& GetFlips() { return m_Flips; }
	vector<unsigned char> GetFlipsInBytes(); 

	void Load(vector<T> &buffer);
	void Load(std::string filename);
	void Prepare();
	void Presort();
	void Sort();
	void Unsort();
	void Delta();
	void Debug();
	void Write(std::string filename);

protected:
	void Distribute();
	bool Sorted(vector<vector<T>> &v);
	bool Sorted(vector<T> &v);
	void ToFlips(vector<vector<vector<bool>>> &v);
	void ToSorted(vector<vector<T>> &v);
	T GetPivot(int partitions, int segment);
	size_t GetBoundary(T pivot);

private:
	int m_MaxSteps, m_ActualSteps;
	std::string m_Filename;
	vector<T> m_Unsorted, m_Presorted, m_Sorted, m_Deltas;
	vector<Tuple<T>> m_Distribution;
	vector<bool> m_Flips, m_Reverses;
	vector<unsigned int> m_Runs; // of ascending or descending elements
};

#endif /* __QUICKSORT_H__ */
