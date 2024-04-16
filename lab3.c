#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

// Estrutura para passar argumentos para a função da thread
typedef struct
{
  float *mat_a;
  float *mat_b;
  float *mat_result;
  int rows;
  int cols;
  int start_row;
  int end_row;
} ThreadArgs;

// Função para multiplicar uma parte das matrizes
void *multiply(void *arguments)
{
  ThreadArgs *args = (ThreadArgs *)arguments;
  for (int i = args->start_row; i < args->end_row; i++)
  {
    for (int j = 0; j < args->cols; j++)
    {
      float sum = 0.0;
      for (int k = 0; k < args->cols; k++)
      {
        sum += args->mat_a[i * args->cols + k] * args->mat_b[k * args->cols + j];
      }
      args->mat_result[i * args->cols + j] = sum;
    }
  }
  pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  if (argc != 5)
  {
    printf("Uso: %s <arquivo_matriz_a> <arquivo_matriz_b> <arquivo_saida> <threads>\n", argv[0]);
    return 1;
  }

  // Tempo de inicialização
  struct timeval start, end;
  gettimeofday(&start, NULL);

  // Carregar argumentos da linha de comando
  char *file_a = argv[1];
  char *file_b = argv[2];
  char *file_out = argv[3];
  int num_threads = atoi(argv[4]);

  // Abrir arquivos de entrada
  FILE *input_a = fopen(file_a, "rb");
  FILE *input_b = fopen(file_b, "rb");

  if (!input_a || !input_b)
  {
    printf("Erro ao abrir arquivos de entrada.\n");
    return 1;
  }

  // Ler dimensões das matrizes
  int rows_a, cols_a, rows_b, cols_b;
  fread(&rows_a, sizeof(int), 1, input_a);
  fread(&cols_a, sizeof(int), 1, input_a);
  fread(&rows_b, sizeof(int), 1, input_b);
  fread(&cols_b, sizeof(int), 1, input_b);

  if (cols_a != rows_b)
  {
    printf("Impossível multiplicar as matrizes: número de colunas da primeira matriz não é igual ao número de linhas da segunda matriz.\n");
    return 1;
  }

  // Alocar memória para matrizes
  float *mat_a = (float *)malloc(rows_a * cols_a * sizeof(float));
  float *mat_b = (float *)malloc(rows_b * cols_b * sizeof(float));
  float *mat_result = (float *)malloc(rows_a * cols_b * sizeof(float));

  // Ler matrizes dos arquivos
  fread(mat_a, sizeof(float), rows_a * cols_a, input_a);
  fread(mat_b, sizeof(float), rows_b * cols_b, input_b);

  // Fechar arquivos de entrada
  fclose(input_a);
  fclose(input_b);

  // Tempo de processamento
  gettimeofday(&end, NULL);
  double init_time = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000;

  gettimeofday(&start, NULL);

  // Inicializar threads
  pthread_t threads[num_threads];
  ThreadArgs thread_args[num_threads];

  int rows_per_thread = rows_a / num_threads;
  for (int i = 0; i < num_threads; i++)
  {
    thread_args[i].mat_a = mat_a;
    thread_args[i].mat_b = mat_b;
    thread_args[i].mat_result = mat_result;
    thread_args[i].rows = rows_a;
    thread_args[i].cols = cols_b;
    thread_args[i].start_row = i * rows_per_thread;
    thread_args[i].end_row = (i == num_threads - 1) ? rows_a : (i + 1) * rows_per_thread;

    pthread_create(&threads[i], NULL, multiply, (void *)&thread_args[i]);
  }

  // Aguardar threads
  for (int i = 0; i < num_threads; i++)
  {
    pthread_join(threads[i], NULL);
  }

  // Tempo de finalização
  gettimeofday(&end, NULL);
  double process_time = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000;

  gettimeofday(&start, NULL);

  // Escrever matriz resultante no arquivo de saída
  FILE *output = fopen(file_out, "wb");
  if (!output)
  {
    printf("Erro ao abrir arquivo de saída.\n");
    return 1;
  }

  fwrite(&rows_a, sizeof(int), 1, output);
  fwrite(&cols_b, sizeof(int), 1, output);
  fwrite(mat_result, sizeof(float), rows_a * cols_b, output);

  fclose(output);

  // Liberar memória alocada
  free(mat_a);
  free(mat_b);
  free(mat_result);

  // Tempo total
  gettimeofday(&end, NULL);
  double write_time = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000;
  double total_time = init_time + process_time + write_time;

  printf("%d\t%s\t%.6f\t%.6f\t%.6f\t%.6f\n", rows_a, argv[4], init_time, process_time, write_time, total_time);
  /*printf("Tamanho da Matriz: %d\n", rows_a);
  printf("Número de Threads: %s\n ", argv[4]);
  printf("Tempo de inicialização: %.6f segundos\n", init_time);
  printf("Tempo de processamento: %.6f segundos\n", process_time);
  printf("Tempo de escrita: %.6f segundos\n", write_time);
  printf("Tempo total: %.6f segundos\n", total_time); */

  return 0;
}
