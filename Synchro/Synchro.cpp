#include <string>

#using <System.dll>

#include "CLI11.hpp" // Pure C++ Command-line parser from https://github.com/CLIUtils/CLI11

#include "Synchronizer/Synchronizer.h"

#include "Socket/ClientSocketHandler.h"
#include "Socket/ServerSocketHandler.h"


int main(int argc, char** argv) {

	CLI::App app{"\nSynchro - is a real-time file/folder synchronizer. Requires atleast .NET Framework 4.5\n\nGitHub: https://github.com/RazCrimson3/Synchro \n"};
		
	std::string address = "";
	app.add_option("-a,--address", address, "Used to specify a IP Address to connect/listen to. It is Mandatory.");

	std::string pathToDir = "";
	app.add_option("-p,--path", pathToDir, "Used to sepecify the path to the Directory to be Monitored. By default, current working directory will be used!");	

	bool isServer = false;
	app.add_flag("--host,!--client", isServer, "--host to make this instance as the server. --client to make it a client. By default a client instance is created.");

	CLI11_PARSE(app, argc, argv);

	String^ path = gcnew String(pathToDir.c_str());
	String^ ipAddress = gcnew String(address.c_str());
	SocketHandler^ socket;

	if (path == "")
		path = Directory::GetCurrentDirectory();
	

	try
	{
		if (isServer)
		{
			ServerSocketHandler^ serverSocket = gcnew ServerSocketHandler(IPAddress::Parse(ipAddress), 64000);
			serverSocket->waitForConnection();
			socket = serverSocket;
		}
		else
		{
			ClientSocketHandler^ clientSocket = gcnew ClientSocketHandler(IPAddress::Parse(ipAddress), 64000);
			clientSocket->connect();
			socket = clientSocket;
		}

		if (Directory::Exists(path))
		{
			Synchronizer^ synchronizer = gcnew Synchronizer(socket, path, isServer);

			synchronizer->synchronize();
		}
		else
		{
			Console::WriteLine("Can't Find a Folder in the path: {0}", path);
		}
	}
	catch(System::Net::Sockets::SocketException^)
	{
		Console::WriteLine("It Seems the Host was not Online  :(");
	}
	catch (System::FormatException^)
	{
		Console::WriteLine("Please give a valid IP Address. Use the --help option for more information");
	}
	
} 