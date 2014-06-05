#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

#include "tsp-types.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"
#include "tsp-job.h"

struct arg_struct {
   struct tsp_queue *queue;
   long long int *cuts;
   tsp_path_t *sol;
   int *sol_len;
 };

/* dernier minimum trouvé */
int minimum;

pthread_mutex_t mutex_job;
pthread_mutex_t mutex_tsp;

/* résolution du problème du voyageur de commerce */
int present (int city, int hops, tsp_path_t path)
{
    for (int i = 0; i < hops; i++) {
        if (path [i] == city) {
            return 1;
        }
    }
    
    return 0 ;
}

void *tsp_thread(void *args)
{
  struct arg_struct *data = args;
  
  struct tsp_queue *q = data->queue;
  tsp_path_t path;
  long long int *cuts = data->cuts;
  tsp_path_t *sol = data->sol;
  int *sol_len = data->sol_len;

  while(!empty_queue(q)) {
    int hops = 0, len = 0; 

    pthread_mutex_lock(&mutex_job);
    get_job(q, path, &hops, &len);
    int local_sol_len = *sol_len;
    long long int local_cuts = *cuts;
    int local_minimum = minimum;
    pthread_mutex_unlock(&mutex_job);

    tsp_path_t local_sol;
    tsp(hops, len, path, &local_cuts, local_sol, &local_sol_len, &local_minimum);

    pthread_mutex_lock(&mutex_tsp);
    if(local_minimum < minimum) {
      printf("JE SUIS DANS LE IF COUCOU\n");
      minimum = local_minimum;
      *sol_len = local_sol_len;
      memcpy(sol, local_sol, nb_towns*sizeof(int));
    }
    *cuts += local_cuts;
    pthread_mutex_unlock(&mutex_tsp);
  }
  return NULL;
}

void tsp (int hops, int len, tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len, int *local_minimum)
{
    if (len + cutprefix[(nb_towns-hops)] >= *local_minimum) {
      (*cuts)++ ;
      return;
    }
    
    if (hops == nb_towns) {
	    int me = path [hops - 1];
	    int dist = distance[me][0]; // retourner en 0

      if ( len + dist < *local_minimum ) {
		    *local_minimum = len + dist;
		    *sol_len = len + dist;
        memcpy(sol, path, nb_towns*sizeof(int));
		    print_solution (path, len+dist);
	    }
    } else {
        int me = path [hops - 1];        
        for (int i = 0; i < nb_towns; i++) {
            if (!present (i, hops, path)) {
                path[hops] = i;
                int dist = distance[me][i];

                tsp (hops + 1, len + dist, path, cuts, sol, sol_len, local_minimum);
            }
        }
    }
}

