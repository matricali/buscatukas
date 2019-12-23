/*
 * This file is part of the buscatukas distribution
 * (https://github.com/matricali/buscatukas).
 *
 * Copyright (c) 2019 Jorge Matricali.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCANNER_H
#define SCANNER_H

#include <pthread.h>

#include "cmwc.h"
#include "stringlist.h"
#include "threadpool.h"

typedef enum {
	SCANNER_SCAN_OK,
	SCANNER_SCAN_ECREATING_SOCKET,
	SCANNER_SCAN_ECONNECTION_REFUSED,
	SCANNER_SCAN_EFCNTL,
	SCANNER_SCAN_ETIMEOUT,
} scanner_scan_result_t;

typedef struct {
	pthread_mutex_t *lock;
	struct cmwc_state *cmwc;
	threadpool_t *pool;
	stringlist_t *paths;
} scanner_worker_args_t;

void *scanner_worker(void *ptr);

void scanner_start(int max_threads, stringlist_t *paths);

int scanner_scan_port(uint32_t serverAddr, unsigned int serverPort,
		      unsigned int tm);

#endif /* SCANNER_H */
