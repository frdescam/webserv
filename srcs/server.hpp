#ifndef _SERVER_HPP_ 
#define _SERVER_HPP_
#include "color.hpp"
# include <signal.h>    // pour le signal Ctrl-C
#include <sys/types.h>	// 
#include <sys/epoll.h>	// for epoll_create1(), epoll_ctl(), struct epoll_event 
#include <sys/socket.h>	// pour socket()
#include "struct_webserv.hpp"

class server
{
#define MAX_EVENTS 1000
#define BUF_LEN 1000000

private:
// https://stackoverflow.com/questions/6946217/how-to-access-the-contents-of-a-vector-from-a-pointer-to-the-vector-in-c
    std::vector<server_info> _s;
    struct_epoll _epoll;
 
    char str[BUF_LEN];

public:
    server(std::string);
    ~server();
    void display_server(void); // affiche les variables d(es) server(s) actuels
    void initialize(void); // creer bind et ajoute à epoll le(s) socket(s) de(s) server(s)
    void run(void); // lance les étapes de configuration des sockets initialize(), de run

    /* PRIVATE */
    size_t is_new_client(int fd);
    int get_time_out(int id_serv);
};

#endif