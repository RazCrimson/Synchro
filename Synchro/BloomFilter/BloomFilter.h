#pragma once

using namespace System;

public ref class BloomFilter 
{
private:
    array<Byte>^ _bitArray;

    UInt32 _size, _noOfHashFun;

    double _probOfError;

    void _setBit(UInt16);

    bool _getBit(UInt16);

public:
    explicit BloomFilter(UInt32);

    void insert(array<Byte>^ line, UInt32 freq); // to insert values into bloomfilter

    bool validate(array<Byte>^ line, UInt32 freq); // to check if the given value is in bloomfilter or not

    void readBloomFilterFromBytes(array<Byte>^);

    array<Byte>^ getBloomFilter();

    UInt32 getSize();

    UInt32 getNFromSize(UInt32 Size);

    UInt32 getNumberOfHashFunctions();

    static BloomFilter^ readBloomFilterOfFile(String^ path);
    
    bool checkIfPresent(array<String^>^ lines);
};