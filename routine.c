/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aredouan <aredouan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/06 14:42:34 by aredouan          #+#    #+#             */
/*   Updated: 2026/09/07 19:27:21 by aredouan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	take_dongles(t_coder *coder)
{
	t_dongle	*d[2];

	queue_for_dongles(coder, d);
	while (1)
	{
		if (check_death(coder))
			return (0);
		if (acquire_dongles(coder, d))
			break ;
	}
	return (1);
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
	if (!take_dongles(coder))
		return (1);
	pthread_mutex_lock(&coder->sim->death_mutex);
	coder->last_compile = get_time();
	pthread_mutex_unlock(&coder->sim->death_mutex);
	print_status(coder, "is compiling");
	custom_sleep(coder->sim->t_compile, coder->sim);
    pthread_mutex_lock(&coder->sim->death_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&coder->sim->death_mutex);
	release_dongle(coder->left_dongle, coder->sim);
	release_dongle(coder->right_dongle, coder->sim);
    
	custom_sleep(1, coder->sim);
	print_status(coder, "is debugging");
	custom_sleep(coder->sim->t_debug, coder->sim);
	print_status(coder, "is refactoring");
	custom_sleep(coder->sim->t_refactor, coder->sim);
    pthread_mutex_lock(&coder->sim->death_mutex);
    if (coder->compile_count >= coder->sim->req_compiles)
    {
        pthread_mutex_unlock(&coder->sim->death_mutex);
        return (1);
    }
    pthread_mutex_unlock(&coder->sim->death_mutex);
	return (0);
}

static void	*single_coder_routine(t_coder *coder)
{
	pthread_mutex_lock(&coder->left_dongle->mutex);
	coder->left_dongle->is_held = 1;
	pthread_mutex_unlock(&coder->left_dongle->mutex);
	print_status(coder, "has taken a dongle");
	custom_sleep(coder->sim->t_burnout, coder->sim);
	release_dongle(coder->left_dongle, coder->sim);
	return (NULL);
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
		custom_sleep((coder->sim->t_compile
				+ coder->sim->cooldown) / 2, coder->sim);
	if (coder->left_dongle == coder->right_dongle)
		return (single_coder_routine(coder));
	while (1)
	{
		if (do_activities(coder))
			break ;
	}
	return (NULL);
}
