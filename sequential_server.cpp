#include <iostream>
#include <cstring>     // For memset
#include <sys/types.h> // For data types
#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <unistd.h>     // For close()

#define PORT 12345
#define BUFFER_SIZE 1024