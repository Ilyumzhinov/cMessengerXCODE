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
