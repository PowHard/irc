#ifndef SERVER_HPP
# define SERVER_HPP

#include "include.hpp"
#include "Command.hpp"

class Client;

class Server
{
	private:

		int 		socketFd;
		int 		port;
		std::string password;
		Command		cmd;

		std::vector<struct pollfd> pollFd;
		std::map<int, Client*> clients;  // FD -> Client
		std::map<std::string, Channel*> channels; //channelName - channel
		struct sockaddr_in serverAddr;
	
	public:

		// Coplien form

		Server(int port, std::string password);
		~Server();
		
		// Getter

		std::string getPassword() const;

		// Public method

		void 		initServer();
		void 		handleClientData(int i);
		void 		newConnection();
		void 		startServer();
		Channel* 	findChannel(const std::string name);
		Client*		findClientByNick(const std::string& nickname);
		void		addChannel(const std::string name, Channel* channel);
		bool		isNickRegistered(std::string nick);
		void		removeChan(std::string channelName);
		void		disconnectClient(int fd);
		void		quitAllChan(Client* client, std::string reason);
		void		setupSignal();
		void		freeAll();
		void		sendLinkedClients(std::string message, Client* client);
		void		updateChanMember(std::string newNick, Client* client);
};



#endif