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

#ifndef __BUSCATUKAS_HTTP_H
#define __BUSCATUKAS_HTTP_H

#include "stringlist.h"

typedef struct {
	uint32_t server_addr;
	unsigned int server_port;
	stringlist_t *paths;
} http_worker_args_t;

int http_head(uint32_t serverAddr, unsigned int serverPort, unsigned int tm,
	      char *path);

void http_worker(void *ptr);

#endif /* __BUSCATUKAS_HTTP_H */
