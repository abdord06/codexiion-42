/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arbitrator.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abdo <abdo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/06 14:41:53 by aredouan          #+#    #+#             */
/*   Updated: 2026/09/06 18:25:27 by abdo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	print_status(t_coder *coder, char *status)
{
	long long	now;

	pthread_mutex_lock(&coder->sim->death_mutex);
	if (coder->sim->is_dead == 1)
	{
		pthread_mutex_unlock(&coder->sim->death_mutex);
		return ;
	}
	now = get_time() - coder->sim->start_time;
	pthread_mutex_lock(&coder->sim->write_mutex);
	printf("%lld %d %s\n", now, coder->id, status);
	pthread_mutex_unlock(&coder->sim->write_mutex);
	pthread_mutex_unlock(&coder->sim->death_mutex);
}

t_request	build_req(t_coder *coder)
{
    t_request   req;

    req.coder_id = coder->id;
    if (coder->sim->scheduler == 0)
    {
        req.priority = get_time();
        req.tie_breaker = 0;
    }
    else
    {
        pthread_mutex_lock(&coder->sim->death_mutex);
        req.priority = coder->last_compile + coder->sim->t_burnout;
        req.tie_breaker = coder->compile_count;
        pthread_mutex_unlock(&coder->sim->death_mutex);
    }
    return (req);
}

void	release_dongle(t_dongle *dongle, t_sim *sim)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->is_held = 0;
	dongle->available_at = get_time() + sim->cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
