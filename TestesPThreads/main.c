/* [SO - 2013-1] Trabalho 1 - Isabella de Albuquerque, Renan Costa. */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>	/* Utilizada para contabilizar o tempo */
#include <math.h>


#define VALOR_MAX_MATRIZ 30000 /* Valor maximo de uma posicao qualquer da matriz */
#define SEED 10 /* Seed utilizada para gerar a matriz principal */
#define LINHAS 10000	/* Quantidade de linhas da matriz principal */
#define COLUNAS 10000	/* Quantidade de colunas da matriz principal */
#define NUM_LINHAS_BLOCO 100 /* Quantidade de linhas de cada macrobloco, precisa ser multiplo de LINHAS*/
#define NUM_COLUNAS_BLOCO 100 /* Quantidade de colunas de cada macrobloco, precisa ser multiplo de COLUNAS*/
#define THREADS 4	/* Quantidade de threads além da principal */

/* --- Prototipos de Funcoes --- */

/* Conta quantos primos existem em um  determinado bloco, que começa na linha e coluna passados como parâmetro */
void contaPrimoEmBloco(int linha, int coluna);

/* Funcao que verifica se um numero eh primo */
int ehPrimo(int num);

/* Funcao que inicializa a matriz de alocacao de blocos com zero, ou seja, todos os blocos estao livres */
void inicializarBlocos();

/* Trabalho a ser executado por uma thread */
void* thread(void* ptr);

/* Preenche a matriz principal com numeros aleatorios */
void preencher();

/* --- Variaveis utilizadas --- */

/* Mariz Principal */
int mat[LINHAS][COLUNAS];

/* Variavel global que serve para as threads registrarem
	a quantidade de numeros primos encontrados */
int qtdPrimos = 0;											

/* Matriz global que guarda as informacoes de 
	alocacao dos blocos da matriz principal.
	O valor 1 indica que bloco esta ocupado
	e o valor zero indica que esta livre. */
int blocos[LINHAS/NUM_LINHAS_BLOCO][COLUNAS/NUM_COLUNAS_BLOCO];  

/* Controla o acesso à matriz blocos */
pthread_mutex_t mutexBlocos;

 /* Controla o acesso à variável qtdPrimos */
pthread_mutex_t mutexPrimo;


/* --- Programa Principal --- */
int main(){

	pthread_t threads [THREADS];

	int i, j, qtdPrimosSequencial = 0;
	double tempoSequencial, tempoThreads;
	clock_t tsInicio, tsFim, ttInicio, ttFim;
	
	srand(SEED);
	preencher();
	
	/* Thread Principal - Execucao Sequencial */
	printf("\nExecucao Sequencial:\n\n");
	tsInicio = clock();
	for(i = 0; i<LINHAS; i++){
		for(j = 0; j<COLUNAS; j++){
			if(ehPrimo(mat[i][j])){
				qtdPrimosSequencial++;
			}
		}
	}
	tsFim = clock();
	tempoSequencial = (double)(tsFim-tsInicio)/CLOCKS_PER_SEC;
	printf("Total de numeros primos encontrados: %d\n", qtdPrimosSequencial);
	printf("Tempo total gasto nesta tarefa: %f segundos.\n\n", tempoSequencial);
	
	
	/* Threads Secundárias - Execucao em paralelo */
	printf("\nExecucao com Threads:\n\n");
	inicializarBlocos();
	pthread_mutex_init(&mutexPrimo, NULL);
	pthread_mutex_init(&mutexBlocos, NULL);
	ttInicio = clock();

	for(i = 0; i < THREADS; i++){
		if (pthread_create(&threads[i], NULL, thread, NULL) != 0)  {
			perror("Pthread_create falhou.");
			exit(1); 
		}
	}
	for (i = 0; i < THREADS; i++){
		if (pthread_join(threads[i], NULL) != 0) {
			perror("Pthread_join falhou.");
			exit(1); 
		}
	}

	ttFim = clock();
	tempoThreads = (double)(ttFim-ttInicio)/CLOCKS_PER_SEC;
	printf("Total de numeros primos encontrados: %d\n", qtdPrimos);
	printf("Tempo total gasto nesta tarefa: %f segundos.\n\n", tempoThreads);
	
	
	system("pause");
	return 0;
}


/* --- Corpos de Funcoes --- */

/* Funcao que verifica se um numero eh primo */
int ehPrimo(int num){
	int i;
	int n;

	if(num <= 1) return 0;
	if(num == 2) return 1;

	n = sqrt((double)num);

	for(i = 3; i < n; i = i + 2)
		if((num%i) == 0)
			return 0;

	return 1;
}


/* Funcao que inicializa a matriz de alocacao de blocos com zero */
void inicializarBlocos(){
	int i, j;
	for(i = 0; i<(LINHAS/NUM_LINHAS_BLOCO); i++){
		for(j = 0; j<(COLUNAS/NUM_COLUNAS_BLOCO); j++){
			blocos[i][j] = 0;
		}
	}
}

/* Trabalho a ser executado por uma thread */  //PRECISA COMPLETAR
void* thread(void* ptr){
   
	int i, j, devoContar;

	for(i = 0; i < LINHAS/NUM_LINHAS_BLOCO; i++){
		for(j = 0; j < COLUNAS/NUM_COLUNAS_BLOCO; j++){
			devoContar = 0;
			pthread_mutex_lock(&mutexBlocos);
			if(blocos[i][j] == 0){
				blocos[i][j] = 1;
				devoContar = 1;
			}
			pthread_mutex_unlock(&mutexBlocos);
			if(devoContar){
				contaPrimoEmBloco(i, j);			
			}
				
		}
	}

	return NULL;
}

/* Conta quantos primos existem em um  determinado bloco, que começa na linha e coluna passados como parâmetro */
void contaPrimoEmBloco(int linha, int coluna){

	int i, j, primosLocal;

	primosLocal = 0;

	for(i = linha * NUM_LINHAS_BLOCO; i < (linha + 1) * NUM_LINHAS_BLOCO; i++){
		for(j = coluna * NUM_COLUNAS_BLOCO; j < (coluna + 1) * NUM_COLUNAS_BLOCO; j++){
			if(ehPrimo(mat[i][j])){
				primosLocal++;
			}
				
		}
	}

	pthread_mutex_lock(&mutexPrimo);
	qtdPrimos += primosLocal;
	pthread_mutex_unlock(&mutexPrimo);

}

/* Preenche a matriz principal com numeros aleatorios */
void preencher(){

	int i, j;

	for(i = 0; i<LINHAS; i++){
		for(j = 0; j<COLUNAS; j++){
			mat[i][j] = rand() % VALOR_MAX_MATRIZ;
		}
	}
}