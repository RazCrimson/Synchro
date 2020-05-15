#include "FileWatcher.h"

#include "..\Event\FolderDetected.h"
#include "..\Event\FileDetected.h"
#include "..\Event\FileChanged.h"
#include "..\Event\DeleteItem.h"

String^ EventInitiator::removeWatchItemPath(String^ path)
{
    Int32 index = path->IndexOf(Path, StringComparison::Ordinal);
    path = (index < 0) ? path : path->Remove(index, Path->Length);
    return path;
}

bool EventInitiator::FileValidator(String^ fullPath)
{
    // Check if file exists
    if (File::Exists(fullPath))
    {
        FileInfo^ file = gcnew FileInfo(fullPath);

        String^ path = gcnew String(fullPath);
        path = removeWatchItemPath(path);

        // Check if the file path and size is valid.
        if (file->Length > 5242880 || path->IndexOf("\\.") >= 0)
            return false;
        else
            return true;
    }
    return false;
}

bool EventInitiator::FolderValidator(String^ fullPath)
{
    // Check if file exists
    if (Directory::Exists(fullPath))
    {
        String^ path = gcnew String(fullPath);
        path = removeWatchItemPath(path); 

        // Check if the folder path is valid.
        if (path->IndexOf("\\.") >= 0)
            return false;
        else
            return true;
    }
    return false;
}

void EventInitiator::enqueueFileDetected(String^ path)
{
    if (EventInitiator::FileValidator(path))
    {
        path = removeWatchItemPath(path);

        Event^ event = gcnew FileDetected(_stopWatch->ElapsedMilliseconds, path);
        _eventsQueue->rateLimitedEnqueue(event, 1000);
    }
}

void EventInitiator::enqueueFolderDetected(String^ path)
{
    if (EventInitiator::FolderValidator(path)) 
    {
        path = removeWatchItemPath(path);

        Event^ event = gcnew FolderDetected(_stopWatch->ElapsedMilliseconds, path);
        _eventsQueue->rateLimitedEnqueue(event, 5000);
    }
}

void EventInitiator::enqueueFileChanged(String^ path)
{
    if (EventInitiator::FileValidator(path))
    {
        path = removeWatchItemPath(path);

        Event^ event = gcnew FileChanged(_stopWatch->ElapsedMilliseconds, path);
        _eventsQueue->rateLimitedEnqueue(event, 1000);
    }
}

void EventInitiator::enqueueFileOrFolderDeleted(String^ path)
{
    path = removeWatchItemPath(path);

    Event^ event = gcnew DeleteItem(_stopWatch->ElapsedMilliseconds, path);
    _eventsQueue->rateLimitedEnqueue(event, 5000);
    
}

void EventInitiator::OnFileOrFolderCreated(Object^ obj, FileSystemEventArgs^ e)
{
    EventInitiator^ eventInitiator = (EventInitiator^)obj;
    
    String^ fullPath = e->FullPath;
        
    // Handle the creation of a file/folder        

    if (Directory::Exists(fullPath))
    {
        eventInitiator->enqueueFolderDetected(fullPath);

        auto files = Directory::EnumerateFiles(fullPath, "*", SearchOption::AllDirectories);
        auto folders = Directory::EnumerateDirectories(fullPath, "*", SearchOption::AllDirectories);

        for each (String ^ folder in folders)
            eventInitiator->enqueueFolderDetected(folder);

        for each (String ^ file in files)
            eventInitiator->enqueueFileDetected(file);
    }
    else if (File::Exists(fullPath))
        eventInitiator->enqueueFileDetected(fullPath);
}

void EventInitiator::OnFileOrFolderDeleted(Object^ obj, FileSystemEventArgs^ e)
{
    EventInitiator^ eventInitiator = (EventInitiator^)obj;

    eventInitiator->enqueueFileOrFolderDeleted(e->FullPath);    
}

void EventInitiator::OnFileOrFolderChanged(Object^ obj, FileSystemEventArgs^ e)
{
    EventInitiator^ eventInitiator = (EventInitiator^)obj;

    eventInitiator->enqueueFileChanged(e->FullPath);
}

void EventInitiator::OnFileOrFolderRenamed(Object^ obj, RenamedEventArgs^ e)
{
    try {
        // Handle the renaming of a file or a folder (both are treated similarly)
        Console::WriteLine("File/Folder: {0} renamed to {1}", e->OldFullPath, e->FullPath);
    }
    catch (...)
    {
        Console::WriteLine("Exception on OnRenamed");
    }
}

void EventInitiator::addAllItemsToQueue()
{
    auto files = Directory::EnumerateFiles(Path, "*", SearchOption::AllDirectories);
    auto folders = Directory::EnumerateDirectories(Path, "*", SearchOption::AllDirectories);

    for each (String ^ folder in folders)
        enqueueFolderDetected(folder);

    for each (String ^ filePath in files)
        enqueueFileDetected(filePath);
}

EventInitiator::EventInitiator(String^ path, EventsQueue^ queue, Stopwatch^ watch)
{
    _eventsQueue = queue;

    _stopWatch = watch;

    Event::setRootPath(path);

    this->Path = path;
    
    // Watch for changes in LastAccess and LastWrite times, and the renaming of files or directories.
    this->NotifyFilter = static_cast<NotifyFilters>(NotifyFilters::LastAccess |
        NotifyFilters::LastWrite | NotifyFilters::FileName | NotifyFilters::DirectoryName);

    // Recursively watch all subdirectories.
    this->IncludeSubdirectories = true;

    // Add event handlers.
    this->Changed += gcnew FileSystemEventHandler(EventInitiator::OnFileOrFolderChanged);
    this->Created += gcnew FileSystemEventHandler(EventInitiator::OnFileOrFolderCreated);
    this->Deleted += gcnew FileSystemEventHandler(EventInitiator::OnFileOrFolderDeleted);
    this->Renamed += gcnew RenamedEventHandler(EventInitiator::OnFileOrFolderRenamed);


    addAllItemsToQueue();
}

void EventInitiator::setActive(Boolean option)
{
    this->EnableRaisingEvents = option;
}

