#include "cMessenger.h"

#include <stdio.h> /* for printf() */
#include <stdlib.h> /* for malloc() */
#include <string.h> /* for strcopy() */

/*FUNCTIONS*/
/* Change a user structure.
 Pass a strPtr that will modify the existing values saved in userPtr.
 Pass mode=1 to modify user name or pass mode=2 to modify user color (must be scaled). */
void ChangeUser(USER* userPtr, char* strPtr, int mode)
{
    /* Validity checks */
    if (!userPtr)
        return;
    if (!strPtr)
        return;
    
    /* Save string pointer into User->name data structure */
    if (1 == mode)
    {
        strncpy(userPtr->userName, strPtr, USERNAMESIZE);
    }
    /* Save string pointer into User->color data structure */
    else if (2 == mode)
    {
        userPtr->userColor = atoi(strPtr);
    }
    
    return;
}

/* A complete procedure that lets a user to create a USER structure and returns it */
void CreateUser()
{
    char userColorInput[1];
    
    AddMessage(SYSTEMUSER, "To create/modify a user profile", 1);
    
    /* Create name */
    {
        char usrNameInput[USERNAMESIZE];
        
        AddMessage(SYSTEMUSER, "Type in your nickname", 1);
        
        /* Receive a name as a user input and save it in the data structure */
        strncpy(usrNameInput, ProcessMessage(USERNAMESIZE, 0), USERNAMESIZE);
        ChangeUser(CUSER, usrNameInput, 1);
        
        AddMessage(CUSER, CUSER->userName, 0);
    }
    
    /* Choose color */
    {
        int i;
        char* tempColor = (char*)malloc(sizeof(char[2]));
        
        /* Print various colors */
        AddMessage(SYSTEMUSER, "Choose color", 0);
        /* Print system action */
        {
            printf("%s\n", SYSTEMACTION);
            for (i = 1; i < 6; i++)
            {
                printf("\x1b[97;%dm %d - %s \x1b[0m ", i + 40, i, "message");
            }
            printf("\n%s\n", SYSTEMACTION);
        }
        
        while (userColorInput[0] < 49 || userColorInput[0] > 53)
        {
            strncpy(userColorInput, ProcessMessage(1, 0), 1);
        }
        
        sprintf(tempColor, "%d", (atoi(userColorInput) + 40));
        
        ChangeUser(CUSER, tempColor, 2);
        
        /* Reference: http://forums.codeguru.com/showthread.php?347081-itoa-error */
        sprintf(tempColor, "%d", (CUSER->userColor - 40));
        
        AddMessage(CUSER, tempColor, 0);
    }
}

/* Add a new node to the dynamic structure */
void AddMessage(USER* userPtr, char* messageStr, int indentation)
{
    struct Message* iMessage;
    struct Message* tempMessage;
    
    iMessage = MESSAGEHIST->top;
    
    /* Fill out the new message */
    {
        tempMessage = (struct Message*)malloc(sizeof(struct Message));
        tempMessage->sender = userPtr;
        
        /* Save a message into a data structure up to max allowed size */
        strncpy(tempMessage->message, messageStr, MESSAGEMAXSIZE);
        
        tempMessage->indentation = indentation;
        
        tempMessage->next = NULL;
    }
    
    if (NULL == iMessage)
    {
        MESSAGEHIST->top = tempMessage;
    }
    else
    {
        while (NULL != iMessage->next)
        {
            iMessage = iMessage->next;
        }
        
        iMessage->next = tempMessage;
    }
    
    PrintHistory(MESSAGEHIST->top);
}
