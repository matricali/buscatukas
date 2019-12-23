/*
Copyright (c) 2014-2018 Jorge Matricali

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "http.h"
#include "scanner.h"
#include "stringlist.h"

#ifndef BUF_SIZE
#define BUF_SIZE 256
#endif

int http_head(uint32_t serverAddr, unsigned int serverPort, unsigned int tm,
	      char *path)
{
	struct sockaddr_in addr;
	int sockfd, ret;
	fd_set fdset;

	sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sockfd < 0)
		return SCANNER_SCAN_ECREATING_SOCKET;

	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(serverPort);
	addr.sin_addr.s_addr = htonl(serverAddr);

	ret = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));

	FD_ZERO(&fdset);
	FD_SET(sockfd, &fdset);

	/* Connection timeout */
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = tm;

	if (select(sockfd + 1, NULL, &fdset, NULL, &tv) == 1) {
		int so_error;
		socklen_t len = sizeof so_error;

		getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
		if (so_error != 0) {
			close(sockfd);
			sockfd = 0;
			return SCANNER_SCAN_ECONNECTION_REFUSED;
		}
	} else {
		close(sockfd);
		sockfd = 0;
		return SCANNER_SCAN_ETIMEOUT;
	}

	/* Set to blocking mode again... */
	if ((ret = fcntl(sockfd, F_GETFL, NULL)) < 0) {
		close(sockfd);
		sockfd = 0;
		return SCANNER_SCAN_EFCNTL;
	}

	long arg = 0;
	arg &= (~O_NONBLOCK);

	if ((ret = fcntl(sockfd, F_SETFL, arg)) < 0) {
		close(sockfd);
		sockfd = 0;
		return SCANNER_SCAN_EFCNTL;
	}

	char sendline[BUF_SIZE + 1], recvline[BUF_SIZE + 1];
	size_t n;

	/* Build request */
	snprintf(sendline, BUF_SIZE,
		 "GET %s HTTP/1.1\r\n"
		 "Host: %d.%d.%d.%d\r\n"
		 "User-Agent: Mozilla/5.0 Jorgee\r\n"
		 "Accept: */*\r\n"
		 "\r\n",
		 path, (serverAddr >> 24) & 0xFF, (serverAddr >> 16) & 0xFF,
		 (serverAddr >> 8) & 0xFF, serverAddr & 0xFF);

	/* Write the request */
	if (write(sockfd, sendline, strlen(sendline)) >= 0) {
		/* Read the response */
		while ((n = read(sockfd, recvline, BUF_SIZE)) > 0) {
			recvline[n] = '\0';

			if (strncmp(recvline, "HTTP/1.1 200", 12) == 0 ||
			    strncmp(recvline, "HTTP/1.0 200", 12) == 0) {
				close(sockfd);
				sockfd = 0;
				return 0;
			}
		}
	}

	close(sockfd);
	sockfd = 0;

	return -1;
}

void http_worker(void *ptr)
{
	http_worker_args_t *args = ptr;

	stringlist_t *paths = args->paths;
	uint32_t ip = args->server_addr;
	unsigned int port = args->server_port;

	for (int n = 0; n < paths->size; ++n) {
		if (http_head(ip, port, 500000, paths->elements[n]) == 0) {
			printf("%d.%d.%d.%d - %s - OK\n", (ip >> 24) & 0xFF,
			       (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF,
			       paths->elements[n]);
		}
	}
}
