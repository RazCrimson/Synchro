#include "FileDetected.h"
#include "..\BloomFilter\BloomFilter.h"

using namespace System::IO;
using namespace System::Text;

using Extensions::Data::XXHash;

FileDetected::FileDetected(Int64 timeElapsed, String^ relPath) : Event(FileDetectedEvent, timeElapsed)
{
	_relativePath = relPath;
}

bool FileDetected::transmitEvent(SocketHandler^ socket)
{
	FileInfo^ file = gcnew FileInfo(rootPath + _relativePath);
	if (!file->Exists)
		return false;
	// Transmitting the event
	String^ eventDataString = String::Concat(char(FileDetectedEvent), "|", _relativePath);
	array<Byte>^ eventData = Encoding::Unicode->GetBytes(eventDataString);
	socket->send(eventData);

	// Waiting for a response
	array<Byte>^ byteArray = socket->receive();
	String^ str = Encoding::Unicode->GetString(byteArray);

	while (str != "End")
	{
		if (str == "Send Length")
			byteArray = Encoding::Unicode->GetBytes(file->Length.ToString());
		else if (str == "Send Contents")
			byteArray = File::ReadAllBytes(rootPath + _relativePath);
		else if (str == "Send BloomFilter")
		{
			BloomFilter^ bf = BloomFilter::readBloomFilterOfFile(rootPath + _relativePath);
			str = bf->getNFromSize(bf->getSize()).ToString();
			byteArray = Encoding::Unicode->GetBytes(str);
			socket->send(byteArray);
			byteArray = bf->getBloomFilter();
		}
		else if (str == "Send Hash")
		{
			str = File::ReadAllText(rootPath + _relativePath);
			byteArray = Encoding::Unicode->GetBytes(str);
			Int64 hash = XXHash::XXH64(byteArray);
			byteArray = BitConverter::GetBytes(hash);
		}
		else
		{
			return false;
		}
		socket->send(byteArray);

		// Waiting for a response
		byteArray = socket->receive();
		str = Encoding::Unicode->GetString(byteArray);
	}
	return true;
}

bool FileDetected::handleEvent(SocketHandler^ socket)
{
	String^ fullPath = rootPath + _relativePath;
	bool completeOverwriteFlag = true;

	// Asking for the size of the file
	array<Byte>^ byteArray = Encoding::Unicode->GetBytes("Send Length");
	socket->send(byteArray);

	// Receiving size of the file
	byteArray = socket->receive();
	String^ str = Encoding::Unicode->GetString(byteArray);
	UInt32 bytes = UInt32::Parse(str);

	if (bytes == 0)
	{
		completeOverwriteFlag = false;
		if (File::Exists(fullPath))
		{
			FileInfo^ file = gcnew FileInfo(fullPath);
			if (file->Length != 0)
				File::CreateText(fullPath)->Close();
		}
		else
		{
			File::CreateText(fullPath)->Close();
		}
	}
	else if (File::Exists(fullPath))
	{
		// Asking for the BF
		byteArray = Encoding::Unicode->GetBytes("Send BloomFilter");
		socket->send(byteArray);

		// Receiving size of the BF
		byteArray = socket->receive();
		str = Encoding::Unicode->GetString(byteArray);
		UInt32 bfSize = UInt32::Parse(str);

		BloomFilter^ bf = gcnew BloomFilter(bfSize);

		array<Byte>^ bloomFilter = socket->receive();
		bf->readBloomFilterFromBytes(bloomFilter);

		array<String^>^ lines = File::ReadAllLines(fullPath);
		if (bf->checkIfPresent(lines))
		{

			byteArray = Encoding::Unicode->GetBytes("Send Hash");
			socket->send(byteArray);

			str = File::ReadAllText(rootPath + _relativePath);
			byteArray = Encoding::Unicode->GetBytes(str);
			Int64 myHash = XXHash::XXH64(byteArray);

			byteArray = socket->receive();
			Int64 otherHash = BitConverter::ToInt64(byteArray, 0);

			if ( myHash != otherHash)
				completeOverwriteFlag = true;
		}

	}

	if (completeOverwriteFlag)
	{
		byteArray = Encoding::Unicode->GetBytes("Send Contents");
		socket->send(byteArray);

		array<Byte>^ fileContents = socket->receive();
		File::WriteAllBytes(fullPath, fileContents);
	}

	byteArray = Encoding::Unicode->GetBytes("End");
	socket->send(byteArray);

	return true;
}

bool FileDetected::Equals(Object^ obj)
{	
	//Check for null and compare run-time types.
	if (obj == nullptr || !this->GetType()->Equals(obj->GetType()))
	{
		return false;
	}
	else {
		try {
			FileDetected^ temp = (FileDetected^)obj;
			if (code == temp->code && _relativePath == temp->_relativePath)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		catch (...)
		{
			return false;
		}
	}	
}
