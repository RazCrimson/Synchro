#include "Synchronizer.h"

#include "..\FileWatcher\FileWatcher.h"

#include "..\Event\FolderDetected.h"
#include "..\Event\FileDetected.h"
#include "..\Event\FileChanged.h"
#include "..\Event\DeleteItem.h"

#include "..\EventsQueue.h"

using System::Text::Encoding;

Synchronizer::Synchronizer(SocketHandler^ socketPtr, String^ path, Boolean isServer)
{
    _rootPath = path;

    _socket = socketPtr;

    _isServer = isServer;
    
    _stopWatch = Stopwatch::StartNew();
    
    _eventsQueue = gcnew EventsQueue();
    
    _eventInitiator = gcnew EventInitiator(_rootPath, _eventsQueue, _stopWatch);

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
        event = gcnew DeleteItem(timeElapsed, data);
    }
    return event;
}

void Synchronizer::synchronize()
{

    String^ sendString, ^ receivedString; 
    Int64 lastTime = Int64::MaxValue;
    array<Byte>^ byteArray;
    Event^ event;

    if (!_isServer)
        _stopWatch->Restart();

    _eventInitiator->setActive(true);

    try {
        

        if (!_isServer)
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
                    Console::WriteLine("Found an Event");
                }
                else
                {
                    lastTime = event->getTimeElapsed();
                    sendString = gcnew String("Ask");
                }
            }
            else if (receivedString == "Transmit Event")
            {
                Console::WriteLine("Sending {0}", event->getEventText());
                _eventsQueue->dequeue();
                event->transmitEvent(_socket);
                Console::WriteLine("Sent Event");

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
                Console::WriteLine("Receiving {0}", event->getEventText());
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
        catch (System::Net::Sockets::SocketException^ e)
        {
            Console::WriteLine("It seems that the connection was terminated :(");
            return;
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
        Threading::Thread::Sleep(500);
    }
}