#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <unistd.h>
# include <string.h>

// Forward declaration bach n9dro nkhdmo biha f t_coder
typedef struct s_sim t_sim;

typedef struct s_request {
    int         coder_id;
    long long   priority;
    int         tie_breaker;
} t_request;

// Hadi hia struct dyal l'Heap (sanda9a li jam3a l'file d'attente)
typedef struct s_heap {
    t_request   *array;
    int         capacity;
    int         size;
} t_heap;

// Struct dial l'clé USB (Dongle)
typedef struct s_dongle {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    long long       available_at;
    int				is_held; 
	t_heap			*wait_queue;
} t_dongle;

// Struct dial l'Coder
typedef struct s_coder {
    int             id;
    int             compile_count;
    long long       last_compile; // Wa9t akhir mra bda l'compile
    pthread_t       thread_id;
    t_dongle        *left_dongle;
    t_dongle        *right_dongle;
    t_sim           *sim; // Pointer l struct principale bach y9ra les args
} t_coder;

// Struct principale dial Simulation (fiha l'args w state global)
typedef struct s_sim {
    int             nb_coders;
    int             t_burnout;
    int             t_compile;
    int             t_debug;
    int             t_refactor;
    int             req_compiles;
    int             cooldown;
    int             scheduler; // 0 l fifo, 1 l edf mathalan

    int             is_dead; // Flag (0 wla 1) bach n3rfo wach chi wa7d dar burnout
    long long       start_time; // Wa9t lbdya dial simulation

    pthread_mutex_t write_mutex; // Bach n-protegiw printf (logs)
    pthread_mutex_t death_mutex; // Bach n-protegiw is_dead w last_compile

    t_dongle        *dongles; // Tableau dial les dongles
    t_coder         *coders; // Tableau dial les coders
} t_sim;



long long   get_time(void);
void        custom_sleep(long long time_in_ms, t_sim *sim);
t_heap *init_heap(int max_capacity);
int is_higher_priority(t_request req1, t_request req2);
void heap_insert(t_heap *h, t_request req);
t_request heap_extract(t_heap *h);
void	print_status(t_coder *coder, char *status);
void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);
void	acquire_dongle(t_coder *coder, t_dongle *dongle);
void	release_dongle(t_dongle *dongle, t_sim *sim);

void	clean_simulation(t_sim *sim);

#endif