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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "cmwc.h"
#include "http.h"
#include "progress.h"
#include "scanner.h"
#include "threadpool.h"

pthread_mutex_t scanner_lock;

static uint32_t random_ip(struct cmwc_state *cmwc)
{
	uint32_t ul_dst;
	uint32_t random_num = cmwc_rand_ts(cmwc);

	ul_dst = (random_num >> 24 & 0xFF) << 24 |
		 (random_num >> 16 & 0xFF) << 16 |
		 (random_num >> 8 & 0xFF) << 8 | (random_num & 0xFF);

	return ul_dst;
}

void *scanner_worker(void *ptr)
{
	scanner_worker_args_t *args = (scanner_worker_args_t *)ptr;
	uint32_t ip;

	for (;;) {
		ip = random_ip(args->cmwc);

		if (scanner_scan_port(ip, 80, 500000) == SCANNER_SCAN_OK) {
			http_worker_args_t worker_args = {
				.server_addr = ip,
				.server_port = 80,
				.paths = args->paths,
			};

			threadpool_add_work(args->pool, http_worker,
					    &worker_args);
		}

		progress_print();
	}

	pthread_exit(NULL);
	return NULL;
}

void scanner_start(int max_threads, stringlist_t *paths)
{
	scanner_worker_args_t args;
	struct cmwc_state cmwc;
	pthread_t scan_threads[max_threads];
	int ret;

	memset(&args, 0, sizeof(scanner_worker_args_t));

	args.paths = paths;

	/* Initialize threadpool */
	threadpool_t *pool;
	pool = threadpool_create(max_threads);
	args.pool = pool;

	/* Initialize PRNG */
	unsigned int seed = time(NULL);
	cmwc_init(&cmwc, seed);
	args.cmwc = &cmwc;

	if (pthread_mutex_init(&scanner_lock, NULL) != 0) {
		fprintf(stderr, "mutex init has failed\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < max_threads; i++) {
		if ((ret = pthread_create(&scan_threads[i], NULL,
					  &scanner_worker, (void *)&args))) {
			fprintf(stderr, "Thread creation failed: %d\n", ret);
		}
	}

	for (int i = 0; i < max_threads; i++) {
		ret = pthread_join(scan_threads[i], NULL);
		if (ret != 0) {
			fprintf(stderr, "Cannot join thread no: %d\n", ret);
		}
	}

	threadpool_wait(pool);
	threadpool_destroy(pool);
}

int scanner_scan_port(uint32_t serverAddr, unsigned int serverPort,
		      unsigned int tm)
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

	close(sockfd);
	sockfd = 0;

	return SCANNER_SCAN_OK;
}
