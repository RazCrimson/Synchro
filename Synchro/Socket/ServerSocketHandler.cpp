#include "ServerSocketHandler.h"

using namespace System::Net;

ServerSocketHandler::ServerSocketHandler(IPAddress^ addr, unsigned short int c_port) : SocketHandler(addr,c_port)
{
	listener = gcnew Socket(address->AddressFamily,SocketType::Stream, ProtocolType::Tcp);
    IPEndPoint^ endpoint = gcnew IPEndPoint(address, port);
	listener->Bind(endpoint);
    listener->Listen(1);
	Console::WriteLine("Started listening on port : {0}", port);
}

void ServerSocketHandler::waitForConnection()
{
    Console::Write("Waiting for a connection... ");
    
    client = listener->Accept();
    Console::WriteLine("Connected!");
}
