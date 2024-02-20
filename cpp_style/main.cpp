//
// Created by bont on 24. 2. 19.
//

#include "Dispatcher.h"

#include <cstdio>
#include <cstdlib>
#include <netinet/ip.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h> // read(), write(), close()
#include <signal.h>

#include <ctime>
#include <event.h> // libevent
#include <iostream>
#include <sstream>


#define MAX 80
#define DEFAULT_PORT 9990
#define TRUE 1
#define FALSE 0
#define SA struct sockaddr
#define LISTENQEUE 1024


void err_sys(const char *s) {
    perror(s);
    exit(0);
}

void err_quit(const char *s) {
    printf("ERROR: %s\n", s);
    exit(0);
}


int Socket(int family, int protocol, int type) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_sys("socket error");
    }
    return sockfd;
}

void Bind(int socketfd, const struct sockaddr *serveraddr, int socklen) {
    if (bind(socketfd, serveraddr, socklen) != 0) {
        err_sys("Binding failed!\n");
    }
}

void Connect(int socketfd, const struct sockaddr *serveraddr, int socketlen) {
    if (connect(socketfd, serveraddr, socketlen) < 0) {
        err_sys("connect error");
    }
}

struct conn
{
    int fd;
    struct event ev_read;
    struct sockaddr_in client_addr;
};


void on_read_cb(int fd, short events, void *arg)
{

    struct conn *c = (struct conn *)arg;
    char rbuff[8196];
    bzero(rbuff, sizeof(rbuff));

    // read data from FD, valdiate if it's a closed connection.
    if (read(fd, rbuff, sizeof(rbuff)) == 0)
    {
        printf("Client [%s:%d] disconnected\n",
               inet_ntoa(c->client_addr.sin_addr), (int)ntohs(c->client_addr.sin_port));
        // Connection is closed, no need to watch again
        event_del(&c->ev_read);
        close(fd);
        free(c);
        return;
    }
    printf("Client [%s:%d] wrote: %s",
           inet_ntoa(c->client_addr.sin_addr), (int)ntohs(c->client_addr.sin_port), rbuff);

    // write back to client.
    char wbuff[80];
    bzero(wbuff, sizeof(wbuff));
    time_t ticks = time(NULL);
    snprintf(wbuff, sizeof(wbuff),
             "Hello from server - %.24s\r\n", ctime(&ticks));
    printf("Client [%s:%d] Server replied: %s",
           inet_ntoa(c->client_addr.sin_addr), (int)ntohs(c->client_addr.sin_port), wbuff);
    write(fd, wbuff, sizeof(wbuff));


}

class Connection {
public:
    Connection() = default;

    int fd_;
    event ev_read_;
    sockaddr_in client_addr_;
    FileEventPtr fileEventPtr_;
};

using connPtr = std::shared_ptr<Connection>;

int main() {
    std::unordered_map<std::string, connPtr> conn_map;

    // 1. Setup server
    int port = 9990;
    int reuseaddr_on = 1;
    int server_fd;
    struct sockaddr_in serveraddr;
    server_fd = Socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(server_fd,
                   SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on)) == -1) {
        err_sys("setsockopt failed");
    }
    printf("Server socket - [%d] created successfully..\n", server_fd);
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(server_fd, (SA *) &serveraddr, sizeof(serveraddr));

    if ((listen(server_fd, LISTENQEUE)) != 0)
        err_sys("Listen failed...\n");


    Dispatcher dispatcher = Dispatcher();
    auto server_event = dispatcher.createFileEvent(server_fd,
                                                   [&](uint32_t) {
                                                       int client_fd;
                                                       struct sockaddr_in client_addr;
                                                       socklen_t client_len = sizeof(client_addr);

                                                       /* Accept the new connection. accept() returns a client socket */
                                                       client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                                                       if (client_fd < 0)
                                                       {
                                                           printf("Failed to connect to client\n");
                                                           return;
                                                       }

                                                       /** Now, handle the connection by adding it to libevent for read */
                                                       connPtr conn = std::make_shared<Connection>();
                                                       conn->fd_ = client_fd;
                                                       conn->client_addr_ = client_addr; // just add it for fun


                                                       auto event = dispatcher.createFileEvent(client_fd,
                                                                                               [connection = conn, &conn_map, &dispatcher](uint32_t events) {
                                                                                                   char rbuff[8196];
                                                                                                   bzero(rbuff, sizeof(rbuff));

                                                                                                   // read data from FD, valdiate if it's a closed connection.
                                                                                                   if (read(connection->fd_, rbuff, sizeof(rbuff)) == 0)
                                                                                                   {
                                                                                                       printf("Client [%s:%d] disconnected\n",
                                                                                                              inet_ntoa(connection->client_addr_.sin_addr),
                                                                                                              (int)ntohs(connection->client_addr_.sin_port));
                                                                                                       // Connection is closed, no need to watch again
                                                                                                       auto remote_ip = inet_ntoa(connection->client_addr_.sin_addr);
                                                                                                       auto remote_port = (int)ntohs(connection->client_addr_.sin_port);
                                                                                                       std::stringstream ss;
                                                                                                       ss << remote_ip << "_" << remote_port;
                                                                                                       auto find_conn = conn_map.find(ss.str());

                                                                                                       find_conn->second->fileEventPtr_->cancelEvent();
                                                                                                       close(connection->fd_);

                                                                                                       conn_map.erase(find_conn);
                                                                                                       event_base_dump_events(&dispatcher.base(), stdout);

                                                                                                       return;
                                                                                                   }
                                                                                                   printf("Client [%s:%d] wrote: %s",
                                                                                                          inet_ntoa(connection->client_addr_.sin_addr),
                                                                                                          (int)ntohs(connection->client_addr_.sin_port),
                                                                                                          rbuff);

                                                                                                   // write back to client.
                                                                                                   char wbuff[80];
                                                                                                   bzero(wbuff, sizeof(wbuff));
                                                                                                   time_t ticks = time(NULL);
                                                                                                      snprintf(wbuff, sizeof(wbuff),
                                                                                                               "Hello from server - %.24s\r\n", ctime(&ticks));
                                                                                                   printf("Client [%s:%d] Server replied: %s",
                                                                                                          inet_ntoa(connection->client_addr_.sin_addr),
                                                                                                          (int)ntohs(connection->client_addr_.sin_port),
                                                                                                          wbuff);
                                                                                                   write(connection->fd_, wbuff, sizeof(wbuff));

                                                                                               },
                                                                                               FileTriggerType::Level,
                                                                                               FileReadyType::Read);


                                                       conn->fileEventPtr_ = std::move(event);

                                                       auto remote_ip = inet_ntoa(conn->client_addr_.sin_addr);
                                                       auto remote_port = (int)ntohs(conn->client_addr_.sin_port);

                                                       std::stringstream ss;
                                                       ss << remote_ip << "_" << remote_port;
                                                       conn_map.insert({ss.str(), conn});


                                                       printf("Client [%s:%d] connected\n",
                                                              inet_ntoa(client_addr.sin_addr), (int)ntohs(client_addr.sin_port));


                                                       event_base_dump_events(&dispatcher.base(), stdout);
                                                   },
                                                   FileTriggerType::Level,
                                                   FileReadyType::Read);

    //server_event->setEnabled(FileReadyType::Read);


    dispatcher.dispatch_loop();

    // Clean it up
    close(server_fd);
    event_base_free(&dispatcher.base());

    return 0;
}