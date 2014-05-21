#include <assert.h>
#include <string.h>
#include <pthread.h>

#include "tsp-types.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"
#include "tsp-job.h"

struct arg_struct {
   struct tsp_queue *queue;
   tsp_path_t path;
   long long int *cuts;
   tsp_path_t sol;
   int *sol_len;
 };

/* dernier minimum trouv� */
int minimum;

pthread_mutex_t mutex_job;
pthread_mutex_t mutex_tsp;

/* r�solution du probl�me du voyageur de commerce */
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
  memcpy(path, data->path, sizeof(tsp_path_t));
  long long int *cuts = data->cuts;
  tsp_path_t sol;
  memcpy(sol, data->sol, sizeof(tsp_path_t));
  int *sol_len = data->sol_len;

  while(!empty_queue(q)) {
    int hops = 0, len = 0; 

    pthread_mutex_lock(&mutex_job);
    get_job(q, path, &hops, &len);
    pthread_mutex_unlock(&mutex_job);

    tsp(hops, len, path, cuts, sol, sol_len);
  }
  return NULL;
}

void tsp (int hops, int len, tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len)
{
    if (len + cutprefix[(nb_towns-hops)] >= minimum) {
      
      pthread_mutex_lock(&mutex_tsp);
      (*cuts)++ ;
      pthread_mutex_unlock(&mutex_tsp);
    }
    
    if (hops == nb_towns) {
	    int me = path [hops - 1];
	    int dist = distance[me][0]; // retourner en 0
            if ( len + dist < minimum ) {
		    minimum = len + dist;
        
        pthread_mutex_lock(&mutex_tsp);
		    *sol_len = len + dist;
        pthread_mutex_unlock(&mutex_tsp);
		    
        memcpy(sol, path, nb_towns*sizeof(int));
		    print_solution (path, len+dist);
	    }
    } else {
        int me = path [hops - 1];        
        for (int i = 0; i < nb_towns; i++) {
            if (!present (i, hops, path)) {
                path[hops] = i;
                int dist = distance[me][i];

                tsp (hops + 1, len + dist, path, cuts, sol, sol_len);
            }
        }
    }
}

