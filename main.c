#include "codexion.h"

static void	free_heap_local(t_heap *heap)
{
	if (heap)
	{
		free(heap->array);
		free(heap);
	}
}

static void	destroy_dongles(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		pthread_cond_destroy(&sim->dongles[i].cond);
		free_heap_local(sim->dongles[i].wait_queue);
		i++;
	}
}

static void	cleanup_init_failure(t_sim *sim)
{
	pthread_mutex_destroy(&sim->write_mutex);
	pthread_mutex_destroy(&sim->death_mutex);
	free(sim->dongles);
	free(sim->coders);
}

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

int	parse_args(t_sim *sim, int argc, char **argv)
{
	if (argc != 9)
	{
		printf("Error: you need 8 arguments.\n");
		return (1);
	}
	sim->nb_coders = atoi(argv[1]);
	sim->t_burnout = atoi(argv[2]);
	sim->t_compile = atoi(argv[3]);
	sim->t_debug = atoi(argv[4]);
	sim->t_refactor = atoi(argv[5]);
	sim->req_compiles = atoi(argv[6]);
	sim->cooldown = atoi(argv[7]);
	if (strcmp(argv[8], "fifo") == 0)
		sim->scheduler = 0;
	else if (strcmp(argv[8], "edf") == 0)
		sim->scheduler = 1;
	else
		return (printf("Error: scheduler should be 'fifo' or 'edf'.\n"), 1);
	if (sim->nb_coders <= 0 || sim->t_burnout < 0 || sim->t_compile < 0
			|| sim->t_debug < 0 || sim->t_refactor < 0 || sim->cooldown < 0)
		return (printf("Error: Arguments invalid.\n"), 1);
	return (0);
}

static int	init_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->nb_coders)
	{
		if (pthread_mutex_init(&sim->dongles[i].mutex, NULL) != 0)
		{
			destroy_dongles(sim, i);
			return (1);
		}
		if (pthread_cond_init(&sim->dongles[i].cond, NULL) != 0)
		{
			pthread_mutex_destroy(&sim->dongles[i].mutex);
			destroy_dongles(sim, i);
			return (1);
		}
		sim->dongles[i].available_at = 0;
		sim->dongles[i].is_held = 0;
		sim->dongles[i].wait_queue = init_heap(sim->nb_coders);
		if (!sim->dongles[i].wait_queue)
		{
			pthread_cond_destroy(&sim->dongles[i].cond);
			pthread_mutex_destroy(&sim->dongles[i].mutex);
			destroy_dongles(sim, i);
			return (1);
		}
		i++;
	}
	return (0);
}

int	init_sim(t_sim *sim)
{
	int	i;

	sim->is_dead = 0;
	sim->coders = NULL;
	sim->dongles = NULL;
	if (pthread_mutex_init(&sim->write_mutex, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&sim->death_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->write_mutex);
		return (1);
	}
	sim->coders = malloc(sizeof(t_coder) * sim->nb_coders);
	sim->dongles = malloc(sizeof(t_dongle) * sim->nb_coders);
	if (!sim->coders || !sim->dongles)
	{
		cleanup_init_failure(sim);
		return (1);
	}
	if (init_dongles(sim) == 1)
	{
		free(sim->dongles);
		free(sim->coders);
		pthread_mutex_destroy(&sim->write_mutex);
		pthread_mutex_destroy(&sim->death_mutex);
		return (1);
	}
	i = 0;
	while (i < sim->nb_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].compile_count = 0;
		sim->coders[i].sim = sim;
		sim->coders[i].left_dongle = &sim->dongles[i];
		sim->coders[i].right_dongle = &sim->dongles[(i + 1) % sim->nb_coders];
		i++;
	}
	return (0);
}

static int	start_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->nb_coders)
	{
		sim->coders[i].last_compile = sim->start_time;
		if (pthread_create(&sim->coders[i].thread_id, NULL, coder_routine,
				&sim->coders[i]) != 0)
		{
			pthread_mutex_lock(&sim->death_mutex);
			sim->is_dead = 1;
			pthread_mutex_unlock(&sim->death_mutex);
			wake_up_everyone(sim);
			while (i-- > 0)
				pthread_join(sim->coders[i].thread_id, NULL);
			return (1);
		}
		i++;
	}
	return (0);
}

static int	start_monitor(t_sim *sim, pthread_t *monitor)
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

int	main(int argc, char **argv)
{
	t_sim		sim;
	pthread_t	monitor;
	int			i;

	if (parse_args(&sim, argc, argv) == 1)
		return (1);
	if (init_sim(&sim) == 1)
		return (1);
	sim.start_time = get_time();
	if (start_coders(&sim) == 1)
	{
		clean_simulation(&sim);
		return (1);
	}
	if (start_monitor(&sim, &monitor) == 1)
	{
		i = 0;
		while (i < sim.nb_coders)
		{
			pthread_join(sim.coders[i].thread_id, NULL);
			i++;
		}
		clean_simulation(&sim);
		return (1);
	}
	pthread_join(monitor, NULL);
	i = 0;
	while (i < sim.nb_coders)
	{
		pthread_join(sim.coders[i].thread_id, NULL);
		i++;
	}
	clean_simulation(&sim);
	return (0);
}
