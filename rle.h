#ifndef __RLE_H__
#define __RLE_H__

#include <vector>
#include <list>

using namespace std;

template <typename T,typename U>
class RLE
{
public:
	RLE();
	virtual ~RLE();

	vector<U>& GetRLE() { return m_RLE; }
	vector<unsigned char>& GetBRLE() { return m_BRLE; }
	vector<U>& GetMTF() { return m_MTF; }
	vector<U> GetOdd(vector<T>& v);
	vector<U> GetEven(vector<T>& v);
	
	void MTF(vector<T>& v);
	void Encode(vector<T>& v);
	void EncodeZ(vector<T>& v);
	void BRLE(vector<bool>& v);
	void UnBRLE(vector<T>& v, size_t sz = 0);
	void Debug();

protected:

private:
	vector<U> m_RLE, m_MTF;
	vector<unsigned char> m_BRLE;
	vector<bool> m_UnBRLE;
};

#endif /* __RLE_H__ */
