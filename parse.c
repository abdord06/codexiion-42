/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aredouan <aredouan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/06 14:42:28 by aredouan          #+#    #+#             */
/*   Updated: 2026/09/06 14:57:16 by aredouan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_valid_number(char *str)
{
	int	i;

	i = 0;
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

static int	check_numeric_args(char **argv)
{
	int	i;

	i = 1;
	while (i <= 7)
	{
		if (!is_valid_number(argv[i]))
			return (printf("Error: Arguments must be positive numeric.\n"), 1);
		i++;
	}
	return (0);
}

int	parse_args(t_sim *sim, int argc, char **argv)
{
	if (argc != 9)
		return (printf("Error: you need 8 arguments.\n"), 1);
	if (check_numeric_args(argv) != 0)
		return (1);
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
	if (sim->nb_coders <= 0)
		return (printf("Error: Arguments invalid.\n"), 1);
	return (0);
}
