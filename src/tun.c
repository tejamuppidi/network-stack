#include "tun.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <linux/if_tun.h>
#include <linux/if.h>

static void tun_configure(char *dev)
{
    int sockfd;
    struct ifreq ifr;
    struct sockaddr_in *addr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /*
     * Assign IPv4 address:
     * 10.0.0.1
     */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, dev, IFNAMSIZ - 1);

    addr = (struct sockaddr_in *)&ifr.ifr_addr;

    addr->sin_family = AF_INET;

    if (inet_pton(
            AF_INET,
            "10.0.0.1",
            &addr->sin_addr) != 1)
    {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (ioctl(sockfd, SIOCSIFADDR, &ifr) < 0)
    {
        perror("SIOCSIFADDR");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    /*
     * Configure netmask:
     * 255.255.255.0 (/24)
     */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, dev, IFNAMSIZ - 1);

    addr = (struct sockaddr_in *)&ifr.ifr_netmask;

    addr->sin_family = AF_INET;

    if (inet_pton(
            AF_INET,
            "255.255.255.0",
            &addr->sin_addr) != 1)
    {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) < 0)
    {
        perror("SIOCSIFNETMASK");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    /*
     * Bring interface UP.
     */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, dev, IFNAMSIZ - 1);

    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        perror("SIOCGIFFLAGS");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    ifr.ifr_flags |= IFF_UP;

    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
    {
        perror("SIOCSIFFLAGS");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    close(sockfd);

    printf(
        "[TUN] Interface configured: %s\n",
        dev
    );

    printf(
        "[TUN] Address: 10.0.0.1/24\n"
    );

    printf(
        "[TUN] Interface is UP\n"
    );
}


int tun_alloc(char *dev)
{
    struct ifreq ifr;
    int fd;

    fd = open("/dev/net/tun", O_RDWR);

    if (fd < 0)
    {
        perror("open(/dev/net/tun)");
        exit(EXIT_FAILURE);
    }

    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    if (*dev)
    {
        strncpy(
            ifr.ifr_name,
            dev,
            IFNAMSIZ - 1
        );
    }

    if (ioctl(fd, TUNSETIFF, &ifr) < 0)
    {
        perror("ioctl(TUNSETIFF)");
        close(fd);
        exit(EXIT_FAILURE);
    }

    strcpy(dev, ifr.ifr_name);

    printf(
        "[TUN] Created/attached to %s\n",
        dev
    );

    /*
     * Automatically configure
     * the TUN interface.
     */
    tun_configure(dev);

    return fd;
}
