#ifndef _UTILS_H
#define _UTILS_H

#include <bits/stdc++.h>
#include <string.h>

using namespace std;

// Descrie un abonament.
struct Subscription {
    string topic;
    int SF;

    Subscription(string topic, int SF) {
        this->topic = topic;
        this->SF = SF;
    }
};

// Descrie un client.
struct Client {
    int socket;
    string ID;
    bool connected;
    vector<Subscription> subscriptions;  // abonamente
    vector<string> message_buffer;  // mesaje neprimite

    Client(int socket, string ID, bool connected,
            vector<Subscription> subscriptions) {
        this->socket = socket;
        this->ID = ID;
        this->connected = connected;
        this->subscriptions = subscriptions;
    }
};

// Abonare la topic.
void subscribe(Client &client, string topic, string sf_string) {
    int already_here = 0;
    int len = sf_string.length();
    vector<Subscription> subs = client.subscriptions;
    char sf[len + 1];
    strcpy(sf, sf_string.c_str());
    // Daca este abonat deja la acest topic, actualizeaza SF.
    for (int i = 0; i < subs.size(); i++) {
        if (subs[i].topic == topic) {
            already_here++;
            subs[i].SF = atoi(sf);
            break;
        }
    }
    client.subscriptions = subs;
    if (already_here == 0) {
        Subscription new_sub(topic, atoi(sf));
        client.subscriptions.push_back(new_sub);
    }
}

// Dezabonare.
void unsubscribe(Client &client, string topic) {
    for (int j = 0; j < client.subscriptions.size(); j++) {
        if (client.subscriptions[j].topic == topic) {
            client.subscriptions.erase(client.subscriptions.begin() + j);
        }
    }
}

// Construire mesaj primit de la publisher (in payload), in functie de
// tipul de date. Buffer este sirul intreg.
void build_message(int type, char *buffer, string &payload,
                    string &type_as_string) {
    if (type == 0) {
        type_as_string = "INT";
        string abs_val;
        uint32_t number;
        // Numarul fara semn
        memcpy(&number, buffer + 52, 4);
        number = ntohl(number);
        abs_val = to_string(number);
        // Adaugare semn
        if (buffer[51] == 1) {
            payload = "-";
        }
        payload.append(abs_val);
    } else {
        if (type == 1) {
            type_as_string = "SHORT_REAL";
            uint16_t number;
            // Numarul intreg.
            memcpy(&number, buffer + 51, 2);
            char res[1500];
            // Cu virgula.
            sprintf(res, "%.2f", ((float)ntohs(number) / 100));
            string res_string(res);
            payload = res;
        } else {
            if (type == 2) {
                // Asemanator cu INT si SHORT_REAL.
                type_as_string = "FLOAT";
                uint32_t number;
                memcpy(&number, buffer + 52, 4);
                number = ntohl(number);
                char res[1500];
                sprintf(res, "%f", ((float)number / pow(10, (int8_t)buffer[56])));
                string abs_val(res);
                if (buffer[51] == 1) {
                    payload = "-";
                }
                payload.append(abs_val);
            } else {
                if (type == 3) {
                    // String simplu.
                    type_as_string = "STRING";
                    char res[1500];
                    strncpy(res, buffer + 51, 1500);
                    string res_string(res);
                    payload = res;
                } else {
                    cout << "Type can be 0 - 3\n";
                }
            }
        }
    }
}

void init(struct sockaddr_in &serv_addr, struct sockaddr_in &cli_addr,
        int portno) {
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = INADDR_ANY;
    cli_addr.sin_port = htons(portno);
}

void usage(char *file) {
    fprintf(stderr, "Usage: %s server_port\n", file);
    exit(0);
}

#endif  // _UTILS_H
