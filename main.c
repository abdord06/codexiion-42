#include "codexion.h"

int	parse_args(t_sim *sim, int argc, char **argv)
{
	if (argc != 9)
	{
		printf("Error: Khass 8 arguments.\n");
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
		return (printf("Error: scheduler khasso ykon 'fifo' wla 'edf'.\n"), 1);
	if (sim->nb_coders <= 0 || sim->t_burnout < 0 || sim->t_compile < 0
		|| sim->t_debug < 0 || sim->t_refactor < 0 || sim->cooldown < 0)
		return (printf("Error: Arguments invalid.\n"), 1);
	return (0);
}

/*
** 9semna l'initialisation d dongles f fonction bo7dha 3la 9bel Norminette
*/
static int	init_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->nb_coders)
	{
		pthread_mutex_init(&sim->dongles[i].mutex, NULL);
		// ZEDNA HADO bach l'arbitre yl9ahom wajdin
		pthread_cond_init(&sim->dongles[i].cond, NULL); 
		sim->dongles[i].available_at = 0;
		sim->dongles[i].is_held = 0;
		// L'Heap khasso wa7d l'capacity d'attente (max howa 3adad l'coders)
		sim->dongles[i].wait_queue = init_heap(sim->nb_coders);
		if (!sim->dongles[i].wait_queue)
			return (1);
		i++;
	}
	return (0);
}

int	init_sim(t_sim *sim)
{
	int	i;

	sim->is_dead = 0;
	pthread_mutex_init(&sim->write_mutex, NULL);
	pthread_mutex_init(&sim->death_mutex, NULL);
	sim->coders = malloc(sizeof(t_coder) * sim->nb_coders);
	sim->dongles = malloc(sizeof(t_dongle) * sim->nb_coders);
	if (!sim->coders || !sim->dongles)
		return (1);
	if (init_dongles(sim) == 1) // 3yetna l fonction jdida hna
		return (1);
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
	i = 0;
	while (i < sim.nb_coders)
	{
		sim.coders[i].last_compile = sim.start_time;
		// MSS7E7A: 7yedna () mn coder_routine
		pthread_create(&sim.coders[i].thread_id, NULL, coder_routine, &sim.coders[i]);
		i++;
	}
	pthread_create(&monitor, NULL, monitor_routine, &sim);
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
