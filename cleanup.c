#include "codexion.h"

static void	free_heap(t_heap *h)
{
	if (h)
	{
		if (h->array)
			free(h->array);
		free(h);
	}
}

void	clean_simulation(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->nb_coders)
	{
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		pthread_cond_destroy(&sim->dongles[i].cond);
		free_heap(sim->dongles[i].wait_queue);
		i++;
	}
	pthread_mutex_destroy(&sim->write_mutex);
	pthread_mutex_destroy(&sim->death_mutex);
	free(sim->dongles);
	free(sim->coders);
}