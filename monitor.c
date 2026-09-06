/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aredouan <aredouan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/06 14:42:24 by aredouan          #+#    #+#             */
/*   Updated: 2026/09/06 14:57:16 by aredouan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	wake_up_everyone(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->nb_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

static int	is_coder_dead(t_coder *coder)
{
	long long	now;

	pthread_mutex_lock(&coder->sim->death_mutex);
	if (coder->compile_count >= coder->sim->req_compiles)
	{
		pthread_mutex_unlock(&coder->sim->death_mutex);
		return (0);
	}
	now = get_time();
	if (now - coder->last_compile >= coder->sim->t_burnout)
	{
		coder->sim->is_dead = 1;
		pthread_mutex_unlock(&coder->sim->death_mutex);
		pthread_mutex_lock(&coder->sim->write_mutex);
		printf("%lld %d burned out\n", now - coder->sim->start_time,
			coder->id);
		pthread_mutex_unlock(&coder->sim->write_mutex);
		return (1);
	}
	pthread_mutex_unlock(&coder->sim->death_mutex);
	return (0);
}

static int	all_compiled(t_sim *sim)
{
	int	i;
	int	finished;

	i = 0;
	finished = 0;
	while (i < sim->nb_coders)
	{
		pthread_mutex_lock(&sim->death_mutex);
		if (sim->coders[i].compile_count >= sim->req_compiles)
			finished++;
		pthread_mutex_unlock(&sim->death_mutex);
		i++;
	}
	if (finished == sim->nb_coders)
	{
		pthread_mutex_lock(&sim->death_mutex);
		sim->is_dead = 1;
		pthread_mutex_unlock(&sim->death_mutex);
		return (1);
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	int		i;

	sim = (t_sim *)arg;
	while (1)
	{
		i = 0;
		while (i < sim->nb_coders)
		{
			if (is_coder_dead(&sim->coders[i]))
			{
				wake_up_everyone(sim);
				return (NULL);
			}
			i++;
		}
		if (all_compiled(sim))
		{
			wake_up_everyone(sim);
			return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}
