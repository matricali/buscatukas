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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stringlist.h"

stringlist_t *stringlist_load_file(char *filename)
{
	FILE *fp;
	stringlist_t *ret;
	ssize_t read;
	char *temp = 0;
	size_t len;

	/* Initialize wordlist */
	ret = malloc(sizeof(stringlist_t));
	if (ret == NULL) {
		fprintf(stderr, "malloc()\n");
		exit(EXIT_FAILURE);
	}
	ret->size = 0;
	ret->elements = NULL;

	/* Open file */
	fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Error opening file. (%s)\n", filename);
		return NULL;
	}

	/* Get number of elements */
	int no_elements = 0;
	while ((read = getline(&temp, &len, fp)) != -1)
		++no_elements;

	ret->elements = calloc(sizeof(temp), no_elements);

	rewind(fp);
	for (int i = 0; i < no_elements; i++) {
		read = getline(&temp, &len, fp);
		strtok(temp, "\n");
		*(ret->elements + i) = strdup(temp);
		++ret->size;
	}
	fclose(fp);

	if (temp != NULL) {
		free(temp);
	}

	return ret;
}

void stringlist_destroy(stringlist_t *stringlist)
{
	for (int i = 0; i < stringlist->size; ++i) {
		free(stringlist->elements[i]);
	}
	free(stringlist->elements);
}
