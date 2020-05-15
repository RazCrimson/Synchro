#pragma once

#include "..\pch.h"

#include "Event.h"

#include "..\BloomFilter\BloomFilter.h"

using namespace System::Collections::Generic;

public ref class FileChanged : public Event
{
private:
	String^ _relativePath = nullptr;

	Dictionary<UInt32, String^>^ getMissingContent(BloomFilter^);

	void syncFile(Dictionary<UInt32, String^>^, Dictionary<UInt32, String^>^);

public:

	explicit FileChanged(Int64 timeElapsed, String^ relPath);

	bool transmitEvent(SocketHandler^ socket) override;

	bool handleEvent(SocketHandler^ socket) override;

	bool Equals(Object^ obj) override;

	String^ getEventText() override;
};