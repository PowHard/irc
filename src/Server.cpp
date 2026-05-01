#include "../include/Server.hpp"
#include "../include/include.hpp"

bool runningServ = true; // to handle SIGINT

/**
 * @param AF_INET ipv4 protocol
 * @param SOCK_STREAM TCP socket
 */
Server::Server(int port, std::string pass)
{
	this->socketFd = socket(AF_INET, SOCK_STREAM, 0);
	this->port = port;
	this->password = pass;
	cmd.setServer(this);
}
Server::~Server()
{
}

// Getter

std::string Server::getPassword() const
{
	return this->password;
}

// Public method

/**
 * @brief Create a server fd and initialize poll/sockaddr struct
 * @param setsockopt set socket option
 * 
 * \li - SOL_SOCKET: Socket level, the scope is only the current socket
 * 
 * \li - SO_REUSEADDR: Give the permission to use the same adresse again and again for one port. 
 * 
 * @param fcntl open fd and configure it:
 * 
 * \li - F_SETFL: Set File Status Flags, overwrites actual flags
 * 
 * \li - O_NONBLOCK: flag non blocking to accept(), connect(), recv(), et send(). 
 * 			if no data socket function return directly −1 and global variable errno is define to EAGAIN or EWOULDBLOCK.
 * @param sockadrr => serverAddr component
 * 
 * \li - AF_INET type of adress, this specify the ipv4 adress (AF_INET6 for ipv6)
 * 
 * \li - htons(this->port): Host To Network Short, convert port to short int (16 bit) for the server to read it => cf big enddian / little endian
 * 
 * \li - INADDR_ANY: The serv accept all local network interface (wifi, internet, loopback)
 * 
 * @param bind() : Ip and port association to one socket
 * 
 * @param listen() : put socket in listening mode to handle input connexion
 * 
 * \li - the backlog is 10 because we don't handle lot of connection in this project.
 * 			backlog is a queue if to handle connection peak
 * @param pollstruct handle events and re-events: wait to read, write, error with appropriate flags
*/
void Server::initServer()
{
	if (this->socketFd < 0)
		std::cerr << "Error: problem when creating socket" << std::endl;

	fcntl(this->socketFd, F_SETFL, O_NONBLOCK);
	int reuse = 1;
	if (setsockopt(this->socketFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) 
	{
		std::cerr << "Error setsockopt(SO_REUSEADDR): " << std::endl;
		close(this->socketFd);
		exit(EXIT_FAILURE);
	}
	this->serverAddr.sin_family = AF_INET;
	this->serverAddr.sin_port = htons(this->port);
	this->serverAddr.sin_addr.s_addr = INADDR_ANY;

	bind(this->socketFd, (struct sockaddr*)&this->serverAddr, sizeof(this->serverAddr));
	listen(this->socketFd, 10);

	struct pollfd listenerPollFd;
	listenerPollFd.fd = this->socketFd;
	listenerPollFd.events = POLLIN; // waiting reading (new connection)
	listenerPollFd.revents = 0;

	this->pollFd.push_back(listenerPollFd);
}

/**
 * @brief handle receive message and parse the command
 * @param ssize_t Use to count bytes
 * @param buffer 1024 octets => conventional size to read TCP (three control protocol) data
*/
void Server::handleClientData(int i)
{
	int clientFd = pollFd[i].fd;
	Client* client(this->clients[clientFd]);
	char buffer[1024];

	ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead > 0)
	{
		buffer[bytesRead] = '\0';
		client->setBuffer(buffer);
		cmd.extractCompleteCommand(client);
		if (client->getWillDisconnect())
		{
			disconnectClient(pollFd[i].fd); // Fd of the current client
			return;
		}
	}
	else if (bytesRead == 0)
	{
		disconnectClient(pollFd[i].fd);
	}
	else //error
	{ 
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		else 
		{
			disconnectClient(pollFd[i].fd);
		}
	}
}

/**
 * @param sockaddr_in socket struct adress ipv4 (sockaddr_in6 for ipv6)
 * @param accept extract connection to create a new fd with the three step handshake (SYN: Synchronize, SYN-ACK: Synchronize-Acknowledge, ACK: Acknowledge)
*/
void Server::newConnection()
{
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

 	int clientFd = accept(socketFd, (struct sockaddr*)&addr, &addrlen);
	if (clientFd >= 0)
	{
		Client* newClient = new Client(clientFd);
		this->clients[clientFd] = newClient;
		fcntl(clientFd, F_SETFL, O_NONBLOCK);
		struct pollfd newPollFd; // to push it in the vector pollFd and we aren't limits by number of user
		newPollFd.fd = clientFd;
		newPollFd.events = POLLIN | POLLOUT; // wire reading and writing
		newPollFd.revents = 0;

		this->pollFd.push_back(newPollFd);
		std::cout << "new client connected: " << clientFd << std::endl;
	}
	else
	{
		  if (errno != EAGAIN && errno != EWOULDBLOCK)
		  	std::cerr << "Erreur accept(): " << std::endl;
	}
}

/**
 * @brief events handling
*/
void Server::startServer()
{
	setupSignal();
	while (runningServ)
	{
		int waitPoll = poll(pollFd.data(), pollFd.size(), 5000);
		if (!runningServ) // Kill program
			break;
		if (waitPoll > 0)
		{
			for (int i = pollFd.size() - 1; i >= 0 ; i--)
			{
				if (pollFd[i].revents == 0) // no events
					continue;
				if (pollFd[i].fd == socketFd) // add new client
					newConnection();
				else // recev data by one client
					handleClientData(i);
			
			}
		}
	}
	freeAll();
}

/**
 * @brief Find Channel
 * @return name of channel if exist, else return NULL
*/
Channel* Server::findChannel(const std::string name)
{
 	std::cout << "Chanel name: '" << name << "'" << std::endl; // TEMP
	
	std::map<std::string, Channel*>::iterator it = channels.find(name);
	if (it != channels.end())
		return it->second;
	return (NULL);
}

Client* Server::findClientByNick(const std::string& nickname)
{
	std::map<int, Client*>::iterator it = this->clients.begin();
	while (it != this->clients.end())
	{
		if (it->second->getNickName() == nickname)
			return (it->second);
		it++;
	}
	return (NULL);
}

void Server::addChannel(const std::string name, Channel* channel)
{
	channels[name] = channel;
}

bool Server::isNickRegistered(std::string nick)
{
	std::map<int, Client*>::iterator it = this->clients.begin();
	while (it != this->clients.end())
	{
		if (nick == it->second->getNickName())
			return (true);
		it++;
	}
	return (false);
}

void Server::removeChan(std::string channelName)
{
	std::map<std::string, Channel*>::iterator it;
	it = channels.find(channelName);
	if (it != channels.end())
	{
		delete it->second;
		it->second = NULL;
		channels.erase(it);
	}
}
/**
 * @brief delete client and pollFd
*/
void Server::disconnectClient(int clientFd)
{
	std::map<int, Client*>::iterator it = clients.find(clientFd);
	if (it == clients.end())
		return;
	
	Client* client = it->second;
	
	quitAllChan(client, "Client disconnected");
	delete client;
	clients.erase(it);
	close(clientFd);
	for (size_t i = 0; i < pollFd.size(); ++i)
	{
		if (pollFd[i].fd == clientFd)
		{
			pollFd.erase(pollFd.begin() + i);
			break;
		}
	}
}
/**
 * @brief Delete all channel of the current client
*/
void Server::quitAllChan(Client* client, std::string reason)
{
	std::map<std::string, Channel*>::iterator it;
	it = channels.begin();

	while (it != channels.end())
	{
		Channel* channel = it->second;
		std::map<std::string, Channel*>::iterator current = it;
		it++;
		if (channel->isMember(client))
		{
			std::string quitMsg = ":" + client->getNickName() + "!"
								+ client->getUser() + "@"
								+ client->getHostname()
								+ " QUIT :" + reason + "\r\n";
			channel->sendAllChanExcept(quitMsg, client);
			channel->removeMember(client);
			if (channel->chanIsEmpty())
			{
				delete channel;
				channels.erase(current);
			}
		}
	}
}
/**
 * @brief Modified flag runningServ if we have signal CTR + c (SIGINT)
*/
void sigint(int sig)
{
	if (sig == SIGINT)
		runningServ = false;
}
/**
 * @brief sigaction configuration (handling signal)
 * @param sigemptyset initialize sa_mask to handle each signal by each signal and not in the same time
 * @param sa_handler take a function to handle signal consequence
 * @param sigaction Link SIGINT signal to sa_handler to do sigint function
 * 
*/
void Server::setupSignal()
{
	struct sigaction act;
	act.sa_flags = 0; // No option
	sigemptyset(&act.sa_mask);
	act.sa_handler = &sigint;
	sigaction(SIGINT, &act, NULL);
}
/**
 * @brief delete Clients Channels and close socket/poll
*/
void Server::freeAll()
{
	std::map<int, Client*>::iterator itClient;
	std::map<std::string, Channel*>::iterator itChan;
	itClient = clients.begin();
	itChan = channels.begin();
	while (itChan != channels.end())
	{
		if (itChan->second)
		{	delete itChan->second;
			itChan->second = NULL;
		}
		itChan++;
	}
	channels.clear();


	while (itClient != clients.end())
	{
		if (itClient->second)
		{	
			close(itClient->first);
			delete itClient->second;
			itClient->second = NULL;
		}
		itClient++;
	}
	clients.clear();

	if (socketFd >= 0)
	{
		close(socketFd);
		socketFd = -1; // Secure socket close
	}
	pollFd.clear();
}

/**
 * @brief Send a message to all the clients who share a channel with a client
 * @param buffer The message to send
 * @param client The pointer of the client
 */
// void Server::sendLinkedClients(std::string buffer, Client* client)
// {
// 	std::map<std::string, Client*> linkedClients;
// 	std::map<std::string, Client*>::iterator itLinked;
// 	std::map<std::string, Channel*>::iterator itChannels;

// 	itChannels = this->channels.begin();
// 	while (itChannels != this->channels.end())
// 	{
// 		Channel* channel = itChannels->second;
// 		std::map<std::string, Channel*>::iterator current = itChannels;
// 		itChannels++;
// 		if (channel->isMember(client))
// 		{
			
// 		}
// 	}
// }

void Server::sendLinkedClients(std::string message, Client* client)
{
	std::map<std::string, Client*> linkedClients;
	std::map<std::string, Client*>::iterator itLinked;
	std::map<std::string, Channel*>::iterator itChannels;

	itChannels = this->channels.begin();
	while (itChannels != this->channels.end())
	{
		Channel* channel = itChannels->second;
		if (channel->isMember(client))
		{
			std::map<std::string, Client*>& channelClients = channel->getMembers();
			std::map<std::string, Client*>::iterator itClients;
			itClients = channelClients.begin();
			while (itClients != channelClients.end())
			{
				Client* currentClient = itClients->second;
				if (currentClient != client)
					linkedClients[currentClient->getNickName()] = currentClient;
				++itClients;
			}
		}
		++itChannels;
	}
	itLinked = linkedClients.begin();
	while (itLinked != linkedClients.end())
	{
		send(itLinked->second->getFd(), message.c_str(), message.length(), 0);
		++itLinked;
	}
}

void Server::updateChanMember(std::string newNick, Client* client)
{
	std::map<std::string, Channel*>::iterator itChannels;

	itChannels = this->channels.begin();
	while (itChannels != this->channels.end())
	{
		Channel* channel = itChannels->second;
		if (channel->isMember(client))
		{
			std::map<std::string, Client*>& channelClients = channel->getMembers();
			std::map<std::string, Client*>::iterator itClients;
			itClients = channelClients.begin();
			while (itClients != channelClients.end())
			{
				Client* currentClient = itClients->second;
				if (currentClient == client)
				{
					channelClients.erase(itClients);
					channelClients[newNick] = client;
					break;
				}
				++itClients;
			}
		}
		++itChannels;
	}
}
