/* GLOBAL SYMBOLIC CONSTANTS */
#define USERNAMESIZE 16
/* Scale the size, so even it is small, the program will be able to process a system call */
#define MESSAGEMINSIZE 2
/* Scale the message size, so it will be restricted to whatever max size specified */
#define MESSAGEMAXSIZE 240

/* STRUCTURES DEFINITIONS */
/* Structure that is used to store data about a user:
 Includes a username (16 characters max) and a color, saved as an int */
struct User
{
    char userName[USERNAMESIZE];
    
    /* Color Reference: https://misc.flogisoft.com/bash/tip_colors_and_formatting */
    int userColor;
} typedef USER;

/* Message structure is a linked-list node that is used to store data about a single message:
 Includes data of the sender (in the User structure's pointer),
 message string's pointer,
 indentation that tells the printing function whether the sender name should be printed
 and an address of the next message */
struct Message
{
    USER* sender;
    
    char message[MESSAGEMAXSIZE];
    int indentation;
    
    struct Message* next;
} typedef MESSAGE;

/* Message History is a linked-list consisting of Message nodes. As such, it contains an address of the first Message structure */
struct MessageHistory
{
    MESSAGE* top;
} typedef MESSAGEHISTORY;
/**/

/* GLOBAL DECLARATION */
/* System user (cMessenger). */
extern USER* SYSTEMUSER;
/* Current User.
 The pointer cannot be changed, but the values pointed to can be */
extern USER* const CUSER;
/* Message thread of the session. */
extern MESSAGEHISTORY* MESSAGEHIST;
/* Standard encapsulation of a system action. */
extern const char* SYSTEMACTION;
/* Determine what message format string should be like. */
extern const int COLORFORMATFLAG;
/**/

/* FUNCTIONS PROTOTYPES */
/* Create a user structure */
void CreateUser(void);

/* Add a new node to the dynamic structure */
void AddMessage(USER*, char*, int);

/* Create a listening host using a socket */
int CreateServer(void);

/* Create a client by connecting to a listening socket at a specified IP address */
int CreateClient(void);

/* A universal string processing method that includes systems calls.
 Allocates memory for a string with the size specified.
 By default, returns the pointer to the string. */
char* ProcessMessage(int, int);

/* Print a message*/
void PrintMessage(USER*, char*, int);

/* Print a passed profile's information */
void PrintProfile(USER*);

/* Traverse the dynamic strucre of messages */
void PrintHistory(MESSAGE*);
