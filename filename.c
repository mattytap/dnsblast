#include <stdio.h>
#include <stdlib.h>

#include <poll.h>
#include <stddef.h>
#include <errno.h>

#define INFTIM -1

int main()
{
    struct pollfd *fds;
    int fdcount;
    int rv;

    /* Allocate and clear out fds */

    fdcount = 6;

    fds = calloc(fdcount, sizeof(struct pollfd));

    /* Assume that we are waiting for fds 4,6 to read,
     * fds 5, 8, 10 to be able to write successfully,
     * and fd 11 to have an exception condition.
     */

    fds[0].fd = 4;
    fds[0].events |= POLLIN;

    fds[1].fd = 6;
    fds[1].events |= POLLIN;

    fds[2].fd = 5;
    fds[2].events |= POLLOUT;

    fds[3].fd = 8;
    fds[3].events |= POLLOUT;

    fds[4].fd = 10;
    fds[4].events |= POLLOUT;

    fds[5].fd = 11;

    /* Call poll and tell it to wait indefinitely for some event */

    rv = poll(fds, fdcount, INFTIM);

    if (rv == -1)
    {
        printf("An error occurred: %d\n", errno);
        return errno;
    }

    printf("Events occurred: %d.\n", rv);

    printf("fd %d is ready to be read: %d\n", fds[0].fd, fds[0].revents & POLLIN);

    printf("fd %d is ready to be read: %d\n", fds[1].fd, fds[1].revents & POLLIN);

    printf("fd %d is ready to write to: %d\n", fds[2].fd, fds[2].revents & POLLOUT);

    printf("fd %d is ready to write to: %d\n", fds[3].fd, fds[3].revents & POLLOUT);

    printf("fd %d is ready to write to: %d\n", fds[4].fd, fds[4].revents & POLLOUT);

    printf("fd %d has a pending error: %d\n", fds[5].fd, fds[5].revents & POLLERR);

    return 0;
}