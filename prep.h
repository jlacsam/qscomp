#ifndef __PREP_H__
#define __PREP_H__

#include <vector>
#include <string>

using namespace std;

class Prep
{
public:
	Prep();
	virtual ~Prep();
	vector<unsigned char>& GetPrepped() { return m_Prepped; }
	void Load(string filename);
	void RLE(vector<unsigned char>& buffer);
	void Swap();
	void Trim();
	size_t Size() { return m_Prepped.size(); }
	void Debug();

protected:

private:
	vector<unsigned char> m_Prepped;
};

#endif /* __PREP_H__ */
