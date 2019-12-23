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

#include <pthread.h>
#include <stdio.h>

const char PROGRESS_CHARS[4] = { '-', '/', '|', '\\' };
pthread_mutex_t lock;

void progress_print()
{
	static int p;

	pthread_mutex_lock(&lock);
	printf("\b%c[2K\r", 27);
	printf("%c", PROGRESS_CHARS[++p % 4]);
	printf("\r");
	fflush(stdout);
	pthread_mutex_unlock(&lock);
}
