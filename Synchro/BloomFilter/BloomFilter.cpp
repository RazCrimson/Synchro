#include "BloomFilter.h"

#include <cmath>

using Extensions::Data::XXHash;
using namespace System::Collections::Generic;
using namespace System::Text; 
using namespace System::IO;

void BloomFilter::_setBit(UInt16 num)
{
	unsigned short index = num / 8;
	_bitArray[index] = _bitArray[index] | (1 << (7 - num % 8));
}

bool BloomFilter::_getBit(UInt16 num)
{
	unsigned short index = num / 8;
	return _bitArray[index] & (1 << (7 - num % 8));
}

BloomFilter::BloomFilter(UInt32 numberOfItems)
{
	_probOfError = 0.02;
	_size = ceil(-float(numberOfItems) * log(_probOfError) / pow(log(2), 2));
	_noOfHashFun = ceil(_size / numberOfItems * log(2));
	_bitArray = gcnew array<Byte>(_size / 8 + 1);
}

void BloomFilter::insert(array<Byte>^ line, UInt32 freq)
{
	Int64 line_hash = XXHash::XXH64(line, freq);
	array<Byte>^ line_hash_bytes = BitConverter::GetBytes(line_hash);
	for (UInt32 i = 0; i < this->_noOfHashFun; i++)
	{
		int index = XXHash::XXH64(line_hash_bytes, i) % this->_size;
		_setBit(index);
	}
}

bool BloomFilter::validate(array<Byte>^ line, UInt32 freq)
{
	Int64 line_hash = XXHash::XXH64(line, freq);
	array<Byte>^ line_hash_bytes = BitConverter::GetBytes(line_hash);
	for (UInt32 i = 0; i < this->_noOfHashFun; i++) {
		int check_at_index = XXHash::XXH64(line_hash_bytes, i) % this->_size;
		if (!_getBit(check_at_index))
			return false;
	}
	return true;
}

void BloomFilter::readBloomFilterFromBytes(array<Byte>^ ptr)
{
	_bitArray = ptr;
}

array<Byte>^ BloomFilter::getBloomFilter()
{
	return _bitArray;
}

UInt32 BloomFilter::getSize()
{
	return _size;
}

UInt32 BloomFilter::getNFromSize(UInt32 Size)
{
	return floor(float(Size) * -1 * (pow(log(2), 2) / log(_probOfError)));
}

UInt32 BloomFilter::getNumberOfHashFunctions()
{
	return _noOfHashFun;
}

BloomFilter^ BloomFilter::readBloomFilterOfFile(String^ path)
{
	array<String^>^ lines = File::ReadAllLines(path);
	BloomFilter^ bf = gcnew BloomFilter(lines->Length < 1 ? 1 : lines->Length);
	IDictionary<String^, UInt32>^ lineMap = gcnew Dictionary<String^, UInt32>();
	for each (String ^ line in lines)
	{
		if (lineMap->ContainsKey(line))
			lineMap[line] += 1;
		else
			lineMap[line] = 1;
		array<Byte>^ lineBytes = Encoding::Unicode->GetBytes(line);
		if (lineBytes->Length == 0)
			Array::Resize(lineBytes, 1);
		bf->insert(lineBytes, lineMap[line]);
	}
	return bf;
}

bool BloomFilter::checkIfPresent(array<String^>^ lines)
{
	IDictionary<String^, UInt32>^ lineMap = gcnew Dictionary<String^, UInt32>();
	for each (String ^ line in lines)
	{
		if (lineMap->ContainsKey(line))
			lineMap[line] += 1;
		else
			lineMap[line] = 1;
		array<Byte>^ lineBytes = Encoding::Unicode->GetBytes(line);
		if (lineBytes->Length == 0)
			Array::Resize(lineBytes, 1);
		if (!validate(lineBytes, lineMap[line]))
			return false;
	}
	return true;
}