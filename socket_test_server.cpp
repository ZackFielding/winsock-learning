#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
// must link to ws_32
// compiling with g++ -lws_32 (links with libws_32.a)

#define C_LOG std::clog
#define BASE_SOCK_INDEX 0

void cleanupAndClose(std::vector<SOCKET> socket_vector)
{
   WSACleanup(); 
   for(SOCKET& sock : socket_vector)
   {
    closesocket(sock);
   } 
}


// Create a TCP server socket that awaits for client connection
int main()
{
    /*
     1. WSADATA is a struct that holds information about the windows socket
     2. WSAStartup initializes and returns information of the socket to the struct 
      - WSAStartup initiates use of WS2_32.dll
      - MAKEWORD(2,2) requests WinSock v2.2 on the system
      - takes a long pointer to WSADATA struct
    */
    WSADATA wsa_data;
    int sock_init_r {WSAStartup(MAKEWORD(2,2), &wsa_data)};
    
    if (sock_init_r != 0)
    {
        C_LOG << "Socket initialization error.  Error: " << WSAGetLastError() << '\n';
        return 1;         
    }
    else
    {
        C_LOG << "winsock dll found .. status of socket post-init: "
            << wsa_data.szSystemStatus << '\n';
    }

    // get address info
    addrinfo *addr_result, match_this; // structs to hold address info
      // match_this->ai_addrlen && ai_addre && ai_next must be 0 or NULL or else will fail func
      // set all memory to 0 using func
    ZeroMemory(&match_this, sizeof match_this); // set to 0s 
    match_this.ai_family = AF_INET; // UDP or TCP
    match_this.ai_socktype = SOCK_STREAM; //TCP
    match_this.ai_protocol = IPPROTO_TCP; //TCP

    int add_err {getaddrinfo(NULL, "55555", &match_this, &addr_result)};
    if (add_err != 0)
    {
        C_LOG << "Error in retrieving address information ... terminating program,\n";
        WSACleanup(); // tell OS to deregister socket - frees up .dll
        return 1;
    }
    else
        C_LOG << "Local address info returned sucessfully.\n";

    // create socket for server
    // this socket uses the information passed back from the getaddrinfo function
    // getaddrinfo finds information based on the match_this, and returns it to addr_result
    std::vector<SOCKET> sock_arr (5);
    sock_arr.at(BASE_SOCK_INDEX)= socket(addr_result->ai_family,
        addr_result->ai_socktype,
        addr_result->ai_protocol);
    
    if (sock_arr.at(BASE_SOCK_INDEX) == INVALID_SOCKET)
    {
        C_LOG << "Socket init failed.  Error: " << WSAGetLastError() << '\n';
        WSACleanup();
        return 1;
    }
    else
    {
        C_LOG << "Socket initialized!\n";
    }

    // now have a socket that is set up for TCP
    // need to bind the socket to an IP address to allow client connections
    int bind_err{bind(sock_arr.at(BASE_SOCK_INDEX), 
                    addr_result->ai_addr,
                    static_cast<int>(addr_result->ai_addrlen)
                    )};
    
    if (bind_err == SOCKET_ERROR)
    {
        C_LOG << "Error in binding address to socket ... terminating program.\n";
        cleanupAndClose(sock_arr);
        freeaddrinfo(addr_result); // free memory allocated for addrinfo return struct
        return 1;
    }
    freeaddrinfo(addr_result); // free memory (stuct to needed post-bind)

    // socket is now bound to address - ready to listen for a connection
    if (listen(sock_arr.at(BASE_SOCK_INDEX), 1) == SOCKET_ERROR)
    {
        C_LOG << "Error in listen function ... terminating program.\n";
        cleanupAndClose(sock_arr);
        return 1;        
    }
    else
    {
        C_LOG << "Socket is listening for client.\n";
    }

    // wait and accept incoming client connections
    size_t current_sock {1}; // start tracking copied sockets for connections
    sock_arr.at(current_sock)= accept(sock_arr.at(0), NULL, NULL);
    if (sock_arr.at(current_sock) == INVALID_SOCKET)
    {
        C_LOG << "Errorr in awaiting for client connection.\n";
        cleanupAndClose(sock_arr);
    }
    else
    {
        C_LOG << "Socket connected...\n";
        constexpr int rec_arr_size {200};
        char rec_arr [rec_arr_size];
        int bytes_rec {};
        bytes_rec = recv(sock_arr.at(current_sock), rec_arr, rec_arr_size, NULL);
        if (bytes_rec == SOCKET_ERROR)
            C_LOG << "Error in recieving data from client.\n";
        else
        {
            C_LOG << bytes_rec << "...number of bytes recieved from client.\n"
                    << "Message from client: " << rec_arr << '\n';
        }
    }

    return 0;
}