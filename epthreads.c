#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>

int how_much_are_ready; /*qtos ja estao prontos ?*/
char tipo_de_velocidade;
int d, n;
int **pista_de_corrida;

pthread_mutex_t start=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t launch_condition;
pthread_mutex_t *coordenador_de_acesso_a_estrada;

typedef struct v{
	int plano;
	int subida;
	int descida;
} vel;

void help(char *name){
   printf("USO: %s <nome do arquivo de entrada>\n", name);
}

void determina_velocidade(vel *velocidades){
	if(tipo_de_velocidade == 'U'){
		velocidades->plano = 50;
		velocidades->subida = 50;
		velocidades->descida = 50;
	}
	else{
		velocidades->plano = rand()%20+30;
		velocidades->subida = rand()%20;
		velocidades->descida = rand()%30+50;
	}
}

int tem_pista_livre(int posicao, int faixa_atual){
	int faixa;
	if(pista_de_corrida[posicao][faixa_atual] == -1)
		return faixa_atual;
	for (faixa = 1; faixa <= n; ++faixa) {
		if(pista_de_corrida[posicao][faixa]==-1)
			return faixa;
	}
	return 0;
}

int escreve_posicao_na_pista(int tid,int posicao,int faixa){
	pthread_mutex_lock(&coordenador_de_acesso_a_estrada[posicao]);
		  if((faixa = tem_pista_livre(posicao, faixa)) != 0){
			  pista_de_corrida[posicao][faixa] = tid;
			  printf("thread: %d , posicao %d faixa %d\n", tid, posicao, faixa);
			  	  fflush(stdout);
		  }
	pthread_mutex_unlock(&coordenador_de_acesso_a_estrada[posicao]);
	return faixa;
}

int retira_ciclista_da_posicao_se_ja_passou(int tid,int posicao,int faixa){
	 if(faixa != 0){
			  pthread_mutex_lock(&coordenador_de_acesso_a_estrada[posicao]);
			  pista_de_corrida[posicao][faixa] = -1;

			  pthread_mutex_unlock(&coordenador_de_acesso_a_estrada[posicao++]);
	 }
	 return posicao;
}
void verifica_velocidade_atual(vel velocidades,int posicao){
	if(pista_de_corrida[posicao][0]==(int)'D'){
		usleep(10000 - velocidades.descida*100);
		printf("descida\n");
	}
	if(pista_de_corrida[posicao][0]==(int)'P'){
		usleep(10000 - velocidades.plano*100);
		printf("plano\n");
	}
	if(pista_de_corrida[posicao][0]==(int)'S'){
		usleep(10000 - velocidades.subida*100);
		printf("subida\n");
	}

}
void comecou_corrida(int tid,vel velocidades){
	int posicao;
	int faixa;

	faixa = 1;
	  for (posicao = 0; posicao < d;) {
		  faixa = escreve_posicao_na_pista(tid,posicao,faixa);
		  verifica_velocidade_atual(velocidades,posicao);
		  posicao = retira_ciclista_da_posicao_se_ja_passou(tid,posicao,faixa);
	  }
}

void preparando_largada_do_ciclista(int tid){

	   pthread_mutex_lock(&start);
	   how_much_are_ready ++;
	   printf("\nciclista %d na linha de largada\n", tid);
	   if(how_much_are_ready == 50){
	      printf("\e[01;31musleep(10000 - velocidades.plano*100);PEDALA FIADAPORRA !\e[00m\n");
	      pthread_cond_broadcast(&launch_condition);
	   }
	   else
	      pthread_cond_wait(&launch_condition, &start);
	   pthread_mutex_unlock(&start);
	   printf("ciclista %d pedalando feito doido\n", tid);

}

void *tour_de_la_france(void *arg){
   int tid;
   vel velocidades;
   tid = *((int *)arg);

   determina_velocidade(&velocidades);
   printf("%d %d %d",velocidades.descida,velocidades.plano,velocidades.subida);
   preparando_largada_do_ciclista(tid);
   comecou_corrida(tid,velocidades);
   printf("chegueiiiiii %d\n",tid);

  return NULL;
}
void criando_a_pista(){
	int i,j;

	if((pista_de_corrida = malloc(d * sizeof(int*)))!=NULL){
		   for(i=0 ; i<d ; i++){
			   pista_de_corrida[i]=malloc((n+1) * sizeof(int));
		   }
	   }
	     else {
		printf("A pista esta sob obras! Corrida adiada para proxima tentativa de alocacao.");
		exit(1);
	   }
	   for (i = 0; i < d; i++) {
	  	   for (j = 1; j <= n; j++) {
	  		   pista_de_corrida[i][j] = -1;
	  	}
	  }
}

//allocando vetro de mutex para atualizar as posicoes na estrada
void criando_coordenador_das_posicoes(){
	int i;
	coordenador_de_acesso_a_estrada = (pthread_mutex_t *)malloc(d * sizeof (pthread_mutex_t));
	   if(coordenador_de_acesso_a_estrada != NULL){
		   for (i = 0; i < d; i++) {
			   if(pthread_mutex_init(coordenador_de_acesso_a_estrada + i, NULL)){
				   perror("Erro no vetor de mutex");
				   exit(1);
			   }
		   }
	   }
	   else{
		   perror("Erro de allocacao");
		   exit(1);
	   }
}
int main(int argc, char **argv){
   FILE *in;
   pthread_t *threads;
   int *thread_args, k;
   int i,j, m,inicio;
   char tipo_de_relevo;
   int trecho_a_percorrer;

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
   
   fscanf(in, "%d %d %c %d", &m, &n, &tipo_de_velocidade, &d);
   printf("ciclistas = %d\npistas = %d\ntipo = %c\ndistancia = %d\n\n", m, n, tipo_de_velocidade, d);

   criando_a_pista();
   criando_coordenador_das_posicoes();

   /*THREAD*/
   how_much_are_ready  = 0;
   /*inicializacao*/
   threads = (pthread_t *)malloc(m * sizeof(pthread_t));
   thread_args = (int *)malloc(m * sizeof(int));


   /*THREAD*/
   inicio=0;
   	while(1){
   		k = fscanf(in,"\n%c",&tipo_de_relevo);
   		k = fscanf(in,"\n%d",&trecho_a_percorrer);
   		if(k!=1)break;
   		printf("\ntipo_de_relevo: %c\ntrecho_a_percorrer: %d\n\n",tipo_de_relevo,trecho_a_percorrer);\

   		for (i=0; i < trecho_a_percorrer; ++i)
   			pista_de_corrida[i+inicio][0]=(int)tipo_de_relevo;

   		printf("\n\n");
   		inicio += trecho_a_percorrer;
   	}

   /*lancando threads*/
   srand(time(NULL));
   for(i = 0; i < m; i++){
	  thread_args[i] = i;
	  if(pthread_create(&threads[i], NULL, tour_de_la_france, &thread_args[i])){
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

   fclose(in);
   return 0;
}
