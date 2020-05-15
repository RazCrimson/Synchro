#pragma once

#include "SocketHandler.h"

using namespace System::Net;

ref class ClientSocketHandler :	public SocketHandler
{
public:
	explicit ClientSocketHandler(IPAddress^ ,unsigned short int);

	void connect();
};

