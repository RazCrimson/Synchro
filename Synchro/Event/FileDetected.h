#pragma once

#include "Event.h"

public ref class FileDetected : public Event 
{
protected:
	String^ _relativePath = nullptr;

public:

	explicit FileDetected(Int64 timeElapsed, String^ relPath);

	bool transmitEvent(SocketHandler^ socket) override;

	bool handleEvent(SocketHandler^ socket) override;

    bool Equals(Object^ obj) override;

	String^ getEventText() override;
    
};