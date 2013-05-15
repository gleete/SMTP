#include <iostream>
#include "WinSock2.h"
#include <fstream>
#include <string>
#include <process.h>
#include <windows.h>
#include <vector>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

#define THREADCOUNT 2

const int STRLEN = 1000;
HANDLE ghMutex;
char* rcpt;
char* mailFrom;

struct DateTimeMsg {
    string currentDate;
    string currentTime;
    string theMessage;
};


class Socket
{
    protected:
        WSADATA wsaData;
        SOCKET mySocket;
        SOCKET myBackup;
        SOCKET acceptSocket;
        sockaddr_in myAddress;

    public:
        Socket();
        ~Socket();
        bool SendData( char* );
        bool RecvData( char*, int );
        void CloseConnection();
        void GetAndSendMessage();
        void Split(vector<string>*, string, string, int);
        DWORD WINAPI receive_cmds(LPVOID );
                char* mailFrom;
        char *rcpt;

};

class ServerSocket : public Socket
{
    public:
        void Listen();
        void Bind( int port );
        void StartHosting( int port );
};

class ClientSocket : public Socket
{
    public:
        void ConnectToServer( const char *ipAddress, int port );
};

Socket::Socket()
{
    if( WSAStartup( MAKEWORD(2, 2), &wsaData ) != NO_ERROR )
    {
        cerr<<"Socket Initialization: Error with WSAStartup\n";
        system("pause");
        WSACleanup();
        exit(10);
    }

    //Create a socket
    mySocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if ( mySocket == INVALID_SOCKET )
    {
        cerr<<"Socket Initialization: Error creating socket"<<endl;
        system("pause");
        WSACleanup();
        exit(11);
    }

    myBackup = mySocket;
}

Socket::~Socket()
{
    WSACleanup();
}

bool Socket::SendData( char *buffer )
{
    send( mySocket, buffer, strlen( buffer ), 0 );
    return true;
}

bool Socket::RecvData( char *buffer, int size )
{
    int i = recv( mySocket, buffer, size, 0 );
    buffer[i] = '\0';
    return true;
}

void Socket::CloseConnection()
{
    //cout<<"CLOSE CONNECTION"<<endl;
    closesocket( mySocket );
    mySocket = myBackup;
}

void Socket::GetAndSendMessage()
{
    char message[STRLEN];
    cin.ignore();//without this, it gets the return char from the last cin and ignores the following one!
    cout<<"Send > ";
    cin.get( message, STRLEN );
    SendData( message );
}

void split (vector<string>* pnt, string stn, string delim, int num)
{
    int found;
    if (stn.length() < 1)
    {
        pnt -> push_back("0");
    }

    delim = delim.substr(0,1);  //the delimeter is only one character
    //cout << "the delimeter is: \"<" << delim << ">\"" << endl;

    found = stn.find_first_of(delim);   //look for the first instance of the delim
    //cout << "first delim found at: " << found << endl;
    while(found != -1)
    {
        if (found > 0 )
        {
            pnt -> push_back(stn.substr(0,found));  //add the first value
            //cout << stn.substr(0,found) << endl;
        }
        stn = stn.substr(found + 1);
        found = stn.find_first_of(delim);
        num++;
        //cout << "found another delim at: " << found << endl;
    } //end while
    if (stn.length() > 0)
    {
        pnt -> push_back(stn);
        //cout << stn.substr(found + 1 ) << endl;
        num++;
    }
    // << num << endl;
}

void ServerSocket::StartHosting( int port )
{
     Bind( port );
     hostent* localHost;
     char* localIp;
     localHost = gethostbyname("");
     localIp = inet_ntoa (*(struct in_addr *)*localHost->h_addr_list);
     cout << "hosting on port " << myAddress.sin_port << "\nIP: " << myAddress.sin_addr.s_addr << endl;
     Listen();
}

void ServerSocket::Listen()
{
    //cout<<"LISTEN FOR CLIENT..."<<endl;

    if ( listen ( mySocket, 1 ) == SOCKET_ERROR )
    {
        cerr<<"ServerSocket: Error listening on socket\n";
        system("pause");
        WSACleanup();
        exit(15);
    }

    //cout<<"ACCEPT CONNECTION..."<<endl;

    acceptSocket = accept( myBackup, NULL, NULL );
    while ( acceptSocket == SOCKET_ERROR )
    {
        acceptSocket = accept( myBackup, NULL, NULL );
    }
    mySocket = acceptSocket;
}

void ServerSocket::Bind( int port )
{
    myAddress.sin_family = AF_INET;
    myAddress.sin_addr.s_addr = inet_addr( "1.2.3.4" );
    myAddress.sin_port = htons( port );

    //cout<<"BIND TO PORT "<<port<<endl;

    if ( bind ( mySocket, (SOCKADDR*) &myAddress, sizeof( myAddress) ) == SOCKET_ERROR )
    {
        cerr<<"ServerSocket: Failed to connect\n";
        system("pause");
        WSACleanup();
        exit(14);
    }
}


void ClientSocket::ConnectToServer( const char *ipAddress, int port )
{
    myAddress.sin_family = AF_INET;
    myAddress.sin_addr.s_addr = inet_addr( ipAddress );
    myAddress.sin_port = htons( port );

    //cout<<"CONNECTED"<<endl;

    if ( connect( mySocket, (SOCKADDR*) &myAddress, sizeof( myAddress ) ) == SOCKET_ERROR )
    {
        cerr<<"ClientSocket: Failed to connect\n";
        system("pause");
        WSACleanup();
        exit(13);
    }
}

//main SMTP function
DWORD WINAPI receive_cmds(LPVOID lpParam)
{
  printf("thread created\r\n");
  DateTimeMsg dtm;           //used to store date info and possibly rcpt and sender
  SOCKET header;
  ghMutex = CreateMutex(NULL, FALSE, NULL);
  WaitForSingleObject(ghMutex, INFINITE);
  vector<string> point;      //vector used to split the input of the user
  int            num;        //for split function
  string         delimeter;  //for split function

  // set our socket to the socket passed in as a parameter
  SOCKET current_client = (SOCKET)lpParam;

  // buffer to hold our recived data
  char   rcvbuf[STRLEN];

  // buffer to hold our sent data
  char   sendData[STRLEN];

  // for error checking
  int    res;

   cout << "Client socket:"<< (int)current_client << endl;

   //Send the welcome message:   "220 smtp.1234.com
   hostent* localHost;
   char* localIp;
   localHost = gethostbyname("");
   localIp   = inet_ntoa (*(struct in_addr *)*localHost->h_addr_list);
   cout << "HOST IP : " << localIp << endl;

   char* twotwenty      = "220 smtp.1234.com ESMTP Postfix";
   char* bye            = "221 Bye";
   char* twoFifty       = "250 Ok";
   char* threeFiftyFour = "354 End Data with <CR><LF>.<CR><LF>";
   char* fourTwentyOne  = "421 Syntax Error (VRFY <username>)";
   char* loginError     = "500";

   send(current_client, twotwenty, strlen(twotwenty), 0);  //SEND 220 (SMTP server address)
   cout << "S -> " << twotwenty << endl;


   //clear the recieve buffer prior to receiving
   memset(rcvbuf, '\0', 100);

   //receive the message EXPECTING A HELO AND THE NAME OF THE CLIENT DOMAIN ex: relay.example.org
   res = recv(current_client,rcvbuf,sizeof(rcvbuf),0);

     //what did I receive
      if(res == -1) {
          cout << "ERROR RECEIVING DATA FROM CLIENT\n";
      }

     cout << "C -> " << rcvbuf << endl;
     delimeter = " ";
     split (&point, rcvbuf, delimeter, num );         //split up client response to store their name
     const char* clientName = point.at(1).c_str();    //relay.smptZ.org

// *************LOGIN INFORMATION*************
     char sendTwoFifty[STRLEN];
     strcat (sendTwoFifty, "250 ");
     strcat (sendTwoFifty, clientName);
     strcat (sendTwoFifty, ", I am glad to meet you (guest account: \"guest@\")");
     send(current_client, sendTwoFifty, strlen(sendTwoFifty), 0);   //SEND 250 glad to meet you

     cout << "S -> " <<sendTwoFifty << endl;
     memset(rcvbuf, '\0', 100);

     res=recv(current_client, rcvbuf,sizeof(rcvbuf), 0);    //expecting to receive login information: VRFY _______ <- login name

     if(res == -1) {
          cout << "ERROR RECEIVING LOGIN DATA FROM CLIENT\n";
          return 0;
     }
     char* tmpMsg = rcvbuf;
     cout << "C -> "  << rcvbuf << endl;
     point.clear();
     string login;

     bool done = true;
     while (done)
     {
         split(&point, rcvbuf, delimeter, num);                 //make sure the user sent VRFY

         if (strncmp(point.at(0).c_str(), "VRFY", 4) != 0) {    //rcvbuf should start with VRFY for user login
              send (current_client, fourTwentyOne, strlen(fourTwentyOne), 0);//send the user an error message
              closesocket(current_client);
              done = false;
         }

         const char* cmp = point.at(1).c_str(); //cmp should be the login name (name@ip)
         cout << "LOGIN NAME: " << cmp << endl;
         if ((strncmp(cmp, "jschoenfeld@",21)==0)){
              cout << "S -> " << twoFifty << endl;
              send (current_client, twoFifty, strlen(twoFifty), 0);
              login = cmp;
              done = false;
              //cout <<"Successful Login by " << cmp << endl;
         }
         else if (strncmp(cmp, "zabd@", 13)==0) {
              cout << "S -> " << twoFifty << endl;
              send (current_client, twoFifty, strlen(twoFifty), 0);
              login = cmp;
              done = false;
              //cout <<"Successful Login by " << cmp << endl;
         }
         else if (strncmp(cmp, "guest@", 14)==0) {
              cout << "S -> " << twoFifty << endl;
              send (current_client, twoFifty, strlen(twoFifty), 0);
              login = cmp;
              done = false;
              //cout <<"Successful Login by " << cmp << endl;
         } else {
             send (current_client, loginError, sizeof(loginError),0);
             cout << "S -> " << loginError << endl;
             memset(rcvbuf, '\0', 100);
             res=recv(current_client, rcvbuf, sizeof(rcvbuf), 0);   //recv VRFY <______>
             cout << "C -> " << rcvbuf << endl;
         }
        point.clear();     //On repeat, make sure the vector is empty
     }
//*************END LOGIN SESSION**********************

    int i = 0;
    while (i < 100)
    {
        res = recv(current_client, rcvbuf, sizeof(rcvbuf), 0); // recv cmds
        //what did I receive
        printf("Thread %d ", GetCurrentThreadId());
        cout << "Received: " << rcvbuf << "\n";

        if (res != 0)
        {
            if (strncmp (rcvbuf, "QUIT", 4) == 0) {
                memset(rcvbuf, 0, 100);
                printf("Thread %d received quit cmd\n", GetCurrentThreadId());
                strcpy(sendData, "received quit cmd\nGoodbye\n");
                send(current_client, sendData, strlen(sendData), 0);
                printf("Thread %d Client Disconnect\nGoodbye\n", GetCurrentThreadId());
                break;
            }
            if (strncmp (rcvbuf, "INBOX", 5) == 0) {
                string filename = login;
                filename += ".txt";
                cout << "FILENAME IS: " << filename << endl;
                ifstream in(filename.c_str());
                string message;
                while (in) {
                    message.push_back(in.get());
                }
                in.close();
                cout << "E-mail inbox is: \n" << message << endl; 
                message = "\n" + message;  
                send(current_client, message.c_str(), strlen(message.c_str()), 0); 

            }
            if(strncmp(rcvbuf, "MESSAGE", 7) == 0)
            {
                 recv(current_client, rcvbuf, sizeof(rcvbuf), 0);
                 cout << "stuff" << rcvbuf << endl;
                 cout << "WE ARE DOING MESSAGE!" << endl;
                 const char* sub = rcvbuf;
                 char* mailFrom;
                 string finalMailFrom;
                 if (strncmp (sub, "MAIL FROM", 9)==0)
                 {
                     mailFrom = rcvbuf + 11;       //extract "MAIL FROM<"
                     int num = strlen(mailFrom);
                     mailFrom[num - 1] = '\0';     //extract the ">" from the end
                 }
                
                 finalMailFrom = mailFrom;      //used to store header information in a STRING

                  if(res == -1){
                      cout << "ERROR RECEIVING \"MAIL FROM\" FROM CLIENT\n";
                      return 0;
                 }

                 send (current_client, twoFifty, sizeof(twoFifty),0);   //Send 250 OK for successfull MAIL FROM:
                 cout << "S -> " << twoFifty << endl;
                 //FINISHED RECEIVING MAIL FROM

                 memset(rcvbuf, '\0', 100);
                 res = recv(current_client,rcvbuf,sizeof(rcvbuf),0);   // recv RCPT TO:<____>
                 cout << "C -> " << rcvbuf << endl; //recv MAIL FROM<ADDRESS>
                 if(res == -1){
                      cout << "ERROR RECEIVING \"RCPT TO\" FROM CLIENT\n";
                      return 0;
                 }


                 const char* sub2 = rcvbuf;
                 char* rcpt;
                 string finalRcpt;
                 if (strncmp (sub2, "RCPT TO", 7)==0){
                     cout << "We received a destination mailing address\n";
                     rcpt = rcvbuf + 9;            //extract "MAIL FROM<"
                     num = strlen(rcpt);
                     rcpt[num - 1] = '\0';         //extract the ">" from the end
                     cout << "RCPT TO: ----> " << rcpt << endl;
                 }

                finalRcpt=rcpt;                    //used to store header information
                send (current_client, twoFifty, sizeof(twoFifty), 0);
                cout << "S -> " << twoFifty << endl;
                memset(rcvbuf, '\0', 100);         //rcpt variable will be reset after this if set to "1000"
                cout <<"\nRCPT TO OUTSIDE after memset stuff: " << rcpt << endl<<endl;

                //START RECEIVING DATA FROM CLIENT
                res = recv (current_client, rcvbuf, sizeof(rcvbuf), 0);    //RECV DATA (deletes rcpt variable)
                cout << "C -> " << rcvbuf << endl;
                if(res == -1){
                    cout << "ERROR RECEIVING \"DATA\" FROM CLIENT\n";
                    return 0;
                }

                if (strncmp (sub2, "DATA", 4)==0){
                    send (current_client, threeFiftyFour, strlen(threeFiftyFour), 0);   //send the 354 for DATA
                    cout << "S -> " << threeFiftyFour << endl;
                }

            //START RECEIVING THE MESSAGE FROM THE CLIENT
                bool endMessage = true;
                string date;
                string hst;
                string msg;
                //WE NEED TO ADD A HEADER TO THE EMAIL
                char theMessage[STRLEN];

                //First Line: From: ________
                strcat(theMessage, "To: ");
                strcat(theMessage, finalRcpt.c_str());
                strcat(theMessage, "\n");

                //Second Line: TO: ______
                strcat(theMessage, "From: ");
                strcat(theMessage, finalMailFrom.c_str());
                strcat(theMessage, "\n");

                //THE DATE: ______
                time_t rawtime;
                time (&rawtime);
                time_t localtime;
                char * pt = ctime(&rawtime);
                date = date.append(pt, 4, 7);
                dtm.currentDate = date.append(pt, 20, 4); //length of 4 will delete the newline at the end of the output (length 5 will include the newline)
                dtm.currentTime = hst.append(pt, 10, 9);
                strcat(theMessage, "Date: ");
                strcat(theMessage, dtm.currentDate.c_str());
                strcat(theMessage, dtm.currentTime.c_str());
                strcat(theMessage, "\n");
                while(endMessage)
                {
                    memset(rcvbuf, '\0', 100);

                    res = recv(current_client, rcvbuf, sizeof(rcvbuf), 0);
                    if (res == -1) {
                        cout <<"ERROR RECEIVING THE MESSAGE FROM CLIENT\n" << endl;

                    }
                    cout << "C -> " << rcvbuf << endl;
                    strcat(theMessage, rcvbuf);
                    strcat(theMessage, "\n");
                    if (strncmp(rcvbuf, ".", 1) ==0) {
                        send (current_client, twoFifty, sizeof(twoFifty),0);
                        cout << "S -> " << twoFifty << endl;
                        cout <<"\n\n" << theMessage << endl;
                        endMessage = false;
                        break;
                    }
                }
                cout <<"FINAL MSG: " << theMessage << endl;
                cout <<"FINAL RCPT: \"" << finalRcpt << "\"" << endl;

                res = recv(current_client, rcvbuf, sizeof(rcvbuf), 0);
                cout << "C -> " << rcvbuf << endl;
                
                string temp;
                ifstream fin;
                ofstream fout;
                fin.open("usernames.txt");


                while (true)
                {
                    getline(fin, temp);
                    if(finalRcpt == temp)
                    {
                        cout << "Found a match for a local user: " << temp <<  endl;
                        /* Email is intended for this server */
                        
                        fout.open((finalRcpt + ".txt").c_str(), ios::out | ios::app);
                        if (fout.is_open())
                        {
                            fout << theMessage << endl;
                            fout.close();
                        }
                        else
                        {
                            cout << "A file IO error has occured for thread ID: " << GetCurrentThreadId() << endl;
                        }
                        break;
                    }
                    else if (fin.eof())
                    {
                        
                        cout << "User is not on this server, forwarding..." << endl;
                        
                        sockaddr_in clientAddress;

                                                
                        int port = 31000;
                        WSADATA wsaData;
                        
                        //Checking for errors
                        if (WSAStartup(MAKEWORD(2, 2), &wsaData ) != NO_ERROR)
                        {
                            cerr << "Socket Initialization: Error with WSAStartup\n";
                            system("pause");
                            WSACleanup();
                            exit(10);
                        }
                        
                        SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                        
                        if (client == INVALID_SOCKET)
                        {
                            cerr << "Socket Initialization: Error creating socket"<<endl;
                            system("pause");
                            WSACleanup();
                            exit(11);
                        }
                        
                        //put IP address we can forward to in here
                        ifstream getips;
                        getips.open("ips.txt");
                        
                        string ip;
                        
                        getline(getips, ip);
                        
                        clientAddress.sin_family = AF_INET;
                        clientAddress.sin_addr.s_addr = inet_addr(ip.c_str());
                        clientAddress.sin_port = htons(port);
                    
                        if (connect(client, (sockaddr *) &clientAddress, sizeof(clientAddress)) == SOCKET_ERROR)
                        {
                            cerr<<" 421: Service Not Available\n";
                            //system("pause");
                            WSACleanup();
                            //exit(13);
                        }
                        
                        char option[256];
                        char response[256];
                        
                        recv(client, response, sizeof(response), 0);
                        cout << response << endl;
                        
                        memset(response, 0, 100);
                    
                    
                        //send command
                        strcpy (option, "guest@");
                        send(client, option, strlen(option), 0);
                        recv(client, response, sizeof(response), 0);
                        cout << response << endl;
                        
                        memset(response, 0, 100);
                        
                        send(client, finalRcpt.c_str(), finalRcpt.size(), 0);
                        recv(client, response, sizeof(response), 0);
                        cout << response << endl;
                        
                        memset(response, 0, 100);
                        
                        send(client, finalMailFrom.c_str(), finalMailFrom.size(), 0);
                        recv(client, response, sizeof(response), 0);
                        cout << response << endl;
                        
                        memset(response, 0, 100);
                        
                        int n = 0;
                        int i = 100;

                        string totalMsg = theMessage;
                        
                        for(i; i < totalMsg.size(); i += 100) {
                            send(client, (totalMsg.substr(n, i)).c_str(), 100, 0);
                            recv(client, response, sizeof(response), 0);
                            cout << response << endl;
                            
                            memset(response, 0, 100);
                            n = i;
                        }
                        
                       send(client, (totalMsg.substr(n)).c_str(), totalMsg.size() - n, 0);
                        recv(client, response, sizeof(response), 0);
                        cout << response << endl;
                        memset(response, 0, 100);
                        
                        send(client, ".", 1, 0);
                        recv(client, response, sizeof(response), 0);
                        cout << response << endl;
                        memset(response, 0, 100);
                        
                        send(client, "QUIT", 4, 0);
                        recv(client, response, sizeof(response), 0);
                        cout << response << endl;
                        memset(response, 0, 100);
                        
                        getips.close();   
                        break; 
                    }
                }
            fin.close();
            }
        }
    i++;
    }
}


int main()
{
    printf("Starting up multi-threaded SMTP server\n");

     // our masterSocket(socket that listens for connections)
     SOCKET sock;
     // for our thread
     DWORD thread;
     WSADATA wsaData;
     sockaddr_in server;


     // start winsock
     int ret = WSAStartup(0x101,&wsaData); // use highest version of winsock avalible

     if(ret != 0) {
        return 0;
     }

     // create our socket
     sock=socket(AF_INET,SOCK_STREAM,0);
     if(sock == INVALID_SOCKET) {
        cout << "INVALID SOCKET\n";
        return 0;
     }

      // fill in winsock struct ...
     server.sin_family=AF_INET;
     server.sin_addr.s_addr=INADDR_ANY;
     server.sin_port=htons(31000); // listen on telnet port 31000

     // bind our socket to a port(port 31000)
     if( bind(sock,(sockaddr*)&server,sizeof(server)) == SOCKET_ERROR ) {
        cout << "BIND FUNC FAIL\n";
        return 0;
     }

     // listen for a connection
     if(listen(sock,5) != 0) {
        cout << "LISTEN FUNCTON FAILED";
        return 0;
     }

     // socket that we sendrecv data on
     SOCKET client;
     sockaddr_in from;
     int fromlen = sizeof(from);

     // loop forever
     //char* welcome = "Welcome to ";
     //char *ip;
     bool done = true;
     while(done)    //accept connectiong from multiple clients
     {
      //we are already listening, let's wait for connections
      client = accept(sock,(struct sockaddr*)&from,&fromlen);
      printf("Client connected\r\n");
       cout << "client before thread:" << (int) client << endl;
      // create our recv_cmds thread and pass client socket as a parameter
      CreateThread(NULL, 0, receive_cmds,(LPVOID)client, 0, &thread); // from the thread, we will handle multiple connections and go through the SMTP process
      cout << "\nIN MAIN SLEEPING";
      Sleep(1000);
     }

     WSACleanup();

 // exit
 cout <<"END OF SMTP PROGRAM\n";
 return 0;


} //end main
