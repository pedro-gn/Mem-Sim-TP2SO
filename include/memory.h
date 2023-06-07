#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Quadro de páginas
typedef struct PageFrame{
	int virtualId;
	int wasRead;     //Se foi lida (0 ou 1)
	int wasWritten;  //Se foi escrita (0 ou 1)
	long long recUsed ; // acessado recentemente
	int refBit; // Um bit de referencia (0 ou 1)
}Page;

// Memória
typedef struct VirtualMemory{ 
	Page *pFrames;    //Conjunto de páginas, tem tamanho de "maxFramesN"
	int dataNum;      //Quantidade de enderecos utilizados.
	int maxFramesN;
	int pageSize;      //Tamanho da página em KB
	int memSize;       //Tamanho da memória em KB
	int s;             //Quantidade de bits a serem descartados
	int occupiedFramesN;
	long long pageFaults; 
	long long dirtyBits;
	long long readBits;
	long long writtenBits;
}Memory;

//Funções de construção
int calcS (int pageSize);
Memory* createMem(int pageSize, int memory_size);
void freeMem (Memory *mem);

//Algoritmos de substituição
int algoChooser(Memory *mem, int size, char *n);
int NRU(Memory* mem, int size);
int LRU(Memory *mem,int size);
int secondChance(Memory *mem);

//Funções para rodar memória
int frameIndex (Memory *mem, int virtualId);
void execMem (Memory *mem, char *file, char *algo);

// Relatorio
void report(Memory *mem);
#endif