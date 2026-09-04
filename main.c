#include "codexion.h"

static void	wake_up_everyone(t_sim *sim)
{
	int	i;

	i = -1;
	while (++i < sim->nb_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
	}
}

static int is_valid_number(char *str)
{
    int i = 0;

    if (str[i] == '+')
        i++;
    if (str[i] == '\0')
        return (0);
    while (str[i])
    {
        if (str[i] < '0' || str[i] > '9')
            return (0);
        i++;
    }
    return (1);
}

int	parse_args(t_sim *sim, int argc, char **argv)
{
    int i;

	if (argc != 9)
		return (printf("Error: you need 8 arguments.\n"), 1);
    
    i = 1;
    while (i <= 7)
    {
        if (!is_valid_number(argv[i]))
            return (printf("Error: Arguments must be positive numeric values.\n"), 1);
        i++;
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

static void	cleanup_init(t_sim *sim, int count)
{
	while (--count >= 0)
	{
		pthread_mutex_destroy(&sim->dongles[count].mutex);
		pthread_cond_destroy(&sim->dongles[count].cond);
		if (sim->dongles[count].wait_queue)
		{
			free(sim->dongles[count].wait_queue->array);
			free(sim->dongles[count].wait_queue);
		}
	}
	pthread_mutex_destroy(&sim->write_mutex);
	pthread_mutex_destroy(&sim->death_mutex);
	if (sim->dongles)
		free(sim->dongles);
	if (sim->coders)
		free(sim->coders);
}

static int	init_dongles(t_sim *sim)
{
	int	i;

	i = -1;
	while (++i < sim->nb_coders)
	{
		if (pthread_mutex_init(&sim->dongles[i].mutex, NULL) != 0)
			return (i);
		if (pthread_cond_init(&sim->dongles[i].cond, NULL) != 0)
			return (pthread_mutex_destroy(&sim->dongles[i].mutex), i);
		sim->dongles[i].available_at = 0;
		sim->dongles[i].is_held = 0;
		sim->dongles[i].wait_queue = init_heap(sim->nb_coders);
		if (!sim->dongles[i].wait_queue)
		{
			pthread_cond_destroy(&sim->dongles[i].cond);
			pthread_mutex_destroy(&sim->dongles[i].mutex);
			return (i);
		}
	}
	return (-1);
}

int	init_sim(t_sim *sim)
{
	int		i;
	t_coder	*c;

	sim->is_dead = 0;
	if (pthread_mutex_init(&sim->write_mutex, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&sim->death_mutex, NULL) != 0)
		return (pthread_mutex_destroy(&sim->write_mutex), 1);
	sim->coders = malloc(sizeof(t_coder) * sim->nb_coders);
	sim->dongles = malloc(sizeof(t_dongle) * sim->nb_coders);
	if (!sim->coders || !sim->dongles)
		return (cleanup_init(sim, 0), 1);
	i = init_dongles(sim);
	if (i != -1)
		return (cleanup_init(sim, i), 1);
	i = -1;
	while (++i < sim->nb_coders)
	{
		c = &sim->coders[i];
		c->id = i + 1;
		c->compile_count = 0;
		c->sim = sim;
		c->left_dongle = &sim->dongles[i];
		c->right_dongle = &sim->dongles[(i + 1) % sim->nb_coders];
	}
	return (0);
}

static int	start_coders(t_sim *sim)
{
	int		i;
	t_coder	*c;

	i = -1;
	while (++i < sim->nb_coders)
	{
		c = &sim->coders[i];
		c->last_compile = sim->start_time;
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

	if (parse_args(&sim, argc, argv) == 1 || init_sim(&sim) == 1)
		return (1);
	sim.start_time = get_time();
	if (start_coders(&sim) == 1)
		return (clean_simulation(&sim), 1);
	if (start_monitor(&sim, &monitor) == 0)
		pthread_join(monitor, NULL);
	i = -1;
	while (++i < sim.nb_coders)
		pthread_join(sim.coders[i].thread_id, NULL);
	clean_simulation(&sim);
	return (0);
}
