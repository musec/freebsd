#include "ipc_utils.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <execinfo.h>
#include <unistd.h>


int send_fd(int sock, int fd)
{
    int nonce = rand();

    struct msghdr msg;
    struct iovec io_vector[1];
    struct cmsghdr *cmsg = NULL;
    
    unsigned char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];
    int available_ancillary_element_buffer_space;

    io_vector[0].iov_base = &nonce;
    io_vector[0].iov_len = sizeof(nonce);

    /* initialize sock message */
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = io_vector;
    msg.msg_iovlen = 1;

    /* provide space for the ancillary data */
    available_ancillary_element_buffer_space = CMSG_SPACE(sizeof(int));
    memset(ancillary_element_buffer, 0, available_ancillary_element_buffer_space);
    msg.msg_control = ancillary_element_buffer;
    msg.msg_controllen = available_ancillary_element_buffer_space;

    /* initialize a single ancillary data element for fd passing */
    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    //*((int *) CMSG_DATA(cmsg)) = fd;
    memcpy(CMSG_DATA(cmsg), &fd, sizeof(fd));

    sendmsg(sock, &msg, 0);
    return nonce;
}


int recv_fd(int sock, int* nonce)
 {
    int sent_fd;
    struct msghdr msg;
    struct iovec io_vector[1];
    struct cmsghdr *cmsg = NULL;
    int message_buffer;
    char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];

    /* start clean */
    memset(&msg, 0, sizeof(struct msghdr));
    memset(ancillary_element_buffer, 0, CMSG_SPACE(sizeof(int)));

    /* setup a place to fill in message contents */
    io_vector[0].iov_base = &message_buffer;
    io_vector[0].iov_len = sizeof(message_buffer);
    msg.msg_iov = io_vector;
    msg.msg_iovlen = 1;

    /* provide space for the ancillary data */
    msg.msg_control = ancillary_element_buffer;
    msg.msg_controllen = CMSG_SPACE(sizeof(int));

    if(recvmsg(sock, &msg, MSG_CMSG_CLOEXEC) < 0)
        return -1;
    *nonce = message_buffer;

    if((msg.msg_flags & MSG_CTRUNC) == MSG_CTRUNC)
    {
        /* we did not provide enough space for the ancillary element array */
        return -1;
    }

    /* iterate ancillary elements */
    for(cmsg = CMSG_FIRSTHDR(&msg);
        cmsg != NULL;
        cmsg = CMSG_NXTHDR(&msg, cmsg))
    {
        if( (cmsg->cmsg_level == SOL_SOCKET) &&
            (cmsg->cmsg_type == SCM_RIGHTS) )
        {
            memcpy(&sent_fd, CMSG_DATA(cmsg), sizeof(int));
            //sent_fd = *((int *) CMSG_DATA(cmsg));
            return sent_fd;
        }
    }

    return -1;
 }
