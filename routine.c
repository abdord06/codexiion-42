#include "codexion.h"

void	print_status(t_coder *coder, char *status)
{
	long long	now;

	// Kan-checkiw wach makayn hta mawt (burnout) 9bl manktbo
	pthread_mutex_lock(&coder->sim->death_mutex);
	if (coder->sim->is_dead == 1)
	{
		pthread_mutex_unlock(&coder->sim->death_mutex);
		return ;
	}
	now = get_time() - coder->sim->start_time;
	
	// Kan-sedou write_mutex bach tawa7d mayktb m3ana f nfs l'wa9t
	pthread_mutex_lock(&coder->sim->write_mutex);
	printf("%lld %d %s\n", now, coder->id, status);
	pthread_mutex_unlock(&coder->sim->write_mutex);
	
	pthread_mutex_unlock(&coder->sim->death_mutex);
}

/*
** Hadi hia l'astuce dyal Deadlock (L'Blocus).
** Les coders bl'id zowji (even) kakhdo lissr 3ad limn, 
** w l'ferdiyin (odd) kakhdo limn 3ad lissr.
*/
static void	take_dongles(t_coder *coder)
{
	if (coder->id % 2 == 0)
	{
		acquire_dongle(coder, coder->left_dongle);
		print_status(coder, "has taken a dongle");
		acquire_dongle(coder, coder->right_dongle);
		print_status(coder, "has taken a dongle");
	}
	else
	{
		acquire_dongle(coder, coder->right_dongle);
		print_status(coder, "has taken a dongle");
		acquire_dongle(coder, coder->left_dongle);
		print_status(coder, "has taken a dongle");
	}
}

/*
** Hada howa l'moteur d kol Coder (L'cycle d l'khedma)
*/
void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	// Kan-decaliw chwia l'bdya dyal coders zowjiyin bach maytza7mouch f d9a 1
	if (coder->id % 2 == 0)
		usleep(15000); // 15ms delay
	while (1)
	{
		// 1. Wach l'programme sala (chi 7d mat wla kolchi compila)?
		pthread_mutex_lock(&coder->sim->death_mutex);
		if (coder->sim->is_dead)
		{
			pthread_mutex_unlock(&coder->sim->death_mutex);
			break ;
		}
		pthread_mutex_unlock(&coder->sim->death_mutex);
		
		// 2. Compiling (ki-hez 2 dongles)
		take_dongles(coder);
		pthread_mutex_lock(&coder->sim->death_mutex);
		coder->last_compile = get_time(); // Kay-mise-a-jour wa9t l'compilation
		pthread_mutex_unlock(&coder->sim->death_mutex);
		print_status(coder, "is compiling");
		custom_sleep(coder->sim->t_compile, coder->sim);
		
		// Kay-7et les dongles
		release_dongle(coder->left_dongle, coder->sim);
		release_dongle(coder->right_dongle, coder->sim);
		
		// 3. Debugging
		print_status(coder, "is debugging");
		custom_sleep(coder->sim->t_debug, coder->sim);
		
		// 4. Refactoring
		print_status(coder, "is refactoring");
		custom_sleep(coder->sim->t_refactor, coder->sim);
		
		coder->compile_count++;
	}
	return (NULL);
}