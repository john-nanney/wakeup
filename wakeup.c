
/*
 * This software is public domain, and has no express or implied warranty.
 */

/*
 * Compile:
 * gcc -Wall wakeup.c -o wakeup
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define SYSTEM_SUCCESS (0)
#define NO_SOCKET (-1)

/* Wakeup packet format:
 * 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF (6 bytes of all ones)
 * MAC Address (repeated 16 times)
 */

struct wakeup_packet_t
{
	uint8_t marker[6];
	uint8_t address[16][6];
};

struct addrinfo *res = NULL;
struct wakeup_packet_t wakeup_packet;

void Usage(void)
{
	printf("\nUsage: Wakeup MAC_ADDRESS\n");
	printf("Sends a Wake On LAN message to the specified MAC addres.\n\n");
	exit(EXIT_FAILURE);
}

unsigned int parse_address(uint8_t *mac, const char *str)
{
	int x;
	long val;
	const char *ptr = str;
	char *endptr = NULL;

	for(x = 0; x < 6; ++x) {
		errno = 0;
		val = strtol(ptr, &endptr, 16);
		if(errno) {
			return(0);
		}

		mac[x] = val;
		ptr = endptr + 1;
	}

	return(1);
}

int udp_socket(const char *service)
{
	int s = NO_SOCKET;
	struct addrinfo hints;
	int sysresult;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = (AI_V4MAPPED | AI_ALL);

	while((sysresult = getaddrinfo("0.0.0.0", service, &hints, &res)) == EAI_AGAIN) {
		/* Keep trying to get socket address */
	}

	if(sysresult == SYSTEM_SUCCESS) {
		if((s = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == NO_SOCKET) {
			printf("socket() returned %s\n", strerror(errno));
		}

	} else {
		printf("getaddrinfo() returned %s\n", gai_strerror(sysresult));
	}

	return(s);
}

unsigned int send_wakeup(void)
{
	unsigned int rv = 0;
	int sock;

	if((sock = udp_socket("9")) == NO_SOCKET) {
		printf("Can't create UDP socket.\n");
		return(rv);
	}

	if(sendto(sock, &wakeup_packet, sizeof(wakeup_packet), 0, res->ai_addr, res->ai_addrlen) == sizeof(wakeup_packet)) {
		rv = 1;
	} else {
		printf("Could not send packet: %s\n", strerror(errno));
	}

	freeaddrinfo(res);

	shutdown(sock, SHUT_RDWR);
	close(sock);

	return(rv);
}

int main(int argc, char **argv)
{
	int x;
	int y;

	if(argc != 2 || strlen(argv[1]) != 17) {
		Usage();
	}

	memset(&wakeup_packet, 0, sizeof(wakeup_packet));
	memset(wakeup_packet.marker, 0xff, 6);

	if(!parse_address(wakeup_packet.address[0], argv[1])) {
		printf("Cannot figure out the MAC address on the command line.\n");
		Usage();
	}

	for(x = 1; x < 16; ++x) {
		for(y = 0; y < 6; ++y) {
			wakeup_packet.address[x][y] = wakeup_packet.address[0][y];
		}
	}

	if(!send_wakeup()) {
		printf("Cannot send wakeup message to %02x:%02x:%02x:%02x:%02x:%02x\n",
				wakeup_packet.address[0][0],
				wakeup_packet.address[0][1],
				wakeup_packet.address[0][2],
				wakeup_packet.address[0][3],
				wakeup_packet.address[0][4],
				wakeup_packet.address[0][5]);
		exit(EXIT_FAILURE);
	} else {
		printf("Sent wakeup message to %02x:%02x:%02x:%02x:%02x:%02x\n",
				wakeup_packet.address[0][0],
				wakeup_packet.address[0][1],
				wakeup_packet.address[0][2],
				wakeup_packet.address[0][3],
				wakeup_packet.address[0][4],
				wakeup_packet.address[0][5]);
	}



	exit(EXIT_SUCCESS);
}
