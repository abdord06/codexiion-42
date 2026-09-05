#include "codexion.h"

void	print_status(t_coder *coder, char *status)
{
	long long	now;

	pthread_mutex_lock(&coder->sim->death_mutex);
	if (coder->sim->is_dead == 1)
	{
		pthread_mutex_unlock(&coder->sim->death_mutex);
		return ;
	}
	now = get_time() - coder->sim->start_time;
	pthread_mutex_lock(&coder->sim->write_mutex);
	printf("%lld %d %s\n", now, coder->id, status);
	pthread_mutex_unlock(&coder->sim->write_mutex);
	pthread_mutex_unlock(&coder->sim->death_mutex);
}

static void take_dongles(t_coder *coder)
{
	t_dongle *first;
	t_dongle *second;

	if (coder->left_dongle < coder->right_dongle)
	{
		first = coder->left_dongle;
		second = coder->right_dongle;
	}
	else
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}

	acquire_dongle(coder, first);
	print_status(coder, "has taken a dongle");
	acquire_dongle(coder, second);
	print_status(coder, "has taken a dongle");
}


void	*coder_routine(void *arg)
{
	t_coder	*coder;
    int     think_time;

	coder = (t_coder *)arg;
    while (1)
    {
        pthread_mutex_lock(&coder->sim->death_mutex);
        if (coder->sim->sim_started == 1 || coder->sim->is_dead == 1)
        {
            pthread_mutex_unlock(&coder->sim->death_mutex);
            break;
        }
        pthread_mutex_unlock(&coder->sim->death_mutex);
        usleep(50);
    }
	pthread_mutex_lock(&coder->sim->death_mutex);
	coder->last_compile = coder->sim->start_time;
	pthread_mutex_unlock(&coder->sim->death_mutex);
	if (coder->id % 2 == 0)
		custom_sleep(10, coder->sim);
	else if (coder->id == coder->sim->nb_coders)
		custom_sleep(20, coder->sim);
	if (coder->left_dongle == coder->right_dongle)
	{
		acquire_dongle(coder, coder->left_dongle);
		print_status(coder, "has taken a dongle");
		custom_sleep(coder->sim->t_burnout, coder->sim);
		release_dongle(coder->left_dongle, coder->sim);
		return (NULL);
	}
	while (1)
	{
		pthread_mutex_lock(&coder->sim->death_mutex);
		if (coder->sim->is_dead)
		{
			pthread_mutex_unlock(&coder->sim->death_mutex);
			break ;
		}
		pthread_mutex_unlock(&coder->sim->death_mutex);
		take_dongles(coder);
		pthread_mutex_lock(&coder->sim->death_mutex);
		coder->last_compile = get_time();
		pthread_mutex_unlock(&coder->sim->death_mutex);
		print_status(coder, "is compiling");
		custom_sleep(coder->sim->t_compile, coder->sim);
		release_dongle(coder->left_dongle, coder->sim);
		release_dongle(coder->right_dongle, coder->sim);
		print_status(coder, "is debugging");
		custom_sleep(coder->sim->t_debug, coder->sim);
		print_status(coder, "is refactoring");
		custom_sleep(coder->sim->t_refactor, coder->sim);
        pthread_mutex_lock(&coder->sim->death_mutex);
		coder->compile_count++;
        if (coder->compile_count >= coder->sim->req_compiles)
        {
            pthread_mutex_unlock(&coder->sim->death_mutex);
            break;
        }
        pthread_mutex_unlock(&coder->sim->death_mutex);
        think_time = (coder->sim->t_burnout / 2) - (coder->sim->t_compile + coder->sim->t_debug + coder->sim->t_refactor);
		if (think_time > 0)
			custom_sleep(think_time, coder->sim);
		else
			custom_sleep(5, coder->sim);
	}
	return (NULL);
}
