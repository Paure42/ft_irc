/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tbajrami <tbajrami@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/13 14:12:23 by tbajrami          #+#    #+#             */
/*   Updated: 2021/05/18 17:19:37 by tbajrami         ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

void Server::setFds(Fds *fds) {_fds = fds;}

std::map<int, Client> Server::getFDClients(void) const {
    return (_fd_clients);
}
std::map<std::string, Client> Server::getNickClients(void) const {
    return (_nick_clients);
}

void Server::setFDClients(int i, Client cl) {
    _fd_clients[i] = cl;
}

void Server::setNickClients(std::string s, Client cl) {
    _nick_clients[s] = cl;
}


Server::Server(Params *pm)
{
    _network = new std::vector<sockaddr_in>;
    if (pm->isnew())
        new_serv(pm);
    else
        connect_serv(pm);
}

//=====================CREATION AND CONNECTION OF THE SERVER============================

void Server::new_serv(Params *pm)
{
    int yes = 1;
    getIP();

    if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        std::cout << SOCKET_ERROR << std::endl;
        exit(1);
    }
    if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        std::cout << SETSOCK_ERROR << std::endl;;
        exit(1);
    }
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(pm->getPort());
    strcpy(_password, pm->getPwd());
    ft_bzero(&(_addr.sin_zero), 8);
    if(bind(listener, (struct sockaddr *)&_addr, sizeof(_addr)) == -1)
    {
        perror(BIND_ERROR);
        exit(1);
    }
    if(listen(listener, 10) == -1)
    {
        perror(LISTEN_ERROR);
        exit(1);
    }
}

void Server::connect_serv(Params *pm)
{
    int yes = 1;
    if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        std::cout << SOCKET_ERROR << std::endl;
        exit(1);
    }
    if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        std::cout << SETSOCK_ERROR << std::endl;
        exit(1);
    }
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(pm->getPort());
    strcpy(_password, pm->getPwd());
    ft_bzero(&(_addr.sin_zero), 8);
    if(bind(listener, (struct sockaddr *)&_addr, sizeof(_addr)) == -1)
    {
        perror(BIND_ERROR);
        exit(1);
    }
    if(listen(listener, 10) == -1)
    {
        perror(LISTEN_ERROR);
        exit(1);
    }
    do_connect(pm);
}

void Server::do_connect(Params *pm)
{
    int net_socket = socket(AF_INET, SOCK_STREAM, 0);
    char message[1024];

    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(pm->getPortNetwork());
    server_address.sin_addr.s_addr = INADDR_ANY;
    this->_network->push_back(server_address);

    int connection_status = connect(net_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    if (connection_status == -1)
        std::cout << "Error making connection to the remote socket\n\n";
    send(net_socket, "SERVER", 6, 0);
}

//========================================================================================



int Server::addclient(Server &serv,  int listener)
{
    Client nc;

    nc.is_server = false;
    nc.is_register = false;
    int addrlen = sizeof(nc.clientaddr);
    if((nc.clfd = accept(listener, (struct sockaddr *)&nc.clientaddr, &nc.addrlen)) == -1)
    {
		std::cout << "Server-accept() error\n";
        return (-1);
    }
    else
        std::cout << "Server-accept() is OK...\n";
    std::cout << "New connection from " << inet_ntoa(nc.clientaddr.sin_addr);
    nc.host = inet_ntoa(nc.clientaddr.sin_addr);
    std::cout << " on socket " << nc.clfd << std::endl;
    serv.setFDClients(nc.clfd, nc);
    return (nc.clfd);
}

void Server::getIP()
{
    const char* google_dns_server = "8.8.8.8";
    int dns_port = 53;
    struct sockaddr_in serv;
    int sock = socket ( AF_INET, SOCK_DGRAM, 0);

    memset( &serv, 0, sizeof(serv) );
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr( google_dns_server );
    serv.sin_port = htons( dns_port );
    int err = connect( sock , (const struct sockaddr*) &serv , sizeof(serv) );
    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (struct sockaddr*) &name, &namelen);
    char buffer[100];
    const char* p = inet_ntoa(name.sin_addr);
    if(p != NULL)
    {
        std::cout << "Local ip is : " << p << "\n";
    }
    strcpy(_ip, p);
    _prefix[0] = ':';
    strcat(_prefix, _ip);
}




//==============================TREAT COMMANDS======================================

void Server::send_reply(std::string s, int fd, int code)
{
    std::string ccmd;
    if (code)
        ccmd = ft_format_cmd(ft_utoa(code));
    std::string to_send;

    to_send += (std::string(_prefix) + " " + ccmd + " " + msg_rpl(s, code, fd) + "\r\n");
    send(fd, to_send.c_str(), strlen(to_send.c_str()), 0);
}

void Server::chan_msg(Message * msg, int fd) {
    std::string s;
    size_t i = 1;

    s += ("<" + _fd_clients[fd].nickname + ">@[" + _fd_clients[fd].current_chan + "] : " += msg->command);
    while (i < msg->params.size())
    {
        s += (msg->params[i] + " ");
        i++;
    }
    s += "\r\n";
    send_reply_broad(_fd_clients[fd], _channels[_fd_clients[fd].current_chan], -1, s);
}

void Server::do_command(Message *msg, int fd)
{
    std::string tmp(msg->command);
    // std::cout << "{" << _fd_clients[fd].nickname << "} says : " << msg->command << std::endl;

    if (tmp == "PASS") {
        passcmd(msg, fd);
    }
    else if (tmp == "NICK")
    {
        if (_fd_clients[fd].password != _password)
            send_reply("", fd, ERR_PASSWDMISMATCH);
        else
            nickcmd(msg, fd);
    }
    else if (tmp == "USER" ) {
        if (_fd_clients[fd].password != _password)
            send_reply("", fd, ERR_PASSWDMISMATCH);
        else if (_fd_clients[fd].nickname[0] == 0)
            send_reply("", fd, ERR_NONICKNAMEGIVEN);
        else
            usercmd(msg, fd);
    }
    else if (_fd_clients[fd].is_register == true)
    {
        if (tmp == "JOIN")
            joincmd(msg, fd);
        else if (tmp == "PRIVMSG")
            privmsgcmd(msg, fd);
        else if (tmp == "NOTICE")
            noticecmd(msg, fd);
        else if (_fd_clients[fd].current_chan.empty() == false)
            chan_msg(msg, fd);
        else
            send_reply(msg->command, fd, ERR_NOTOCHANNEL);
    }
    else
        send_reply("", fd, ERR_NOTREGISTERED);
    delete msg;
}

void Server::send_reply_broad(Client &sender, std::vector<Client> &cl, int code, std::string s)
{
    for (size_t i = 0; i < cl.size(); i++)
    {
        if (cl[i].clfd != sender.clfd)
        {
            if (code != -1)
                send_reply("", cl[i].clfd, code);
            else
                send_reply(s, cl[i].clfd, 0);
        }
    }
}

void Server::privmsgcmd(Message *msg, int fd)
{
    // ERR_NORECIPIENT --NO                ERR_NOTEXTTOSEND -- Yes
    // ERR_CANNOTSENDTOCHAN --No            ERR_NOTOPLEVEL -- No
    // ERR_WILDTOPLEVEL --No                ERR_TOOMANYTARGETS --No
    // ERR_NOSUCHNICK --Yes
    //  RPL_AWAY --No
    size_t i = 0;
    std::list<Client> list;
    std::vector<Client> vec;
    std::string s;
    std::string tmp_current_chan;
    Message text;
    while (i < msg->params.size() && msg->params[i] != ":")
        i++;
    i++;
    while (i < msg->params.size())
    {
        s += msg->params[i];
        text.params.push_back(msg->params[i]);
        i++;
    }
    if (s.size() == 0)
    {
        send_reply("", fd, ERR_NOTEXTTOSEND);
        return;
    }
    i = 0;
    while (i < msg->params.size() && msg->params[i][0] != ':')
    {
        if (msg->params[i] !=  "," && msg->params[i] != " ")
        {
            if (_nick_clients.count(msg->params[i]) == 0)
            {
                if (_channels.count(msg->params[i]) == 0)
                    send_reply(msg->params[i], fd, ERR_NOSUCHNICK);
                else
                {
                    tmp_current_chan = _fd_clients[fd].current_chan;
                    _fd_clients[fd].current_chan = msg->params[i];
                    chan_msg(&text, fd);
                    _fd_clients[fd].current_chan = tmp_current_chan;
                }
            }
            else
               list.push_back(_nick_clients[msg->params[i]]);
        }
        i++;
    }
    list.sort();
    list.unique();
    vec.assign(list.begin(), list.end());
    s.insert(0, std::string("<" + _fd_clients[fd].nickname + ">"));
    send_reply_broad(_fd_clients[fd], vec, -1, s);
}

void Server::noticecmd(Message *msg, int fd)
{
    size_t i = 0;
    std::list<Client> list;
    std::vector<Client> vec;
    std::string s;
    std::string tmp_current_chan;
    while (i < msg->params.size() && msg->params[i] != ":")
        i++;
    i++;
    while (i < msg->params.size())
    {
        s += msg->params[i];
        i++;
    }
    if (s.size() == 0)
        return;
    i = 0;
    while (i < msg->params.size() && msg->params[i][0] != ':')
    {
        if (msg->params[i] !=  "," && msg->params[i] != " ")
        {
            if (_nick_clients.count(msg->params[i]) != 0)
               list.push_back(_nick_clients[msg->params[i]]);
        }
        i++;
    }
    list.sort();
    list.unique();
    vec.assign(list.begin(), list.end());
    s.insert(0, std::string("<" + _fd_clients[fd].nickname + ">"));
    send_reply_broad(_fd_clients[fd], vec, -1, s);
}

//===============================================================================
