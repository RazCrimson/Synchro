#pragma once

#include "..\Socket\SocketHandler.h"
#include "..\FileWatcher\FileWatcher.h"
#include "..\Event\Event.h"
#include "..\EventsQueue.h"

public ref class Synchronizer 
{
private:
	String^ _rootPath;

	Boolean _isServer;

	Stopwatch^ _stopWatch;
	
	SocketHandler^ _socket;	

	EventsQueue^ _eventsQueue;

	EventInitiator^ _eventInitiator;

public:

	explicit Synchronizer(SocketHandler^ socketPtr, String^ _rootWatchPath, Boolean isServer);

	Event^ getEventFromString(String^ data, Int64 timeElapsed);

	void synchronize();

};