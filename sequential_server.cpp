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

    
}