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
		now = get_time();
		// Wach dongle mtlou9, w wach had l'coder howa li 3ndo asba9iya f l'heap?
		if (!dongle->is_held && dongle->wait_queue->array[0].coder_id == coder->id)
		{
			// Wach l'cooldown sala?
			if (now >= dongle->available_at)
				break ;
			// Ila masalach cooldown, kheli l'mutex ytnfes w ssnna chwia
			pthread_mutex_unlock(&dongle->mutex);
			custom_sleep(dongle->available_at - now, coder->sim);
			pthread_mutex_lock(&dongle->mutex);
			continue ;
		}
		// Ila dongle mchedod, n3es (cond_wait) 7ta y3yit lik l'coder lakhor
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	}
	heap_extract(dongle->wait_queue);
	dongle->is_held = 1;
	pthread_mutex_unlock(&dongle->mutex);
}

/*
** L'Coder kay7et l'dongle w kay3lm lokhrin beli ra mtlou9.
*/
void	release_dongle(t_dongle *dongle, t_sim *sim)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->is_held = 0;
	// Hna kanzidou l'cooldown dylactement f l'wa9t li t7et fih l'dongle
	dongle->available_at = get_time() + sim->cooldown;
	// Kan3ytou l ga3 les threads li na3sin f cond_wait bach yfi9o w ycheckiw
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}