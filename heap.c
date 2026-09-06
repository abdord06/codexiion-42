/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aredouan <aredouan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/06 14:42:02 by aredouan          #+#    #+#             */
/*   Updated: 2026/09/06 14:45:40 by aredouan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_higher_priority(t_request req1, t_request req2)
{
	if (req1.priority < req2.priority)
		return (1);
	if (req1.priority == req2.priority)
	{
		if (req1.tie_breaker < req2.tie_breaker)
			return (1);
	}
	return (0);
}

void	swap_req(t_request *a, t_request *b)
{
	t_request	temp;

	temp = *a;
	*a = *b;
	*b = temp;
}

void	heap_insert(t_heap *h, t_request req)
{
	int	current;
	int	parent;

	if (h->size == h->capacity)
		return ;
	current = h->size;
	h->array[current] = req;
	h->size++;
	parent = (current - 1) / 2;
	while (current != 0 && is_higher_priority(h->array[current],
			h->array[parent]))
	{
		swap_req(&h->array[current], &h->array[parent]);
		current = parent;
		parent = (current - 1) / 2;
	}
}

static void	heapify_down(t_heap *h, int current)
{
	int	left;
	int	right;
	int	smallest;

	while (1)
	{
		left = 2 * current + 1;
		right = 2 * current + 2;
		smallest = current;
		if (left < h->size && is_higher_priority(h->array[left],
				h->array[smallest]))
			smallest = left;
		if (right < h->size && is_higher_priority(h->array[right],
				h->array[smallest]))
			smallest = right;
		if (smallest == current)
			break ;
		swap_req(&h->array[current], &h->array[smallest]);
		current = smallest;
	}
}

t_request	heap_extract(t_heap *h)
{
	t_request	top;
	t_request	empty;

	empty.coder_id = -1;
	empty.priority = -1;
	empty.tie_breaker = -1;
	if (h->size <= 0)
		return (empty);
	top = h->array[0];
	h->array[0] = h->array[h->size - 1];
	h->size--;
	heapify_down(h, 0);
	return (top);
}
