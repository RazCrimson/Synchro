#pragma once

using namespace System;
using namespace System::Net;
using namespace System::Net::Sockets;


public ref class SocketHandler abstract
{
protected:
	static UInt32 receiveBufferSize = 16384;

	IPAddress^ address;

	UInt16 port;

	Socket^ client;

public:

	explicit SocketHandler(IPAddress^, UInt16);

	bool isValid();

	bool send(array<Byte>^%);

	array<Byte>^ receive();
};
