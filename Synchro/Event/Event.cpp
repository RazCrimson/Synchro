#include "Event.h"

Event::Event(EventType eventCode, Int64 timeElapsed)
{
	code = eventCode;
	timeElapsedMSecs = timeElapsed;
}

void Event::setRootPath(String^ path)
{
	rootPath = path;
}

EventType Event::getEventCode()
{
	return code;
}


Int64 Event::getTimeElapsed()
{
	return timeElapsedMSecs;
}
