#include "FileWatcher/FileWatcher.h"
#include "Synchronizer/Synchronizer.h"
#include "Socket/ClientSocketHandler.h"
#include "Socket/ServerSocketHandler.h"
#include "Event/Event.h"
#include "EventsQueue.h"


using namespace System::Threading;

int main() {
	
	String^ path = "C:\\Users\\bhara\\Desktop\\test2";

	ClientSocketHandler^ servsoc = gcnew ClientSocketHandler(IPAddress::Parse("127.0.0.1"),64000);
	servsoc->connect();

	SocketHandler^ socket = servsoc;
	EventsQueue^ queue = gcnew EventsQueue();

	FileWatcher::initialize(path, queue);
	
	Synchronizer^ sync = gcnew Synchronizer(socket, queue, path);

	FileWatcher::run();	

} 