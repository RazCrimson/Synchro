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
	FileInfo^ file = gcnew FileInfo(_rootWatchPath + _relativePath);
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
		{
			Console::WriteLine("Sending length for {0}", getEventText()); 
			byteArray = Encoding::Unicode->GetBytes(file->Length.ToString());
			socket->send(byteArray);
		}
		else if (str == "Send Contents")
		{
			Console::WriteLine("Sending Contents for {0}", getEventText());
			byteArray = File::ReadAllBytes(_rootWatchPath + _relativePath);
			socket->send(byteArray);
		}
		else if (str == "Send Hash")
		{
			for(UInt16 i = 0; i < 4; i++)
			{
				Console::WriteLine("Sending Hash - {0} for {1}", i + 1, getEventText());
				str = File::ReadAllText(_rootWatchPath + _relativePath);
				byteArray = Encoding::Unicode->GetBytes(str);
				Int64 hash = XXHash::XXH64(byteArray, i);
				byteArray = BitConverter::GetBytes(hash);
				socket->send(byteArray);
			}
		}
		else
		{
			return false;
		}
		
		// Waiting for a response
		byteArray = socket->receive();
		str = Encoding::Unicode->GetString(byteArray);
	}
	return true;
}

bool FileDetected::handleEvent(SocketHandler^ socket)
{
	String^ fullPath = _rootWatchPath + _relativePath;
	bool completeOverwriteFlag = true;

	// Asking for the size of the file
	Console::WriteLine("Requesting length for {0}", getEventText());
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
		Console::WriteLine("Requesting Hash for {0}", getEventText());
		byteArray = Encoding::Unicode->GetBytes("Send Hash");
		socket->send(byteArray);

		for (UInt16 i = 0; i < 4; i++)
		{
			
			str = File::ReadAllText(_rootWatchPath + _relativePath);
			byteArray = Encoding::Unicode->GetBytes(str);
			Int64 myHash = XXHash::XXH64(byteArray, i);

			byteArray = socket->receive();
			Int64 otherHash = BitConverter::ToInt64(byteArray, 0);
			Console::WriteLine("Received Hash - {0} for {1}", i + 1, getEventText());

			if (myHash != otherHash)
				completeOverwriteFlag = true;
		}

	}

	if (completeOverwriteFlag)
	{
		Console::WriteLine("Requesting File Contents for {0}", getEventText());
		byteArray = Encoding::Unicode->GetBytes("Send Contents");
		socket->send(byteArray);

		array<Byte>^ fileContents = socket->receive();
		Console::WriteLine("Received File Contents");
		File::WriteAllBytes(fullPath, fileContents);
		Console::WriteLine("Updated File Contents for {0}", getEventText());
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

String^ FileDetected::getEventText()
{
	return gcnew String("File Detected Event at " + _relativePath);
}
