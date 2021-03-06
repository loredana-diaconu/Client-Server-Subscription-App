#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <bits/stdc++.h>
#include <string.h>
#include <iostream>
#include "helpers.h"
#include "utils.h"
#define FD_START 4

using namespace std;

int main(int argc, char *argv[]) {
    int tcp_sockfd, newsockfd, portno, udp_sockfd;
    char buffer[BUFLEN];
    struct sockaddr_in serv_addr, cli_addr;
    int n, i, ret;
    int ok = 0;
    socklen_t clilen;
    vector<Client> clients;

    // Verificare argumente.
    if (argc < 2) {
        usage(argv[0]);
    }

	portno = atoi(argv[1]);

    int flag_delay = 1;
    int tcp_no_delay = setsockopt(tcp_sockfd, IPPROTO_TCP, TCP_NODELAY,
                                &flag_delay, sizeof(int));

    fd_set read_fds;  // multimea de citire folosita in select()
    fd_set tmp_fds;  // multime folosita temporar
    int fdmax;  // valoare maxima fd din multimea read_fds

    // Se goleste multimea de descriptori de citire si multimea temporara
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(tcp_sockfd < 0, "tcp socket fail");
    udp_sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    DIE(udp_sockfd < 0, "udp socket fail");

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    init(serv_addr, cli_addr, portno);

    ret = bind(tcp_sockfd, (struct sockaddr *) &serv_addr,
            sizeof(struct sockaddr));
    DIE(ret < 0, "bind fail tcp");
    ret = bind(udp_sockfd, (struct sockaddr *) &cli_addr,
            sizeof(struct sockaddr));
    DIE(ret < 0, "bind fail udp");
    ret = listen(tcp_sockfd, MAX_CLIENTS);
    DIE(ret < 0, "listen fail");

    // se adauga socketii pe care se asculta conexiuni in multimea read_fds
    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(tcp_sockfd, &read_fds);
    FD_SET(udp_sockfd, &read_fds);
    fdmax = max(tcp_sockfd, udp_sockfd);

    while (ok == 0) {
        tmp_fds = read_fds;
        ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(ret < 0, "select");

        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &tmp_fds)) {
                if (!i) {
                    // Citire stdin
                    fgets(buffer, BUFLEN - 1, stdin);
                    if (!strcmp(buffer, "exit\n")) {
                        ok = 1;
                        break;
                    } else {
                        printf("The only accepted command is exit.\n");
                    }
                } else {
                      if (i == udp_sockfd) {
                        // Receptionare mesaje de la publisher
                        memset(buffer, 0, BUFLEN);
                          n = recv(i, buffer, BUFLEN, 0);
                          DIE(n < 0, "recv");

                        // Topic
                        char topic_array[BUFLEN];
                          memset(topic_array, 0, BUFLEN);
                          strncpy(topic_array, buffer, 49);
                        string topic(topic_array);

                        // Tip
                          int type;
                          type = (int8_t)buffer[50];

                          // Mesaj
                        string payload;
                        string type_as_string;
                        // Construieste mesajul ce va fi trimis, in functie
						// de tip.
                        build_message(type, buffer, payload, type_as_string);
                        string message(payload);
                        stringstream ss;
                          ss << inet_ntoa(cli_addr.sin_addr) << ":" <<
                                  htons(cli_addr.sin_port);
                          string print_result = ss.str();
                        // Stringul afisat in subscriber
                        print_result.append(" - ").append(topic).append(" - ");
                        print_result.append(type_as_string).append(" - ");
                        print_result.append(message);
                        int size = print_result.size();

                        // Cautam clientul abonat la topic.
                        for (int j = 0; j < clients.size(); j++) {
                            vector<Subscription> s = clients[j].subscriptions;
                            for (Subscription sub : s) {
                                if (topic == sub.topic) {
                                    if (clients[j].connected) {
                                        // Daca este conectat, trimite;
                                        n = send(clients[j].socket,
												print_result.c_str(), size, 0);
                                    } else {
                                        if (sub.SF == 1) {
                                            // Daca nu este conectat,
											// dar SF = 1, retine in buffer.
                                            clients[j].message_buffer.push_back(print_result);
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        if (i == tcp_sockfd) {
                            // Cerere de conexiune pe socketul activ
                            clilen = sizeof(cli_addr);
                            newsockfd = accept(tcp_sockfd,
										(struct sockaddr *) &cli_addr, &clilen);
                            DIE(newsockfd < 0, "accept");
                            // Se adauga noul socket intors de accept() la
                            // multimea descriptorilor de citire
                            FD_SET(newsockfd, &read_fds);
                            if (newsockfd > fdmax) {
                                fdmax = newsockfd;
                            }
                            // Primire ID
                            memset(buffer, 0, BUFLEN);
                              n = recv(newsockfd, buffer, BUFLEN, 0);
                             DIE(n < 0, "tcp recv fail");
                            string id(buffer);
                            int sameID = 0;
                            for (int j = 0; j < clients.size(); j++) {
                                if (clients[j].ID == id) {
                                    // Daca ID-ul exista deja
                                    sameID++;
                                    if (!clients[j].connected) {
                                        // Daca clientul este inactiv,
										// reconectare
                                        clients[j].connected = true;
                                        clients[j].socket = newsockfd;
                                        printf("Client %s connected from %s:%d\n", buffer,
                                                inet_ntoa(cli_addr.sin_addr),
												ntohs(cli_addr.sin_port));
                                        for (int k = 0; k < clients[j].message_buffer.size(); k++) {
                                            // Primire mesaje din buffer.
                                            string to_send = clients[j].message_buffer[k];
                                            if (k != clients[j].message_buffer.size() - 1) {
                                                to_send.append("\n");
                                            }
                                            n = send(clients[j].socket, to_send.c_str(),
                                                    to_send.size(), 0);
                                        }
                                        clients[j].message_buffer.clear();
                                    } else {
                                        // Exista deja un client cu acest ID
                                        string already_connected("ID already in use");
                                        n = send(newsockfd, already_connected.c_str(),
                                                already_connected.size(), 0);
                                        cout << "Client disconnected.\n";
                                        close(newsockfd);
                                        FD_CLR(newsockfd, &read_fds);
                                    }
                                    break;
                                }
                            }
                            if (sameID == 0) {
                                // Altfel, se creeaza un client nou.
                                printf("New client %s connected from %s:%d\n", buffer,
                                        inet_ntoa(cli_addr.sin_addr),
										ntohs(cli_addr.sin_port));
                                vector<Subscription> subs;
                                Client c(newsockfd, buffer, true, subs);
                                clients.push_back(c);
                            } else {
                                continue;
                            }
                        } else {
                            // Primire mesaje de la client.
                            memset(buffer, 0, BUFLEN);
                              n = recv(i, buffer, sizeof(buffer), 0);
                              DIE(n < 0, "recv");
                            int k;
                            // Gasire client dupa socket.
                            for (int j = 0; j < clients.size(); j++) {
                                if (clients[j].socket == i) {
                                    k = j;
                                }
                            }
                            if (n) {
                                // Comanda subscribe sau unsubscribe.
                                vector<string> command;
                                string word;
                                stringstream X(buffer);
                                while (getline(X, word, ' ')) {
                                    command.push_back(word);
                                   }
                                if (command[0] == "subscribe") {
                                    subscribe(clients[k], command[1],
											command[2]);
                                } else {
                                    unsubscribe(clients[k], command[1]);
                                }
                            } else {
                                // Inchidere conexiune.
                                cout << "Client " << clients[k].ID <<
										" disconnected.\n";
                                clients[k].connected = false;
                                close(i);
                                FD_CLR(i, &read_fds);
                            }
                        }
                    }
                }
            }
        }
    }

    close(tcp_sockfd);
    close(udp_sockfd);
    close(newsockfd);
    return 0;
}
