#ifndef _IPC_UTILS_H
#define _IPC_UTILS_H

int send_fd( int sock, int fd );
int recv_fd( int sock, int* nonce );

#endif
