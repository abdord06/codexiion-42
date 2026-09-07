/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   take_dongle.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abdo <abdo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/07 15:11:29 by aredouan          #+#    #+#             */
/*   Updated: 2026/09/07 22:25:36 by abdo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	queue_for_dongles(t_coder *c, t_dongle **d)
{
    t_request	req;

	if (c->left_dongle < c->right_dongle)
	{
		d[0] = c->left_dongle;
		d[1] = c->right_dongle;
	}
	else
	{
		d[0] = c->right_dongle;
		d[1] = c->left_dongle;
	}
	req = build_req(c);
	pthread_mutex_lock(&d[0]->mutex);
	heap_insert(d[0]->wait_queue, req);
	pthread_mutex_unlock(&d[0]->mutex);
	pthread_mutex_lock(&d[1]->mutex);
	heap_insert(d[1]->wait_queue, req);
	pthread_mutex_unlock(&d[1]->mutex);
}

static void	sleep_and_unlock(t_coder *c, t_dongle **d, long long wait_time)
{
	pthread_mutex_unlock(&d[1]->mutex);
	pthread_mutex_unlock(&d[0]->mutex);
	custom_sleep(wait_time, c->sim);
}

static void	wait_for_dongles(t_coder *c, t_dongle **d, int f_mine, int s_mine)
{
	long long	now;

	now = get_time();
	if (f_mine && now < d[0]->available_at)
		sleep_and_unlock(c, d, d[0]->available_at - now);
	else if (s_mine && now < d[1]->available_at)
		sleep_and_unlock(c, d, d[1]->available_at - now);
	else if (!f_mine)
	{
		pthread_mutex_unlock(&d[1]->mutex);
		pthread_cond_wait(&d[0]->cond, &d[0]->mutex);
		pthread_mutex_unlock(&d[0]->mutex);
	}
	else
	{
		pthread_mutex_unlock(&d[0]->mutex);
		pthread_cond_wait(&d[1]->cond, &d[1]->mutex);
		pthread_mutex_unlock(&d[1]->mutex);
	}
}

static	int	do_it(t_coder *c, t_dongle **d)
{
	heap_extract(d[0]->wait_queue);
	heap_extract(d[1]->wait_queue);
	d[0]->is_held = 1;
	d[1]->is_held = 1;
	pthread_mutex_unlock(&d[1]->mutex);
	pthread_mutex_unlock(&d[0]->mutex);
	print_status(c, "has taken a dongle");
	print_status(c, "has taken a dongle");
	return (1);
}

int	acquire_dongles(t_coder *c, t_dongle **d)
{
	long long	now;
	int			f_mine;
	int			s_mine;

	pthread_mutex_lock(&d[0]->mutex);
	pthread_mutex_lock(&d[1]->mutex);
	now = get_time();
	f_mine = (!d[0]->is_held
			&& d[0]->wait_queue->array[0].coder_id == c->id);
	s_mine = (!d[1]->is_held
			&& d[1]->wait_queue->array[0].coder_id == c->id);
	if (f_mine && s_mine && now >= d[0]->available_at
		&& now >= d[1]->available_at)
	{
		return (do_it(c, d));
	}
	wait_for_dongles(c, d, f_mine, s_mine);
	return (0);
}
