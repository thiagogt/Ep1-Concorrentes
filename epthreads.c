#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>

int how_much_are_ready; /*qtos ja estao prontos ?*/
pthread_mutex_t start=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t launch_condition;



void help(char *name){
   printf("USO: %s <nome do arquivo de entrada>\n", name);
}

void *rider(void *arg){
   int tid;
   
   tid = *((int *)arg);

   pthread_mutex_lock(&start);
   how_much_are_ready ++;
   printf("ciclista %d na linha de largada\n", tid);
   if(how_much_are_ready == 50){
      printf("\e[01;31mPEDALA FIADAPORRA !\e[00m\n");
      pthread_cond_broadcast(&launch_condition);
   }
   else
      pthread_cond_wait(&launch_condition, &start);
   pthread_mutex_unlock(&start);
   printf("ciclista %d pedalando feito doido\n", tid);

   return NULL;
}

int main(int argc, char **argv){
   FILE *in;
   pthread_t *threads;
   int *thread_args;
   int i, m, n, d;
   char s;
   
   /*arquivo de entrada*/
   if(argc != 2){
      help(argv[0]);
      exit(1);
   }
   
   in = fopen(argv[1], "r");
   if(in == NULL){
      printf("Erro ao abrir o arquivo \"%s\"\n", argv[1]);
      exit(1);
   }
   
   fscanf(in, "%d %d %c %d", &m, &n, &s, &d);
   printf("ciclistas = %d\npistas = %d\ntipo = %c\ndistancia = %d\n\n", m, n, s, d);
   /*arquivo de entrada*/
   
   /*THREAD*/
   how_much_are_ready  = 0;
   /*inicializacao*/
   threads = (pthread_t *)malloc(m * sizeof(pthread_t));
   thread_args = (int *)malloc(m * sizeof(int));

   /*lancando threads*/
   srand(time(NULL));
   for(i = 0; i < m; i++){
      thread_args[i] = i;
      if(pthread_create(&threads[i], NULL, rider, &thread_args[i])){
         perror("ciclista nao veio");
         exit(1);
      }
      /*printf("thread %u lancada\n", threads[i]);*/
      printf("ciclista %d pronto !\n", i);
   }

   printf("[%d]\n", how_much_are_ready); 


   /*esperando threads terminar*/
   for(i = 0; i < m; i++){
      if(pthread_join(threads[i], NULL))
         perror("ciclista morreu no caminho");
   }
   /*THREAD*/
   fclose(in);

   return 0;
}
