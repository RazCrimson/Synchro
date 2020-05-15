#include "DeleteItem.h"

using namespace System::IO;

DeleteItem::DeleteItem(Int64 timeElapsed, String^ relPath):Event(DeletedEvent, timeElapsed)
{
	_relativePath = relPath;
}

bool DeleteItem::transmitEvent(SocketHandler^ socket)
{
	String^ eventDataString = String::Concat(char(DeletedEvent), "|", _relativePath);
	array<Byte>^ eventData = Text::Encoding::Unicode->GetBytes(eventDataString);
	socket->send(eventData);

	return true;
}

bool DeleteItem::handleEvent(SocketHandler^ socket)
{
	String^ fullPath = _rootWatchPath + _relativePath;
	if (File::Exists(fullPath))
	{
		File::Delete(fullPath);
		Console::WriteLine("Deleted file for {0}", getEventText());
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
		Console::WriteLine("Deleted all files and folders for {0}", getEventText());
	}
	return false;
}

bool DeleteItem::Equals(Object^ obj)
{
	//Check for null and compare run-time types.
	if (obj == nullptr || !this->GetType()->Equals(obj->GetType()))
	{
		return false;
	}
	else {
		try {
			DeleteItem^ temp = (DeleteItem^)obj;
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

String^ DeleteItem::getEventText()
{
	return gcnew String("Delete Event at " + _relativePath);
}
