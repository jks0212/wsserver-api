#ifndef _WSSERVER_H
#define _WSSERVER_H

int wsaccept(int client_sock, char* recv_data);
int wssend(int client_sock, const char* send_data, int length);
int wsrecv(int client_sock, char* recv_data, int length);

int base64Encode(const unsigned char* buffer, size_t length, char** b64text);

#endif
