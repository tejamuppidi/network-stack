#ifndef TUN_H
#define TUN_H

#include <linux/if.h>

#define BUFFER_SIZE 2048

int tun_alloc(char *dev);

#endif
