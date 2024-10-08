#include <iostream>
#include <pthread.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstdlib>

// Define the processing states
enum ProcessingState { WAIT_FOR_MSG, IN_MSG };

// Structure to hold the socket file descriptor for each thread
struct thread_config_t {
    int sockfd;
};

// Function to handle communication with a client
void serve_connection(int sockfd) {
    // Send initial acknowledgment
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
                        // Increment the byte and send it back
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

// Function executed by each thread to handle the connection
void* server_thread(void* arg) {
    thread_config_t* config = (thread_config_t*)arg;
    int sockfd = config->sockfd;
    free(config);

    // Thread ID (for logging purposes)
    pthread_t tid = pthread_self();
    std::cout << "Thread " << tid << " created to handle connection on socket " << sockfd << std::endl;

    // Serve the connection
    serve_connection(sockfd);

    std::cout << "Thread " << tid << " done" << std::endl;
    return nullptr;
}

int main(int argc, char* argv[]) {
    // Set the port number (default is 9090)
    int portnum = 9090;
    if (argc >= 2) {
        portnum = std::atoi(argv[1]);
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

    // Step 4: Accept and handle clients in separate threads
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // Accept a client connection
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            std::cerr << "Failed to accept client connection" << std::endl;
            continue;
        }

        // Log the connection
        std::cout << "Client connected with socket " << client_socket << std::endl;

        // Allocate memory for thread configuration and set up the client socket
        thread_config_t* config = (thread_config_t*)malloc(sizeof(thread_config_t));
        if (!config) {
            std::cerr << "Out of memory" << std::endl;
            close(client_socket);
            continue;
        }
        config->sockfd = client_socket;

        // Create a new thread to handle the client
        pthread_t thread_id;
        if (pthread_create(&thread_id, nullptr, server_thread, config) != 0) {
            std::cerr << "Failed to create thread" << std::endl;
            free(config);
            close(client_socket);
            continue;
        }

        // Detach the thread so that it cleans up automatically when done
        pthread_detach(thread_id);
    }

    // Close the server socket (never reached in this example)
    close(server_socket);
    return 0;
}
