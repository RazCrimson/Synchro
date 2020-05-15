#include "Synchronizer.h"

#include "..\FileWatcher\FileWatcher.h"

#include "..\Event\FolderDetected.h"
#include "..\Event\FileDetected.h"
#include "..\Event\FileChanged.h"
#include "..\Event\Deleted.h"

using namespace System::IO;
using namespace System::Text;
using namespace System::Threading;

Synchronizer::Synchronizer(SocketHandler^ socketPtr, EventsQueue^ queue, String^ path)
{
    _rootPath = path;
    _socket = socketPtr;
    _eventsQueue = queue;

    ThreadStart^ threadDelegate = gcnew ThreadStart(this, &Synchronizer::synchronize);
    Thread^ newThread = gcnew Thread(threadDelegate);
    newThread->Start();
}

void Synchronizer::addAllToQueue(Int64 timeElaspsed)
{
    auto files = Directory::EnumerateFiles(_rootPath, "*", SearchOption::AllDirectories);
    auto folders = Directory::EnumerateDirectories(_rootPath, "*", SearchOption::AllDirectories);

    for each (String ^ folder in folders)
    {
        FileWatcher::enqueueFolderDetected(folder, timeElaspsed);
    }

    for each (String ^ file in files)
    {
        FileWatcher::enqueueFileDetected(file, timeElaspsed);
    }
}

Event^ Synchronizer::getEventFromString(String^ data, Int64 timeElapsed)
{
    Event^ event = nullptr;
    char eventCode = data[0] - 48;
    data = data->Remove(0, 2);
    if (eventCode == FolderDetectedEvent)
    {
        event = gcnew FolderDetected(timeElapsed, data);
    }
    else if (eventCode == FileDetectedEvent)
    {
        event = gcnew FileDetected(timeElapsed, data);
    }
    else if (eventCode == FileChangedEvent)
    {
        event = gcnew FileChanged(timeElapsed, data);
    }
    else if (eventCode == DeletedEvent)
    {
        event = gcnew Deleted(timeElapsed, data);
    }
    return event;
}

void Synchronizer::synchronize()
{
    String^ sendString, ^ receivedString;
    array<Byte>^ byteArray;
    Event^ event;
    Int64 lastTime = Int64::MaxValue;

    try {
        bool isServer = 0;
        addAllToQueue(!isServer * 100);

        if (!isServer)
        {
            sendString = gcnew String("Have Event?");
            byteArray = Encoding::Unicode->GetBytes(sendString);
            _socket->send(byteArray);
        }
    }
    catch (...)
    {
        Console::WriteLine("ERROR in Event Creation at Synchronizer");
    }

    while (1)
    {
        byteArray = _socket->receive();
        receivedString = Encoding::Unicode->GetString(byteArray);

        try
        {
            if (receivedString == "Have Event?")
            {
                event = _eventsQueue->Peek();
                lastTime = event->getTimeElapsed();
                sendString = "Event Time|" + lastTime.ToString();
            }
            else if (receivedString->Contains("Event Time|"))
            {
                String^ temp = gcnew String("Event Time|");
                int index = receivedString->IndexOf(temp, StringComparison::Ordinal);
                receivedString = (index < 0) ? receivedString : receivedString->Remove(index, temp->Length);
                auto currentEventTime = Int64::Parse(receivedString);
                if (lastTime >= currentEventTime)
                {
                    lastTime = currentEventTime;
                    sendString = gcnew String("Transmit Event");
                }
                else
                {
                    lastTime = event->getTimeElapsed();
                    sendString = gcnew String("Ask");
                }
            }
            else if (receivedString == "Transmit Event")
            {
                _eventsQueue->dequeue();
                event->transmitEvent(_socket);
                sendString = gcnew String("Have Event?");
            }
            else if (receivedString->Equals("Ask"))
                sendString = gcnew String("Have Event?");

            byteArray = Encoding::Unicode->GetBytes(sendString);
            _socket->send(byteArray);

            if (sendString == "Transmit Event")
            {
                byteArray = _socket->receive();
                receivedString = Encoding::Unicode->GetString(byteArray);

                event = getEventFromString(receivedString, lastTime);
                event->handleEvent(_socket);

                auto Node = _eventsQueue->FindLast(event);
                if (Node != nullptr)
                    _eventsQueue->Remove(Node);

                Console::WriteLine("Received Event");
            }

            continue;

        }
        catch (InvalidOperationException^ e)
        {
            
        }
        catch (SystemException^ e)
        {
            Console::WriteLine("System Exception: {0}", e->ToString());
        }
        catch (...)
        {
            Console::WriteLine("ERROR!!");
        }

        sendString = gcnew String("Have Event?");
        byteArray = Encoding::Unicode->GetBytes(sendString);
        _socket->send(byteArray);
        lastTime = Int64::MaxValue;
        Threading::Thread::Sleep(2000);
    }
}