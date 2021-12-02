#ifndef SNIFFER_H
#define SNIFFER_H

#include <thread>
#include <csignal>
#include <pcap/pcap.h>

#include "../includes/timer.h"

#define SNAP_LEN 65535
#define SIZE_ETHERNET 14

std::string bytesToString(long long bytes);
std::string packetsToString(long long packets);

struct SniffIp
{
        u_char ip_vhl;
        u_char ip_tos;
        u_short ip_len = 0;
        u_short ip_id;
        u_short ip_off;
#define IP_RF 0x8000
#define IP_DF 0x4000
#define IP_MF 0x2000
#define IP_OFFMASK 0x1fff
        u_char ip_ttl;
        u_char ip_p;
        u_short ip_sum;
        struct in_addr ip_src, ip_dst;
};

class Sniffer
{
private:
        std::string device;

        pcap_t *handle;
        std::string filter_exp;
        struct bpf_program fp;
        bpf_u_int32 mask;
        bpf_u_int32 net;
        int num_packets;

        long long trafficAll;
        long long trafficPerSec;
        long long packetsAll;
        long long packetsPerSec;

public:
        Sniffer(const std::string &dev);
        ~Sniffer();

        void sniff(Timer &timer);
        void countTraffic(Timer &timer);

        std::string getDevice();
        long long getTrafficAll();
        long long getTrafficPerSec();
        long long getPacketsAll();
        long long getPacketsPerSec();
};

#endif