#pragma once
#include "Event.h"

public ref class Deleted :public Event 
{
private:
	String^ _relativePath = nullptr;

public:

	explicit Deleted(Int64 timeElapsed, String^ relPath);

	bool transmitEvent(SocketHandler^ socket) override;

	bool handleEvent(SocketHandler^ socket) override;
	
	bool Equals(Object^ obj) override;
};