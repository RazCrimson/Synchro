#include "FileWatcher.h"

#include "..\Event\FolderDetected.h"
#include "..\Event\FileDetected.h"
#include "..\Event\FileChanged.h"
#include "..\Event\Deleted.h"

String^ FileWatcher::removeRootPath(String^ path)
{
    Int32 index = path->IndexOf(rootPath, StringComparison::Ordinal);
    path = (index < 0) ? path : path->Remove(index, rootPath->Length);
    return path;
}

bool FileWatcher::FileValidator(String^ fullPath)
{
    // Check if file exists
    if (File::Exists(fullPath))
    {
        FileInfo^ file = gcnew FileInfo(fullPath);

        String^ path = gcnew String(fullPath);
        path = removeRootPath(path);

        // Check if the file path and size is valid.
        if (file->Length > 5242880 || path->IndexOf("\\.") >= 0)
            return false;
        else
            return true;
    }
    return false;
}

bool FileWatcher::FolderValidator(String^ fullPath)
{
    // Check if file exists
    if (Directory::Exists(fullPath))
    {
        String^ path = gcnew String(fullPath);
        path = removeRootPath(path); 

        // Check if the folder path is valid.
        if (path->IndexOf("\\.") >= 0)
            return false;
        else
            return true;
    }
    return false;
}

void FileWatcher::enqueueFileDetected(String^ path, Int64 timeElapsed)
{
    if (FileWatcher::FileValidator(path))
    {
        Console::WriteLine("File: {0} Detected", path);
        path = removeRootPath(path);

        Event^ event = gcnew FileDetected(timeElapsed, path);
        eventsQueue->rateLimitedEnqueue(event, 1000);
    }
}

void FileWatcher::enqueueFolderDetected(String^ path, Int64 timeElapsed)
{
    if (FileWatcher::FolderValidator(path)) 
    {
        Console::WriteLine("Folder: {0} Detected", path);
        path = removeRootPath(path);

        Event^ event = gcnew FolderDetected(timeElapsed, path);
        eventsQueue->rateLimitedEnqueue(event, 5000);
    }
}

void FileWatcher::OnCreated(Object^, FileSystemEventArgs^ e)
{
    try {
        // Handle the creation of a file/folder
        String^ fullPath = e->FullPath;

        if (Directory::Exists(fullPath))
        {
            enqueueFolderDetected(fullPath, stopWatch->ElapsedMilliseconds);

            auto files = Directory::EnumerateFiles(fullPath, "*", SearchOption::AllDirectories);
            auto folders = Directory::EnumerateDirectories(fullPath, "*", SearchOption::AllDirectories);

            for each (String ^ folder in folders)
            {
                enqueueFolderDetected(folder, stopWatch->ElapsedMilliseconds);
            }

            for each (String ^ file in files)
            {
                enqueueFileDetected(file, stopWatch->ElapsedMilliseconds);
            }
        }
        else if (File::Exists(fullPath))
        {
            enqueueFileDetected(fullPath, stopWatch->ElapsedMilliseconds);
        }
    }
    catch (...)
    {
        Console::WriteLine("Exception on OnCreated");
    }
}

void FileWatcher::OnDeleted(Object^, FileSystemEventArgs^ e)
{
    String^ path = e->FullPath;
    try {
        // Handle the deletion of a file or a folder (both are treated similarly)
        Console::WriteLine("File/Folder: {0} Deleted", path);
        String^ path = e->FullPath;
        
        Event^ event = gcnew Deleted(stopWatch->ElapsedMilliseconds, path);
        eventsQueue->rateLimitedEnqueue(event, 1000);
    }
    catch (...)
    {
        Console::WriteLine("Exception on OnDeleted");
    }
}

void FileWatcher::OnChanged(Object^, FileSystemEventArgs^ e)
{
    try {
        // Handle the change of file contents 
        String^ path = e->FullPath;
        if (FileWatcher::FileValidator(path))
        {
            Console::WriteLine("File: {0} Changed", path);
            path = removeRootPath(path);

            Event^ event = gcnew FileChanged(stopWatch->ElapsedMilliseconds, path);
            eventsQueue->rateLimitedEnqueue(event, 1000);
        }
        // Files more than 1MB are ignored
    }
    catch (...)
    {
        Console::WriteLine("Exception on OnChanged");
    }
}

void FileWatcher::OnRenamed(Object^, RenamedEventArgs^ e)
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

void FileWatcher::initialize(String^ path, EventsQueue^ queue)
{
    eventsQueue = queue;
    rootPath = path;
    Event::setRootPath(path);
}

void FileWatcher::run()
{
    // Initializing value
    stopWatch = gcnew Stopwatch();

    // Create a new FileSystemWatcher and set its properties.
    FileSystemWatcher^ watcher = gcnew FileSystemWatcher;
    watcher->Path = rootPath;

    /* Watch for changes in LastAccess and LastWrite times, and
        the renaming of files or directories. */
    watcher->NotifyFilter = static_cast<NotifyFilters>(NotifyFilters::LastAccess |
        NotifyFilters::LastWrite | NotifyFilters::FileName | NotifyFilters::DirectoryName);

    // Recursively watch all subdirectories.
    watcher->IncludeSubdirectories = true;

    // Add event handlers.
    watcher->Changed += gcnew FileSystemEventHandler(FileWatcher::OnChanged);
    watcher->Created += gcnew FileSystemEventHandler(FileWatcher::OnCreated);
    watcher->Deleted += gcnew FileSystemEventHandler(FileWatcher::OnDeleted);
    watcher->Renamed += gcnew RenamedEventHandler(FileWatcher::OnRenamed);

    // Begin watching.
    stopWatch->Start();
    watcher->EnableRaisingEvents = true;

    while (1);
}