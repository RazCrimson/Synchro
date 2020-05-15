#include "FolderDetected.h"

using namespace System::IO;

FolderDetected::FolderDetected(Int64 timeElapsed, String^ relPath): Event(FolderDetectedEvent,timeElapsed)
{
	_relativePath = relPath;
}

bool FolderDetected::transmitEvent(SocketHandler^ socket)
{
	if (Directory::Exists(_rootWatchPath + _relativePath))
	{
		String^ eventDataString = String::Concat(char(FolderDetectedEvent), "|", _relativePath);
		array<Byte>^ eventData = Text::Encoding::Unicode->GetBytes(eventDataString);
		return socket->send(eventData);
	}
	return false;
}

bool FolderDetected::handleEvent(SocketHandler^ socket)
{
	if (!Directory::Exists(_rootWatchPath + _relativePath))
		Directory::CreateDirectory(_rootWatchPath + _relativePath);
	return true;
}

bool FolderDetected::Equals(Object^ obj)
{
	//Check for null and compare run-time types.
	if (obj == nullptr || !this->GetType()->Equals(obj->GetType()))
	{
		return false;
	}
	else {
		try 
		{
			FolderDetected^ temp = (FolderDetected^)obj;

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

String^ FolderDetected::getEventText()
{
	return gcnew String("Folder Detected Event at " + _relativePath);
}
