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

#include "threadpool.h"

static threadpool_work_t *threadpool_work_create(thread_func_t func, void *arg)
{
	threadpool_work_t *work;

	if (func == NULL)
		return NULL;

	work = malloc(sizeof(*work));
	work->func = func;
	work->arg = arg;
	work->next = NULL;
	return work;
}

static void threadpool_work_destroy(threadpool_work_t *work)
{
	if (work == NULL)
		return;
	free(work);
}

static threadpool_work_t *threadpool_work_get(threadpool_t *tm)
{
	threadpool_work_t *work;

	if (tm == NULL)
		return NULL;

	work = tm->work_first;
	if (work == NULL)
		return NULL;

	if (work->next == NULL) {
		tm->work_first = NULL;
		tm->work_last = NULL;
	} else {
		tm->work_first = work->next;
	}

	return work;
}

static void *threadpool_worker(void *arg)
{
	threadpool_t *tm = arg;
	threadpool_work_t *work;

	for (;;) {
		pthread_mutex_lock(&(tm->work_mutex));
		if (tm->stop)
			break;

		if (tm->work_first == NULL)
			pthread_cond_wait(&(tm->work_cond), &(tm->work_mutex));

		work = threadpool_work_get(tm);
		tm->working_cnt++;
		pthread_mutex_unlock(&(tm->work_mutex));

		if (work != NULL) {
			work->func(work->arg);
			threadpool_work_destroy(work);
		}

		pthread_mutex_lock(&(tm->work_mutex));
		tm->working_cnt--;
		if (!tm->stop && tm->working_cnt == 0 && tm->work_first == NULL)
			pthread_cond_signal(&(tm->working_cond));
		pthread_mutex_unlock(&(tm->work_mutex));
	}

	tm->thread_cnt--;
	pthread_cond_signal(&(tm->working_cond));
	pthread_mutex_unlock(&(tm->work_mutex));
	return NULL;
}

threadpool_t *threadpool_create(size_t num)
{
	threadpool_t *tm;
	pthread_t thread;
	size_t i;

	if (num == 0)
		num = 2;

	tm = calloc(1, sizeof(*tm));
	tm->thread_cnt = num;

	pthread_mutex_init(&(tm->work_mutex), NULL);
	pthread_cond_init(&(tm->work_cond), NULL);
	pthread_cond_init(&(tm->working_cond), NULL);

	tm->work_first = NULL;
	tm->work_last = NULL;

	for (i = 0; i < num; i++) {
		pthread_create(&thread, NULL, threadpool_worker, tm);
		pthread_detach(thread);
	}

	return tm;
}

void threadpool_destroy(threadpool_t *tm)
{
	threadpool_work_t *work;
	threadpool_work_t *work2;

	if (tm == NULL)
		return;

	pthread_mutex_lock(&(tm->work_mutex));
	work = tm->work_first;
	while (work != NULL) {
		work2 = work->next;
		threadpool_work_destroy(work);
		work = work2;
	}
	tm->stop = true;
	pthread_cond_broadcast(&(tm->work_cond));
	pthread_mutex_unlock(&(tm->work_mutex));

	threadpool_wait(tm);

	pthread_mutex_destroy(&(tm->work_mutex));
	pthread_cond_destroy(&(tm->work_cond));
	pthread_cond_destroy(&(tm->working_cond));

	free(tm);
}

bool threadpool_add_work(threadpool_t *tm, thread_func_t func, void *arg)
{
	threadpool_work_t *work;

	if (tm == NULL)
		return false;

	work = threadpool_work_create(func, arg);
	if (work == NULL)
		return false;

	pthread_mutex_lock(&(tm->work_mutex));
	if (tm->work_first == NULL) {
		tm->work_first = work;
		tm->work_last = tm->work_first;
	} else {
		tm->work_last->next = work;
		tm->work_last = work;
	}

	pthread_cond_broadcast(&(tm->work_cond));
	pthread_mutex_unlock(&(tm->work_mutex));

	return true;
}

void threadpool_wait(threadpool_t *tm)
{
	if (tm == NULL)
		return;

	pthread_mutex_lock(&(tm->work_mutex));
	for (;;) {
		if ((!tm->stop && tm->working_cnt != 0) ||
		    (tm->stop && tm->thread_cnt != 0)) {
			pthread_cond_wait(&(tm->working_cond),
					  &(tm->work_mutex));
		} else {
			break;
		}
	}
	pthread_mutex_unlock(&(tm->work_mutex));
}
