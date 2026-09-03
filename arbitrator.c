#include "codexion.h"

static t_request	build_req(t_coder *coder)
{
	t_request	req;

	req.coder_id = coder->id;
	req.tie_breaker = coder->id;
	if (coder->sim->scheduler == 0)
	{
		req.priority = get_time();
	}
	else
	{
		req.priority = coder->last_compile + coder->sim->t_burnout;
	}
	return (req);
}

void	acquire_dongle(t_coder *coder, t_dongle *dongle)
{
	t_request	req;
	long long	now;

	pthread_mutex_lock(&dongle->mutex);
	req = build_req(coder);
	heap_insert(dongle->wait_queue, req);
	while (1)
	{
        pthread_mutex_lock(&coder->sim->death_mutex);
        if (coder->sim->is_dead)
        {
            pthread_mutex_unlock(&coder->sim->death_mutex);
            pthread_mutex_unlock(&dongle->mutex);
            return ;
        }
        pthread_mutex_unlock(&coder->sim->death_mutex);
		now = get_time();
		if (!dongle->is_held && dongle->wait_queue->array[0].coder_id
				== coder->id)
		{
			if (now >= dongle->available_at)
				break ;
			pthread_mutex_unlock(&dongle->mutex);
			custom_sleep(dongle->available_at - now, coder->sim);
			pthread_mutex_lock(&dongle->mutex);
			continue ;
		}
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	}
	heap_extract(dongle->wait_queue);
	dongle->is_held = 1;
	pthread_mutex_unlock(&dongle->mutex);
}

void	release_dongle(t_dongle *dongle, t_sim *sim)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->is_held = 0;
	dongle->available_at = get_time() + sim->cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
