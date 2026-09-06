/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aredouan <aredouan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/06 14:42:41 by aredouan          #+#    #+#             */
/*   Updated: 2026/09/06 14:42:44 by aredouan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	get_time(void)
{
	struct timeval	tv;

	if (gettimeofday(&tv, NULL) == -1)
		return (-1);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void	custom_sleep(long long time_in_ms, t_sim *sim)
{
	long long	start;

	start = get_time();
	while ((get_time() - start) < time_in_ms)
	{
		pthread_mutex_lock(&sim->death_mutex);
		if (sim->is_dead == 1)
		{
			pthread_mutex_unlock(&sim->death_mutex);
			break ;
		}
		pthread_mutex_unlock(&sim->death_mutex);
		usleep(100);
	}
}
