#pragma once
#include "..\Event\Event.h"
#include "..\Socket\SocketHandler.h"
#include "..\EventsQueue.h"

public ref class Synchronizer 
{
private:
	String^ _rootPath;
	SocketHandler^ _socket;
	EventsQueue^ _eventsQueue;

public:

	explicit Synchronizer(SocketHandler^ socketPtr, EventsQueue^ queue, String^ rootPath);

	void addAllToQueue(Int64 timeElaspsed);

	Event^ getEventFromString(String^ data, Int64 timeElapsed);

	void synchronize();

};