#include "codexion.h"

t_heap *init_heap(int max_capacity)
{
    t_heap *h = malloc(sizeof(t_heap));
    if (!h) return (NULL);
    
    h->array = malloc(sizeof(t_request) * max_capacity);
    if (!h->array) {
        free(h);
        return (NULL);
    }
    h->capacity = max_capacity;
    h->size = 0;
    return (h);
}

int is_higher_priority(t_request req1, t_request req2)
{
    if (req1.priority < req2.priority)
        return (1); // req1 3ndo ra9m sgher (asba9)
    if (req1.priority == req2.priority) {
        // Hada howa l'Tie-Breaker rule li mtolb f l'projet!
        if (req1.tie_breaker < req2.tie_breaker)
            return (1);
    }
    return (0);
}

// 3. Fonction d l'echange (Swap)
void swap_req(t_request *a, t_request *b)
{
    t_request temp = *a;
    *a = *b;
    *b = temp;
}

void heap_insert(t_heap *h, t_request req)
{
	int	current;
	int	parent;

	if (h->size == h->capacity)
		return;
	current = h->size;
	h->array[current] = req;
	h->size++;
	parent = (current - 1) / 2;

	while (current != 0 && is_higher_priority(h->array[current], h->array[parent])) {
		swap_req(&h->array[current], &h->array[parent]);
		current = parent;
		parent = (current - 1) / 2;
	}
}

t_request heap_extract(t_heap *h)
{
    t_request empty = {-1, -1, -1};
    if (h->size <= 0) return (empty);

    t_request top = h->array[0];

    h->array[0] = h->array[h->size - 1];
    h->size--;

    int current = 0;
    while (1) {
        int left = 2 * current + 1;
        int right = 2 * current + 2;
        int smallest = current;

        if (left < h->size && is_higher_priority(h->array[left], h->array[smallest]))
            smallest = left;
        if (right < h->size && is_higher_priority(h->array[right], h->array[smallest]))
            smallest = right;
        if (smallest == current)
            break;
        swap_req(&h->array[current], &h->array[smallest]);
        current = smallest;
    }

    return (top);
}
