#pragma once
#using <System.dll>

using namespace System;
using namespace System::IO;
using namespace System::Diagnostics;
using namespace System::Security::Permissions;

#include "..\Event\Event.h"
#include "..\EventsQueue.h"

public ref class FileWatcher
{
private:
    static String^ rootPath; 
    
    static Stopwatch^ stopWatch;

    static EventsQueue^ eventsQueue;   

    static String^ removeRootPath(String^);
    
    // To check if the changes to a file needs to be handled
    static bool FileValidator(String^ fullPath);

    static bool FolderValidator(String^ fullPath);

    // Define the event handlers.
    static void OnCreated(Object^, FileSystemEventArgs^);

    static void OnDeleted(Object^, FileSystemEventArgs^);

    static void OnChanged(Object^, FileSystemEventArgs^);

    static void OnRenamed(Object^, RenamedEventArgs^ );


public:
    static void initialize(String^ path, EventsQueue^ queue);

    [PermissionSet(SecurityAction::Demand, Name = "FullTrust")]
    static void run();

    static void enqueueFileDetected(String^ fullPath, Int64 timeElapsed);

    static void enqueueFolderDetected(String^ fullPath, Int64 timeElapsed);

};