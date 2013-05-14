#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <iostream>

using namespace std;

// SOCKETS
SOCKET sock,client;

void s_cl(char *a, int x)
{
    cout<<a;
}

int main()
{
    //Declarations
    DWORD poll;
    int res,i=1,ret;
    char buf[1000],ip[15],tempBuf[1000],RecvdData[1000];
    WSADATA data;
    bool done;

    cout<<"412-SMTP Project Client\n";

    cout<<"\nEnter IP to connect to: ";
    gets(ip);

    sockaddr_in ser;
    sockaddr addr;

    ser.sin_family=AF_INET;
    ser.sin_port=htons(31000);               //Set the port
    ser.sin_addr.s_addr=inet_addr(ip);       //Set the address we want to connect to

    memcpy(&addr,&ser,sizeof(SOCKADDR_IN));

    res = WSAStartup(MAKEWORD(1,1),&data);      //Start Winsock
    int STRLEN = 256;

    if(res != 0)
        s_cl("WSAStarup failed",WSAGetLastError());

    sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);       //Create the socket
        if(sock==INVALID_SOCKET )
            s_cl("Invalid Socket ",WSAGetLastError());
        else if(sock==SOCKET_ERROR)
            s_cl("Socket Error)",WSAGetLastError());
        else
            cout<<"\nSocket Established"<<endl;

    res=connect(sock,&addr,sizeof(addr));               //Connect to the server
        if(res !=0 )
        {
            s_cl("SERVER UNAVAILABLE",res);
        }
        else
        {
            cout<<"Connected to Server:\n";
            memcpy(&ser,&addr,sizeof(SOCKADDR));
        }

    //hard code the smtp protocol
    memset(buf,'\0',256);
    res = recv(sock,RecvdData,sizeof(RecvdData),0); // recv 220 smtp.example.com
    if(res <= 0)
    {
        cout << "Error\n";
        closesocket(client);
        WSACleanup();
        return 9;
    }
    cout<< "\nServer -> " << RecvdData;

    memset(buf,'\0',256);

    strcpy(buf,"HELO relay.smtpZ.org"); //using relay.smtpZ.org
    cout<<"\nClient -> " << buf;
    send(sock,buf,sizeof(buf),0);       //send HELO

    memset(buf,'\0',1000);
    memset(RecvdData,'\0',1000);

    res = recv(sock,RecvdData,sizeof(RecvdData),0); //Expect to receive 250 Hello <domain>, glad to meet you....

    if(res <=0 || strncmp(RecvdData, "250", 3) != 0)
    {
        cout << "\nError\n";
        closesocket(client);
        WSACleanup();
        return 10;
    }
    cout<< "\nServer -> " << RecvdData;
    memset(RecvdData,'\0',1000);
    /*
    strcat(buf, "VRFY ");
    cout << "\nUser Name in <name@ip> format: ";     //enter username for server authentication: VRFY <name@ip>
    char username[100];
    cin.getline(username, 100);

    //fgets(username,sizeof(username),stdin);
    username[strcspn(username, "\n")] = '\0';
    const char* USERNAME = username;
    strcat(buf, USERNAME);          //concatenate the username to VRFY
*/
    //cout << "Client -> " << buf;
    //send(sock,buf,sizeof(buf),0);   //send VRFY <name@ip>

    //memset(buf,'\0',1000);
    //memset(RecvdData,'\0',1000);

    //res = recv(sock,RecvdData,sizeof(RecvdData),0); //recvd either 250 or 500
    //cout<< "\nServer -> " << RecvdData;

            strcat(buf, "VRFY ");
            cout << "\nUser Name in <name@ip> format: ";     //enter username for server authentication: VRFY <name@ip>
            char username[100];
            cin.getline(username, 100);

            username[strcspn(username, "\n")] = '\0';
            const char* USERNAME = username;
            strcat(buf, USERNAME);          //concatenate the username to VRFY
            cout << "Client -> " << buf;
            send(sock,buf,sizeof(buf),0);   //send VRFY <name@ip>

            memset(buf,'\0',1000);
            memset(RecvdData,'\0',1000);

            res = recv(sock,RecvdData,sizeof(RecvdData),0); //recvd either 250 or 500
            cout<< "\nServer -> " << RecvdData;


    done = true;
    while(done)
    {
        /*
            strcat(buf, "VRFY ");
            cout << "\nUser Name in <name@ip> format: ";     //enter username for server authentication: VRFY <name@ip>
            char username[100];
            cin.getline(username, 100);

            username[strcspn(username, "\n")] = '\0';
            const char* USERNAME = username;
            strcat(buf, USERNAME);          //concatenate the username to VRFY
            cout << "Client -> " << buf;
            send(sock,buf,sizeof(buf),0);   //send VRFY <name@ip>

            memset(buf,'\0',1000);
            memset(RecvdData,'\0',1000);

            res = recv(sock,RecvdData,sizeof(RecvdData),0); //recvd either 250 or 500
            cout<< "\nServer -> " << RecvdData;
            */
            if (strncmp(RecvdData, "500", 3) == 0 )
            {
                strcat(buf, "VRFY ");
                cout << "\nUser Name in <name@ip> format: ";     //enter username for server authentication: VRFY <name@ip>
                char username[100];
                cin.getline(username, 100);
                //fgets(username,sizeof(username),stdin);
                username[strcspn(username, "\n")] = '\0';
                const char* USERNAME = username;
                strcat(buf, USERNAME);          //concatenate the username to VRFY
                cout << "Client -> " << buf;
                send(sock,buf,sizeof(buf),0);   //send VRFY <name@ip>
                memset(buf,'\0',1000);
            }
            if(strncmp(RecvdData, "250",3) == 0)   //login success
            {
                    //done = false;
                    break;
            }
            memset(RecvdData,'\0',1000);

            res = recv(sock,RecvdData,sizeof(RecvdData),0); //recvd either 250 or 500
            cout<< "\nServer -> " << RecvdData;

    }   //end while


    bool QUIT = 0;
    while (QUIT != 1)
    {
        memset(buf,'\0',1000);
        cout << "\nClient -> ";
        fgets(tempBuf,sizeof(tempBuf),stdin);
        tempBuf[strcspn(tempBuf, "\n")] = '\0';
        strcat(buf,tempBuf);
        send(sock,buf,sizeof(buf),0);

        if(strcmp(buf, "INBOX")== 0)
        {
            cout << "WE ARE DOING INBOX!" << endl;
            res = recv(sock,RecvdData,sizeof(RecvdData),0);
            string toDecrypt = RecvdData;
            string msgTmp = toDecrypt;
            string frmDate;
            bool endMail = 0;
            while(endMail!=1){
                
                int rdCount = 2;
                int rdIndex = 0;
                frmDate = msgTmp;
                for(int j =0;j < 3; j++)
                {
                    rdIndex = frmDate.find(':');
                    if (j == 2){
                        frmDate = frmDate.substr(rdIndex-4,39);
                        break;
                    }
                    frmDate = frmDate.substr(rdIndex+1);
                    j++;
                }

                int count = 5;
                int i = 0;
                int index;
                while (i < count)
                {
                    index = msgTmp.find(':');
                    msgTmp = msgTmp.substr(index+1);
                    i++;
                }
                msgTmp = msgTmp.substr(3);
                index = msgTmp.find('.');
                toDecrypt = msgTmp.substr(0,index-1);
                msgTmp = msgTmp.substr(index+1);
                //cout <<"THIS IS TMPMSG: " << msgTmp << endl;
                //cout <<"THIS IS TEDECRYPT: " << toDecrypt << endl;
                
                int Size = strlen(toDecrypt.c_str());
                int x = 0;
                int key =5;
                cout << "\nDECRYPTED MAIL: \n" << frmDate << "\n";
                while(x != Size)
                {
                    char oneChar = toDecrypt.at(x);
                    char newVal;
                    int val = (int)oneChar;

                    val = val - 32;
                    val = val - key;
                    if(val < 0)
                        val = val + 95;

                    val = val + 32;
                    //itoa(val, newVal, 10);
                    newVal = (char)val;
                    cout << newVal ;
                    x++;
                }
                cout << "\n";
                int isDone = msgTmp.find('.');
                if (isDone == -1){
                    //cout << "THIS IS ISDONE:" << isDone << endl;
                    endMail = 1;
                }
            }
            //cout<< "\nServer -> " << RecvdData;
            memset(buf,'\0',1000);
            cout << "\nClient -> ";
            fgets(tempBuf,sizeof(tempBuf),stdin);
            tempBuf[strcspn(tempBuf, "\n")] = '\0';
            strcat(buf,tempBuf);
            send(sock,buf,sizeof(buf),0);
        }
        if(strcmp(buf, "MESSAGE")== 0)
        {
            cout << "WE ARE DOING MESSAGE!" << endl;
            //Cocatenate username to mail from
            memset(buf,'\0',1000);
            strcpy(buf,"MAIL FROM:<");
            cout << "\nMail From: ";
            fgets(tempBuf,sizeof(tempBuf),stdin);
            tempBuf[strcspn(tempBuf, "\n")] = '\0';
            strcat(buf,tempBuf);
            strcat(buf,">");
            send(sock,buf,sizeof(buf),0);       //Sending source mail address
            memset(buf,'\0',1000);
            memset(RecvdData,'\0',1000);
            res = recv(sock,RecvdData,sizeof(RecvdData),0);//a=make sure server understands MAIL FROM
            cout<< "\nServer -> " << RecvdData;
            //cout << "WE ARE DOING MESSAGE!" << endl;
            if(res <=0 || strncmp(RecvdData, "250",3)!=0)
            {
                cout << "Error\n";
                closesocket(client);
                WSACleanup();
                return 12;
            }

            done = 0;
            while(done != 1) //Cocatenate username to RCPT TO until the user press "n"
            {
                char* answer;
                strcpy(buf,"RCPT TO:<");
                cout << "\nRCPT TO: ";
                fgets(tempBuf,sizeof(tempBuf),stdin);
                tempBuf[strcspn(tempBuf, "\n")] = '\0';
                strcat(buf,tempBuf);
                strcat(buf,">");
                send(sock,buf,sizeof(buf),0);
                cout << "Client -> " << buf << endl;
                memset(buf,'\0',256);

                res = recv(sock,RecvdData,sizeof(RecvdData),0);
                cout<< "\nServer -> " << RecvdData;
                if(res <=0 || strncmp(RecvdData, "250",3)!=0)
                {
                    cout << "Error\n";
                    closesocket(client);
                    WSACleanup();
                    return 13;
                }

                //cout << "\nDo you want to enter another recipient (y/n) ";
                //cin >> answer;
                //cin.ignore(1000,'\n');
                answer = "n";
                memset(tempBuf,'\0',256);
                if(strcmp(answer, "n") == 0)
                {
                    done = 1;
                }
            }

            strcpy(buf,"DATA");
            cout<<"\nClient -> " << buf;
            send(sock,buf,sizeof(buf),0);

            memset(buf,'\0',256);
            memset(RecvdData,'\0',256);
            res = recv(sock,RecvdData,sizeof(RecvdData),0);
            if(res <=0 || strncmp(RecvdData, "354",3)!=0)
            {
                cout << "Error\n";
                closesocket(client);
                WSACleanup();
                return 14;
            }
            cout<< "\nServer -> " << RecvdData;

            done = 0;

            while(done != 1) //Writing the message until recieve "."
            {
                cout << "\nSEND: ";
                fgets(buf,sizeof(buf),stdin);
                buf[strcspn(buf, "\n")] = '\0';

                int      key;
                string   line;
                char*    text;
                char*    cipher;
                key = 5;

                string encMsg = buf;
                int Size = strlen(encMsg.c_str());
                int x = 0;
                string fMsg;
                while(x != Size)
                {
                    char oneChar = encMsg.at(x);
                    char newVal;
                    cout <<oneChar << "------>" ;
                    int val = (int)oneChar;

                    //cout << val << endl;
                    val = val - 32;
                    val = val + key;
                    if(val > 95)
                        val = val - 95;

                    val = val + 32;
                    //itoa(val, newVal, 10);
                    newVal = (char)val;
                    cout << val << "------>" << newVal << endl;
                    fMsg += newVal;
                    x++;
                }

                cout << encMsg <<" has been encrypted to: " <<fMsg << endl;
                if(strcmp(buf, ".") == 0){
                    done = 1;
                    fMsg = ".";
                    send(sock,fMsg.c_str(),sizeof(fMsg.c_str()),0);
                }
                char cs[1000];

                strcpy(cs ,fMsg.c_str());
                cout << "THIS IS CS: " << cs << endl;
                send(sock,cs,sizeof(cs),0);

                memset(cs,'\0',256);
                memset(buf,'\0',256);

            }
            memset(RecvdData,'\0',256);
            res = recv(sock,RecvdData,sizeof(RecvdData),0);
            cout<< "\nServer -> " << RecvdData;
            if(res <=0 || strncmp(RecvdData, "250",3)!=0)
            {
                cout << "Error\n";
                closesocket(client);
                WSACleanup();
                return 15;
            }
        }
        else if(strcmp(buf, "QUIT")== 0)
        {
            memset(buf,'\0',1000);
            strcpy(buf,"QUIT");
            cout<<"\nClient -> " << buf;
            send(sock,buf,sizeof(buf),0);

            memset(buf,'\0',256);
            memset(RecvdData,'\0',256);
            res = recv(sock,RecvdData,sizeof(RecvdData),0);
            if(res <=0 || strncmp(RecvdData, "221",3)!=0)
            {
                cout << "Error\n";
                closesocket(client);
                WSACleanup();
                return 11;
            }
            cout<< "\nServer -> " << RecvdData;

            printf("\nSERVER terminated connection\n");
            Sleep(40);
            closesocket(client);       //Closing the connection
            client = 0;
        }
        else{
            memset(buf,'\0',1000);
            cout << "\nClient -> ";
            fgets(tempBuf,sizeof(tempBuf),stdin);
            tempBuf[strcspn(tempBuf, "\n")] = '\0';
            strcat(buf,tempBuf);
            memset(buf,'\0',1000);
            memset(RecvdData,'\0',1000);
            res = recv(sock,RecvdData,sizeof(RecvdData),0);//a=make sure server understands MAIL FROM
            cout<< "\nServer -> " << RecvdData;
        }
    }
    //closesocket(client);
    //WSACleanup();
}
