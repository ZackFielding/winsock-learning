#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#define C_LOG std::clog

int main()
{
    /*
     1. WSADATA is a struct that holds information about the windows socket
     2. WSAStartup initializes and returns information of the socket to the struct 
      - WSAStartup initiates use of WS2_32.dll
      - MAKEWORD(2,2) requests WinSock v2.2 on the system
      - takes a long pointer to WSADATA struct
    */
    WSADATA* wsa_data;
    int sock_init_r {WSAStartup(MAKEWORD(2,2), wsa_data)};
    
    if (sock_init_r != 0)
    {
        C_LOG << "Socket initialization error ... terminating program.\n";
        return 1;         
    }
    else
    {
        C_LOG << "winsock dll found .. status of socket post-init: "
            << wsa_data->szSystemStatus << '\n';
    }

    // get address info
    addrinfo *addr_result, *match_this; // structs to hold address info
      // match_this->ai_addrlen && ai_addre && ai_next must be 0 or NULL or else will fail func
      // set all memory to 0 using func
    SecureZeroMemory(match_this, sizeof match_this); // set to 0s 
    match_this->ai_family = AF_INET; // UDP or TCP
    match_this->ai_socktype = SOCK_STREAM; //TCP
    match_this->ai_protocol = IPPROTO_TCP; //TCP

    int add_err {getaddrinfo(NULL, "55555", match_this, &addr_result)};
    if (add_err != 0)
    {
        C_LOG << "Error in retrieving address information ... terminating program,\n";
        WSACleanup(); // tell OS to deregister socket - frees up .dll
        return 1;
    }

    // create socket for server
    // this socket uses the information passed back from the getaddrinfo function
    // getaddrinfo finds information based on the match_this, and returns it to addr_result
    SOCKET ser_sock {socket(addr_result->ai_family,
        addr_result->ai_socktype,
        addr_result->ai_protocol)};
    
    if (ser_sock == INVALID_SOCKET)
    {
        C_LOG << "Socket init failed ... terminating program.\n";    
        WSACleanup();
        return 1;
    }

    // now have a socket that is set up for TCP
    // need to bind the socket to an IP address to allow client connections
    int bind_err{bind(ser_sock, 
                    addr_result->ai_addr,
                    static_cast<int>(addr_result->ai_addrlen)
                    )};
    
    if (bind_err == SOCKET_ERROR)
    {
        C_LOG << "Error in binding address to socket ... terminating program.\n";
        WSACleanup(); // free .dll
        closesocket(ser_sock); // close socket
        freeaddrinfo(addr_result); // free memory allocated for addrinfo return struct
        return 1;
    }

    freeaddrinfo(addr_result); // free memory (stuct to needed post-bind)

    return 0;
}