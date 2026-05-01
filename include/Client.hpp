#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "include.hpp"

class Client
{
	private:

		std::string nickName;
		std::string userName;
		std::string realName; 
		std::string hostName; // IP client
		std::string passWord;
		std::string buffer;
		
		bool isAuthenticated;

		bool	isNick;
		bool	isUser;
		bool	isRegistered; // Pass -> Nick -> User
		int 		clientFd;
		bool	willDisconnect;
		bool	isWelcomed;

	public:
	
		Client();
		Client(int fd);
		~Client();

		// getters
		std::string getNickName() const;
		std::string getUser() const;
		std::string getHostname() const;
		std::string& getBuffer();
		std::string getRealName() const;
		int getFd() const;
		bool getIsRegistered() const;
		bool getIsAuthenticated() const;
		bool getIsNick() const;
		bool getIsUser() const;
		bool getWillDisconnect() const;
		std::string getStringFd() const;
		bool getIsWelcomed() const;

		// setters
		void setRealName(const std::string& real);
		void setHostname(const std::string& hostname);
		void setNickName(const std::string& nick);
		void setUserName(const std::string& user);
		void setBuffer(const std::string& buffer);
		void setIsRegistered(const bool registered);
		void setIsAuthenticated(bool isAuthenticated);
		void setIsNick(bool nick_registered);
		void setIsUser(bool user_registered);
		void setWillDisconnect(bool state);
		void setIsWelcomed(bool welcome);

};

#endif