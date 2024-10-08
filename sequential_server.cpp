#include <iostream>
#include <cstring>     // For memset
#include <sys/types.h> // For data types
#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <unistd.h>     // For close()

#define PORT 12345
#define BUFFER_SIZE 1024
using namespace std;

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
        cerr << "Failed to bind socket" << endl;
        close(server_socket);
        return 1;
    }
    
    // listen for incoming connections
    if (listen(server_socket, 5) == -1){
        cerr << "Failed to listen on socket" << endl;
        close(server_socket);
        return 1;
    }
    cout << "Server is listening on port " << PORT << "..." << endl;

    //start of the stateful machine
    while (true){
        // accept a new connection
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == -1){
            cout << "Failed to accept client connection" << endl;
            continue;
        }
        // receive data from client 
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received == -1) {
            cerr << "Failed to receive data from client" << endl;
        } else {
            cout << "Received data: " << buffer << endl;

             // send data to client
            const char* response = "Response from server";
            ssize_t bytes_sent = send(client_socket, response, strlen(response), 0);
            if (bytes_sent == -1) {
                cerr << "Failed to send response to client" << endl;
            }
        }

        // close the connection
        close(client_socket);
        cout<<"Client connection closed"<<endl;
    }

    // close connection
    close(server_socket);
    return 0;
}