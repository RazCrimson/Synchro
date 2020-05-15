#include "FileChanged.h"

using namespace System::IO;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace System::Runtime::Serialization::Formatters::Binary;

using Extensions::Data::XXHash;

Dictionary<UInt32, String^>^ FileChanged::getMissingContent(BloomFilter^ bf)
{
	Dictionary<UInt32, String^>^ missingContent = gcnew Dictionary<UInt32, String^>();
	Dictionary<String^, UInt32>^ currentContent = gcnew Dictionary<String^, UInt32>();
	UInt32 lineNumber = 0;
	array<String^>^ lines = File::ReadAllLines(_rootWatchPath + _relativePath);
	for each (String ^ line in lines)
	{
		lineNumber += 1;
		if (currentContent->ContainsKey(line))
			currentContent[line] += 1;
		else
			currentContent[line] = 1;

		array<Byte>^ lineBytes = Encoding::Unicode->GetBytes(line);
		// Creating a Empty Array to accomodate a Empty String
		if (lineBytes->Length == 0)
			Array::Resize(lineBytes, 1);

		if (!bf->validate(lineBytes, currentContent[line]))
			missingContent[lineNumber] = line;
	}
	return missingContent;
}

void FileChanged::syncFile(Dictionary<UInt32, String^>^ my_missing_content, Dictionary<UInt32, String^>^ received_missing_content)
{
	Collections::SortedList^ contentsToModify = gcnew Collections::SortedList;

	array<String^>^ fileContents = File::ReadAllLines(_rootWatchPath + _relativePath);
	List<String^>^ lines = gcnew List<String^>(fileContents);
	for each (UInt32 lineNumber in received_missing_content->Keys)
	{
		if (my_missing_content->ContainsKey(lineNumber))
		{
			lines[lineNumber - 1] = received_missing_content[lineNumber];
			my_missing_content->Remove(lineNumber);
		}
		else
			contentsToModify->Add(lineNumber * 2, received_missing_content[lineNumber]);
	}

	for each (UInt32 lineNumber in my_missing_content->Keys)
		contentsToModify->Add(lineNumber * 2 + 1, nullptr);

	Int32 lineNumberShift = 0;
	for each (UInt32 modifiedLineNumber  in contentsToModify->Keys)
	{
		Int32 lineNumber;
		if (contentsToModify[modifiedLineNumber] == nullptr)
		{
			lineNumber = (modifiedLineNumber - 1) / 2;
			lineNumber += lineNumberShift;
			lines->RemoveAt(lineNumber - 1);
			lineNumberShift -= 1;
		}
		else
		{
			lineNumber = modifiedLineNumber / 2;
			if (lineNumber - 1 > lines->Count)
				lines->Add((String^)contentsToModify[modifiedLineNumber]);
			else
				lines->Insert(lineNumber - 1, (String^)contentsToModify[modifiedLineNumber]);
			lineNumberShift -= 1;
		}
	}

	fileContents = lines->ToArray();
	File::WriteAllLines(_rootWatchPath + _relativePath, fileContents);
	Console::WriteLine("Updating file contents for {0}", getEventText());
}

FileChanged::FileChanged(Int64 timeElapsed, String^ relPath) : Event(FileChangedEvent, timeElapsed)
{
	_relativePath = relPath;
}

bool FileChanged::transmitEvent(SocketHandler^ socket)
{
	FileInfo^ file = gcnew FileInfo(_rootWatchPath + _relativePath);
	if (!file->Exists)
		return false;

	// Transmitting the event
	String^ eventDataString = String::Concat(char(FileChangedEvent), "|", _relativePath);
	array<Byte>^ eventData = Encoding::Unicode->GetBytes(eventDataString);
	socket->send(eventData);

	// Waiting for a response
	array<Byte>^ byteArray = socket->receive();
	String^ str = Encoding::Unicode->GetString(byteArray);

	while (str != "End")
	{
		if (str == "Send Contents")
		{
			byteArray = File::ReadAllBytes(_rootWatchPath + _relativePath);
			Console::WriteLine("Sending File Contents for {0}", getEventText());
		}
		else if (str == "Send BloomFilter")
		{
			BloomFilter^ bf = BloomFilter::readBloomFilterOfFile(_rootWatchPath + _relativePath);
			str = bf->getNFromSize(bf->getSize()).ToString();
			byteArray = Encoding::Unicode->GetBytes(str);
			socket->send(byteArray);
			byteArray = bf->getBloomFilter();
			Console::WriteLine("Sending Bloom Filter for {0}", getEventText());
		}
		else if (str == "Receive BloomFilter and Send lines")
		{
			byteArray = socket->receive();
			str = Encoding::Unicode->GetString(byteArray);
			UInt32 bfEntriesCount = UInt32::Parse(str);

			BloomFilter^ handlerBloomFilter = gcnew BloomFilter(bfEntriesCount);

			array<Byte>^ bloomFilterBytes = socket->receive();
			handlerBloomFilter->readBloomFilterFromBytes(bloomFilterBytes);

			Dictionary<UInt32, String^>^ missingContent = getMissingContent(handlerBloomFilter);

			MemoryStream^ stream = gcnew MemoryStream();
			BinaryFormatter^ formatter = gcnew BinaryFormatter;
			formatter->Serialize(stream, missingContent);

			byteArray = stream->ToArray();
			Console::WriteLine("Sending Missing lines from transmitter for {0}", getEventText());

		}
		else if (str == "Send Hash")
		{
			str = File::ReadAllText(_rootWatchPath + _relativePath);
			byteArray = Encoding::Unicode->GetBytes(str);
			Int64 hash = XXHash::XXH64(byteArray);
			byteArray = BitConverter::GetBytes(hash);
			Console::WriteLine("Sending Hash for {0}", getEventText());
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

bool FileChanged::handleEvent(SocketHandler^ socket)
{
	String^ fullPath = _rootWatchPath + _relativePath;
	bool completeOverwriteFlag = false;

	array<Byte>^ byteArray;
	String^ str;

	if (File::Exists(fullPath))
	{
		// Asking for the BF
		Console::WriteLine("Requesting Bloom Filter for {0}", getEventText());
		byteArray = Encoding::Unicode->GetBytes("Send BloomFilter");
		socket->send(byteArray);

		// Receiving size of the BF
		byteArray = socket->receive();
		str = Encoding::Unicode->GetString(byteArray);
		UInt32 bfEntriesCount = UInt32::Parse(str);

		BloomFilter^ transmitterBloomFilter = gcnew BloomFilter(bfEntriesCount);

		array<Byte>^ bloomFilterBytes = socket->receive();
		transmitterBloomFilter->readBloomFilterFromBytes(bloomFilterBytes);
		Console::WriteLine("Received Bloom Filter for {0}", getEventText());

		byteArray = Encoding::Unicode->GetBytes("Receive BloomFilter and Send lines");
		Console::WriteLine("Sending Bloom Filter and Requesting Missing Lines for {0}", getEventText());
		socket->send(byteArray);

		Dictionary<UInt32, String^>^ handlerMissingContent = getMissingContent(transmitterBloomFilter);

		BloomFilter^ handlerBloomFilter = BloomFilter::readBloomFilterOfFile(fullPath);

		str = handlerBloomFilter->getNFromSize(handlerBloomFilter->getSize()).ToString();
		byteArray = Encoding::Unicode->GetBytes(str);
		socket->send(byteArray);
		byteArray = handlerBloomFilter->getBloomFilter();
		socket->send(byteArray);

		byteArray = socket->receive();
		MemoryStream^ stream = gcnew MemoryStream(byteArray);
		BinaryFormatter^ formatter = gcnew BinaryFormatter;
		Dictionary<UInt32, String^>^ transmitterMissingContent = dynamic_cast<Dictionary<UInt32, String^>^>(formatter->Deserialize(stream));
		Console::WriteLine("Received Missing Content for {0}", getEventText());

		if(handlerMissingContent->Count || transmitterMissingContent->Count)
			syncFile(handlerMissingContent, transmitterMissingContent);

		byteArray = Encoding::Unicode->GetBytes("Send Hash");
		Console::WriteLine("Requesting Hash for {0}", getEventText());
		socket->send(byteArray);

		str = File::ReadAllText(_rootWatchPath + _relativePath);
		byteArray = Encoding::Unicode->GetBytes(str);
		Int64 myHash = XXHash::XXH64(byteArray);

		byteArray = socket->receive();
		Int64 otherHash = BitConverter::ToInt64(byteArray, 0);

		if (myHash != otherHash)
		{
			completeOverwriteFlag = true;
			Console::WriteLine("Hack check Failed for {0}", getEventText());
		}

	}
	else
	{
		completeOverwriteFlag = true;
	}


	if (completeOverwriteFlag)
	{
		byteArray = Encoding::Unicode->GetBytes("Send Contents");
		Console::WriteLine("Requesting File Contents for {0}", getEventText());
		socket->send(byteArray);

		array<Byte>^ fileContents = socket->receive();
		Console::WriteLine("Updating file contents for {0}", getEventText());
		File::WriteAllBytes(fullPath, fileContents);
	}

	byteArray = Encoding::Unicode->GetBytes("End");
	socket->send(byteArray);

	return true;
}

bool FileChanged::Equals(Object^ obj)
{
	//Check for null and compare run-time types.
	if (obj == nullptr || !this->GetType()->Equals(obj->GetType()))
	{
		return false;
	}
	else {
		try
		{
			FileChanged^ temp = (FileChanged^)obj;

			if (code == temp->code && _relativePath == temp->_relativePath)
				return true;
			else
				return false;
		}
		catch (...)
		{
			return false;
		}
	}
}

String^ FileChanged::getEventText()
{
	return gcnew String("File Changed Event at " + _relativePath);
}
