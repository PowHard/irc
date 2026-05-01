#include "../include/Channel.hpp"
#include "../include/include.hpp"

Channel::Channel(std::string name, Client* clientOp) : name(name), password(""), topic(""), limitMember(0), nbMember(0), invitedOnly(false), topicRestricted(false)
{
	members[clientOp->getNickName()] = clientOp;
	nbMember = 1;
	operators[clientOp->getNickName()] = clientOp;
}

Channel::~Channel() {}

// Getters 

std::map<std::string, Client*>& Channel::getMembers() { return members; }

std::string Channel::getTopic() const { return topic; }

std::string Channel::getName() const { return name; }

bool Channel::getTopicRestricted() const { return topicRestricted; }

bool Channel::getInviteOnly() const { return invitedOnly; }

std::string Channel::getPassword() const { return password; }

// Setters

void Channel::setTopic(const std::string& newTopic) { this->topic = newTopic; }

void Channel::setTopicRestricted(bool value) { this->topicRestricted = value; }

void Channel::setInviteOnly(bool value) {this->invitedOnly = value; }

void Channel::setPassword(const std::string& pass) {this->password = pass;}

void Channel::setLimitMember(unsigned int limit) { this->limitMember = limit; }

void Channel::addOperator(Client* client)
{
	std::string nickName = client->getNickName();
	if (operators.find(nickName) == operators.end())
		operators[nickName] = client;
}

void Channel::removeOperator(Client* client)
{
	std::string nickName = client->getNickName();
	std::map<std::string, Client*>::iterator it = operators.find(nickName);
	if (it != operators.end())
		operators.erase(it);
}

// Public methods

/**
 * @brief check all Mode operator before add user in std::map members
*/
void	Channel::addUser(const std::string key, Client* client)
{
	Command command;
	std::string nickName = client->getNickName();
	if (!password.empty() && key != password )
	{
		command.sendErrorCode(client, ERR_BADCHANNELKEY, this->name);
		return;
	}
	if (limitMember > 0 && nbMember >= limitMember)
	{
		command.sendErrorCode(client, ERR_CHANNELISFULL, this->name);
		return;
	}
	if (invitedOnly && !isInvited(client))
	{
		command.sendErrorCode(client, ERR_INVITEONLYCHAN, this->name);
		return;
	}
	if (members.find(nickName) == members.end())
	{
		members[nickName] = client;
		nbMember++;
	}
	if (nbMember == 1)
		operators[nickName] = client;
	std::string message = ":" + client->getNickName() + "!" + client->getUser() + "@" + client->getHostname() + " JOIN " + this->name + "\r\n";
	sendAllChanExcept(message, NULL);
	std::cout << "new client: " << nickName << std::endl;
}

/**
 * @brief Send message to all channel members except one client (used for the sender, NULL for no exception none)
 */
void Channel::sendAllChanExcept(std::string message, Client* exclude)
{
	std::cout << message << std::endl;
	std::map<std::string, Client*>::iterator it;
	
	it = members.begin();
	while(it != members.end())
	{
		if (exclude == NULL || it->second->getFd() != exclude->getFd())
			send(it->second->getFd(), message.c_str(), message.size(), 0);
		it++;
	}
}

bool Channel::isMember(Client* client)
{
	std::string nickName = client->getNickName();
	std::map<std::string, Client*>::iterator it = members.find(nickName);
	return (it != members.end());
}

bool Channel::isOperator(Client* client)
{
	std::string nickName = client->getNickName();
	std::map<std::string, Client*>::iterator it = operators.find(nickName);
	return (it != operators.end());
}

void Channel::removeMember(Client* client)
{
	std::map<std::string, Client*>::iterator it;
	it = members.find(client->getNickName());
	if (it != members.end())
	{
		this->nbMember--;
		this->members.erase(it);
	}

	// Remove from operator map if member is one, leaving a zombie chan if it was the last.
	std::map<std::string, Client*>::iterator itOp;
	itOp = operators.find(client->getNickName());
	if (itOp != operators.end())
		operators.erase(itOp);
}

bool Channel::chanIsEmpty()
{
	if (nbMember == 0)
		return (true);
	return (false);
}

void Channel::addInvited(Client* client)
{
	this->invited[client->getNickName()] = client;
}

void Channel::removeInvited(Client* client)
{
	std::map<std::string, Client*>::iterator it = invited.find(client->getNickName());
	if (it != invited.end())
		invited.erase(it);
}

/**
 * @brief Returns if a client is in the invited list of the channel
 * @param client The pointer of the client
 */
bool Channel::isInvited(Client* client)
{
	std::map<std::string, Client*>::iterator it = this->invited.begin();
	while (it != this->invited.end())
	{
		if (it->second == client)
			return (true);
		it++;
	}
	return (false);
}