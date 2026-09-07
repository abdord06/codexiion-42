/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abdo <abdo@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/06 14:42:28 by aredouan          #+#    #+#             */
/*   Updated: 2026/09/07 13:58:12 by abdo             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long long	ft_atoi(char *str)
{
	long long	res;
	int			i;

	res = 0;
	i = 0;
	if (str[i] == '+')
		i++;
	while (str[i] >= '0' && str[i] <= '9')
	{
		res = res * 10 + (str[i] - '0');
		if (res > 2147483647)
			return (-1);
		i++;
	}
	return (res);
}

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
	sim->nb_coders = ft_atoi(argv[1]);
	sim->t_burnout = ft_atoi(argv[2]);
	sim->t_compile = ft_atoi(argv[3]);
	sim->t_debug = ft_atoi(argv[4]);
	sim->t_refactor = ft_atoi(argv[5]);
	sim->req_compiles = ft_atoi(argv[6]);
	sim->cooldown = ft_atoi(argv[7]);
	if (strcmp(argv[8], "fifo") == 0)
		sim->scheduler = 0;
	else if (strcmp(argv[8], "edf") == 0)
		sim->scheduler = 1;
	else
		return (printf("Error: scheduler should be 'fifo' or 'edf'.\n"), 1);
	if (sim->nb_coders <= 0 || sim->t_burnout < 0 || sim->t_compile < 0
		|| sim->t_debug < 0 || sim->t_refactor < 0 || sim->req_compiles < 0
		|| sim->cooldown < 0)
		return (printf("Error: Arguments invalid.\n"), 1);
	return (0);
}
