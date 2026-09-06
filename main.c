/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aredouan <aredouan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/06 14:42:18 by aredouan          #+#    #+#             */
/*   Updated: 2026/09/06 14:42:19 by aredouan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void    wake_up_everyone(t_sim *sim)
{
    int i;

    i = -1;
    while (++i < sim->nb_coders)
    {
        pthread_mutex_lock(&sim->dongles[i].mutex);
        pthread_cond_broadcast(&sim->dongles[i].cond);
        pthread_mutex_unlock(&sim->dongles[i].mutex);
    }
}

static int  start_coders(t_sim *sim)
{
    int     i;
    t_coder *c;

    i = -1;
    while (++i < sim->nb_coders)
    {
        c = &sim->coders[i];
        if (pthread_create(&c->thread_id, NULL, coder_routine, c) != 0)
        {
            pthread_mutex_lock(&sim->death_mutex);
            sim->is_dead = 1;
            pthread_mutex_unlock(&sim->death_mutex);
            wake_up_everyone(sim);
            while (--i >= 0)
                pthread_join(sim->coders[i].thread_id, NULL);
            return (1);
        }
    }
    return (0);
}

static int  start_monitor(t_sim *sim, pthread_t *monitor)
{
    if (pthread_create(monitor, NULL, monitor_routine, sim) != 0)
    {
        pthread_mutex_lock(&sim->death_mutex);
        sim->is_dead = 1;
        pthread_mutex_unlock(&sim->death_mutex);
        wake_up_everyone(sim);
        return (1);
    }
    return (0);
}

int main(int argc, char **argv)
{
    t_sim       sim;
    pthread_t   monitor;
    int         i;

    if (parse_args(&sim, argc, argv) == 1 || init_sim(&sim) == 1)
        return (1);
    if (start_coders(&sim) == 1)
        return (clean_simulation(&sim), 1);
    sim.start_time = get_time();

    pthread_mutex_lock(&sim.death_mutex);
    i = -1;
    while (++i < sim.nb_coders)
        sim.coders[i].last_compile = sim.start_time;
    sim.sim_started = 1;
    pthread_mutex_unlock(&sim.death_mutex);
    if (start_monitor(&sim, &monitor) == 0)
        pthread_join(monitor, NULL);
    i = -1;
    while (++i < sim.nb_coders)
        pthread_join(sim.coders[i].thread_id, NULL);
    clean_simulation(&sim);
    return (0);
}
