#pragma once

#include "..\Socket\SocketHandler.h"

enum EventType {
    NullEvent = -1,
    FolderDetectedEvent,
    FileDetectedEvent,
    FileChangedEvent,
    DeletedEvent,
};

public ref class Event abstract {

protected:
    static String^ _rootWatchPath = nullptr;

    Int64 timeElapsedMSecs;

    EventType code;

    explicit Event(EventType eventCode, Int64 timeElapsed);

public:

    static void setRootPath(String^ path);

    EventType getEventCode();

    Int64 getTimeElapsed();

    virtual bool transmitEvent(SocketHandler^) = 0;

    virtual bool handleEvent(SocketHandler^) = 0;

    virtual String^ getEventText() = 0;
};