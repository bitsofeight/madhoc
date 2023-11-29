#ifndef PACKETCAP_H
#define PACKETCAP_H

#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <fcntl.h>

#include "utils.h"
#include "RT.h"

#define SP_TYPE_IP   1
#define SP_TYPE_ARP  2
#define SP_TYPE_ICMP 3

#define MAXLINE   4096
#define READ         0
#define WRITE        1

struct scanpac
{
  unsigned char type;
  u_int32_t ip;
};

int packetcaptureinit(char *interface);

int packetcapture(int maxpackage);

#endif















