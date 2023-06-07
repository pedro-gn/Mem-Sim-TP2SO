#include "../include/memory.h"

//-----------------------------------------CONSTRUÇÃO E DESCONSTRUÇÃO-------------------------------------------------------

//Acha quantos bits serao descartados
int calcS (int pageSize){
	unsigned temp;
	temp = pageSize * 1024;
	unsigned s = 0;
	while(temp > 1){
		temp = temp >> 1;
		s++;
	}
	return s;
}

// Cria a memoria 
Memory* createMem(int pageSize, int memory_size){
	Memory *vir_mem;
	vir_mem = (Memory*) malloc(sizeof(Memory));
	
	//Iniciando alguns valores.
	vir_mem->pageSize = pageSize;
	vir_mem->memSize = memory_size;
	vir_mem->maxFramesN = memory_size/pageSize;
	vir_mem->occupiedFramesN = 0;
	vir_mem->dataNum = 0;
	vir_mem->pageFaults = 0;
	vir_mem->dirtyBits = 0;
	vir_mem->readBits = 0;
	vir_mem->writtenBits = 0;
	vir_mem->s = calcS(vir_mem->pageSize);
	
	//Criando os Page Frames e iniciando todos como vazio.
	vir_mem->pFrames = malloc(vir_mem->maxFramesN * sizeof(Page));
	for (int i = 0; i < vir_mem->maxFramesN; i++){
		vir_mem->pFrames[i].virtualId = -1;
		vir_mem->pFrames[i].wasRead = 0;
		vir_mem->pFrames[i].wasWritten = 0;
		vir_mem->pFrames[i].recUsed = 0;
		vir_mem->pFrames[i].refBit = 0;
	}
	
	return vir_mem;
}

// Limpa a memoria alocada para classe memoria
void freeMem (Memory *mem){
	free(mem->pFrames);
	free(mem);
}


//-----------------------------------ALGORITMOS DE SUBSTITUIÇÃO-------------------------------

// Algoritmo NRU (Not Recently Used) para substituição de páginas
int NRU(Memory* mem, int size) {
    int element = -1;
    int class = -1;
  
    // Primeira classe: R = 0, M = 0
    for (int i = 0; i < size; i++) {
        if (mem->pFrames[i].refBit == 0 && mem->pFrames[i].wasWritten == 0) {
            element = i;
            class = 0;
            break;
        }
    }
  
    // Segunda classe: R = 0, M = 1
    if (class == -1) {
        for (int i = 0; i < size; i++) {
            if (mem->pFrames[i].refBit == 0 && mem->pFrames[i].wasWritten == 1) {
                element = i;
                class = 1;
                break;
            }
        }
    }
  
    // Terceira classe: R = 1, M = 0
    if (class == -1) {
        for (int i = 0; i < size; i++) {
            if (mem->pFrames[i].refBit == 1 && mem->pFrames[i].wasWritten == 0) {
                element = i;
                class = 2;
                break;
            }
        }
    }
  
    // Quarta classe: R = 1, M = 1
    if (class == -1) {
        for (int i = 0; i < size; i++) {
            if (mem->pFrames[i].refBit == 1 && mem->pFrames[i].wasWritten == 1) {
                element = i;
                class = 3;
                break;
            }
        }
    }
  
    // Limpa o bit de referência de todos os frames
    for (int i = 0; i < size; i++) {
        mem->pFrames[i].refBit = 0;
    }
  
    if (element == -1) {
        // Se não encontrar nenhum frame, retorna um valor inválido
        return -1;
    }
  
    // Atualiza o bit de referência do frame selecionado
    mem->pFrames[element].refBit = 1;
  
    if (mem->pFrames[element].wasWritten == 1) {
        mem->dirtyBits++;
    }
  
    return element;
}

// Algoritmo LRU (Least Recently Used) para substituição de páginas
int LRU(Memory *mem,int size){
	long long min = 100000000;
	int element = 0;
	for(int i=0;i<size;i++){
		if(mem->pFrames[i].recUsed < min){
			min = mem->pFrames[i].recUsed;
			element = i;
		}
	}
	if(mem->pFrames[element].wasWritten == 1){
		mem->dirtyBits++;
	}
	return element;
}

// Algoritmo Second Chance para substituição de páginas
int secondChance(Memory *mem) {
    int element = 0;
    int firstElement = 0;
    int value = 1;
    int size = mem->maxFramesN;
    int *visited = malloc(size * sizeof(int));

    for (int i = 0; i < size; i++) {
        visited[i] = 0;
    }

    while (value) {
        if (mem->pFrames[element].refBit == 0 && visited[element] == 0) {
            value = 0;
        } else if (mem->pFrames[element].refBit == 0 && visited[element] == 1) {
            mem->pFrames[element].refBit = 0;
            visited[element] = 0;
        } else if (mem->pFrames[element].refBit == 1) {
            mem->pFrames[element].refBit = 0;
            visited[element] = 1;
        }

        element = (element + 1) % size;
    }

    firstElement = (element + 1) % size;
    if (mem->pFrames[element].wasWritten == 1) {
        mem->dirtyBits++;
    }

    free(visited);
    return element;
}

// Função para escolher o algoritmo de substituição com base no nome fornecido
int algoChooser(Memory *mem, int size, char *n){
	if(strcmp(n,"lru") == 0){
		return LRU(mem,size);
	}
	else if(strcmp(n,"segunda_chance") == 0){
		return secondChance(mem);
	}
	else if(strcmp(n,"nru") == 0){
		return NRU(mem, size);
	}else{
		printf("Politica Invalida!");
		exit(1);
	}
}


//-----------------------------------------------ALGORITMO DE EXECUÇÃO-----------------------------------


//Função que retorna o índice do frame que possui o endereço virtual dado
int frameIndex (Memory *mem, int virtualId){
	for (int i = 0; i < mem->occupiedFramesN; i++){
		if (mem->pFrames[i].virtualId == virtualId){
			return i;
		}
	}
	return -1;
}

// Executa cada endereço no arquivo de log
void execMem (Memory *mem, char *file, char *algo){
	unsigned address;
	char rw;
	int i = 0;
	long long used = 0;
	unsigned page_id;
	int frame;
	
	FILE *arq;
	arq = fopen(file,"r");
	if(arq == NULL){
		printf("Arquivo nao encontrado");
		exit(EXIT_FAILURE);
	}

	while(fscanf(arq, "%x %c", &address, &rw) != EOF){
		
		page_id = address >> mem->s;
		frame = frameIndex(mem, page_id);
		
		//printf("Endereço: %x\n", address);
		
		if(frame == -1){
			//Page fault
			mem->pageFaults++;
			//Se tem espaço na tabela, adiciona
			if(mem->occupiedFramesN < mem->maxFramesN){
				mem->pFrames[i].virtualId = page_id;
				mem->pFrames[i].wasRead = 1;
				mem->pFrames[i].recUsed = used;
				if(rw == 'W'){
					mem->pFrames[mem->occupiedFramesN].wasWritten = 1;
					mem->writtenBits++;
				}else{
					mem->readBits++;
				}
				mem->occupiedFramesN++;
				i++;
			}
			//Se não tem mais espaço, escolhe uma página para ser substituída 
			else{
				frame = algoChooser(mem,mem->maxFramesN,algo);
				mem->pFrames[frame].wasRead = 0;
				mem->pFrames[frame].wasWritten = 0;
				
				mem->pFrames[frame].virtualId = page_id;
				mem->pFrames[frame].wasRead = 1;
				mem->pFrames[frame].recUsed = used;
				if(rw == 'W'){
					mem->pFrames[frame].wasWritten = 1;
					mem->writtenBits++;
				}else{
					mem->readBits++;
				}
			}
		}else{
			//Achou página
			mem->pFrames[frame].wasRead = 1;
			mem->pFrames[frame].recUsed = used;
			mem->pFrames[frame].refBit = 1;
			if(rw == 'W'){
				mem->pFrames[frame].wasWritten = 1;
				mem->writtenBits++;
			}else{
			mem->readBits++;
			}
		}
		used++;
	} 
	mem->dataNum = used;
	fclose(arq);
}

// Relatorio
void report(Memory *mem){
	// Relatório
	printf("Total de acessos: %d\n", mem->dataNum);
	printf("Page faults: %lld\n", mem->pageFaults);
	printf("Dirty pages: %lld\n", mem->dirtyBits);
	printf("Memoria lida: %lld\n", mem->readBits);
	printf("Memoria escrita: %lld\n", mem->writtenBits);
}