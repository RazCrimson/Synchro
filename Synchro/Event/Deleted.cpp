#include "Deleted.h"
using namespace System::IO;

Deleted::Deleted(Int64 timeElapsed, String^ relPath):Event(DeletedEvent, timeElapsed)
{
	_relativePath = relPath;
}

bool Deleted::transmitEvent(SocketHandler^ socket)
{
	String^ eventDataString = String::Concat(char(DeletedEvent), "|", _relativePath);
	array<Byte>^ eventData = Text::Encoding::Unicode->GetBytes(eventDataString);
	socket->send(eventData);

	// Waiting for a response
	array<Byte>^ byteArray = socket->receive();
	String^ str = Text::Encoding::Unicode->GetString(byteArray);

	if (str == "End")
		return true;
	return false;
}

bool Deleted::handleEvent(SocketHandler^ socket)
{
	String^ fullPath = rootPath + _relativePath;
	if (File::Exists(fullPath))
	{
		File::Delete(fullPath);
	}
	else if (Directory::Exists(fullPath))
	{
		for each (String ^ file in Directory::EnumerateFiles(fullPath, "*", SearchOption::AllDirectories))
		{
			File::Delete(file);
		}
		for each (String ^ folder in Directory::EnumerateDirectories(fullPath, "*", SearchOption::AllDirectories))
		{
			Directory::Delete(folder);
		}
		Directory::Delete(fullPath);
	}
	array<Byte>^ byteArray = Text::Encoding::Unicode->GetBytes("End");
	socket->send(byteArray);
	return false;
}

bool Deleted::Equals(Object^ obj)
{
	//Check for null and compare run-time types.
	if (obj == nullptr || !this->GetType()->Equals(obj->GetType()))
	{
		return false;
	}
	else {
		try {
			Deleted^ temp = (Deleted^)obj;
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
