#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <bitset>
#include <vector>

#define WITH_TLB true
std::ofstream out("F:\\345462662.txt");

class BitMap
{
public:
	BitMap()
	{
		for (int i = 0; i < 32; ++i) BM[i] = 0;

		mask[0] = 0x80000000;
		for (int i = 1; i < 32; ++i)
			mask[i] = mask[i - 1] / 2;
	}

	void setBitMap(int pos)
	{
		BM[pos / 32] |= mask[pos % 32];
	}

	int findFreeFrame(bool PT = false)
	{
		for (int i = 0; i < 1; ++i)
		{
			std::bitset<32> bit = BM[i];
			for (int j = 31; j >= (PT == false ? 0 : 1); --j)
			{
				if (PT == false)
					if (bit[j] == 0)
						return i * 32 + 31 - j;

				if (PT == true)
					if (bit[j] == 0 && bit[j - 1] == 0)
						return i * 32 + 31 - j;
			}
		}
		return -1;
	}

private:
	unsigned int mask[32];
	int BM[32];
};

class TLBuffer
{
public:
	TLBuffer()
	{
		for (int i = 0; i < 4; ++i)
		{
			LRU[i] = i;
			sp[i] = f[i] = -1;
		}
	}

	int findMatch(unsigned int sp)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (this->sp[i] == sp)
			{
				out << "h ";
				return i;
			}
		}
		out << "m ";
		return -1;
	}

	int hit(int line, unsigned int w)
	{
		for (int i = 0; i < 4; ++i)
			if (LRU[i] > LRU[line]) --LRU[i];
		LRU[line] = 3;
		return f[line] + w; // PA
	}

	void miss(unsigned int sp, unsigned int f)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (--LRU[i] == -1)
			{
				LRU[i] = 3;
				this->sp[i] = sp;
				this->f[i] = f;
			}
		}
	}

private:
	unsigned int LRU[4];
	unsigned int sp[4];
	unsigned int f[4];
};

void getSPW(unsigned int& sp, unsigned int& s, 
	unsigned int& p, unsigned int& w, unsigned int VA)
{
	sp = VA << 4 >> 13;
	s = VA << 4 >> 23;
	p = VA << 13 >> 22;
	w = VA << 23 >> 23;
}

void read(unsigned int* PM, TLBuffer& TLB, unsigned int VA)
{
	unsigned int sp, s, p, w;
	getSPW(sp, s, p, w, VA);
	int line = (WITH_TLB) ? TLB.findMatch(sp) : -1;

	if (line == -1)
	{
		if (PM[s] == -1 || PM[PM[s] + p] == -1)
			out << "pf ";
		else if (PM[s] == 0 || PM[PM[s] + p] == 0)
			out << "err ";
		else
		{
			if (WITH_TLB) TLB.miss(sp, PM[PM[s] + p]);
			out << PM[PM[s] + p] + w << ' ';
		}
	}
	else out << TLB.hit(line, w) << ' ';
}

void write(unsigned int* PM, BitMap& BM, TLBuffer& TLB, unsigned int VA)
{
	unsigned int sp, s, p, w;
	getSPW(sp, s, p, w, VA);
	int line = (WITH_TLB) ? TLB.findMatch(sp) : -1;

	if (line == -1)
	{
		if (PM[s] == -1 || PM[PM[s] + p] == -1)
		{
			out << "pf ";
			return;
		}

		if (PM[s] == 0)
		{
			int frame = BM.findFreeFrame(true);
			for (int i = frame * 512; i < 1024; ++i) PM[i] = 0; // allocate new PT (all zeroes)
			PM[s] = frame * 512; // update ST entry
			BM.setBitMap(frame); // update BM
			BM.setBitMap(frame + 1);
		}

		if (PM[PM[s] + p] == 0)
		{
			int frame = BM.findFreeFrame();
			for (int i = frame * 512; i < 512; ++i) PM[i] = 0; // allocate new page (all zeroes)
			PM[PM[s] + p] = frame * 512; // update PT entry
			BM.setBitMap(frame); // update BM
		}

		if (WITH_TLB) TLB.miss(sp, PM[PM[s] + p]);
		out << PM[PM[s] + p] + w << ' ';
	}
	else out << TLB.hit(line, w) << ' ';
}

void init(unsigned int* PM, BitMap& BM)
{
	std::ifstream in("F:\\input1.txt");
	std::vector<int> v;
	std::string s;
	int num;

	std::getline(in, s);
	std::stringstream ss1(s);
	while (ss1 >> num) v.push_back(num);
	for (int i = 0; i < v.size(); i += 2)
	{
		PM[v[i]] = v[i + 1];
		if (v[i + 1] > 0)
		{
			BM.setBitMap(v[i + 1] / 512);
			BM.setBitMap(v[i + 1] / 512 + 1);
		}
	}

	v.clear();
	std::getline(in, s);
	std::stringstream ss2(s);
	while (ss2 >> num) v.push_back(num);
	for (int i = 0; i < v.size(); i += 3)
	{
		PM[PM[v[i + 1]] + v[i]] = v[i + 2];
		if (PM[v[i + 1]] > 0)
		{
			BM.setBitMap(PM[v[i + 1]] / 512);
			BM.setBitMap(PM[v[i + 1]] / 512 + 1);
		}
		if (v[i + 2] > 0)
			BM.setBitMap(v[i + 2] / 512);
	}
	in.close();
}

void run(unsigned int* PM, BitMap& BM, TLBuffer& TLB)
{
	std::ifstream in("F:\\input2.txt");
	std::vector<int> v;
	std::string s;
	int num;

	std::getline(in, s);
	std::stringstream ss1(s);
	while (ss1 >> num) v.push_back(num);
	for (int i = 0; i < v.size(); i += 2)
		(v[i] == 0) ? read(PM, TLB, v[i + 1]) : write(PM, BM, TLB, v[i + 1]);
	in.close();
}

int main()
{
	unsigned int* PM = new unsigned int[524288]; // too large to be on the stack, have to dynamically allocate
	for (int i = 0; i < 524288; ++i) PM[i] = 0;

	BitMap BM;
	BM.setBitMap(0); // frame 0 is occupied by ST
	TLBuffer TLB;

	init(PM, BM);
	run(PM, BM, TLB);

	delete[] PM;
	return 0;
}