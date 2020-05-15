#pragma once

using namespace System;
using namespace System::IO;
using System::Diagnostics::Stopwatch;
using namespace System::Security::Permissions;

#include "..\EventsQueue.h"

private ref class EventInitiator : public FileSystemWatcher
{
private:
    Stopwatch^ _stopWatch;

    EventsQueue^ _eventsQueue; 

    String^ removeWatchItemPath(String^);
    
    // Define the event handlers.
    static void OnFileOrFolderCreated(Object^, FileSystemEventArgs^);

    static void OnFileOrFolderDeleted(Object^, FileSystemEventArgs^);

    static void OnFileOrFolderChanged(Object^, FileSystemEventArgs^);

    static void OnFileOrFolderRenamed(Object^, RenamedEventArgs^);
    
    // To check if the changes to a file/folder needs to be handled
    bool FileValidator(String^ fullPath);

    bool FolderValidator(String^ fullPath);

    // Enqueue 
    void enqueueFileDetected(String^ fullPath);

    void enqueueFileChanged(String^ fullPath);
    
    void enqueueFolderDetected(String^ fullPath);    

    void enqueueFileOrFolderDeleted(String^ fullPath);


public:

    explicit EventInitiator(String^ path, EventsQueue^ queue, Stopwatch^ watch);

    void addAllItemsToQueue();
    
    [PermissionSet(SecurityAction::Demand, Name = "FullTrust")]
    void setActive(Boolean);   

};