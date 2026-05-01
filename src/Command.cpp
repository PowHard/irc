#include "../include/Server.hpp"
#include "../include/include.hpp"

// Coplien form

Command::Command() : server(NULL)
{
	set_map();
}

Command::Command(const Command& c) : CommandMap(c.CommandMap) {}

Command::~Command() {}

Command& Command::operator=(const Command& c)
{
	if (this != &c)
	{
		this->CommandMap = c.CommandMap;
	}
	return (*this);
}

// Setters

void Command::setServer(Server *srv) { this->server = srv; }

// Methods to handle commands.

/**
 * @brief Sets the CommandMap attribute
*/
void Command::set_map(void)
{
	CommandMap["PASS"] = &Command::pass_serv;
	CommandMap["NICK"] = &Command::nick;
	CommandMap["USER"] = &Command::user;
	CommandMap["JOIN"] = &Command::join;
	CommandMap["MODE"] = &Command::mode;
	CommandMap["WHO"] = &Command::mode;
	CommandMap["PRIVMSG"] = &Command::privmsg;
	CommandMap["PART"] = &Command::part;
	CommandMap["QUIT"] = &Command::quit;
	CommandMap["KICK"] = &Command::kick;
	CommandMap["TOPIC"] = &Command::topic;
	CommandMap["INVITE"] = &Command::invite;
}

/**
 * @brief Extract the command and params.
 * Performs the appropriate rights check depending on the command called.
 * Call the command from the command map.
 * 
*/
void Command::prepareCommand(Client* client, std::string line)
{
	std::istringstream ss(line);
	std::string command;
	ss >> command;

	std::cout << "line received: '" << line << "'" << std::endl;
	std::transform(command.begin(), command.end(), command.begin(), ::toupper); // upper conversion command

	std::string param;
	std::getline(ss, param);

	param.erase(0, param.find_first_not_of(" ")); //erase space befor params

	// Authentication checking
	if (command != "PASS" && command != "CAP" && command != "QUIT")
	{
		if (!client->getIsAuthenticated())
		{
			sendErrorCode(client, ERR_NOTREGISTERED, command);
			return;
		}
	}

	// Registration checking
	if (command == "JOIN" || command == "PRIVMSG" || command == "MODE" 
		|| command == "WHO" || command == "TOPIC" || command == "INVITE"
		|| command == "KICK" || command == "PART")
	{
		if (!client->getIsRegistered())
		{
			sendErrorCode(client, ERR_NOTREGISTERED, command);
			return;
		}
	}

	if (CommandMap.find(command) != CommandMap.end())
	{
		FtCommand funcMap = CommandMap[command];
		(this->*funcMap)(client, param); 
	}
	else
	{
		sendErrorCode(client, ERR_UNKNOWNCOMMAND, command);
	}
}

/**
 * @brief Extract all command to pass it in the handler command
 * @param buffer Input from the client
 * @param client Client's fd 
*/
void Command::extractCompleteCommand(Client* client)
{
	std::string& buffer = client->getBuffer();
	//std::cout << "buffer: " << buffer << std::endl; //For defense purpose
	size_t pos;
	while ((pos = buffer.find("\r\n")) != std::string::npos)
	{
		std::string line = buffer.substr(0, pos);
		buffer.erase(0, pos + 2); // Supp \r\n for the next  command
		prepareCommand(client, line);
	}
}

/**
 * @brief Handles error codes and send the appropriate message
 * @param Client The pointer of the client
 * @param errorCode <ERR_ERROREXAMPLE> the type of error
 * @param errorMsg The string appended at the end of the error message
 */
void Command::sendErrorCode(Client* client, ErrorCode errorCode, std::string errorMsg)
{
	std::string nickname = client->getNickName();
	if (nickname.empty())
		nickname = "*";
	std::stringstream 	error;
	std::stringstream	ss(errorMsg);
	std::string			token;
	switch (errorCode)
	{
		case ERR_NOSUCHNICK: // "<client> <nick> :No such nick/channel"
			error << client->getNickName() << " " << errorMsg << " :No such nick/channel";
			break;
		case ERR_NOSUCHCHANNEL: // "<client> <channel> :No such channel"
			error << client->getNickName() << " " << errorMsg << " :No such channel";
			break;
		case ERR_CANNOTSENDTOCHAN: // "<client> <channel> :Cannot send to channel"
			error << client->getNickName() << " " << errorMsg << " :Cannot send to channel";
			break;
		case ERR_UNKNOWNCOMMAND: // "<client> <command> :Unknown command"
			error << client->getNickName() << " " << errorMsg << " :Unknown command.";
			break;
		case ERR_NONICKNAMEGIVEN: // "<client> :No nickname given"
			error << client->getNickName() << " :No nickname given.";
			break;
		case ERR_ERRONEUSNICKNAME: // "<client> <nick> :Erroneus nickname"
			error << client->getNickName() << " " << errorMsg << " :Erroneus nickname. The first character of your nickname cannot be \"#\" neither \":\". You can't have space character in your nickname.";
			break;
		case ERR_NICKNAMEINUSE: // "<client> <nick> :Nickname is already in use"
			error << client->getNickName() << " " << errorMsg << " :Nickname is already in use. Please try again with another one.";
			break;
		case ERR_USERNOTINCHANNEL: // "<client> <nick> <channel> :They aren't on that channel"
			ss >> token;
			error << client->getNickName() << " " << token << " ";
			ss >> token;
			error << token << " :They aren't on that channel.";
			break;
		case ERR_NOTONCHANNEL: // "<client> <channel> :You're not on that channel"
			error << client->getNickName() << " " << errorMsg << " :You're not on that channel.";
			break;
		case ERR_USERONCHANNEL: // "<client> <nick> <channel> :is already on channel"
			ss >> token;
			error << client->getNickName() << " " << token << " ";
			ss >> token;
			error << token << " :is already on channel.";
			break;
		case ERR_NOTREGISTERED: // "<client> :You have not registered"
			error << client->getNickName() << " :You have not registered";
			break;
		case ERR_NEEDMOREPARAMS: // "<client> <command> :Not enough parameters"
			error << client->getNickName() << " " << errorMsg << " :Not enough parameters.";
			break;
		case ERR_ALREADYREGISTERED: // "<client> :You may not reregister"
			error << client->getNickName() << " :You may not register.";
			break;
		case ERR_PASSWDMISMATCH: // "<client> :Password incorrect"
			error << client->getNickName() << " :Password incorrect";
			break;
		case ERR_CHANNELISFULL: //"<client> <channel> :Cannot join channel (+l)"
			error << client->getNickName() << " " << errorMsg << " :Cannot join channel (+l)";
			break;
		case ERR_UNKNOWNMODE: // "<client> <char> :is unknown mode char to me"
			error << client->getNickName() << " " << errorMsg << " :is unknown mode char to me";
			break;
		case ERR_INVITEONLYCHAN: // "<client> <channel> :Cannot join channel (+i)"
			error << client->getNickName() << " " << errorMsg << " :Cannot join channel (+i)";
			break;
		case ERR_BADCHANNELKEY: // "<client> <channel> :Cannot join channel (+k)"
			error << client->getNickName() << " " << errorMsg << " :Cannot join channel (+k), bad channel key";
			break;
		case ERR_BADCHANMASK: // "<client> <channel> :Bad Channel Mask"
			error << client->getNickName() << " " << errorMsg << " :Bad Channel Mask. Usage is /JOIN {#,!,&,+}{<channelname1>,<channelname2,...} {channelkey1,channelkey2,...}.";
			break;
		case ERR_CHANOPRIVSNEEDED: // "<client> <channel> :You're not channel operator"
			error << client->getNickName() << " " << errorMsg << " :You're not channel operator.";
			break;
		default:
			break;
	}
	error << "\r\n";
	std::string stringError = error.str();
	send(client->getFd(), stringError.c_str(), stringError.length(), 0);
}

/**
 * @brief Handles the PASS command for client authentication.
 * 
 * Verifies the password provided by the client against the server password.
 * Sends appropriate error codes in cases of errors
 * 
 * @param client Pointer to the Client attempting to authenticate.
 * @param buffer String containing the password provided by the client.
 */
void	Command::pass_serv(Client* client, std::string buffer)
{
	if (client->getIsAuthenticated())
	{
		sendErrorCode(client, ERR_ALREADYREGISTERED, "PASS");
		return;
	}

	if (buffer.empty())
	{
		sendErrorCode(client, ERR_NEEDMOREPARAMS, "PASS");
		return;
	}

	if (buffer == server->getPassword())
	{
		client->setIsAuthenticated(true);
		std::cout << "[AUTH] Client " << client->getFd() << " - Authentication SUCCESS" << std::endl;
		std::string response = ":localhost NOTICE * :Password accepted\r\n"; // TODO Replace \n by std::endl and '*' by good name ?
		send(client->getFd(), response.c_str(), response.length(), 0);
	}
	else
	{
		sendErrorCode(client, ERR_PASSWDMISMATCH, "PASS");
		std::cout << "[AUTH] Client " << client->getFd() << " - Wrong password" << std::endl;
	}
}

/**
 * @brief Handles the /NICK command
 * @param client the pointer of the client
 * @param buffer the parameters of the command
 */
void Command::nick(Client* client, std::string buffer)
{
	if (buffer.empty())
	{
		sendErrorCode(client, ERR_NONICKNAMEGIVEN, "");
		return ;
	}
	else if (this->server->isNickRegistered(buffer))
	{
		sendErrorCode(client, ERR_NICKNAMEINUSE, buffer);
		return ;
	}
	else if (buffer[0] == '#' || buffer[0] == ':' || buffer.find(" ") != std::string::npos)
	{
		sendErrorCode(client, ERR_ERRONEUSNICKNAME, "");
		return ;
	}
	std::string message = ":" + client->getNickName() + "!" + client->getUser() + "@localhost NICK :" + buffer + "\r\n";
	this->server->updateChanMember(buffer, client);
	client->setNickName(buffer);
	client->setIsNick(true);
	send(client->getFd(), message.c_str(), message.length(), 0);
	this->server->sendLinkedClients(message, client);
	if (client->getIsUser())
	{
		client->setIsRegistered(true);
		if(!(client->getIsWelcomed()))
		{
			client->setIsWelcomed(true);
			sendWelcome(client);
		}
	}
}

/**
 * @brief Handles the /USER command
 * @param client the pointer of the client
 * @param buffer the parameters of the command
 */
void Command::user(Client* client, std::string buffer)
{
	std::vector<std::string>	params;
	std::stringstream			ss(buffer);
	std::string					token;

	if (client->getIsUser())
	{
		sendErrorCode(client, ERR_ALREADYREGISTERED, "");
		return ;
	}

	while (ss >> token)
	{
		if (token[0] == ':')
		{
			std::string real = token.substr(1);
			std::string tmp;
			while (ss >> tmp)
				real += " " + tmp;
			params.push_back(real);
			break;
		}
		params.push_back(token);
	}
	if (params.size() < 4)
	{
		sendErrorCode(client, ERR_NEEDMOREPARAMS, "USER"); // "Syntax : /USER <username> 0 * :<realname>"
		return;
	}
	if (params.size() > 4)
	{
		sendErrorCode(client, ERR_NEEDMOREPARAMS, "USER"); // "You need \":\" before your realname if it contains spaces."
		return;
	}
	client->setIsRegistered(true);
	std::string message = client->getNickName() + " set his username to " + params[0] + " (" + params[3] + ").\r\n";
	client->setUserName(params[0]);
	client->setRealName(params[3]);
	client->setIsUser(true);
	send(client->getFd(), message.c_str(), message.length(), 0);
	if (client->getIsNick())
	{
		client->setIsRegistered(true);
		if(!(client->getIsWelcomed()))
		{
			client->setIsWelcomed(true);
			sendWelcome(client);
		}
	}
}

/**
 * @brief Create new channel if he exist
 * @param channelV channel name
 * @param keyV password
*/
void	Command::join(Client* client, std::string buffer)
{	
	std::vector<std::string> channelV;
	std::vector<std::string> keyV;
	std::string channelStr;
	std::string keyStr;

	std::stringstream ss(buffer);
	ss >> channelStr;
	if (!ss.eof())
		ss >> keyStr;
	std::stringstream channelSS(channelStr);
	std::string out;
	while (std::getline(channelSS, out, ','))
	{
		if (out[0] == '#' || out[0] == '&' || out[0] == '!' || out[0] == '+')
			channelV.push_back(out);
		else
			sendErrorCode(client, ERR_BADCHANMASK, out);
	}
	if (!keyStr.empty())
	{
		std::stringstream keySS(keyStr);
		std::string keyS;
		while (std::getline(keySS, keyS, ','))
			keyV.push_back(keyS);
	}

	for (size_t i = 0; i < channelV.size(); i++)
	{
		std::string channelName = channelV[i];
		std::string pass;
		if (i < keyV.size())
			pass = keyV[i];
		else
			pass = "";
		Channel* channel = server->findChannel(channelName);
		if (channel == NULL) // Canal don't exist, we create it
		{
			channel = new Channel(channelName, client);
			server->addChannel(channelName, channel);
		}
		channel->addUser(pass, client);

		// 332 RPL_TOPIC
		std::string topic = channel->getTopic();
		if (!topic.empty())
		{
			std::string topicMsg = ":ft_irc 332 " + client->getNickName()
								 + " " + channelName
								 + " :" + topic + "\r\n";
			send(client->getFd(), topicMsg.c_str(), topicMsg.length(), 0);
		}
		else
		{
			// 331 RPL_NOTOPIC
			std::string noTopicMsg = ":ft_irc 331 " + client->getNickName()
								   + " " + channelName
								   + " :No topic is set\r\n";
			send(client->getFd(), noTopicMsg.c_str(), noTopicMsg.length(), 0);
		}

		// 353 RPL_NAMREPLY
		std::string namreply = ":ft_irc 353 " + client->getNickName()
							 + " = " + channelName + " :";

		std::map<std::string, Client*>& members = channel->getMembers();
		for (std::map<std::string, Client*>::iterator it = members.begin();
			 it != members.end(); ++it)
		{
			if (channel->isOperator(it->second))
				namreply += "@";
			namreply += it->first + " ";
		}
		namreply += "\r\n";
		send(client->getFd(), namreply.c_str(), namreply.length(), 0);

		// 366 RPL_ENDOFNAMES
		std::string endofnames = ":ft_irc 366 " + client->getNickName()
							   + " " + channelName
							   + " :End of /NAMES list\r\n";
		send(client->getFd(), endofnames.c_str(), endofnames.length(), 0);
	}
}

void Command::who(Client* client, std::string buffer)
{
	if (buffer.empty())
	{
		sendErrorCode(client, ERR_NEEDMOREPARAMS, "MODE");
		return;
	}
	//RPL_WHOREPLY 352
	std::ostringstream oss;
	oss << client->getFd() << " " << buffer 
						<< " " << client->getUser() 
						<< " " << client->getHostname() 
						<< client->getNickName() << " :" 
						<< "0 " << client->getRealName() << "\r\n"; 

	std::string msg = oss.str();
	send(client->getFd(), msg.c_str() , msg.size(), 0);
}

/**
 * @brief Handles the PRIVMSG command for sending messages to users or channels
 * usage : PRIVMSG <target> :<message>
 * - target can be a channel (#chan) or a user nickname
 */
void Command::privmsg(Client* client, std::string buffer)
{
	std::stringstream ss(buffer);
	std::string targetsStr;
	std::string message;

	ss >> targetsStr;
	std::getline(ss, message);

	if (targetsStr.empty() || message.empty())
	{
		sendErrorCode(client, ERR_NEEDMOREPARAMS, "PRIVMSG");
		return;
	}

	message.erase(0, message.find_first_not_of(" "));
	if (!message.empty() && message[0] == ':')
		message.erase(0, 1);
	if (message.empty())
	{
		sendErrorCode(client, ERR_NEEDMOREPARAMS, "PRIVMSG");
		return;
	}

	// Multiple PRIVMSG target handling (<target1>,<target2>)
	std::vector<std::string> targets;
	std::stringstream targetsSS(targetsStr);
	std::string singleTarget;
	while (std::getline(targetsSS, singleTarget, ','))
	{
		if (!singleTarget.empty())
			targets.push_back(singleTarget);
	}

	for (size_t i = 0; i < targets.size(); i++)
	{
		std::string target = targets[i];
	
		std::string ircMsg = ":" + client->getNickName() + "!"
						   + client->getUser() + "@"
						   + client->getHostname()
						   + " PRIVMSG " + target
						   + " :" + message + "\r\n";

		if (target[0] == '#' || target[0] == '&' || target[0] == '!' || target[0] == '+') // PRIVMSG to current #channel
		{
			Channel* channel = server->findChannel(target);
			if (channel == NULL)
			{
				sendErrorCode(client, ERR_NOSUCHCHANNEL, target);
				continue;
			}
			if (!channel->isMember(client))
			{
				sendErrorCode(client, ERR_CANNOTSENDTOCHAN, target);
				continue;
			}
			channel->sendAllChanExcept(ircMsg, client);
		}
		else // PRIVMSG to current user
		{
			Client* targetClient = server->findClientByNick(target);
			if (targetClient == NULL)
			{
				sendErrorCode(client, ERR_NOSUCHNICK, target);
				continue;
			}
			send(targetClient->getFd(), ircMsg.c_str(), ircMsg.length(), 0);
		}
	}
}

// "<client> :Welcome to the <networkname> Network, <nick>[!<user>@<host>]"
// "<client> :Your host is <servername>, running version <version>"
// "<client> :This server was created <datetime>"
// "<client> <servername> <version> <available user modes> <available channel modes> [<channel modes with a parameter>]"
void Command::sendWelcome(Client* client)
{
	std::string welcomeMsg;
	welcomeMsg = ":ft_irc 001 " + client->getNickName() + " :Welcome to the irc42 Network, " + client->getNickName() + "[!" + client->getUser() + "@localhost]\r\n";
	welcomeMsg += ":ft_irc 002 " + client->getNickName() + " :Your host is ft_irc, running version 42.42.\r\n";
	welcomeMsg += ":ft_irc 003 " + client->getNickName() + " :This server was created the 11/13/25.\r\n";
	welcomeMsg += ":ft_irc 004 " + client->getNickName() + " ft_irc 42.42. The user mode available is o (operator). The available channel modes are : i (invite only), t (restricted topic), o (operator privilege), l (user limit). The available channel mode with a parameter is k (channel key).\r\n";
	send(client->getFd(), welcomeMsg.c_str() , welcomeMsg.size(), 0);
	sendMOTD(client);
}

// 375 "<client> :- <server> Message of the day - "
// 372  "<client> :<line of the motd>"
// 376  "<client> :End of /MOTD command."
void Command::sendMOTD(Client* client)
{
	std::string msgMOTD;
	msgMOTD = client->getNickName() + " :- ft_irc Message of the day -\r\n";
	msgMOTD += client->getNickName() + " :Merry Xmas !\r\n";
	msgMOTD += client->getNickName() + " :End of MOTD command\r\n";
	send(client->getFd(), msgMOTD.c_str() , msgMOTD.size(), 0);
}

void Command::part(Client* client, std::string buffer)
{
	if (buffer.empty())
	{
		sendErrorCode(client, ERR_NEEDMOREPARAMS, "PART");
		return;
	}

	std::vector<std::string> channelV;
	std::string channelStr;
	std::string reason;

	std::stringstream ss(buffer);
	ss >> channelStr;

	if (!ss.eof())
	{
		std::getline(ss, reason);
		if (!reason.empty() && reason[0] == ' ')
			reason.erase(0, 1);
		if (!reason.empty() && reason[0] == ':')
			reason.erase(0, 1);
	}

	std::stringstream channelSS(channelStr);
	std::string chanName;
	while (std::getline(channelSS, chanName, ','))
	{
		if (!chanName.empty() && (chanName[0] == '#' || chanName[0] == '&' ||chanName[0] == '!' || chanName[0] == '+'))
			channelV.push_back(chanName);
	}

	for (size_t i = 0; i < channelV.size(); i++)
	{
		std::string channelName = channelV[i];
		Channel* channel = server->findChannel(channelName);

		if (!channel)
		{
			sendErrorCode(client, ERR_NOSUCHCHANNEL, channelName);
			continue;
		}

		if (!channel->isMember(client))
		{
			sendErrorCode(client, ERR_NOTONCHANNEL, channelName);
			continue;
		}

		std::string partMsg = ":" + client->getNickName() + "!"
							+ client->getUser() + "@"
							+ client->getHostname()
							+ " PART " + channelName;

		if (!reason.empty())
			partMsg += " :" + reason;

		partMsg += "\r\n";

		channel->sendAllChanExcept(partMsg, NULL);
		channel->removeMember(client);
		if (channel->chanIsEmpty())
			server->removeChan(channelName);
	}
}

// Parameters: <channel> <user> [<comment>]
/**
 * @brief Handles the /KICK command
 * @param client the pointer of the client
 * @param buffer the parameters of the command
 */
void Command::kick(Client* client, std::string buffer)
{
	std::vector<std::string>	params;
	std::stringstream			ss(buffer);
	std::string					token;
	std::string					kick_msg;
	while (ss >> token)
	{
		if (token[0] == ':')
		{
			std::string real = token.substr(1);
			std::string tmp;
			while (ss >> tmp)
				real += " " + tmp;
			params.push_back(real);
			break;
		}
		params.push_back(token);
	}
	if (params.size() < 2 || params.size() > 3)
	{
		sendErrorCode(client, ERR_NEEDMOREPARAMS, "KICK");
		return;
	}
	Channel* channel = this->server->findChannel(params[0]);
	if (!channel)
	{
		sendErrorCode(client, ERR_NOSUCHCHANNEL, params[0]);
		return;
	}
	if (!(channel->isMember(client)))
	{
		sendErrorCode(client, ERR_NOTONCHANNEL, params[0]);
		return;
	}
	if (!(channel->isOperator(client)))
	{
		sendErrorCode(client, ERR_CHANOPRIVSNEEDED, params[0]);
		return;
	}
	Client* user = this->server->findClientByNick(params[1]);
	if (!user)
	{
		std::stringstream error;
		error << params[1] << " " << params[0];
		sendErrorCode(client, ERR_USERNOTINCHANNEL, error.str());
		return;
	}
	if (!(channel->isMember(user)))
	{
		std::stringstream error;
		error << params[1] << " " << params[0];
		sendErrorCode(client, ERR_USERNOTINCHANNEL, error.str());
		return;
	}
	kick_msg = ":" + client->getNickName() + " KICK " + params[0] + " " + params[1] + " :";
	if (params.size() == 2)
		kick_msg += "Goodbye " + params[1] + "!";
	else
		kick_msg += params[2];
	kick_msg += "\r\n";
	channel->sendAllChanExcept(kick_msg, NULL);
	channel->removeMember(user);
	if (channel->chanIsEmpty())
		server->removeChan(params[0]);
	return;
}

/**
 * @brief Display current channel modes (RPL_CHANNELMODEIS 324)
 */
void Command::displayCurrentModes(Client* client, Channel* channel, const std::string& channelName)
{
	std::string currentModes = "+";

	if (channel->getInviteOnly())
		currentModes += "i";
	if (channel->getTopicRestricted())
		currentModes += "t";
	if (!channel->getPassword().empty())
		currentModes += "k";

	std::string msg = ":ft_irc 324 " + client->getNickName()
					+ " " + channelName + " " + currentModes + "\r\n";
	send(client->getFd(), msg.c_str(), msg.size(), 0);
}

/**
 * @brief Validate that client has permissions to change modes
 */
bool Command::validateModePermissions(Client* client, Channel* channel, const std::string& channelName)
{
	if (!channel->isMember(client))
	{
		sendErrorCode(client, ERR_NOTONCHANNEL, channelName);
		return false;
	}

	if (!channel->isOperator(client))
	{
		sendErrorCode(client, ERR_CHANOPRIVSNEEDED, channelName);
		return false;
	}

	return true;
}

/**
 * @brief Broadcast mode changes to all channel members
 */
void Command::broadcastModeChanges(Client* client, Channel* channel, const std::string& channelName,
								   const std::string& appliedModes, const std::string& modeArgs)
{
	if (appliedModes.empty())
		return;

	std::string modeMsg = ":" + client->getNickName() + "!"
						+ client->getUser() + "@"
						+ client->getHostname()
						+ " MODE " + channelName
						+ " " + appliedModes
						+ modeArgs + "\r\n";

	channel->sendAllChanExcept(modeMsg, NULL);
}

/**
 * @brief the MODE command for channel mode changes
 * 
 *		  syntax: MODE <#channel> <+/-><mode letter> [params]
 *
 * 		  Available modes: i (invite-only), t (topic restricted), k (set/remove password),
 *        o (op/deop members), l (set/remove users limit to channels).
 * 
 */
void Command::mode(Client* client, std::string buffer)
{
	std::stringstream ss(buffer);
	std::string channelName;
	std::string modeString;
	std::vector<std::string> modeParams;

	ss >> channelName;

	if (channelName.empty())
	{
		sendErrorCode(client, ERR_NEEDMOREPARAMS, "MODE");
		return;
	}
	if (channelName[0] != '#' && channelName[0] != '&' && channelName[0] != '!' && channelName[0] != '+')
	{
		sendErrorCode(client, ERR_NOSUCHCHANNEL, channelName);
		return;
	}

	Channel* channel = server->findChannel(channelName);

	if (!channel)
	{
		sendErrorCode(client, ERR_NOSUCHCHANNEL, channelName);
		return;
	}
	if (ss.eof())
	{
		displayCurrentModes(client, channel, channelName);
		return;
	}

	ss >> modeString;
	
	if (modeString.empty() || (modeString[0] != '+' && modeString[0] != '-'))
	{
		sendErrorCode(client, ERR_UNKNOWNMODE, modeString);
		return;
	}

	std::string param;

	while (ss >> param)
		modeParams.push_back(param);
	if (!validateModePermissions(client, channel, channelName))
	{
		sendErrorCode(client, ERR_CHANOPRIVSNEEDED, "");
		return;
	}
	bool adding = true;
	size_t paramIndex = 0;
	std::string appliedModes = "";
	std::string modeArgs = "";

	for (size_t i = 0; i < modeString.length(); i++)
	{
		char mode = modeString[i];

		if (mode == '+')
		{
			adding = true;
			continue;
		}
		else if (mode == '-')
		{
			adding = false;
			continue;
		}

		switch (mode)
		{
			case 'i': // Invite-only
				channel->setInviteOnly(adding);
				if (adding)
					appliedModes += "+i";
				else
					appliedModes += "-i";
				break;

			case 't': // Topic restricted
				channel->setTopicRestricted(adding);
				if (adding)
					appliedModes += "+t";
				else
					appliedModes += "-t";
				break;

			case 'k': // Channel key (password)
			{
				if (adding)
				{
					if (paramIndex >= modeParams.size())
					{
						sendErrorCode(client, ERR_NEEDMOREPARAMS, "MODE +k");
						return;
					}

					channel->setPassword(modeParams[paramIndex]);
					appliedModes += "+k";
					modeArgs += " ";
					modeArgs += modeParams[paramIndex];
					paramIndex++;
				}
				else
				{
					channel->setPassword("");
					appliedModes += "-k";
				}
				break;
			}

			case 'o': // Channel operator privilege
			{
				if (paramIndex >= modeParams.size())
				{
					sendErrorCode(client, ERR_NEEDMOREPARAMS, "MODE +/-o");
					return;
				}

				std::string targetNick = modeParams[paramIndex];
				Client* targetClient = server->findClientByNick(targetNick);

				if (!targetClient)
				{
					sendErrorCode(client, ERR_NOSUCHNICK, targetNick);
					paramIndex++;
					continue;
				}

				if (!channel->isMember(targetClient))
				{
					std::stringstream error;
					error << targetNick << " " << channelName;
					sendErrorCode(client, ERR_USERNOTINCHANNEL, error.str());
					paramIndex++;
					continue;
				}

				if (adding)
					channel->addOperator(targetClient);
				else
					channel->removeOperator(targetClient);

				if (adding)
					appliedModes += "+o";
				else
					appliedModes += "-o";
				modeArgs += " ";
				modeArgs += targetNick;
				paramIndex++;
				break;
			}

			case 'l': // User limit to channel
			{
				if (adding)
				{
					if (paramIndex >= modeParams.size())
					{
						sendErrorCode(client, ERR_NEEDMOREPARAMS, "MODE +l");
						return;
					}

					std::stringstream convert(modeParams[paramIndex]);
					unsigned int limit;
					convert >> limit;

					if (convert.fail())
					{
						paramIndex++;
						continue;
					}

					channel->setLimitMember(limit);
					appliedModes += "+l";
					modeArgs += " ";
					modeArgs += modeParams[paramIndex];
					paramIndex++;
				}
				else
				{
					channel->setLimitMember(0);
					appliedModes += "-l";
				}
				break;
			}

			default:
				std::string unknownMode(1, mode);
				sendErrorCode(client, ERR_UNKNOWNMODE, unknownMode);
				continue;
		}
	}

	broadcastModeChanges(client, channel, channelName, appliedModes, modeArgs);
}

/**
 * @brief Quit all chan and disconnect Client
*/
void	Command::quit(Client* client, std::string buffer)
{
	if (!server->isNickRegistered(client->getNickName()))
		sendErrorCode(client, ERR_NOTREGISTERED, "");
	
	std::string reason = buffer;
	if (!reason.empty() && reason[0] == ' ')
		reason.erase(0, 1);
	if (!reason.empty() && reason[0] == ':')
		reason.erase(0, 1);
	
	server->quitAllChan(client, reason);
	client->setWillDisconnect(true);
	client->getBuffer().clear();
}

/**
 * @brief Handles the /INVITE command
 * @param client the pointer of the client
 * @param buffer the parameters of the command
 * syntax : INVITE <nickname> <#channel>
 */
void	Command::invite(Client* client, std::string buffer)
{
	std::vector<std::string>	params;
	std::stringstream			ss(buffer);
	std::string					token;
	std::string					invite_msg;

	while (ss >> token)
		params.push_back(token);
	if (params.size() != 2)
	{
		sendErrorCode(client, ERR_NEEDMOREPARAMS, "INVITE");
		return; 
	}
	Channel* channel = this->server->findChannel(params[1]);
	if (!channel)
	{
		sendErrorCode(client, ERR_NOSUCHCHANNEL, params[1]);
		return;
	}
	if (!(channel->isMember(client)))
	{
		sendErrorCode(client, ERR_NOTONCHANNEL, params[1]);
		return;
	}
	if (!(channel->isOperator(client)))
	{
		sendErrorCode(client, ERR_CHANOPRIVSNEEDED, params[1]);
		return;
	}
	Client* user = this->server->findClientByNick(params[0]);
	if (!user)
	{
		sendErrorCode(client, ERR_NOSUCHNICK, params[0]);
		return;
	}
	if (channel->isMember(user))
	{
		std::stringstream error;
		error << params[0] << " " << params[1];
		sendErrorCode(client, ERR_USERONCHANNEL, error.str());
		return;
	}
	// RPL_INVITING (341)
	channel->addInvited(user);
	invite_msg = ":" + client->getNickName() + " INVITE " + params[0] + " " + params[1] + "\r\n";
	send(user->getFd(), invite_msg.c_str(), invite_msg.length(), 0);
	invite_msg = ":ft_irc 341 " + client->getNickName() + " " + params[0] + " " + params[1] + "\r\n";
	send(client->getFd(), invite_msg.c_str(), invite_msg.length(), 0);
	return;
}

/**
 * @brief Handles the TOPIC command
 * Syntax: TOPIC #channel [new topic]
 */
void Command::topic(Client* client, std::string buffer)
{
	std::stringstream ss(buffer);
	std::string channelName;
	std::string newTopic;
	ss >> channelName;

	if (channelName.empty())
	{
		sendErrorCode(client, ERR_NEEDMOREPARAMS, "TOPIC");
		return;
	}

	Channel* channel = server->findChannel(channelName);
	if (!channel)
	{
		sendErrorCode(client, ERR_NOSUCHCHANNEL, channelName);
		return;
	}

	if (!channel->isMember(client))
	{
		sendErrorCode(client, ERR_NOTONCHANNEL, channelName);
		return;
	}

	if (!ss.eof())
	{
		std::getline(ss, newTopic);

		if (!newTopic.empty() && newTopic[0] == ' ')
			newTopic.erase(0, 1);
		if (!newTopic.empty() && newTopic[0] == ':')
			newTopic.erase(0, 1);
	}

	if (newTopic.empty())
	{
		std::string currentTopic = channel->getTopic();

		if (currentTopic.empty())
		{
			// RPL_NOTOPIC 331
			std::string msg = ":ft_irc 331 " + client->getNickName()
							+ " " + channelName + " :No topic is set\r\n";
			send(client->getFd(), msg.c_str(), msg.size(), 0);
		}
		else
		{
			// RPL_TOPIC 332
			std::string msg = ":ft_irc 332 " + client->getNickName()
							+ " " + channelName + " :" + currentTopic + "\r\n";
			send(client->getFd(), msg.c_str(), msg.size(), 0);
		}
		return;
	}

	if (channel->getTopicRestricted())
	{
		if (!channel->isOperator(client))
		{
			sendErrorCode(client, ERR_CHANOPRIVSNEEDED, channelName);
			return;
		}
	}
	channel->setTopic(newTopic);

	std::string topicMsg = ":" + client->getNickName() + "!"
						 + client->getUser() + "@"
						 + client->getHostname()
						 + " TOPIC " + channelName
						 + " :" + newTopic + "\r\n";

	channel->sendAllChanExcept(topicMsg, NULL);
}
