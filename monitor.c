#include "codexion.h"

/*
** Hadi astuce s7riya. Mli kaymot chi wa7d, y9dr ykono coders lokhrin 
** na3sin f cond_wait kaytsnaw dongle. Ila l'programme sala, ghayb9aw m7bsin tmma.
** Had l'fonction kat-gheni 3la kolchi bach ifi9o yl9aw is_dead = 1 w ykhrjo.
*/
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

/*
** Kat-checki wach had l'coder dar burnout wla ba9i 3aych.
*/
static int	is_coder_dead(t_coder *coder)
{
	long long	now;

	pthread_mutex_lock(&coder->sim->death_mutex);
	now = get_time();
	// Wach l'wa9t li daz mn akher compile kber mn wa9t l'mawt?
	if (now - coder->last_compile >= coder->sim->t_burnout)
	{
		coder->sim->is_dead = 1; // Kan-declariw l'mawt bach kolchi yw9f
		pthread_mutex_unlock(&coder->sim->death_mutex);
		
		// Kan-affichiw l'log dyal l'mawt m-protegé
		pthread_mutex_lock(&coder->sim->write_mutex);
		printf("%lld %d burned out\n", now - coder->sim->start_time, coder->id);
		pthread_mutex_unlock(&coder->sim->write_mutex);
		return (1);
	}
	pthread_mutex_unlock(&coder->sim->death_mutex);
	return (0);
}

/*
** Kat-checki wach ga3 les coders darou l'3adad dyal compiles li mtlob.
*/
static int	all_compiled(t_sim *sim)
{
	int	i;
	int	finished;

	i = 0;
	finished = 0;
	while (i < sim->nb_coders)
	{
		pthread_mutex_lock(&sim->death_mutex);
		if (sim->coders[i].compile_count >= sim->req_compiles)
			finished++;
		pthread_mutex_unlock(&sim->death_mutex);
		i++;
	}
	// Ila l'3adad d li salaw kay-ssawi 3adad l'coders kamlin
	if (finished == sim->nb_coders)
	{
		pthread_mutex_lock(&sim->death_mutex);
		sim->is_dead = 1; // Stopi l'simulation b naja7
		pthread_mutex_unlock(&sim->death_mutex);
		return (1);
	}
	return (0);
}

/*
** Hada howa l'thread dyal l'3ssass li ki-dowr 3lihom.
*/
void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	int		i;

	sim = (t_sim *)arg;
	while (1)
	{
		i = 0;
		while (i < sim->nb_coders)
		{
			// Ila l9a chi wa7d mat, ki-fiyye9 lokhrin w kaykhrj
			if (is_coder_dead(&sim->coders[i]))
			{
				wake_up_everyone(sim);
				return (NULL);
			}
			i++;
		}
		// Ila kolchi sala l'compilation mzyan, ki-fiyye9 w kaykhrj
		if (all_compiled(sim))
		{
			wake_up_everyone(sim);
			return (NULL);
		}
		// Kay-n3es 1ms. Hakka maghay7re9ch l'CPU, w nfs l'wa9t ghay3i9 
		// b l'mawt dylactement f a9al mn 10ms (1ms < 10ms).
		usleep(1000);
	}
	return (NULL);
}