#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include <string.h>
#include <iostream>
#include "helpers.h"

using namespace std;

void usage(char *file) {
    fprintf(stderr, "Usage: %s <ID_CLIENT> <IP_Server> <Port_Server>\n", file);
    exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd, n, ret;
    struct sockaddr_in serv_addr;
    char arr[BUFLEN];
    string buffer;
    fd_set read_fds;  // multimea de citire folosita in select()
    fd_set tmp_fds;  // multime folosita temporar
    int fdmax;  // valoare maxima fd din multimea read_fds

    // Verificare argumente.
    if (argc < 4) {
        usage(argv[0]);
    }

    // Ne asiguram ca ID este suficient de scurt.
    if (strlen(argv[1]) >= 10) {
        fprintf(stderr, "Client ID must be shorter than 10 characters\n");
        exit(0);
    }

    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    // Initializare socket tcp.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));
    ret = inet_aton(argv[2], &serv_addr.sin_addr);
    DIE(ret == 0, "inet_aton");
    ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    DIE(ret < 0, "connect");

    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(sockfd, &read_fds);
    fdmax = sockfd;

    // Trimite ID catre server.
    n = send(sockfd, argv[1], strlen(argv[1]), 0);
    DIE(n < 0, "send");

    while (1) {
        tmp_fds = read_fds;
        ret = select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(ret < 0, "select");

        if (FD_ISSET(STDIN_FILENO, &tmp_fds)) {
            // Citire stdin.
            getline(cin, buffer);
            if (buffer == "exit") {
                break;
            }
            // Daca este alta comanda decat exit, desparte in cuvinte.
            vector<string> command;
            string word;
            stringstream X(buffer);
            while (getline(X, word, ' ')) {
                command.push_back(word);
            }
            // Verificare comanda.
            int ok = 0;
            if (command[0] == "subscribe") {
                if (command.size() != 3) {
                    cout << "Wrong number of arguments.\n";
                    ok++;
                } else {
                    if (command[1].length() > 50) {
                        cout << "Topic must be shorter than 50.\n";
                        ok++;
                    }
                    if (command[2] != "0" && command[2] != "1") {
                        cout << "SF can be either 0 or 1.\n";
                        ok++;
                    }
                }
            } else {
                if (command[0] == "unsubscribe") {
                    if (command.size() != 2) {
                        cout <<"Wrong number of arguments\n";
                        ok++;
                    } else {
                        if (command[1].length() > 50) {
                            cout << "Topic must be shorter than 50\n";
                            ok++;
                        }
                    }
                } else {
                    cout << "You can only subscribe, unsubscribe or exit\n";
                    ok++;
                }
            }
            // Trimite mesaj la server.
            if (ok == 0) {
                int len = buffer.length();
                char charArr[len + 1];
                strcpy(charArr, buffer.c_str());
                n = send(sockfd, charArr, strlen(charArr), 0);
                DIE(n < 0, "send");
            }
        }

        if (FD_ISSET(sockfd, &tmp_fds)) {
            // Primire mesaj de la server.
            memset(arr, 0, BUFLEN);
            n = recv(sockfd, arr, BUFLEN, 0);
            if (n == 0) {
                break;
            }
            DIE(n < 0, "recv");
            printf("%s\n", arr);
            string recv_message(arr);
            if (recv_message == "ID already in use") {
                break;
            }
        }
    }

    close(sockfd);
    return 0;
}
