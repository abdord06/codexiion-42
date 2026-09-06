/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arbitrator.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aredouan <aredouan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/06 14:41:53 by aredouan          #+#    #+#             */
/*   Updated: 2026/09/06 14:41:55 by aredouan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_request	build_req(t_coder *coder)
{
	t_request	req;
	long long	last_compile;

	req.coder_id = coder->id;
	req.tie_breaker = coder->id;
	if (coder->sim->scheduler == 0)
	{
		req.priority = get_time();
	}
	else
	{
		pthread_mutex_lock(&coder->sim->death_mutex);
		last_compile = coder->last_compile;
		pthread_mutex_unlock(&coder->sim->death_mutex);
		req.priority = last_compile + coder->sim->t_burnout;
	}
	return (req);
}

static int	wait_for_turn(t_coder *coder, t_dongle *dongle)
{
	long long	now;

	while (1)
	{
		pthread_mutex_lock(&coder->sim->death_mutex);
		if (coder->sim->is_dead)
		{
			pthread_mutex_unlock(&coder->sim->death_mutex);
			return (1);
		}
		pthread_mutex_unlock(&coder->sim->death_mutex);
		now = get_time();
		if (!dongle->is_held && dongle->wait_queue->array[0].coder_id
			== coder->id)
		{
			if (now >= dongle->available_at)
				break ;
			pthread_mutex_unlock(&dongle->mutex);
			custom_sleep(dongle->available_at - now, coder->sim);
			pthread_mutex_lock(&dongle->mutex);
			continue ;
		}
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	}
	return (0);
}

void	acquire_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	heap_insert(dongle->wait_queue, build_req(coder));
	if (wait_for_turn(coder, dongle))
	{
		pthread_mutex_unlock(&dongle->mutex);
		return ;
	}
	heap_extract(dongle->wait_queue);
	dongle->is_held = 1;
	pthread_mutex_unlock(&dongle->mutex);
}

void	release_dongle(t_dongle *dongle, t_sim *sim)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->is_held = 0;
	dongle->available_at = get_time() + sim->cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
