#include <time.h>
#include "../include/memory.h"

int main(int argc, char *argv[])
{
	if (argc < 5){
		printf("./tp2virtual <politica_subs> <arquivo> <tamanhopagina>  <tamanhoMemoria>");
		exit(1);
	}

	printf("Iniciando execução...\n");
	printf("Arquivo de entrada: %s\n", argv[2]);
	printf("Tamanho da memória: %s KB\n", argv[4]);
	printf("Tamanho da página: %s KB\n", argv[3]);
	printf("Método de substituição: %s\n", argv[1]);

	// Cria e inicializa a memoria 
	Memory *mem = createMem(atoi(argv[3]), atoi(argv[4]));

	// Executa as entradas dos logs 
	execMem(mem, argv[2], argv[1]);

	// Faz o relatorio da execução
	report(mem);

	// Libera as estruturas
	freeMem(mem);

	return 0;
}