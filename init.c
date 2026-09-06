#include "codexion.h"

static void cleanup_init(t_sim *sim, int count)
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

static int  init_dongles(t_sim *sim)
{
    int i;

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

static void	init_coders(t_sim *sim)
{
	int		i;
	t_coder	*c;

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
}

int	init_sim(t_sim *sim)
{
	int	i;

	sim->is_dead = 0;
	sim->sim_started = 0;
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
	init_coders(sim);
	return (0);
}
