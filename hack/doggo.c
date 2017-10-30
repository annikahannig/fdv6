
/*
 * Doggo. Sniff for ipv6 echo requests.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <net/ethernet.h>


#define IS_IPV6(packet) (packet[0] >> 4 == 6)

// IPv6 Header Decoding
#define IPV6_TRAFFIC_CLASS(packet) ((uint8_t)((packet[0] << 4) | (packet[1] >> 4)))

#define IPV6_FLOW_LABEL(packet)  ((uint32_t)((packet[1] & 0x0f) << 16 | \
                                             (packet[2] << 8) | \
                                              packet[3]))

#define IPV6_PAYLOAD_LEN(packet) ((uint16_t)(packet[4] << 8 | packet[5]))

#define IPV6_NEXT_HEADER(packet) ((uint8_t)packet[6])
#define IPV6_HOP_LIMIT(packet)   ((uint8_t)packet[7])
#define IPV6_SRC_ADDR(packet)    ((struct in6_addr *)(packet + 8))
#define IPV6_DST_ADDR(packet)    ((struct in6_addr *)(packet + 8 + 16))

#define IS_ICMP6(packet) ((IPV6_NEXT_HEADER(packet) == 58))

#define IS_ICMP6_ECHO_REQUEST(packet) (packet[40] == 0x80)

/*
 * Setup capturing and return pcap descriptor
 */
pcap_t* init_pcap(const char* dev)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program filter;
    bpf_u_int32 net_addr;
    bpf_u_int32 net_mask;


    // Open capture dev
    pcap_t* cap = pcap_open_live(dev,
                                 BUFSIZ, 0, 50,
                                 errbuf);
    if (cap == NULL) {
        printf("Error while opening dev %s\n",
               errbuf);
        exit(-1);
    }

    // Get netaddr from dev
    if(pcap_lookupnet(dev, &net_addr, &net_mask, errbuf) == -1) {
        printf("Could not get net addr and mask from device: %s\n",
               errbuf);
        exit(-1);
    }

    // Setup filter
    if(pcap_compile(cap, &filter, "icmp6", 0, net_addr) == -1) {
        printf("Could not compile filter\n");
        exit(-1);
    }

    if(pcap_setfilter(cap, &filter) == -1 ) {
        printf("Could not set filter\n");
        exit(-1);
    }

    return cap;
}


void hexdump(const unsigned char *data, size_t len)
{
    for(size_t i = 0; i < len; i++) {
        printf("%x ", data[i]);
    }
    printf("\n");
}


void handle_packet(u_char* user,
                   const struct pcap_pkthdr* hdr,
                   const unsigned char* packet)
{
    // Decode packet:
    // Do we care much?
    // Nah. Let's just decode the header and assume it's an
    // ICMPv6 packet. (Hey we set a filter for icmp6, remember?)
    //
    // Since we receive packets from ANY device, libpcap omits
    // the ethernet frame and starts directly with the packet.

    if (IPV6_NEXT_HEADER(packet) != 58) {
        printf("ERROR: Received unexpected IPv6 NEXT HEADER (%x)\n",
               IPV6_NEXT_HEADER(packet));
    }

    if (!IS_ICMP6_ECHO_REQUEST(packet)) {
        return; // Nah.
    }

    struct in6_addr* src_addr = IPV6_SRC_ADDR(packet);
    struct in6_addr* dst_addr = IPV6_DST_ADDR(packet);


    char str_src[INET6_ADDRSTRLEN];
    char str_dst[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, src_addr, str_src, INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, dst_addr, str_dst, INET6_ADDRSTRLEN);

    printf("%s %s\n", str_src, str_dst);
    fflush(stdout);
}


int main(int argc, const char** argv)
{
    printf("Starting to capture on ANY device.");

    pcap_t *cap = init_pcap("any");
    pcap_loop(cap, -1, handle_packet, NULL);

    return 0;
}

