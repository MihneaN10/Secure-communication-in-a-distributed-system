#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <iostream>
#include <string>
#include <map>
#include "Sha1.h"
#include "Aes.h"
#include "Rsa.h"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT_CLIENT "27014"
#define DEFAULT_PORT_SERVER "27015"

using namespace std;

map <string, string> ipToKey;
string ip, ipServer;
unsigned char key[256] = "r5u8x/A?D(G+KbPeShVmYp3s6v9y$B&E";

int Client() {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    char sendbuf[DEFAULT_BUFLEN];
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    bool STOP = true;
    while (STOP) {
        cout << "\nText de transmis(sau STOP pentru a inchide conversatia): ";
        ZeroMemory(&sendbuf, sizeof(sendbuf));
        cin.getline(sendbuf, DEFAULT_BUFLEN);
        if (strcmp(sendbuf, "STOP") == 0)
        {
            STOP = false;
            if (ipToKey[ip] == "")
                continue;
        }
        // Initialize Winsock
        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            printf("WSAStartup failed with error: %d\n", iResult);
            return 1;
        }

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        // Resolve the server address and port
        iResult = getaddrinfo(ip.c_str(), DEFAULT_PORT_CLIENT, &hints, &result);
        if (iResult != 0) {
            printf("getaddrinfo failed with error: %d\n", iResult);
            WSACleanup();
            return 1;
        }
        // Attempt to connect to an address until one succeeds
            // Create a SOCKET for connecting to server
        ConnectSocket = socket(result->ai_family, result->ai_socktype,
            result->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
        }

        freeaddrinfo(result);

        if (ipToKey[ip] == "")
        {
            iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
            if (iResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
            }
            cout << "Am primit cheia public pentru RSA impreuna cu hash-ul generat de SHA1 pe care il compar cu propriul calcul\n";
            SHA1 sha1;
            sha1.update(strstr(recvbuf, " E:"));
            string s = recvbuf;
            string ss = sha1.final();
            //cout << ss << '\n';
            //cout << s.substr(0, s.find(" "));
            if (ss.compare(s.substr(0, s.find(" "))) == 0)
            {
                //printf("corect\n");
                ipToKey[ip] = (char*)key;
                string e = s.substr(s.find(" E:") + 3, s.find(" N:") - s.find(" E:") - 3);
                string N = s.substr(s.find(" N:") + 3, s.size() - s.find(" N:") - 3);
                string k = (char*)key;
                BigInteger MM = rsaEncrypt(k, e, N);
                string mm = bigIntegerToString(MM);
                mm = mm + '\0';
                iResult = send(ConnectSocket, mm.c_str(), mm.size(), 0);
                cout << "Trimit cheia de sesiune pentru AES criptata cu cheia public pentru RSA\n";
                if (iResult == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }
            }
            else
            {
                cout << "SHA1 primit nu se potriveste cu cel calculat\n";
                iResult = send(ConnectSocket, "BADHASH", 7, 0);
                if (iResult == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }
            }
        }
        istringstream is(sendbuf);
        string s = "";
        char INPUT[16] = "", OUTPUT[16];
        int i = 0;
        while (1) {
            i = i + 16;
            is.read(INPUT, 16);
            if (i >= strlen(sendbuf))
                break;
            encryptAES_BCD((unsigned char*)INPUT, (unsigned char*)OUTPUT, key);
            s = s + OUTPUT;
            s = s.substr(0, i);
        }
        encryptAES_BCD((unsigned char*)INPUT, (unsigned char*)OUTPUT, key);
        s = s + OUTPUT;
        s = s.substr(0, i);
        // Send an initial buffer
        iResult = send(ConnectSocket, s.c_str(), i, 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

        // shutdown the connection since no more data will be sent
        iResult = shutdown(ConnectSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

        // Receive until the peer closes the connection
        do {

            iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
            if (iResult < 0)
                printf("recv failed with error: %d\n", WSAGetLastError());

        } while (iResult > 0);

        // cleanup
        closesocket(ConnectSocket);
        WSACleanup();
    }
    return 0;
}

int Server() {
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    while (1) {
        memset(recvbuf, ' ', sizeof(recvbuf));
        // Initialize Winsock
        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            printf("WSAStartup failed with error: %d\n", iResult);
            return 1;
        }

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        // Resolve the server address and port
        iResult = getaddrinfo(ipServer.c_str(), DEFAULT_PORT_SERVER, &hints, &result);
        if (iResult != 0) {
            printf("getaddrinfo failed with error: %d\n", iResult);
            WSACleanup();
            return 1;
        }
        struct sockaddr_in* sockaddr_ipv4;
        // Create a SOCKET for the server to listen for client connections.
        ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (ListenSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            freeaddrinfo(result);
            WSACleanup();
            return 1;
        }

        // Setup the TCP listening socket
        iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            printf("bind failed with error: %d\n", WSAGetLastError());
            freeaddrinfo(result);
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        freeaddrinfo(result);
        iResult = listen(ListenSocket, SOMAXCONN);
        if (iResult == SOCKET_ERROR) {
            printf("listen failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        // Accept a client socket
        struct sockaddr_in sa = { 0 }; /* for TCP/IP */
        socklen_t socklen = sizeof(sockaddr);
        ClientSocket = accept(ListenSocket, (struct sockaddr*)&sa, &socklen);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
#pragma warning(suppress : 4996)
        char* IP = inet_ntoa(sa.sin_addr);

        // No longer need server socket
        closesocket(ListenSocket);
        if (ipToKey[IP] == "") {
            cout << IP << " incearca sa se conecteze\n";
            cout << "Trimit cheie publica RSA cu criptare SHA1\n";
            string s = " E:65537 ";
            s = s + "N:7243867490989233467384967162493995507079997637730128226620729167848417921603363941441104994233585312212872796728534164766298500253607968097500254417259297";
            SHA1 sha1;
            sha1.update(s);
            string ss = sha1.final();
            ss = ss + s;
            ss = ss + '\0';
            iSendResult = send(ClientSocket, ss.c_str(), ss.size(), 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
            if (iResult == SOCKET_ERROR) {
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            if (strcmp(recvbuf, "BADHASH") == 0) {
                closesocket(ClientSocket);
                WSACleanup();
                continue;
            }
            cout << "Am primit cheie de sesiune pentru AES criptata cu cheia public de la RSA si retin cheia de sesiune decriptata\n";
            ipToKey[IP] = rsaDecrypt(stringToBigInteger(recvbuf), "4542601372425889452409896401180680033415837510344275903041926352140535571387887052951323950781974357961693345053465308015087386170660785695019453523218433", "7243867490989233467384967162493995507079997637730128226620729167848417921603363941441104994233585312212872796728534164766298500253607968097500254417259297");
        }

        // Receive until the peer shuts down the connection
        printf("\n%s: ", IP);
        do {

            iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
            if (iResult > 0) {
                istringstream is(recvbuf);
                char INPUT[16], OUTPUT[16];
                int i = 0;
                while (1) {
                    strcpy_s(INPUT, "");
                    i = i + 16;
                    is.read(INPUT, 16);
                    if (i >= iResult)
                        break;
                    decryptAES_BCD((unsigned char*)INPUT, (unsigned char*)OUTPUT, key);
                    for (int j = 0; j <= 15; j++)
                        cout << OUTPUT[j];
                }
                decryptAES_BCD((unsigned char*)INPUT, (unsigned char*)OUTPUT, key);

                if (strcmp("STOP", OUTPUT) == 0) {
                    cout << "a inchis conexiunea\n";
                    ipToKey[IP] = "";
                }
                else {
                    for (int j = 0; j <= 15; j++)
                        cout << OUTPUT[j];
                    cout << '\n';
                }

                // Echo the buffer back to the sender
                iSendResult = send(ClientSocket, OUTPUT, 16, 0);
                if (iSendResult == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(ClientSocket);
                    WSACleanup();
                    return 1;
                }
            }
            else if (iResult < 0) {
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }

        } while (iResult > 0);

        // shutdown the connection since we're done
        iResult = shutdown(ClientSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

        // cleanup
        closesocket(ClientSocket);
        WSACleanup();
    }
}

int main() {
    FILE* pin = _popen("\"c:\\Program Files (x86)\\Nmap\\nmap.exe\" -sn 192.168.91.0/24", "r");
    if (pin) {
        string s;
        while (!feof(pin)) {
            char line[100];
            fgets(line, 100, pin);
            if (strstr(line, "192.168") != NULL)
            {
                s = strstr(line, "192.168");
                s[s.length() - 1] = '\0';
                ipToKey[s] = "";
            }
        }
        ipToKey.erase(s);
        ipServer = s;
        fclose(pin);
    }
    thread s(Server);
    while (1) {
        cout << "Introdu un ip din lista pentru conversatie sau 0 pentru a parasi aplicatia!\n";
        for (map<string, string>::iterator it = ipToKey.begin(); it != ipToKey.end(); ++it)
        {
            cout << it->first << '\n';
        }
        cin >> ip;
        cin.get();
        if (ip == "0")
            break;
        if (ipToKey.find(ip) != ipToKey.end())
            Client();
        ipToKey[ip] = "";
        ip = "";
    }
    exit(0);
    return 0;
}