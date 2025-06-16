#ifndef __BWT_H__
#define __BWT_H__

#include <vector>

using namespace std;

class BWT
{
public:
	BWT();
	BWT(vector<unsigned char> &buffer);
	virtual ~BWT();

	void Load(vector<unsigned char> &buffer);
	void Load(string filename);
	void Transform();
	void Stats();
	void Debug();
	void Invert();
	void Check();

	vector<unsigned char>& GetBWT() { return m_BWT; }
	vector<unsigned char>& GetRaw() { return m_Unsorted; }
	size_t Size() { return m_Unsorted.size(); }
	vector<unsigned char> Substring (unsigned char c);

protected:

private:
	string m_Filename;
	size_t m_Front, m_Back;
	vector<unsigned char> m_Unsorted, m_BWT, m_Inverted;
	vector<size_t> m_Indexes;
};

#endif /* __BWT_H__ */
