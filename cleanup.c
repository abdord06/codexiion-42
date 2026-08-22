#include "codexion.h"

/*
** Hadi katms7 l'Heap li saybna f Phase 2.
** Kat-freeyi l'tableau li ldkhel, 3ad kat-freeyi l'struct.
*/
static void	free_heap(t_heap *h)
{
	if (h)
	{
		if (h->array)
			free(h->array);
		free(h);
	}
}

/*
** Hadi hia l'mksla dyal l'programme. Kat-destroyi ga3 les locks 
** w kat-freeyi ga3 l'mémoire li t-allouat b malloc.
*/
void	clean_simulation(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->nb_coders)
	{
		// 1. Destroy dyal les mutexes w conditions dyal l'dongles
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		pthread_cond_destroy(&sim->dongles[i].cond);
		
		// 2. Free dyal l'file d'attente (Heap) dyal kol dongle
		free_heap(sim->dongles[i].wait_queue);
		i++;
	}
	
	// 3. Destroy dyal les mutexes généraux
	pthread_mutex_destroy(&sim->write_mutex);
	pthread_mutex_destroy(&sim->death_mutex);
	
	// 4. Free dyal les tableaux kbar
	free(sim->dongles);
	free(sim->coders);
}