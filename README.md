![irc](irc.png)

# ft_irc - Internet Relay Chat Server

## General Project Presentation

The ft_irc project consists of creating an instant messaging server based on the IRC (Internet Relay Chat) protocol. Developed in C++ 98, this server allows multiple users to connect simultaneously, authenticate, join discussion channels, and exchange messages in real time.

### Key Concepts

*   **IRC Protocol**: A text-based communication protocol on the Internet for instant messaging, whether public or private.
*   **Multiplexing**: Use of a single event loop to manage multiple network connections without using separate processes (fork).
*   **Client-Server Model**: The server manages resources (channels, users) and relays information between clients.
*   **Channels**: Thematic discussion spaces identified by a prefix (usually #) where users can communicate collectively.

## Technical Aspects

The implementation relies on several technical pillars compliant with C++ 98 standard requirements:

### Connection Management and Multiplexing

The server uses the `poll()` system call to monitor the state of all open file descriptors (sockets). This approach enables the detection of new connections, incoming data, and disconnections efficiently and in a non-blocking manner. All input/output (I/O) operations are configured in non-blocking mode via `fcntl()`.

### Software Architecture

The code is modularly structured around four main classes:

*   **Server**: Central entry point managing the main socket, the poll loop, and coordination between clients and channels.
*   **Client**: Represents a user connection, storing authentication state, nickname, username, and read/write buffers.
*   **Channel**: Manages the internal logic of a channel, including the list of members, operators, invitations, modes, and the topic.
*   **Command**: Parses incoming messages according to the IRC protocol and dispatches execution to the appropriate methods.

### Implemented Commands

The server supports essential protocol commands:

*   **Authentication**: PASS, NICK, USER.
*   **Channel Management**: JOIN, PART, TOPIC, INVITE, KICK.
*   **Messaging**: PRIVMSG.
*   **Channel Modes**: MODE (+i, +t, +k, +o, +l).
*   **Miscellaneous**: QUIT, WHO.

## Project Structure

| File | Utility |
| :--- | :--- |
| `include/Server.hpp` | Declaration of the Server class and the main loop. |
| `src/Server.cpp` | Implementation of network logic, initialization, and signal handling. |
| `include/Client.hpp` | Declaration of the Client class representing a user. |
| `src/Client.cpp` | Management of individual user state and buffers. |
| `include/Channel.hpp` | Declaration of the Channel class for channel management. |
| `src/Channel.cpp` | Logic for message distribution and privilege management. |
| `include/Command.hpp` | Declaration of the Command class for IRC parsing. |
| `src/Command.cpp` | Implementation of the command dictionary and their logic. |
| `src/main.cpp` | Program entry point, argument verification. |
| `include/include.hpp` | Grouping of standard libraries and dependencies. |
| `Makefile` | Automated compilation script (all, clean, fclean, re). |

## Installation and Usage

### Compilation

To compile the project, use the following command at the root:

```bash
make
```

This will generate the executable named `ircserv`.

### Execution

The server requires two arguments: the listening port and the connection password.

```bash
./ircserv <port> <password>
```

Example: `./ircserv 6667 my_password`

### Connection and Testing

#### Using HexChat (Reference Client)

1. Launch HexChat.
2. Add a new network pointing to `localhost` on the chosen port.
3. Configure the password in the connection settings.
4. Connect and start using commands (/join, /msg, etc.).

#### Manual Testing with Netcat (nc)

To test robustness against fragmented packets:

```bash
nc -C localhost 6667
PASS my_password
NICK alice
USER alice 0 * :alice_real
```