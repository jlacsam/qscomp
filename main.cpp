#include <iostream>
#include "bwt.h"
#include "quicksort.h"
#include "iac.h"
#include "rle.h"
#include "prep.h"
#include "dict.h"

void test1(std::string filename);
void test2(std::string filename);
void test3(std::string filename, int wordlen = WORD_LEN);
void test4(std::string filename, int wordlen = WORD_LEN);
void test5(std::string filename);
void test6(std::string filename);
void test7(std::string filename);

int main (int argc, char *argv[])
{
	if (argc < 2)
	{
		cout << "comp9 <file>\n";
		cout << "Text file compression using a combination and extension of known algorithms.\n";
		cout << "Author: Jose Antonio Lacsam\n";
		cout << "2021 August 26\n";
		return -1;
	}

	test1(string(argv[1]));
	//test2(string(argv[1]));
	//test3(string(argv[1]),atoi(argv[2]));
	//test4(string(argv[1]),atoi(argv[2]));
	//test5(string(argv[1]));
	//test6(string(argv[1]));
	//test7(string(argv[1]));

	return 0;
}

void test1(std::string filename)
{
	cerr << "BWT on unsigned char input file, then Quicksort Transform.\n";
	cerr << "-------------------------------------------------------------------------------\n";

	Prep prep;
	prep.Load(filename);
	prep.Swap();
	cerr << "Loaded from file " << filename << ". " << prep.Size() << " bytes.\n";
	prep.Debug();

	IAC<unsigned char> iap;
	iap.Load(prep.GetPrepped());
	iap.Show();
	iap.Debug();

	BWT bwt;
	bwt.Load(prep.GetPrepped());
	bwt.Transform();
	bwt.Stats();

	// BWT validations
	bwt.Invert();
	bwt.Check();
	bwt.Debug();

	Quicksort<unsigned char> qst;
	qst.Load(bwt.GetBWT());
	qst.Prepare();
	qst.Sort();
	qst.Delta();
	qst.Debug();
	qst.Write(filename + ".flp");

	RLE<unsigned char,unsigned int> rle;
	rle.Encode(qst.GetDeltas());
	rle.Debug();

	cerr << "RLE of Deltas>>\n";
	IAC<unsigned int> iac;
	iac.Load(rle.GetRLE());
	iac.Show();
	iac.Debug();

	cerr << "RLE of Flips>>\n"; // Best encoding so far, but not good enough
	RLE<bool,unsigned int> brle;
	brle.Encode(qst.GetFlips());
	brle.Debug();

	iac.Load(brle.GetRLE());
	iac.Show();
	iac.Debug();
/*
	cerr << "Examine entropies if the odd and even sequences are separated.\n";
	RLE<unsigned int,unsigned int> xrle;
	vector<unsigned int> odd = xrle.GetOdd(brle.GetRLE());
	vector<unsigned int> even = xrle.GetEven(brle.GetRLE());

	iac.Load(odd);
	iac.Show();
	iac.Load(even);
	iac.Show();

	DC<bool> dc;
	dc.SetMinLen(16);
	dc.SetMaxLen(65535);
	dc.SetMaxDist(4096);
	dc.Compress(qst.GetFlips());
	dc.Stats();
	dc.Debug();

	cerr << "BRLE of Flips (uchar)>>>\n";
	RLE<bool,unsigned char> brleuc;
	brleuc.BRLE(qst.GetFlips());
	brleuc.Debug();

	IAC<unsigned char> iacuc;
	iacuc.Load(brleuc.GetBRLE());
	iacuc.Show();
	iacuc.Debug();

	cerr << "RLE of Flips (uchar)>>>\n";
	brleuc.Encode(qst.GetFlips());
	brleuc.Debug();

	iacuc.Load(brleuc.GetRLE());
	iacuc.Show();
	iacuc.Debug();

	cerr << "RLE of BWT>>>\n";
	iacuc.Load(bwt.GetBWT());
	iacuc.Show();
	iacuc.Debug();
*/
}

void test2(std::string filename)
{
	cerr << "PREP > BWT > MTF > RLEZ\n";
	cout << "-------------------------------------------------------------------------------\n";

	Prep prep;
	prep.Load(filename);
	cerr << "Loaded from file " << filename << ". " << prep.Size() << " bytes.\n";
	prep.Debug();

	BWT bwt;
	bwt.Load(prep.GetPrepped());
	bwt.Transform();
	bwt.Stats();
	bwt.Debug();

	RLE<unsigned char,unsigned char> mtf;
	mtf.MTF(bwt.GetBWT());
	mtf.Debug();

	RLE<unsigned char,unsigned int> rle;
	rle.EncodeZ(mtf.GetMTF());
	rle.Debug();

	IAC<unsigned int> iac;
	iac.Load(rle.GetRLE());
	iac.Show();
	iac.Debug();
/*
	cerr << "BWT > MTF > QST > RLE\n";
	cout << "-------------------------------------------------------------------------------\n";

	Quicksort<unsigned char> qst;
	qst.Load(mtf.GetMTF());
	qst.Prepare();
	qst.Sort();
	qst.Delta();
	qst.Debug();

	cerr << "RLE of Flips>>\n"; // Best encoding so far, but not good enough
	RLE<bool,unsigned int> brle;
	brle.Encode(qst.GetFlips());
	brle.Debug();

	iac.Load(brle.GetRLE());
	iac.Show();
	iac.Debug();
*/
}

void test3(std::string filename, int wordlen)
{
	cerr << "DICT > BWT > QST > RLE\n";
	cerr << "-------------------------------------------------------------------------------\n";

	DC<unsigned char> dc;
	dc.Load(filename);
	cerr << "Loaded from file " << filename << ". " << dc.Size() << " bytes.\n";
	dc.SetWordLen(wordlen);
	dc.SetCodeLen(3);
	dc.SetMinCount(2);
	dc.Build(); 
	dc.Replace();
	dc.Debug();

	BWT bwt;
	bwt.Load(dc.GetCoded());
	bwt.Transform();
	bwt.Stats();
	bwt.Debug();

	Quicksort<unsigned char> qst;
	qst.Load(bwt.GetBWT());
	qst.Prepare();
	qst.Sort();
	qst.Delta();
	qst.Debug();

	RLE<unsigned char,unsigned int> rle;
	rle.Encode(qst.GetDeltas());
	rle.Debug();

	cerr << "RLE of Deltas>>\n";
	IAC<unsigned int> iac;
	iac.Load(rle.GetRLE());
	iac.Show();
	iac.Debug();

	cerr << "RLE of Flips>>\n"; // Best encoding so far, but not good enough
	RLE<bool,unsigned int> brle;
	brle.Encode(qst.GetFlips());
	brle.Debug();

	iac.Load(brle.GetRLE());
	iac.Show();
	iac.Debug();
}

void test4(std::string filename, int wordlen)
{
	cerr << "DICT > QST > RLE\n";
	cerr << "-------------------------------------------------------------------------------\n";

	DC<unsigned char> dc;
	dc.Load(filename);
	cerr << "Loaded from file " << filename << ". " << dc.Size() << " bytes.\n";
	dc.SetWordLen(wordlen);
	dc.SetCodeLen(3);
	dc.SetMinCount(10);
	dc.Build(); 
	dc.Replace();
	dc.Debug();

	Quicksort<unsigned char> qst;
	qst.Load(dc.GetCoded());
	qst.Prepare();
	qst.Sort();
	qst.Delta();
	qst.Debug();

	RLE<unsigned char,unsigned int> rle;
	rle.Encode(qst.GetDeltas());
	rle.Debug();

	cerr << "RLE of Deltas>>\n";
	IAC<unsigned int> iac;
	iac.Load(rle.GetRLE());
	iac.Show();
	iac.Debug();

	cerr << "RLE of Flips>>\n"; // Best encoding so far, but not good enough
	RLE<bool,unsigned int> brle;
	brle.Encode(qst.GetFlips());
	brle.Debug();

	iac.Load(brle.GetRLE());
	iac.Show();
	iac.Debug();
}

void test5(std::string filename)
{
	cerr << "BWT > BWT > QST > BRLE.\n";
	cerr << "-------------------------------------------------------------------------------\n";

	BWT bwt;
	bwt.Load(filename);
	bwt.Transform();
	bwt.Stats();

	BWT bwt2;
	bwt2.Load(bwt.GetBWT());
	bwt2.Transform();
	bwt2.Stats();

	Quicksort<unsigned char> qst;
	qst.Load(bwt2.GetBWT());
	qst.Prepare();
	qst.Sort();
	qst.Delta();
	qst.Debug();

	RLE<unsigned char,unsigned int> rle;
	rle.Encode(qst.GetDeltas());
	rle.Debug();

	cerr << "RLE of Deltas>>\n";
	IAC<unsigned int> iac;
	iac.Load(rle.GetRLE());
	iac.Show();
	iac.Debug();

	cerr << "RLE of Flips>>\n"; // Best encoding so far, but not good enough
	RLE<bool,unsigned int> brle;
	brle.Encode(qst.GetFlips());
	brle.Debug();

	iac.Load(brle.GetRLE());
	iac.Show();
	iac.Debug();
}

void test6(std::string filename)
{
	cerr << "Swap > Trim > BWT > QST > BRLE\n";
	cerr << "-------------------------------------------------------------------------------\n";

	Prep prep;
	prep.Load(filename);
	prep.Swap();
	prep.Trim();
	cerr << "Loaded from file " << filename << ". " << prep.Size() << " bytes.\n";
	prep.Debug();

	IAC<unsigned char> iap;
	iap.Load(prep.GetPrepped());
	iap.Show();
	iap.Debug();

	BWT bwt;
	bwt.Load(prep.GetPrepped());
	bwt.Transform();
	bwt.Stats();

	// BWT validations
	bwt.Invert();
	bwt.Check();
	bwt.Debug();

	Quicksort<unsigned char> qst;
	qst.Load(bwt.GetBWT());
	qst.Prepare();
	qst.Sort();
	qst.Delta();
	qst.Debug();

	RLE<unsigned char,unsigned int> rle;
	rle.Encode(qst.GetDeltas());
	rle.Debug();

	cerr << "RLE of Deltas>>\n";
	IAC<unsigned int> iac;
	iac.Load(rle.GetRLE());
	iac.Show();
	iac.Debug();

	cerr << "RLE of Flips>>\n"; // Best encoding so far, but not good enough
	RLE<bool,unsigned int> brle;
	brle.Encode(qst.GetFlips());
	brle.Debug();

	iac.Load(brle.GetRLE());
	iac.Show();
	iac.Debug();
}

void test7(std::string filename)
{
	cerr << "DICT.Parse > BWT > QST > RLE\n";
	cerr << "-------------------------------------------------------------------------------\n";

	DC<unsigned char> dc;
	dc.Load(filename);
	cerr << "Loaded from file " << filename << ". " << dc.Size() << " bytes.\n";
	dc.Parse(); 
	dc.Replace();
	dc.Debug();

	BWT bwt;
	bwt.Load(dc.GetCoded());
	bwt.Transform();
	bwt.Stats();
	bwt.Debug();

	Quicksort<unsigned char> qst;
	qst.Load(bwt.GetBWT());
	qst.Prepare();
	qst.Sort();
	qst.Delta();
	qst.Debug();

	RLE<unsigned char,unsigned int> rle;
	rle.Encode(qst.GetDeltas());
	rle.Debug();

	cerr << "RLE of Deltas>>\n";
	IAC<unsigned int> iac;
	iac.Load(rle.GetRLE());
	iac.Show();
	iac.Debug();

	cerr << "RLE of Flips>>\n"; // Best encoding so far, but not good enough
	RLE<bool,unsigned int> brle;
	brle.Encode(qst.GetFlips());
	brle.Debug();

	iac.Load(brle.GetRLE());
	iac.Show();
	iac.Debug();
}
