#ifndef TSP_TSP_H
#define TSP_TSP_H

/* dernier minimum trouv� */
extern int minimum;

int present (int city, int hops, tsp_path_t path);
void tsp (int hops, int len, tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len, int *local_minimum);
//void *tsp (void *args);
void *tsp_thread (void *args);

#endif
