# Chat Server with Groups and Private Messages

## Objective
Develop a multi-threaded chat server that supports private messages, group communication, and user authentication. This project helps in understanding socket programming, multithreading, and data synchronization in networked systems.

## Working of the Code
The server (`server_grp.cpp`) is responsible for managing user authentication, handling client connections, and processing different types of messages. It loads user credentials from a `users.txt` file and verifies them during login. Users can send broadcast messages, private messages, and group messages. The server supports group creation, joining, leaving, and messaging within groups.

The client (`client_grp.cpp`) connects to the server, prompts the user for login credentials, and allows sending and receiving messages. It runs a separate thread to listen for incoming messages while enabling the user to send commands.

## Server Implementation Details

### 1. Server Initialization
- Creates a **TCP socket** for communication.
- **Binds** it to `PORT 12345` so clients can connect.
- Starts **listening** for client connections.

### 2. User Authentication
- Reads `users.txt` for username-password pairs.
- Asks the client for login details and validates credentials.
- Disconnects users on failed authentication.

### 3. Handling Client Messages
The server listens for commands from clients:
- **Broadcast messages:** `/broadcast <message>` (sent to all users)
- **Private messages:** `/msg <username> <message>` (sent to a specific user)
- **Group messages:**
  - `/create_grp <group_name>` (create a group)
  - `/join_grp <group_name>` (join an existing group)
  - `/leave_grp <group_name>` (leave a group)
  - `/group_msg <group_name> <message>` (send a message to a group)

### 4. Client Disconnect Handling
- Removes the user from the active client list when they disconnect.

## Instructions to Run
1. Navigate to the directory containing all the files.
2. Use the `make` command to compile both the server and client code:
   ```sh
   make
   ```
3. Start the server by running:
    ```sh
    ./server_grp
    ```
4. Start a client by running:
    ```sh
    ./client_grp
    ```


## Features
### Implemented Features:
- **User Authentication:** Users must provide valid credentials to log in.
- **Broadcast Messages:** Users can send messages to all connected clients.
- **Private Messages:** Users can send messages to specific individuals.
- **Group Management:** Users can create, join, and leave groups.
- **Group Messaging:** Users can send messages to a specific group.

### Not-Implemented Features:
- **File Sharing:** Sending files between users or within groups is not supported.
- **Encryption:** Messages are not encrypted for security.
- **User Profile Management:** No feature to update or manage user profiles beyond authentication.

## Design Decisions
- **Threading Model:** 
  - I chose to create a new thread for each client connection. This allows each client to have an independent communication channel and not block the server from handling multiple clients simultaneously.
  - Threads allow better scalability, as the server can handle many clients concurrently without blocking operations.
  
- **Synchronization:** 
  - To manage synchronization, I implemented a mutex to protect shared resources such as the list of active users and groups. This is necessary because multiple threads could try to modify these lists at the same time.
  - Used condition variables for waiting clients when handling message delivery and group notifications, ensuring that no data races occur.

- **No Forking:**
  - Instead of using multiple processes for each connection, I opted for threads because threads are lighter in terms of memory usage and context switching, making it ideal for this use case with a large number of clients.
  
- **Why no message size limit?:**
  - I did not put a specific size limit on messages in the initial design, but I added checks to handle extremely large messages that could potentially overload the server buffer.
  - This decision was made to simplify initial development and focus on other core functionality. Future updates can address this with size limits and chunked message transfers.

## Implementation

### High-level Idea of Key Functions
- **Authentication:** Reads `users.txt` file to verify credentials and ensure users are authenticated before connecting.
- **Client Handling:** A new thread is created for each connected client to handle both sending and receiving messages asynchronously.
- **Message Broadcasting:** Sends messages to all clients currently connected to the server.
- **Group Management:** Manages the creation, joining, leaving, and messaging of groups, storing group members and messages in shared data structures.

### Code Flow
1. **Server Startup:** Initializes the TCP socket, binds to port `12345`, and begins listening for incoming connections.
2. **Client Login:** Requests client credentials, validates them, and establishes a session for the client.
3. **Message Handling:** Based on the type of message (broadcast, private, or group), the server routes messages to the appropriate recipients.
4. **Client Disconnect:** When a client disconnects, their session is terminated, and they are removed from the active clients list.

## Testing

### Testing Strategy
- **Correctness Testing:** 
  - I tested the core functionality by logging in with different sets of credentials and verifying whether users could send/receive broadcast, private, and group messages.
  - I used multiple clients to simulate real-world use and check whether the server handled concurrent connections correctly.

### Challenges Faced
- **Bug in Thread Synchronization:** Initially, when multiple threads accessed shared resources like user and group lists, it led to data races and crashes. I solved this by introducing proper synchronization mechanisms (mutex and condition variables) to ensure thread safety.
- **Message Handling Bugs:** Early versions of the code failed to handle large messages correctly, leading to message truncation. I fixed this by ensuring the server handled message buffers properly, checking for buffer overflows, and introducing size limits for messages.

## Restrictions in Server
- **Max Group Members:** Each group can have up to 10 members. This can be adjusted based on server capacity.
- **Message Size Limit:** Messages are limited to 1024 characters by default. Larger messages are not supported without implementing chunked transfers.
