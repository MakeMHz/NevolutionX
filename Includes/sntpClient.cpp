#include <stdint.h>
#include <atomic>
#include <string>
#include <time.h>
#include <config.hpp>

#ifdef NXDK
#include <lwip/opt.h>
#include <lwip/arch.h>
#include <lwip/netdb.h>
#include <lwip/errno.h>
#include <lwip/sockets.h>
#include <lwip/debug.h>
#include <lwip/dhcp.h>
#include <lwip/init.h>
#include <lwip/netif.h>
#include <lwip/sys.h>
#include <lwip/tcpip.h>
#include <lwip/timeouts.h>
#include <netif/etharp.h>
#include <pktdrv.h>
#include <xboxkrnl/xboxkrnl.h>
#include <xboxkrnl/xboxdef.h>
#include <outputLine.h>
#else
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#define outputLine(...) printf(__VA_ARGS__)
#endif

static const uint64_t FROM1600TO1900 = (uint64_t)((3600*24)*(uint64_t)(300*365 + 72));

static const uint64_t FACTOR100NSTO1S = 10000000;

#pragma pack(1)
typedef union tsData {
  struct u {
    uint32_t seconds;
    uint32_t fractions;
  } u;
  uint64_t full = 0;
} tsData;

// * = only significant in server messages
typedef struct timeData {
  union header {
    struct u{
      uint32_t mode:3;
      uint32_t vn:3;
      uint32_t li:2;
      uint32_t stratum:8; // *
      uint32_t poll:8; // *
      int32_t precision:8; // *
    } u;
    uint32_t full = 0;
  } header;
  uint32_t root_delay = 0; // *
  uint32_t root_dispersion = 0; // *
  uint32_t ref_id = 0; // *
  tsData ref_ts;
  tsData orig_ts;
  tsData rx_ts;
  tsData tx_ts;
} timeData;
#pragma pack()

#ifdef NXDK
static
LARGE_INTEGER timeStamp_to_sysTime(tsData const& ts) {
  LARGE_INTEGER ret;
  ret.QuadPart = ((uint64_t)ts.u.seconds + FROM1600TO1900) * FACTOR100NSTO1S;
  return ret;
}
#endif

int sntp_getTime(sntpConfig* conf) {
  if (conf->getAddress().length() <= 3) {
    outputLine("WARN: SNTP: Server address too short (%ld) - exiting early.\n",
      conf->getAddress().length());
    return 1;
  }

  timeData t;
  t.header.u.vn = 4;
  t.header.u.mode = 3;
  t.header.u.precision = -23;
  t.tx_ts.u.seconds = htonl(time(NULL) + 2208988800);

  struct addrinfo hints, *ai, *p;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  int sockfd;

  if (getaddrinfo(conf->getAddress().c_str(), std::to_string(conf->getPort()).c_str(), &hints, &ai) != 0) {
    outputLine("ERROR: SNTP getaddrinfo() failed.\n");
    return 2;
  }

  for (p = ai; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
      continue;
    }

    break;
  }
  if (p == nullptr) {
    outputLine("ERROR: SNTP: Failed to create socket.\n");
    return 5;
  }

  // Send request to SNTP server
  if (sendto(sockfd, &t, sizeof(t), 0, p->ai_addr, p->ai_addrlen) != sizeof(t)) {
    outputLine("ERROR: SNTP: Nothing sent.\n");
    return 8;
  }

  // Grab the response
  if (recvfrom(sockfd, &t, sizeof(t), 0, p->ai_addr,  &p->ai_addrlen) == -1) {
    outputLine("ERROR: SNTP: Nothing received.\n");
    return 9;
  }

#ifdef NXDK
  LARGE_INTEGER sysTime = timeStamp_to_sysTime(t.rx_ts);
  NtSetSystemTime(&sysTime, NULL);
#else
  outputLine("Timestamp received!\n");
  std::cout << t.rx_ts.full << std::endl;
#endif
  close(sockfd);

  return 0;
}
