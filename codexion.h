/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abdo <abdo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/06 14:57:46 by aredouan          #+#    #+#             */
/*   Updated: 2026/09/06 18:29:41 by abdo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <unistd.h>
# include <string.h>

typedef struct s_sim	t_sim;

typedef struct s_request
{
	int			coder_id;
	long long	priority;
	int			tie_breaker;
}				t_request;

typedef struct s_heap
{
	t_request	*array;
	int			capacity;
	int			size;
}				t_heap;

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	long long		available_at;
	int				is_held;
	t_heap			*wait_queue;
}				t_dongle;

typedef struct s_coder
{
	int			id;
	int			compile_count;
	long long	last_compile;
	pthread_t	thread_id;
	t_dongle	*left_dongle;
	t_dongle	*right_dongle;
	t_sim		*sim;
}				t_coder;

typedef struct s_sim
{
	int				nb_coders;
	int				t_burnout;
	int				t_compile;
	int				t_debug;
	int				t_refactor;
	int				req_compiles;
	int				cooldown;
	int				scheduler;
	int				is_dead;
	int				sim_started;
	long long		start_time;
	pthread_mutex_t	write_mutex;
	pthread_mutex_t	death_mutex;
	t_dongle		*dongles;
	t_coder			*coders;
}				t_sim;

int			parse_args(t_sim *sim, int argc, char **argv);
int			init_sim(t_sim *sim);
long long	get_time(void);
void		custom_sleep(long long time_in_ms, t_sim *sim);
t_heap		*init_heap(int max_capacity);
int			is_higher_priority(t_request req1, t_request req2);
void		heap_insert(t_heap *h, t_request req);
t_request	heap_extract(t_heap *h);
void		print_status(t_coder *coder, char *status);
void		*coder_routine(void *arg);
void		*monitor_routine(void *arg);
void		release_dongle(t_dongle *dongle, t_sim *sim);
t_request	build_req(t_coder *coder);

void		clean_simulation(t_sim *sim);

#endif