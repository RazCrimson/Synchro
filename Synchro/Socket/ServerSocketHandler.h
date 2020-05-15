#pragma once

#include "SocketHandler.h"

ref class ServerSocketHandler :	public SocketHandler
{
private:
	Socket^ listener = nullptr;

public:
	explicit ServerSocketHandler(IPAddress^, unsigned short int);

	void waitForConnection();

};

