#include <iostream>
#include <cstring>      // For memset
#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <unistd.h>     // For close()
#include <cstdint>      // For uint8_t

// Define the processing states
enum ProcessingState { WAIT_FOR_MSG, IN_MSG };

// Function to handle communication with a client
void serve_connection(int sockfd) {
    // Send an initial acknowledgment to the client
    if (send(sockfd, "*", 1, 0) < 1) {
        perror("send error");
        close(sockfd);
        return;
    }

    // Start in the WAIT_FOR_MSG state
    ProcessingState state = WAIT_FOR_MSG;
    uint8_t buf[1024];  // Buffer for receiving data

    while (true) {
        // Receive data from the client
        int len = recv(sockfd, buf, sizeof(buf), 0);
        if (len < 0) {
            perror("recv error");
            close(sockfd);
            return;
        } else if (len == 0) {
            // Client disconnected
            break;
        }

        // Process the received data
        for (int i = 0; i < len; ++i) {
            switch (state) {
                case WAIT_FOR_MSG:
                    if (buf[i] == '^') {
                        state = IN_MSG;  // Start of a message
                    }
                    break;

                case IN_MSG:
                    if (buf[i] == '$') {
                        state = WAIT_FOR_MSG;  // End of the message
                    } else {
                        // Increment the byte and echo it back
                        buf[i] += 1;
                        if (send(sockfd, &buf[i], 1, 0) < 1) {
                            perror("send error");
                            close(sockfd);
                            return;
                        }
                    }
                    break;
            }
        }
    }

    // Close the connection with the client
    close(sockfd);
}

int main(int argc, char* argv[]) {
    int portnum = 9090;  // Default port number
    if (argc >= 2) {
        portnum = atoi(argv[1]);  // Set port number from argument
    }

    std::cout << "Serving on port " << portnum << std::endl;

    // Step 1: Create a socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // Step 2: Bind the socket to an IP and port
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));  // Zero out the structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Bind to any available network interface
    server_addr.sin_port = htons(portnum);     // Set the port

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(server_socket);
        return 1;
    }

    // Step 3: Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(server_socket);
        return 1;
    }

    std::cout << "Waiting for clients..." << std::endl;

    while (true) {
        // Step 4: Accept a client connection
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);

        if (client_socket < 0) {
            std::cerr << "Failed to accept client connection" << std::endl;
            close(server_socket);
            return 1;
        }

        std::cout << "Client connected" << std::endl;

        // Serve the connection using the state machine logic
        serve_connection(client_socket);

        std::cout << "Client disconnected" << std::endl;
    }

    // Close the server socket (this is never reached in this example)
    close(server_socket);
    return 0;
}
