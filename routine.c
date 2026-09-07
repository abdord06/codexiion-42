/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abdo <abdo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/06 14:42:34 by aredouan          #+#    #+#             */
/*   Updated: 2026/09/06 23:58:33 by abdo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	check_death(t_coder *coder)
{
	int	dead;

	pthread_mutex_lock(&coder->sim->death_mutex);
	dead = coder->sim->is_dead;
	pthread_mutex_unlock(&coder->sim->death_mutex);
	return (dead);
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

static int	acquire_dongles(t_coder *c, t_dongle **d)
{
	long long	now;
	int			f_mine;
	int			s_mine;

	pthread_mutex_lock(&d[0]->mutex);
	pthread_mutex_lock(&d[1]->mutex);
	now = get_time();
	f_mine = (!d[0]->is_held && d[0]->wait_queue->array[0].coder_id == c->id);
	s_mine = (!d[1]->is_held && d[1]->wait_queue->array[0].coder_id == c->id);
	if (f_mine && s_mine && now >= d[0]->available_at && now >= d[1]->available_at)
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
	wait_for_dongles(c, d, f_mine, s_mine);
	return (0);
}

static void	queue_for_dongles(t_coder *c, t_dongle **d)
{
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
	pthread_mutex_lock(&d[0]->mutex);
	heap_insert(d[0]->wait_queue, build_req(c));
	pthread_mutex_unlock(&d[0]->mutex);
	pthread_mutex_lock(&d[1]->mutex);
	heap_insert(d[1]->wait_queue, build_req(c));
	pthread_mutex_unlock(&d[1]->mutex);
}

static void	take_dongles(t_coder *coder)
{
	t_dongle	*d[2];

	queue_for_dongles(coder, d);
	while (1)
	{
		if (check_death(coder))
			return ;
		if (acquire_dongles(coder, d))
			break ;
	}
}

static void	wait_for_start(t_coder *coder)
{
	while (1)
	{
		pthread_mutex_lock(&coder->sim->death_mutex);
		if (coder->sim->sim_started == 1 || coder->sim->is_dead == 1)
		{
			pthread_mutex_unlock(&coder->sim->death_mutex);
			break ;
		}
		pthread_mutex_unlock(&coder->sim->death_mutex);
		usleep(50);
	}
}

static int	do_activities(t_coder *coder)
{
	if (check_death(coder))
		return (1);
	take_dongles(coder);
	pthread_mutex_lock(&coder->sim->death_mutex);
	coder->last_compile = get_time();
	pthread_mutex_unlock(&coder->sim->death_mutex);
	print_status(coder, "is compiling");
	custom_sleep(coder->sim->t_compile, coder->sim);
	release_dongle(coder->left_dongle, coder->sim);
	release_dongle(coder->right_dongle, coder->sim);
	print_status(coder, "is debugging");
	custom_sleep(coder->sim->t_debug, coder->sim);
	print_status(coder, "is refactoring");
	custom_sleep(coder->sim->t_refactor, coder->sim);
	pthread_mutex_lock(&coder->sim->death_mutex);
	coder->compile_count++;
	if (coder->compile_count >= coder->sim->req_compiles)
	{
		pthread_mutex_unlock(&coder->sim->death_mutex);
		return (1);
	}
	pthread_mutex_unlock(&coder->sim->death_mutex);
	return (0);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	wait_for_start(coder);
	pthread_mutex_lock(&coder->sim->death_mutex);
	coder->last_compile = coder->sim->start_time;
	pthread_mutex_unlock(&coder->sim->death_mutex);
	if (coder->id % 2 == 0)
		custom_sleep((coder->sim->t_compile + coder->sim->cooldown) / 2, coder->sim);

	if (coder->left_dongle == coder->right_dongle)
	{
		pthread_mutex_lock(&coder->left_dongle->mutex);
		coder->left_dongle->is_held = 1;
		pthread_mutex_unlock(&coder->left_dongle->mutex);
		
		print_status(coder, "has taken a dongle");
		custom_sleep(coder->sim->t_burnout, coder->sim);
		release_dongle(coder->left_dongle, coder->sim);
		return (NULL);
	}
	while (1)
	{
		if (do_activities(coder))
			break ;
	}
	return (NULL);
}
