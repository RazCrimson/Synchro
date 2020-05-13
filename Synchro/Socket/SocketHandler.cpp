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
	try {
		if (!isValid())
			return false;
		client->Send(BitConverter::GetBytes(data->Length));
		Threading::Thread::Sleep(30);
		client->Send(data);
		Console::WriteLine("Sent: {0}", Text::Encoding::Unicode->GetString(data));
		return true;
	}
	catch (System::Net::Sockets::SocketException^ e)
	{
		Console::WriteLine("Socket Exception: {0}", e->ToString());
	}
}

array<Byte>^ SocketHandler::receive()
{
	array<Byte>^ byteArray;
	Int32 bytes = 0;
	try {
		byteArray = gcnew array<Byte>(4);
		bytes = client->Receive(byteArray, 0, 4 ,SocketFlags::None);
		if (bytes != 4)
			Console::WriteLine("Integer not read properly!");
		bytes = BitConverter::ToInt32(byteArray, 0);
		byteArray = gcnew array<Byte>(bytes);
		bytes = client->Receive(byteArray, 0, bytes, SocketFlags::None);
		//Console::WriteLine("Received: {0}", Text::Encoding::Unicode->GetString(data));
		return byteArray;
	}
	catch (System::Net::Sockets::SocketException^ e)
	{
		Console::WriteLine("Socket Exception: {0}", e->ToString());
	}
}
