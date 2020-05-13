#include "ClientSocketHandler.h"

ClientSocketHandler::ClientSocketHandler(IPAddress^ addr, unsigned short int c_port) : SocketHandler(addr, c_port)
{
	address = addr;
	client = gcnew Socket(address->AddressFamily, SocketType::Stream, ProtocolType::Tcp);
	
}

void ClientSocketHandler::connect()
{
	Console::WriteLine("Connecting to {0} on port {1}", address, port);

	IPEndPoint^ endpoint = gcnew IPEndPoint(address, port);
	client->Connect(address, port);
	Console::WriteLine("Connected!");
}
