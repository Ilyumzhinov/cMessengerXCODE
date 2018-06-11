#include "cMessenger.h"

#include <stdio.h> /* for printf() */
#include <stdlib.h> /* for malloc(), exit() */
#include <unistd.h> /* for close() */
#include <string.h> /* for strcopy(), strlen(), memset() */
#include <arpa/inet.h> /* for struct sockaddr_in, SOCK_STREAM */

/*SYMBOLIC CONSTANTS*/
#define PORT 8080
/* Max number of characters received and sent */
#define MAXSIZE 1024
/* Max participants waiting for a connection */
#define MAXPENDING 1
/**/

/*FUNCTION PROTOTYPES */
/* Exchange messages in a synchronous manner; one by one, that is
 specify a client and pass sendFirst=1 for a client */
void ExchangeMsgs(int xSocket, int sendFirst);
/**/

/*GLOBAL VARIABLES*/
/* String in which received messages are saved */
char msgBuffer[MAXSIZE];
/* Connection user information is received on client connection */
USER* connectionUser = &(USER){ .userName = "\0", .userColor = 0 };
/**/

/*FUNCTIONS*/
/* Create a listening host using a socket */
/* Source code:
 https://www.geeksforgeeks.org/socket-programming-cc/
 https://codereview.stackexchange.com/questions/13461/two-way-communication-in-tcp-server-client-implementation?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
 http://www.csd.uoc.gr/~hy556/material/tutorials/cs556-3rd-tutorial.pdf */
int CreateServer()
{
    /* Socket descriptor for serverAddress */
    int serverSocket;
    /* Socket descriptor for client */
    int clientSocket;
    /* Local address:
     struct sockaddr_in - data type specific to TCP/IP addresses */
    struct sockaddr_in serverAddress;
    /* Client address */
    struct sockaddr_in clientAddress;
    unsigned int size = sizeof(clientAddress);
    int yes = 1;
    /**/
    
    /* SOCKET: Create socket for incoming connections.
     AF_INET - IPv4 protocols family,
     SOCK_STREAM - TCP socket type,
     0 - default protocol */
    if (-1 == (serverSocket = socket(AF_INET, SOCK_STREAM, 0)))
    {
        AddMessage(SYSTEMUSER, "Socket failure!", 0);
    }
    
    /* Construct local address structure */
    serverAddress.sin_family = AF_INET; /* Internet address family */
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
    serverAddress.sin_port = htons(PORT); /* Local port */
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    
    /* BIND: Assign address to socket.
     serverSocket - socket descriptor,
     serverAddress - the address and port,
     size - size of sockaddr_in structure */
    if (-1 == (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress))))
    {
        AddMessage(SYSTEMUSER, "Binding Failure!", 0);
        close(serverSocket);
        return -2;
    }
    
    /* LISTEN: Mark the socket to listen for incoming connections */
    if (-1 == listen(serverSocket, MAXPENDING))
    {
        AddMessage(SYSTEMUSER, "Listening Failure!", 0);
        close(serverSocket);
        return -3;
    }
    
    PrintMessage(SYSTEMUSER, "Waiting for a connection...", 0);
    
    /* Allow multiple consecutive connections to a listening socket */
    for(;;)
    {
        /* ACCEPT: Dequeue the net connection on the socket (blocking).
         clientSocket - socket used for data transfer,
         serverSocket - being listened to socket,
         &clientAddress - address of the active participant (filled in upon return),
         size - size of sockaddr_in structure */
        if (-1 == (clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &size)))
        {
            AddMessage(SYSTEMUSER, "Accept error!", 0);
            close(serverSocket);
            return -1;
        }
        
        /* COMMUNICATING */
        {
            int connectionStatus = 0;
            
            /* Receive client user info */
            connectionStatus = ReceiveUserInfo(clientSocket, connectionUser, 1);
            if (1 == connectionStatus)
            {
                /* Send current user info */
                connectionStatus = SendUserInfo(clientSocket, CUSER, 1);
                if (1 == connectionStatus)
                {
                    ExchangeMsgs(clientSocket, 0);
                }
            }
        }
    }
    
    return 0;
}

/* Create a client by connecting to a listening socket at a specified IP address */
/* Source code:
 https://www.geeksforgeeks.org/socket-programming-cc/
 https://codereview.stackexchange.com/questions/13461/two-way-communication-in-tcp-server-client-implementationutm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
 http://www.csd.uoc.gr/~hy556/material/tutorials/cs556-3rd-tutorial.pdf */
int CreateClient()
{
    /* Socket data */
    int clientSocket;
    struct sockaddr_in serverAddress;
    
    char ipServer[64];
    /* To print Enter IP only once */
    int mem = 0;
    /**/
    
    /* MAKING A CONNECTION */
    {
        /* SOCKET: Create an active TCP socket */
        if (-1 == (clientSocket = socket(AF_INET, SOCK_STREAM, 0)))
        {
            AddMessage(SYSTEMUSER, "Socket creation error!", 0);
            return -1;
        }
        
        serverAddress.sin_family = AF_INET; /* Internet address family */
        serverAddress.sin_port = htons(PORT); /* Server port */
        
        /* Receive a server IP address from a user.
         Convert IPv4 and IPv6 addresses from text to binary form */
        while (0 >= inet_pton(AF_INET, ipServer, &serverAddress.sin_addr))
        {
            /* Receive an IP address from the user */
            if (0 == mem++)
                AddMessage(SYSTEMUSER, "Enter IP", 0);
            
            {
                printf("%s\n", SYSTEMACTION);
                PrintMessage(SYSTEMUSER, "Type 'x' to connect to localhost (127.0.0.1)", 1);
                strncpy(ipServer, ProcessMessage(64, 0), 64);
                
                if ('x' == ipServer[0])
                    strncpy(ipServer, "127.0.0.1", 64);
            }
            
            AddMessage(CUSER, ipServer, 0);
            
            if(0 >= inet_pton(AF_INET, ipServer, &serverAddress.sin_addr))
            {
                PrintMessage(SYSTEMUSER, "Invalid address", 0);
            }
            printf("%s\n", SYSTEMACTION);
        }
        
        /* CONNECT: Establish a connection with the server.
         clientSocket - socket to be used in connection,
         serverAddress - address of the server,
         size - size of sockaddr_in structure */
        if (-1 == connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)))
        {
            AddMessage(SYSTEMUSER, "Connection failed", 0);
            close(clientSocket);
            return -1;
        }
    }
    
    /* COMMUNICATING */
    {
        int connectionStatus = 0;
        
        /* Send current user info */
        connectionStatus = SendUserInfo(clientSocket, CUSER, 1);
        if (1 == connectionStatus)
        {
            /* Receive server user info */
            connectionStatus = ReceiveUserInfo(clientSocket, connectionUser, 1);
            if (1 == connectionStatus)
            {
                ExchangeMsgs(clientSocket, 1);
            }
        }
    }
    
    return 0;
}

/* Exchange messages in a synchronous manner; one by one, that is */
void ExchangeMsgs(int xSocket, int sendFirst)
{
    int connectionStatus = 1;
   
    AddMessage(SYSTEMUSER, "Connection established! Say hello;)", 0);
    
    /* Send a message first */
    if (sendFirst)
    {
        if (-1 == SendAndDisplayMsg(xSocket))
        {
            connectionStatus = -1;
        }
    }
    
    /* Allow multiple send/receives */
    if (1 == connectionStatus)
    {
        while(1)
        {
            /* Receive a message */
            if (-1 == ReceiveAndDisplayMsg(xSocket))
                break;
            
            /* Send a message */
            if (-1 == SendAndDisplayMsg(xSocket))
                break;
        }
    }
    
    /* CLOSE: Close connection, free port used */
    close(xSocket);
    
    AddMessage(SYSTEMUSER, "Connection closed!", 0);
}

/* Send user info to a socket and validate if it was received */
int SendUserInfo(int xSocket, USER* sender, int validate)
{
    /* Send username */
    strncpy(msgBuffer, sender->userName, USERNAMESIZE);
    if (USERNAMESIZE != (send(xSocket, msgBuffer, USERNAMESIZE, 0)))
    {
        return -1;
    }
    
    /* Send color */
    sprintf(msgBuffer, "%d", sender->userColor);
    if (2 != (send(xSocket, msgBuffer, 2, 0)))
    {
        return -2;
    }
    
    /* Validate the transaction */
    if (validate)
    {
        USER* validateUser = (USER*)malloc(sizeof(USER));
        
        /* Receive the user info obtained that the other side */
        ReceiveUserInfo(xSocket, validateUser, 0);
        
        if (strncmp(sender->userName, validateUser->userName, USERNAMESIZE))
            return 0;
        else if (sender->userColor != validateUser->userColor)
            return 0;
    }
    
    return 1;
}

/* Receive user info to a socket and validate if it was received correctly */
int ReceiveUserInfo(int xSocket, USER* sender, int validate)
{
    char senderUsername[USERNAMESIZE] = "\0";
    char senderColor[2] = "\0";
    
    /* Receive username */
    if (-1 == recv(xSocket, senderUsername, USERNAMESIZE, 0))
    {
        return -1;
    }
    
    /* Receive color */
    if (-1 == recv(xSocket, senderColor, 2, 0))
    {
        return -2;
    }
    
    /* Assemble the User */
    ChangeUser(sender, senderUsername, 1);
    ChangeUser(sender, senderColor, 2);
    
    /* Validate the user info */
    if (validate)
        return SendUserInfo(xSocket, sender, 0);
    
    return 1;
}

/* Send a message to a socket and display it */
int SendAndDisplayMsg(int xSocket)
{
    /* Input and save a message */
    strncpy(msgBuffer, ProcessMessage(MAXSIZE, 1), MAXSIZE);
    /* Display it in the message history */
    AddMessage(CUSER, msgBuffer, 0);
    
    /* SEND: Send data (blocking).
     msgBuffer - message to send,
     SENDBUFFERSIZE - size of the message,
     0 - no flags */
    if (MAXSIZE != (send(xSocket, msgBuffer, MAXSIZE, 0)))
    {
        AddMessage(SYSTEMUSER, "Failure Sending Message", 0);
        
        return -1;
    }
    
    return 0;
}

/* Receive a message to a socket and display it */
int ReceiveAndDisplayMsg(int xSocket)
{
    long int msgSize = -1;
    
    if (-1 == (msgSize = recv(xSocket, msgBuffer, MAXSIZE, 0)))
    {
        AddMessage(SYSTEMUSER, "Connection Closed", 0);
        
        return -1;
    }
    
    /* Display a received message */
    AddMessage(connectionUser, msgBuffer, 0);
    /**/
    
    return (int)msgSize;
}
