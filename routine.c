/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aredouan <aredouan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/06 14:42:34 by aredouan          #+#    #+#             */
/*   Updated: 2026/09/06 15:03:00 by aredouan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	take_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left_dongle < coder->right_dongle)
	{
		first = coder->left_dongle;
		second = coder->right_dongle;
	}
	else
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	acquire_dongle(coder, first);
	print_status(coder, "has taken a dongle");
	acquire_dongle(coder, second);
	print_status(coder, "has taken a dongle");
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

static int	check_death(t_coder *coder)
{
	int	dead;

	pthread_mutex_lock(&coder->sim->death_mutex);
	dead = coder->sim->is_dead;
	pthread_mutex_unlock(&coder->sim->death_mutex);
	return (dead);
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
	custom_sleep(5, coder->sim);
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
	custom_sleep(coder->id * 10, coder->sim);
	if (coder->left_dongle == coder->right_dongle)
	{
		acquire_dongle(coder, coder->left_dongle);
		print_status(coder, "has taken a dongle");
		custom_sleep(coder->sim->t_burnout, coder->sim);
		release_dongle(coder->left_dongle, coder->sim);
		return (NULL);
	}
	print_status(coder, "started");
	while (1)
	{
		if (do_activities(coder))
			break ;
	}
	return (NULL);
}
