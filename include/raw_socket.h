#ifndef RAW_SOCKET_IF_H
#define RAW_SOCKET_IF_H
void socket_init(char *if_name, int protocol);
int socket_read(char *buf, int len);
int socket_write(char *buf, int len);
void free_socket();
#endif