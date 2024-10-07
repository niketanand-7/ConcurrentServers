#include <iostream>
#include <cstring>     // For memset
#include <sys/types.h> // For data types
#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <unistd.h>     // For close()

#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    // create a socket using socket()
    // AF_INET: indicates IPV4
    // SOCK_STREAM: indicates TCP
    // 0: indicates the protocol (TCP)
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // binding socket to an ip address and port
    struct sockaddr_in server_addr;
    // &server_addr: pointer to the server_addr (which has the information about the server's ip address and port number)
    // writing 0 to every block in the memory
    // sizeof(server_addr): 
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; // IPV4
    server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to any available network interface
    server_addr.sin_port = htons(PORT); // Convert port to network byte order

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(server_socket);
        return 1;
    }

    
}