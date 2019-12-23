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

#include <getopt.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cmwc.h"
#include "progress.h"
#include "scanner.h"
#include "stringlist.h"

#define VERSION "0.1"

#ifndef MAX_THREADS
#define MAX_THREADS 64
#endif

void bt_print_banner()
{
	printf("buscatukas v%s - (https://github.com/matricali/buscatukas)\n",
	       VERSION);
}

void bt_usage(const char *p)
{
	printf("usage: %s\n", p);
}

int main(int argc, char **argv)
{
	int opt;
	int option_index = 0;

	static struct option long_options[] = {
		{ "version", no_argument, 0, 'v' },
		{ "help", no_argument, 0, 'h' },
		{ "port", required_argument, 0, 'p' },
		{ 0, 0, 0, 0 }
	};

	while ((opt = getopt_long(argc, argv, "vhp:", long_options,
				  &option_index)) != -1) {
		switch (opt) {
		case 'v':
			bt_print_banner();
			exit(EXIT_SUCCESS);
			break;

		case 'h':
			bt_print_banner();
			puts("");
			bt_usage(argv[0]);
			exit(EXIT_SUCCESS);
			break;

		case 'p':
			//port = atoi(optarg);
			break;

		case '?':
			/* getopt_long already printed an error message. */
			exit(EXIT_FAILURE);
			break;

		default:
			abort();
		}
	}

	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if (sigemptyset(&sa.sa_mask) == -1 ||
	    sigaction(SIGPIPE, &sa, 0) == -1) {
		perror("failed to ignore SIGPIPE; sigaction");
		exit(EXIT_FAILURE);
	}

	stringlist_t *paths = stringlist_load_file("paths.txt");

	scanner_start(MAX_THREADS, paths);

	stringlist_destroy(paths);

	exit(EXIT_SUCCESS);
}
