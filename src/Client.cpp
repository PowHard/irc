#include "../include/Client.hpp"
#include "../include/include.hpp"

Client::Client() : nickName("guest42"), userName(""), hostName("127.0.0.1"), isAuthenticated(false), isNick(false), isUser(false), isRegistered(false), willDisconnect(false), isWelcomed(false)
{
}

Client::Client(int fd) : nickName("guest42"), userName(""), hostName("127.0.0.1"), isAuthenticated(false), isNick(false), isUser(false), isRegistered(false), clientFd(fd), willDisconnect(false), isWelcomed(false)
{
}

Client::~Client()
{
}

// Getters

std::string Client::getNickName() const { return (this->nickName); }

std::string Client::getUser() const { return (this->userName); }

std::string Client::getHostname() const { return (this->hostName); }

std::string& Client::getBuffer() { return (this->buffer); }

std::string Client::getRealName() const { return (this->realName); }

int Client::getFd() const { return (this->clientFd); }

bool Client::getIsRegistered() const { return (this->isRegistered); }

bool Client::getIsAuthenticated() const { return (this->isAuthenticated); }

bool Client::getIsNick() const { return (this->isNick); }

bool Client::getIsUser() const { return (this->isUser); }

std::string Client::getStringFd() const
{
	std::ostringstream oss;
	oss << this->getFd();
	return(oss.str());
}

bool Client::getWillDisconnect() const { return (willDisconnect); }

bool Client::getIsWelcomed() const { return (this->isWelcomed); }

// Setters

void Client::setRealName(const std::string &real) { this->realName = real; }

void Client::setHostname(const std::string& hostname) { this->hostName = hostname; }

void Client::setNickName(const std::string &nick) { this->nickName = nick; }

void Client::setUserName(const std::string &user) { this->userName = user; }

void Client::setBuffer(const std::string& appendBuffer) { this->buffer += appendBuffer; }

void Client::setIsRegistered(const bool registered) { this->isRegistered = registered; }

void Client::setIsAuthenticated(bool authenticated) { this->isAuthenticated = authenticated; }

void Client::setIsNick(bool nick_registered) { this->isNick = nick_registered; }

void Client::setIsUser(bool user_registered) { this->isUser = user_registered; }

void Client::setWillDisconnect(bool state)
{
	if (state == true)
		willDisconnect = true;
	else if (state == false)
		willDisconnect = false;
}

void Client::setIsWelcomed(bool welcome) { this->isWelcomed = welcome; }