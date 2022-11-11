#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#define C_LOG std::clog
// create client TCP connection to communicate with server
int main()
{
    // initialize winsock
    WSADATA client_wsa_data;
    int wsa_err {WSAStartup(MAKEWORD(2,2), &client_wsa_data)};

    if (wsa_err != 0)
    {
       C_LOG << "Error while determining WinSock set-up ... terminating.\n"; 
       return 1;
    }
    else
    {
        C_LOG << "winsock dll found .. status of socket post-init: "
            << client_wsa_data.szSystemStatus << '\n';
    }

    // get local address information
    addrinfo c_match_addr, *c_result_addr;
    SecureZeroMemory(&c_match_addr, sizeof c_match_addr);
    c_match_addr.ai_family = AF_INET; 
    c_match_addr.ai_socktype = SOCK_STREAM; 
    c_match_addr.ai_protocol = IPPROTO_TCP;

    // store return variable
    // NULL.should be IP address of server-to-be connected to (NEED TO CHANGE)
    int get_addr_err {getaddrinfo(NULL, "55555", &c_match_addr, &c_result_addr)};
    if (get_addr_err != 0)
    {
        C_LOG << "Error in getting local address information ... terminating.\n";
        freeaddrinfo(c_result_addr); // free memory allocated to result info
        WSACleanup(); // delink .dll
        return 1;
    }
    else
    {
        C_LOG << "Local address info returned sucessfully.\n";
    }

    freeaddrinfo(c_result_addr); // free memory allocated to result info

    SOCKET base_sock {socket(c_result_addr -> ai_family, c_result_addr -> ai_socktype, 
                            c_result_addr -> ai_protocol)}; 
    if (base_sock == INVALID_SOCKET)
    {
        C_LOG << "Error while establishing socket: " << WSAGetLastError()
                << " ... terminating.\n";
        WSACleanup();
        return 1;
    }
    else
    {
        C_LOG << "Socket initialized!\n";
    }

    // try to connect to server (binding is done automatically in connect())
    // SHOULD try and connect to next address in c_result_addr if connect() fails
    // ... not going to while first testing
    int connect_err {connect(base_sock, c_result_addr -> ai_addr,
                static_cast<int>(c_result_addr -> ai_addrlen))};
    if (connect_err == SOCKET_ERROR)
    {
        int err_code {WSAGetLastError()};
        C_LOG << "Error in connecting to host... "; 
        switch (err_code)
        {
            case 10061:
            {
                C_LOG << "Server connection refused - server may not have server application running.\n";
                break;
            }
            default:
                C_LOG << err_code << '\n';
        }
        WSACleanup();
        closesocket(base_sock);
        freeaddrinfo(c_result_addr);
        return 1;
    }
    else
    {
        C_LOG << "Client connection established .. data transmission OK.\n";
    }

    return 0;
}