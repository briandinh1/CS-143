#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <bitset>


// ======================================================================
// SHELL COMMANDS
// ======================================================================

void lseek(int index, int pos, bool main = false);

void open(std::string fileName);

void close(int index);

void create(std::string fileName);

void destroy(std::string fileName);

void write(int index, char c, int count);

void read(int index, int count);

void directory();

void save(std::string fileName);

void init();

void init(std::string fileName);

// ======================================================================
// HELPER FUNCTIONS 
// ======================================================================

// reduced version of regular open func, specific for directory
void openDirectory();

// find and allocate new block
void expand(int index, int pos);

// remove one directory entry given a filename
void removeEntry(char(&name)[4]);

// copy contents of ldisk into buffer
void read_block(int index, int blockNum);

// write contents of buffer into ldisk
void write_block(int index, int blockNum);

// take an int and assign value to 4 chars (byte)
// has one overload for cache vs buffer
void setNum(int blockNum, int blockPos, int num);
void setNum(unsigned char(&buffer)[64], int pos, int num);

// take 4 chars (bytes) and recompose value into one int
// has one overload for cache vs buffer
int getNum(int blockNum, int blockPos);
int getNum(unsigned char(&buffer)[64], int pos);

// set bitmap value 
void setMap(int blockNum);

// clear bitmap value
void clearMap(int blockNum);

// get corresponding block number given the descriptor index
int getDiskBlock(int index);

// get position in the block given descriptor index
int getDiskPos(int index);

// determine if the n-th bit in a byte is set; used for the bitmap
bool isNthBitSet(unsigned char c, int n);

// look through bitmap and try to find an open block
int findOpenBlock();

// find an existing file descriptor index
int findFileDescIndex(char(&name)[4]);

// find free descriptor index
int findFreeDescIndex();

// find free directory position
int findFreeDirPos(const char(&name)[4]);

// converts string to character
void string2char(char(&name)[4], std::string fileName);

// show name of file
void displayName(int index);

// check if name matches
bool doesNameMatch(const char(&name)[4], int pos);

// error message
void error();

// ======================================================================
// STRUCTURES USED TO REPRESENT FILE SYSTEM
// ======================================================================

struct OpenFileTable
{
	OpenFileTable()
	{
		descriptorIndex = currentBlock = currentPos = length = -1;
		for (int i = 0; i < 3; ++i) blocks[i] = -1;
		for (int i = 0; i < 64; ++i) rw_buffer[i] = -1;
	}

	void reset()
	{
		descriptorIndex = currentBlock = currentPos = length = -1;
		for (int i = 0; i < 3; ++i) blocks[i] = -1;
		for (int i = 0; i < 64; ++i) rw_buffer[i] = -1;
	}

	unsigned char rw_buffer[64];
	int descriptorIndex;
	int currentPos; // current position in r/w buffer
	int length; // how many characters (or bytes) the file contains

	int blocks[3];
	int currentBlock;
};

OpenFileTable oft[4];
unsigned char ldisk[64][64]; // represents disk. can only read or write entire 64 byte blocks
unsigned char cache[7][64]; // used to store the first 7 blocks in "main memory"
unsigned int mask[32];
std::ofstream out("F:\\34546266.txt");

// ======================================================================
// MAIN
// ======================================================================

int main()
{
	std::ifstream in("F:\\proj1\\win\\input01.txt");
	std::string input;

	while (std::getline(in, input))
	{
		if (input != "")
		{
			try
			{
				std::string params[4]; // parse command from cin
				std::string token;
				std::stringstream ss(input);
				for (int i = 0; ss >> token; ++i) params[i] = token;

				if (params[0] == "in" && params[1] == "") init();
				if (params[0] == "in" && params[1] != "") init(params[1]);
				if (params[0] == "cr") create(params[1]);
				if (params[0] == "de") destroy(params[1]);
				if (params[0] == "op") open(params[1]);
				if (params[0] == "cl") close(std::stoi(params[1]));
				if (params[0] == "rd")
					read(std::stoi(params[1]), std::stoi(params[2]));
				if (params[0] == "wr")
					write(std::stoi(params[1]), params[2][0], std::stoi(params[3]));
				if (params[0] == "sk")
					lseek(std::stoi(params[1]), std::stoi(params[2]), true);
				if (params[0] == "dr") directory();
				if (params[0] == "sv") save(params[1]);
			}
			catch (int e) { out << "error"; }
		}
		out << std::endl;
	}

	in.close();
	out.close();
	return 0;
}

// ======================================================================
// IMPLEMENTATIONS
// ======================================================================

void lseek(int index, int pos, bool main)
{
	// error if index > oft size, pos > length of file, or entry doesnt exist
	if (index >= 4 || pos > oft[index].length || oft[index].descriptorIndex == -1)
		error();

	// save current block to disk
	write_block(index, oft[index].currentBlock);

	// find out new block to copy into buffer according to pos
	if (pos < 64) oft[index].currentBlock = oft[index].blocks[0];
	else if (pos < 128) oft[index].currentBlock = oft[index].blocks[1];
	else if (pos < 192) oft[index].currentBlock = oft[index].blocks[2];

	// if the block does not exist, add it
	if (oft[index].currentBlock == -1)
		expand(index, pos);

	// copy block into buffer and set the position
	read_block(index, oft[index].currentBlock);
	oft[index].currentPos = pos;

	if (main) out << "position is " << pos;
}

void open(std::string fileName)
{
	// convert string to char array for easier matching
	char name[4];
	string2char(name, fileName);

	// search directory to find index of file descriptor
	int fileDescIndex = findFileDescIndex(name);
	if (fileDescIndex == -1) error();

	// allocate free OFT entry
	for (int i = 1; i < 4; ++i)
	{
		if (oft[i].descriptorIndex == fileDescIndex) break; // file exists in oft

		if (oft[i].descriptorIndex == -1)
		{
			int blockNum = getDiskBlock(fileDescIndex);
			int blockPos = getDiskPos(fileDescIndex);

			// fill in current pos and file desc index, etc
			oft[i].descriptorIndex = fileDescIndex;
			oft[i].length = getNum(blockNum, blockPos);
			oft[i].blocks[0] = getNum(blockNum, blockPos + 4);
			oft[i].blocks[1] = getNum(blockNum, blockPos + 8);
			oft[i].blocks[2] = getNum(blockNum, blockPos + 12);
			oft[i].currentBlock = oft[i].blocks[0];
			oft[i].currentPos = 0;

			// read block 0 of file into buffer
			read_block(i, oft[i].blocks[0]);

			for (int j = 0; j < 4; ++j)
				if (name[j] != 0) out << name[j];
			out << " opened " << i;
			return;
		}
	}

	out << "error";
}

void close(int index)
{
	// oft entry has nothing to close
	if (oft[index].descriptorIndex == -1 || index >= 4) error();

	// write buffer to disk
	write_block(index, oft[index].currentBlock);

	// save contents to cache
	int blockNum = getDiskBlock(oft[index].descriptorIndex);
	int blockPos = getDiskPos(oft[index].descriptorIndex);
	setNum(blockNum, blockPos, oft[index].length);
	setNum(blockNum, blockPos + 4, oft[index].blocks[0]);
	setNum(blockNum, blockPos + 8, oft[index].blocks[1]);
	setNum(blockNum, blockPos + 12, oft[index].blocks[2]);

	// free oft entry
	oft[index].reset();

	out << index << " closed";
}

void create(std::string fileName)
{
	// convert string to char array for easier matching
	char name[4];
	string2char(name, fileName);

	// find free file descriptor
	int freeDescIndex = findFreeDescIndex();
	if (freeDescIndex == -1) error();

	// find free entry in the directory
	int freeDirPos = findFreeDirPos(name);
	if (freeDirPos == -1) error();

	// increment directory length
	++oft[0].length;

	// set name of new file in directory
	for (int i = 0; i < 4; ++i)
		oft[0].rw_buffer[freeDirPos + i] = name[i];

	// set index in directory to the free descriptor index
	setNum(oft[0].rw_buffer, freeDirPos + 4, freeDescIndex);

	// update new descriptor index and allocate an open block to it
	int blockNum = getDiskBlock(freeDescIndex);
	int blockPos = getDiskPos(freeDescIndex);
	int openBlock = findOpenBlock();
	setNum(blockNum, blockPos, 0); // length of new file = 0
	setNum(blockNum, blockPos + 4, openBlock); // allocate new block to index
	setMap(openBlock); // set bitmap

	for (int i = 0; i < 4; ++i)
		if (name[i] != 0) out << name[i];
	out << " created";
}

void destroy(std::string fileName)
{
	// convert string to char array for easier matching
	char name[4];
	string2char(name, fileName);

	// search directory to find index of file descriptor
	int fileDescIndex = findFileDescIndex(name);
	if (fileDescIndex == -1) error();

	// if file exists in OFT, remove it 
	for (int i = 1; i < 4; ++i)
		if (oft[i].descriptorIndex == fileDescIndex) close(i);

	// remove directory entry
	removeEntry(name);

	// decrement directory length 
	--oft[0].length;

	int blockNum = getDiskBlock(fileDescIndex);
	int blockPos = getDiskPos(fileDescIndex);
	setNum(blockNum, blockPos, -1); // set descriptor length back to -1

	for (int i = 4; i < 16; i += 4) // clear rest of the blocks
	{
		// get block number of the descriptor slot
		int num = getNum(blockNum, blockPos + i);
		if (num != -1)
		{
			for (int j = 0; j < 64; ++j) ldisk[num][j] = -1;
			setNum(blockNum, blockPos + i, -1);
			clearMap(num);
		}
	}

	for (int i = 0; i < 4; ++i)
		if (name[i] != 0) out << name[i];
	out << " destroyed";
}

void write(int index, char c, int count)
{
	// trying to write to unopened file
	if (oft[index].descriptorIndex == -1) error();

	int bytesWritten = 0;

	// compute position in the r/w buffer
	int pos = oft[index].currentPos % 64;

	// figure out how many bytes are needed 
	// only write to available bytes (192 limit)
	int left = oft[index].currentPos + count < 192
		? count : 192 - oft[index].currentPos;
	int leftInBlock = 64 - pos;
	if (left < leftInBlock) leftInBlock = left;

	// write bytes down into buffer, expand if necessary
	for (int i = 0; i < 3; ++i)
	{
		if (oft[index].currentPos >= 192) break;
		lseek(index, oft[index].currentPos);

		for (int j = 0; j < leftInBlock; ++j)
		{
			oft[index].rw_buffer[pos + j] = c;
			++oft[index].length;
			++oft[index].currentPos;
			++bytesWritten;
		}

		pos = oft[index].currentPos % 64;
		left -= leftInBlock;
		leftInBlock = 64 - pos;
		if (left < leftInBlock) leftInBlock = left;
		if (left <= 0) break;
	}

	// update file length in descriptor
	int blockNum = getDiskBlock(oft[index].descriptorIndex);
	int blockPos = getDiskPos(oft[index].descriptorIndex);
	setNum(blockNum, blockPos, oft[index].length);

	out << bytesWritten << " bytes written";
}

void read(int index, int count)
{
	// trying to read from unopened file
	if (oft[index].descriptorIndex == -1) error();

	// compute position in the r/w buffer
	int pos = oft[index].currentPos % 64;

	// figure out how many bytes are needed 
	// only read the available bytes (length of file)
	int left = oft[index].currentPos + count < oft[index].length
		? count : oft[index].length - oft[index].currentPos;
	int leftInBlock = 64 - pos;
	if (left < leftInBlock) leftInBlock = left;

	// read bytes from buffer
	for (int i = 0; i < 3; ++i)
	{
		if (oft[index].currentPos >= 192) break;
		lseek(index, oft[index].currentPos);

		for (int j = 0; j < leftInBlock; ++j)
		{
			// show only if byte contains a proper value (ex: not -1, null)
			if (oft[index].rw_buffer[pos + j] < 128)
				out << oft[index].rw_buffer[pos + j];
			++oft[index].currentPos;
		}

		pos = oft[index].currentPos % 64;
		left -= leftInBlock;
		leftInBlock = 64 - pos;
		if (left < leftInBlock) leftInBlock = left;
		if (left <= 0) break;
	}
}

void directory()
{
	lseek(0, 0);

	for (int i = 0; i < 3; ++i)
	{
		write_block(0, oft[0].currentBlock);
		if (oft[0].blocks[i] == -1) break;
		oft[0].currentBlock = oft[0].blocks[i];
		read_block(0, oft[0].currentBlock);

		for (int j = 0; j < 64; j += 8, oft[0].currentPos += 8)
			if (getNum(oft[0].rw_buffer, j) != -1) displayName(j);
	}
}

void save(std::string fileName)
{
	std::ofstream out(fileName);

	for (int i = 0; i < 4; ++i)
	{
		// save contents to cache
		int blockNum = getDiskBlock(oft[i].descriptorIndex);
		int blockPos = getDiskPos(oft[i].descriptorIndex);
		setNum(blockNum, blockPos, oft[i].length);
		setNum(blockNum, blockPos + 4, oft[i].blocks[0]);
		setNum(blockNum, blockPos + 8, oft[i].blocks[1]);
		setNum(blockNum, blockPos + 12, oft[i].blocks[2]);

		// write current buffer to disk
		write_block(i, oft[i].currentBlock);

		// free oft entry
		oft[i].reset();
	}

	// copy cache descriptor contents to disk
	for (int i = 0; i < 7; ++i)
		for (int j = 0; j < 64; ++j)
			ldisk[i][j] = cache[i][j];

	// write disk contents to file 
	for (int i = 0; i < 64; ++i)
		for (int j = 0; j < 64; ++j)
			out << unsigned int(ldisk[i][j]) << " ";
	// saved as space delimited int

	out.close();
	out << "disk saved";
}

void init()
{
	// create mask values that will be used for bitmask
	mask[0] = 0x80000000;
	for (int i = 1; i < 32; ++i)
		mask[i] = mask[i - 1] / 2;

	// default -1 for every slot in ldisk (first 8 bytes are 0 for bitmap)
	for (int i = 0; i < 64; ++i)
		for (int j = 0; j < 64; ++j)
			ldisk[i][j] = -1;

	for (int i = 0; i < 8; ++i) ldisk[0][i] = 0;

	// copy first 7 blocks to "main memory" cache
	for (int i = 0; i < 7; ++i)
		for (int j = 0; j < 64; ++j)
			cache[i][j] = ldisk[i][j];

	// set block 0 (bitmap) and 1 through 6 (descriptors) to used
	for (int i = 0; i < 7; ++i) setMap(i);

	// initialize directory descriptor's length to 0;
	setNum(1, 0, 0); // directory is at block 1, position 0

	// create directory from scratch
	openDirectory();

	out << "disk initialized";
}

void init(std::string fileName)
{
	std::ifstream in(fileName);

	// create mask values that will be used for bitmask
	mask[0] = 0x80000000;
	for (int i = 1; i < 32; ++i)
		mask[i] = mask[i - 1] / 2;

	// write file contents to disk
	for (int i = 0; i < 64; ++i)
	{
		for (int j = 0; j < 64; ++j)
		{
			unsigned int byte;
			in >> byte;
			ldisk[i][j] = byte;
		}
	}

	// copy descriptor contents of disk to cache
	for (int i = 0; i < 7; ++i)
		for (int j = 0; j < 64; ++j)
			cache[i][j] = ldisk[i][j];

	// initialize directory in oft
	openDirectory();

	in.close();
	out << "disk restored";
}

// ======================================================================
// ======================================================================

void openDirectory()
{
	oft[0].currentPos = 0;
	oft[0].descriptorIndex = 0;
	oft[0].length = getNum(1, 0);
	oft[0].blocks[0] = getNum(1, 4);
	oft[0].blocks[1] = getNum(1, 8);
	oft[0].blocks[2] = getNum(1, 12);

	if (oft[0].blocks[0] == -1) // if not initialized from txt file
	{
		int openBlock = findOpenBlock(); // assigned first open block
		oft[0].blocks[0] = openBlock;
		setNum(1, 4, openBlock);
		setMap(openBlock); // set new block index in bitmap
	}

	oft[0].currentBlock = oft[0].blocks[0];
	read_block(0, oft[0].currentBlock); // copy contents of block 0 into buffer
}

void expand(int index, int pos)
{
	int blockNum = getDiskBlock(oft[index].descriptorIndex);
	int blockPos = getDiskPos(oft[index].descriptorIndex);
	int openBlock;

	if (pos >= 64 && oft[index].blocks[1] == -1)
	{
		openBlock = findOpenBlock();
		oft[index].blocks[1] = openBlock;
		setNum(blockNum, blockPos + 8, openBlock);
		setMap(openBlock);
		oft[index].currentBlock = openBlock;
	}

	if (pos >= 128 && oft[index].blocks[2] == -1)
	{
		openBlock = findOpenBlock();
		oft[index].blocks[2] = openBlock;
		setNum(blockNum, blockPos + 12, openBlock);
		setMap(openBlock);
		oft[index].currentBlock = openBlock;
	}
}

void removeEntry(char(&name)[4])
{
	lseek(0, 0);

	for (int i = 0; i < 3; ++i)
	{
		write_block(0, oft[0].currentBlock);
		if (oft[0].blocks[i] == -1) break;
		oft[0].currentBlock = oft[0].blocks[i];
		read_block(0, oft[0].currentBlock);

		for (int j = 0; j < 64; j += 8, oft[0].currentPos += 8)
		{
			if (doesNameMatch(name, j))
			{
				setNum(oft[0].rw_buffer, j, -1);
				setNum(oft[0].rw_buffer, j + 4, -1);
				return;
			}
		}
	}

	error(); // else name was not found in directory
}

void read_block(int index, int blockNum)
{
	for (int i = 0; i < 64; ++i)
		oft[index].rw_buffer[i] = ldisk[blockNum][i];
}

void write_block(int index, int blockNum)
{
	for (int i = 0; i < 64; ++i)
		ldisk[blockNum][i] = oft[index].rw_buffer[i];
}

void setNum(int blockNum, int blockPos, int num)
{
	cache[blockNum][blockPos++] = (num >> 24);
	cache[blockNum][blockPos++] = (num >> 16);
	cache[blockNum][blockPos++] = (num >> 8);
	cache[blockNum][blockPos] = (num);
}

void setNum(unsigned char(&buffer)[64], int pos, int num)
{
	buffer[pos++] = (num >> 24);
	buffer[pos++] = (num >> 16);
	buffer[pos++] = (num >> 8);
	buffer[pos] = (num);
}

int getNum(int blockNum, int blockPos)
{
	int num = (cache[blockNum][blockPos++] << 24);
	num |= (cache[blockNum][blockPos++] << 16);
	num |= (cache[blockNum][blockPos++] << 8);
	num |= (cache[blockNum][blockPos]);
	return num;
}

int getNum(unsigned char(&buffer)[64], int pos)
{
	int num = (buffer[pos++] << 24);
	num |= (buffer[pos++] << 16);
	num |= (buffer[pos++] << 8);
	num |= (buffer[pos]);
	return num;
}

void setMap(int blockNum)
{
	if (blockNum < 32)
	{
		cache[0][0] |= (mask[blockNum] >> 24);
		cache[0][1] |= (mask[blockNum] >> 16);
		cache[0][2] |= (mask[blockNum] >> 8);
		cache[0][3] |= (mask[blockNum]);
	}
	else
	{
		blockNum -= 32;
		cache[0][4] |= (mask[blockNum] >> 24);
		cache[0][5] |= (mask[blockNum] >> 16);
		cache[0][6] |= (mask[blockNum] >> 8);
		cache[0][7] |= (mask[blockNum]);
	}
}

void clearMap(int blockNum)
{
	if (blockNum < 32)
	{
		cache[0][0] &= (~mask[blockNum] >> 24);
		cache[0][1] &= (~mask[blockNum] >> 16);
		cache[0][2] &= (~mask[blockNum] >> 8);
		cache[0][3] &= (~mask[blockNum]);
	}
	else
	{
		blockNum -= 32;
		cache[0][4] &= (~mask[blockNum] >> 24);
		cache[0][5] &= (~mask[blockNum] >> 16);
		cache[0][6] &= (~mask[blockNum] >> 8);
		cache[0][7] &= (~mask[blockNum]);
	}
}

int getDiskBlock(int index)
{
	int blockNum = 1;
	for (int i = 4; i < 24; i += 4)
		if (index >= i) ++blockNum;
	return blockNum;
}

int getDiskPos(int index)
{
	return index % 4 * 16;
}

bool isNthBitSet(unsigned char c, int n)
{
	static unsigned char bits[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };
	return (c & bits[n]) != 0;
}

int findOpenBlock()
{
	for (int i = 0; i < 8; ++i) // iteration through bitmap (8 chars = 2 ints)
		for (int j = 0; j < 8; ++j) // iteration through 8 bits of each char
			if (!isNthBitSet(cache[0][i], j))
				return j + i * 8;

	return -1; // all blocks filled, error
}

int findFileDescIndex(char(&name)[4])
{
	lseek(0, 0);

	for (int i = 0; i < 3; ++i)
	{
		write_block(0, oft[0].currentBlock);
		if (oft[0].blocks[i] == -1) break;
		oft[0].currentBlock = oft[0].blocks[i];
		read_block(0, oft[0].currentBlock);

		for (int j = 0; j < 64; j += 8, oft[0].currentPos += 8)
			if (doesNameMatch(name, j))
				return getNum(oft[0].rw_buffer, j + 4);
	}

	// else no matching name was found, return error
	return -1;
}

int findFreeDescIndex()
{
	int freeDescIndex = 0;

	for (int i = 1; i <= 6; ++i)
		for (int j = 0; j < 64; j += 16, ++freeDescIndex)
			if (getNum(i, j) == -1)
				return freeDescIndex;

	// otherwise, all descriptor slots are full
	return -1;
}

int findFreeDirPos(const char(&name)[4])
{
	int freeDirPos = -1;

	// find free block, make note of it if found
	lseek(0, 0);
	for (int i = 0; i < 3; ++i)
	{
		if (freeDirPos != -1) break;

		write_block(0, oft[0].currentBlock);
		oft[0].currentBlock = oft[0].blocks[i];
		if (oft[0].currentBlock == -1) expand(0, i * 64);
		read_block(0, oft[0].currentBlock);

		for (int j = 0; j < 64; j += 8, oft[0].currentPos += 8)
		{
			if (getNum(oft[0].rw_buffer, j) == -1)
			{
				freeDirPos = j;
				break;
			}
		}
	}

	// search entire directory for matching names
	lseek(0, 0);
	for (int i = 0; i < 3; ++i)
	{
		if (oft[0].blocks[i] == -1) break;
		write_block(0, oft[0].currentBlock);
		oft[0].currentBlock = oft[0].blocks[i];
		read_block(0, oft[0].currentBlock);

		for (int j = 0; j < 64; j += 8, oft[0].currentPos += 8)
			if (doesNameMatch(name, j)) return -1;
	}

	// if still -1, then no open space left, return error
	return freeDirPos;
}

void string2char(char(&name)[4], std::string fileName)
{
	for (int i = 0; i < 4; ++i) name[i] = 0; // first zero out all values
	// copy string to char array only up to 4 characters
	int size = (fileName.size() > 4 ? 4 : fileName.size());
	for (int i = 0; i < size; ++i)
		name[i] = fileName[i];
}

void displayName(int index)
{
	for (int i = 0; i < 4; ++i)
		if (oft[0].rw_buffer[index + i] != 0)
			out << oft[0].rw_buffer[index + i];

	out << " ";
}

bool doesNameMatch(const char(&name)[4], int pos)
{
	for (int i = 0; i < 4; ++i)
		if (oft[0].rw_buffer[pos + i] != name[i]) return false;

	return true;
}

void error() { throw -1; }

// ======================================================================
// ======================================================================