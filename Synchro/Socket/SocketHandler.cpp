#include "SocketHandler.h"

SocketHandler::SocketHandler(IPAddress^ addr, UInt16 c_port)
{
	address = addr;
	
	port = c_port;
	
	client = nullptr;
}

bool SocketHandler::isValid()
{
	return client->Connected;
}

bool SocketHandler::send(array<Byte>^% data)
{
	
	if (!isValid())
		return false;
		
	client->Send(BitConverter::GetBytes(data->Length));
	Threading::Thread::Sleep(20);
		
	client->Send(data);
	Threading::Thread::Sleep(10);
		
	return true;	
}

array<Byte>^ SocketHandler::receive()
{
	array<Byte>^ byteArray;
	Int32 bytes = 0;
	
	byteArray = gcnew array<Byte>(4);
	bytes = client->Receive(byteArray, 0, 4 ,SocketFlags::None);
	
	bytes = BitConverter::ToInt32(byteArray, 0);
	byteArray = gcnew array<Byte>(bytes);
	bytes = client->Receive(byteArray, 0, bytes, SocketFlags::None);
	
	return byteArray;	
}
