/*
 * PROJETO DE TEORIA DOS GRAFOS - PARTE 2
 * MODELAGEM DE BAIRRO E CÁLCULO DE PASSEIO
 *
 * Diego Spagnuolo Sugai - RA 10417329
 * Kaue Henrique Matias Alves - RA: 10417894
 * Victor Maki tarcha - RA 10419861
 * Marcos Arambasic - RA 10443260
 */

// Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <memory.h>
#include <string.h> // Necessário para strcmp

#define BRANCO 0
#define CINZA  1
#define PRETO  2

/*
 * Estrutura de dados para representar grafos
 */
typedef struct a{ /* Celula de uma lista de arestas */
    int    extremo2;
    int    peso;     // Na Parte 2, 'peso' é a distância em METROS
    struct a *prox;
} Arest;

typedef struct v{  /* Cada vertice tem um ponteiro para uma lista de arestas incidentes nele */
    int nome;
    int cor;
    Arest *prim;
} Vert;

// --- PARTE 2 ---
// Estrutura de Localidades atualizada conforme requisito do PDF
typedef struct {
    const char* nome; // Nome do local, ex: "Padaria"
    int vert1;        // Um dos vértices da rua (esquina 1)
    int vert2;        // O outro vértice da rua (esquina 2)
    int dist_vert1;   // Distância em metros do local até o vert1
    int dist_vert2;   // Distância em metros do local até o vert2
} Localidades;


/*
 * Declarações das Funções
 */
void criaGrafo(Vert **G, int ordem);
void destroiGrafo(Vert **G, int ordem);
int  acrescentaAresta(Vert G[], int ordem, int vert1, int vert2, int peso);
void imprimeGrafo(Vert G[], int ordem);

// --- PARTE 2 ---
// Funções novas e modificadas para a Parte 2
int calculaDistanciaModificado(Vert G[], int ordem, int origem, int destino, int **caminho_vertices, int *tamanho_caminho);
int getDistanciaEntrePOIs(Vert G[], int ordem, Localidades poi_A, Localidades poi_B, int **melhor_caminho, int *tam_melhor_caminho);
void calcularPasseio(Vert G[], int ordem, Localidades minha_casa, Localidades locais[], int num_locais);
void encontrarPOIsNoCaminho(int v1, int v2, Localidades locais_todos[], int num_locais_todos, const char* nome_destino);

/*
 * Implementação das Funções (Básicas)
 */

void criaGrafo(Vert **G, int ordem){
    int i;
    *G = malloc(sizeof(Vert) * ordem);  
    if (*G == NULL) {
        perror("Erro ao alocar memoria para o grafo");
        exit(1);
    }
    for(i=0; i<ordem; i++){
        (*G)[i].nome = i;
        (*G)[i].cor = BRANCO;
        (*G)[i].prim = NULL;
    }
}

void destroiGrafo(Vert **G, int ordem){
    int i;
    Arest *a, *n;
    for(i=0; i<ordem; i++){
        a = (*G)[i].prim;
        while (a!= NULL){
            n = a->prox;
            free(a);
            a = n;
        }
    }
    free(*G);
}

int acrescentaAresta(Vert G[], int ordem, int vert1, int vert2, int peso){
    Arest *A1, *A2;
    if (vert1<0 || vert1 >= ordem || vert2<0 || vert2 >= ordem) return 0;

    A1 = (Arest *) malloc(sizeof(Arest));
    A1->extremo2 = vert2;
    A1->peso = peso;
    A1->prox = G[vert1].prim;
    G[vert1].prim = A1;

    if (vert1 == vert2) return 1;

    A2 = (Arest *) malloc(sizeof(Arest));
    A2->extremo2 = vert1;
    A2->peso = peso;
    A2->prox = G[vert2].prim;
    G[vert2].prim = A2;
    return 1;
}

void imprimeGrafo(Vert G[], int ordem){
    int i;
    Arest *aux;
    printf("\nOrdem:    %d",ordem);
    printf("\nLista de Adjacencia:\n");
    for (i=0; i<ordem; i++){
        printf("\n    v%d: ", i); 
        aux = G[i].prim;
        for( ; aux != NULL; aux = aux->prox)
            printf("  v%d(dist:%dm)", aux->extremo2, aux->peso); // Atualizado para 'm' (metros)
    }
    printf("\n\n");
}

// --- PARTE 2 ---
// Dijkstra modificado para retornar o caminho (sequência de vértices)
// e a distância.
int calculaDistanciaModificado(Vert G[], int ordem, int origem, int destino, int **caminho_vertices, int *tamanho_caminho) {
    int *dist = (int *) malloc(ordem * sizeof(int));
    int *visitado = (int *) malloc(ordem * sizeof(int));
    int *predecessor = (int *) malloc(ordem * sizeof(int)); // Array para guardar o caminho
    int i, u, count;

    for (i = 0; i < ordem; i++) {
        dist[i] = INT_MAX;
        visitado[i] = 0;
        predecessor[i] = -1; // -1 indica sem predecessor
    }
    
    dist[origem] = 0;

    for (count = 0; count < ordem; count++) {
        
        int min = INT_MAX, min_index = -1;
        for (i = 0; i < ordem; i++) {
            if (visitado[i] == 0 && dist[i] <= min) {
                min = dist[i];
                min_index = i;
            }
        }

        if(min_index == -1) break; 
        
        u = min_index;
        visitado[u] = 1;
        
        if (u == destino) break; // Encontrou o destino
        
        Arest *adj = G[u].prim;
        
        while (adj != NULL) {
            int v = adj->extremo2;
            int peso = adj->peso;

            if (!visitado[v] && dist[u] != INT_MAX && dist[u] + peso < dist[v]) {
                dist[v] = dist[u] + peso;
                predecessor[v] = u; // Guarda o predecessor
            }
            adj = adj->prox;
        }
    }

    int distanciaFinal = dist[destino];
    
    // --- Reconstrução do Caminho ---
    if (distanciaFinal != INT_MAX) {
        int *caminho_reverso = (int *) malloc(ordem * sizeof(int));
        int atual = destino;
        int tam = 0;
        while(atual != -1) {
            caminho_reverso[tam++] = atual;
            atual = predecessor[atual];
        }
        
        // Inverte o caminho para ter a ordem correta (Origem -> Destino)
        *caminho_vertices = (int *) malloc(tam * sizeof(int));
        for(i = 0; i < tam; i++) {
            (*caminho_vertices)[i] = caminho_reverso[tam - 1 - i];
        }
        *tamanho_caminho = tam;
        
        free(caminho_reverso);
    } else {
        *caminho_vertices = NULL;
        *tamanho_caminho = 0;
    }
    
    free(dist);
    free(visitado);
    free(predecessor); // Libera o array de predecessores
    
    return distanciaFinal;
}

// --- PARTE 2 ---
/*
 * Calcula a distância real entre dois locais (POIs),
 * já incluindo as distâncias parciais nas ruas.
 * Testa as 4 combinações de vértices e retorna o menor caminho.
 */
int getDistanciaEntrePOIs(Vert G[], int ordem, Localidades poi_A, Localidades poi_B, int **melhor_caminho, int *tam_melhor_caminho) {
    
    int dist_total = INT_MAX;
    // Libera o ponteiro *melhor_caminho* ANTES de usá-lo, 
    // pois ele é reutilizado em loop e pode conter lixo/ponteiro antigo
    if(*melhor_caminho) free(*melhor_caminho);
    *melhor_caminho = NULL; // Garante que esteja limpo
    *tam_melhor_caminho = 0;

    int *caminho_atual = NULL;
    int tam_caminho_atual = 0;

    // --- Cenário 1: A(via vA1) -> ... -> B(via vB1) ---
    int dist_vA1_vB1 = calculaDistanciaModificado(G, ordem, poi_A.vert1, poi_B.vert1, &caminho_atual, &tam_caminho_atual);
    if (dist_vA1_vB1 != INT_MAX) {
        int dist_c1 = poi_A.dist_vert1 + dist_vA1_vB1 + poi_B.dist_vert1;
        if (dist_c1 < dist_total) {
            dist_total = dist_c1;
            if(*melhor_caminho) free(*melhor_caminho); // Libera o anterior
            *melhor_caminho = caminho_atual;
            *tam_melhor_caminho = tam_caminho_atual;
        } else {
            if (caminho_atual) free(caminho_atual); // Libera o caminho não usado
        }
    }
    caminho_atual = NULL;

    // --- Cenário 2: A(via vA1) -> ... -> B(via vB2) ---
    int dist_vA1_vB2 = calculaDistanciaModificado(G, ordem, poi_A.vert1, poi_B.vert2, &caminho_atual, &tam_caminho_atual);
     if (dist_vA1_vB2 != INT_MAX) {
        int dist_c2 = poi_A.dist_vert1 + dist_vA1_vB2 + poi_B.dist_vert2;
        if (dist_c2 < dist_total) {
            dist_total = dist_c2;
            if(*melhor_caminho) free(*melhor_caminho);
            *melhor_caminho = caminho_atual;
            *tam_melhor_caminho = tam_caminho_atual;
        } else {
            if (caminho_atual) free(caminho_atual);
        }
    }
    caminho_atual = NULL;

    // --- Cenário 3: A(via vA2) -> ... -> B(via vB1) ---
    int dist_vA2_vB1 = calculaDistanciaModificado(G, ordem, poi_A.vert2, poi_B.vert1, &caminho_atual, &tam_caminho_atual);
     if (dist_vA2_vB1 != INT_MAX) {
        int dist_c3 = poi_A.dist_vert2 + dist_vA2_vB1 + poi_B.dist_vert1;
        if (dist_c3 < dist_total) {
            dist_total = dist_c3;
            if(*melhor_caminho) free(*melhor_caminho);
            *melhor_caminho = caminho_atual;
            *tam_melhor_caminho = tam_caminho_atual;
        } else {
            if (caminho_atual) free(caminho_atual);
        }
    }
    caminho_atual = NULL;

    // --- Cenário 4: A(via vA2) -> ... -> B(via vB2) ---
    int dist_vA2_vB2 = calculaDistanciaModificado(G, ordem, poi_A.vert2, poi_B.vert2, &caminho_atual, &tam_caminho_atual);
     if (dist_vA2_vB2 != INT_MAX) {
        int dist_c4 = poi_A.dist_vert2 + dist_vA2_vB2 + poi_B.dist_vert2;
        if (dist_c4 < dist_total) {
            dist_total = dist_c4;
            if(*melhor_caminho) free(*melhor_caminho);
            *melhor_caminho = caminho_atual;
            *tam_melhor_caminho = tam_caminho_atual;
        } else {
            if (caminho_atual) free(caminho_atual);
        }
    }

    return dist_total;
}

// --- PARTE 2 ---
/*
 * Função para calcular o passeio (TSP - Heurística do Vizinho Mais Próximo)
 * Isso atende o requisito principal da Parte 2
 */
void calcularPasseio(Vert G[], int ordem, Localidades minha_casa, Localidades locais[], int num_locais) {
    
    // --- 1. Definir o conjunto de locais a visitar ---
    // CORREÇÃO: Aloca dinamicamente o conjunto para caber "Minha Casa" + TODOS os locais.
    int num_visita = num_locais + 1; // 20 locais + 1 "minha casa"
    
    // Aloca memória para o conjunto de visita
    Localidades *conjunto_visita = (Localidades*) malloc(num_visita * sizeof(Localidades));
    if (conjunto_visita == NULL) {
        perror("Erro ao alocar memoria para conjunto_visita");
        return;
    }

    // Posição 0 é sempre "minha casa"
    conjunto_visita[0] = minha_casa; 
    
    // Copia os 20 locais da lista "locais" para o resto do "conjunto_visita"
    for(int i = 0; i < num_locais; i++) {
        conjunto_visita[i+1] = locais[i];
    }

    // --- 2. Exibir os locais a serem visitados ---
    printf("\n  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("  |            Calculando Passeio (Parte 2)            |\n");
    printf("  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
    printf("Iniciando passeio em: %s\n", conjunto_visita[0].nome);
    printf("Visitando os seguintes %d locais:\n", num_visita - 1);
    for(int i = 1; i < num_visita; i++) {
        printf("  - %s\n", conjunto_visita[i].nome);
    }
    printf("\n--- Estrategia: Heuristica do Vizinho Mais Proximo ---\n");
    
    // --- 3. Implementar a Heurística ---
    int *ordem_visita = (int*) malloc(num_visita * sizeof(int)); // Guarda os *índices* do conjunto_visita
    int *visitados = (int*) calloc(num_visita, sizeof(int)); // 0 = não visitado, 1 = visitado
    
    int local_atual_idx = 0; // Começa na "minha casa" (índice 0)
    ordem_visita[0] = local_atual_idx;
    visitados[local_atual_idx] = 1;
    
    int distancia_total_passeio = 0;
    
    printf("\n--- Detalhes do Percurso ---\n");

    for (int i = 1; i < num_visita; i++) { // Encontra os próximos (n-1) locais
        
        int melhor_dist_trecho = INT_MAX;
        int proximo_local_idx = -1;
        int *melhor_caminho_trecho = NULL;
        int tam_melhor_caminho_trecho = 0;

        // Procura o vizinho mais próximo *não visitado*
        for (int k = 0; k < num_visita; k++) {
            if (visitados[k] == 0) { // Se ainda não visitou
                int *caminho_teste = NULL;
                int tam_caminho_teste = 0;
                
                int dist_teste = getDistanciaEntrePOIs(G, ordem, 
                                    conjunto_visita[local_atual_idx], 
                                    conjunto_visita[k], 
                                    &caminho_teste, &tam_caminho_teste);
                                    
                if (dist_teste < melhor_dist_trecho) {
                    melhor_dist_trecho = dist_teste;
                    proximo_local_idx = k;
                    
                    // NÂO libere o melhor_caminho_trecho antigo aqui, pois 'caminho_teste'
                    // é o ponteiro retornado.
                    // A função getDistanciaEntrePOIs já limpou o ponteiro
                    // 'melhor_caminho_trecho' antes de retornar 'caminho_teste'.
                    // Apenas transferimos a propriedade.
                    melhor_caminho_trecho = caminho_teste;
                    tam_melhor_caminho_trecho = tam_caminho_teste;
                } else {
                    if (caminho_teste) free(caminho_teste); // Libera o caminho não usado
                }
            }
        }
        
        // --- CORREÇÃO DE BUG ---
        // Se não foi encontrado caminho para NENHUM local, proximo_local_idx será -1
        if (proximo_local_idx == -1) {
            printf("\nERRO: Nao foi possivel encontrar caminho para nenhum local nao visitado a partir de %s.\n", 
                   conjunto_visita[local_atual_idx].nome);
            printf("Verifique se o grafo e conexo ou se todas as distancias foram preenchidas.\n");
            
            // Libera memória alocada dentro do loop antes de sair
            if (melhor_caminho_trecho) free(melhor_caminho_trecho);
            break; // Sai do loop principal 'for (int i ...)'
        }
        // --- FIM DA CORREÇÃO ---
        
        // --- 4. Exibir o resultado deste trecho ---
        Localidades loc_A = conjunto_visita[local_atual_idx];
        Localidades loc_B = conjunto_visita[proximo_local_idx];
        
        printf("\n%d. Trecho: %s -> %s\n", i, loc_A.nome, loc_B.nome);
        printf("   - Distancia do trecho: %d metros\n", melhor_dist_trecho);
        printf("   - Sequencia de vertices: ");
        for(int j = 0; j < tam_melhor_caminho_trecho; j++) {
            printf("v%d", melhor_caminho_trecho[j]);
            if (j < tam_melhor_caminho_trecho - 1) {
                printf(" -> ");
                // Encontra POIs no meio do caminho (na rua/aresta)
                encontrarPOIsNoCaminho(melhor_caminho_trecho[j], melhor_caminho_trecho[j+1], locais, num_locais, loc_B.nome);
            }
        }
        printf("\n");

        if (melhor_caminho_trecho) free(melhor_caminho_trecho);
        
        // Atualiza para o próximo loop
        distancia_total_passeio += melhor_dist_trecho;
        local_atual_idx = proximo_local_idx;
        ordem_visita[i] = local_atual_idx;
        visitados[local_atual_idx] = 1;
    }
    
    // --- 5. Voltar para casa ---
    int *caminho_final = NULL;
    int tam_caminho_final = 0;
    int dist_volta = getDistanciaEntrePOIs(G, ordem, 
                        conjunto_visita[local_atual_idx], // Último local
                        conjunto_visita[0],               // Minha casa
                        &caminho_final, &tam_caminho_final);

    printf("\n%d. Trecho (Volta): %s -> %s\n", num_visita, conjunto_visita[local_atual_idx].nome, conjunto_visita[0].nome);
    printf("   - Distancia do trecho: %d metros\n", dist_volta);
    printf("   - Sequencia de vertices: ");
    for(int j = 0; j < tam_caminho_final; j++) {
        printf("v%d", caminho_final[j]);
        if (j < tam_caminho_final - 1) {
            printf(" -> ");
            encontrarPOIsNoCaminho(caminho_final[j], caminho_final[j+1], locais, num_locais, conjunto_visita[0].nome);
        }
    }
    printf("\n");
    if (caminho_final) free(caminho_final);

    distancia_total_passeio += dist_volta;

    // --- 6. Exibir Resultado Final ---
    printf("\n--------------------------------------------------------");
    printf("\n>>> RESULTADO FINAL DO PASSEIO <<<\n");
    printf("\nSequencia de visita sugerida:\n");
    for(int i = 0; i < num_visita; i++) {
        printf("  %d. %s\n", i+1, conjunto_visita[ordem_visita[i]].nome);
    }
    printf("  %d. %s (Volta)\n", num_visita+1, conjunto_visita[0].nome);
    
    printf("\nDistancia Total Percorrida: %d metros\n", distancia_total_passeio);
    printf("--------------------------------------------------------\n\n");
    
    // Libera a memória alocada no início desta função
    free(conjunto_visita); 
    free(ordem_visita);
    free(visitados);
}

// --- PARTE 2 ---
// Função auxiliar para encontrar POIs *pelas quais você passa* no caminho
void encontrarPOIsNoCaminho(int v1, int v2, Localidades locais_todos[], int num_locais_todos, const char* nome_destino) {
    for (int i = 0; i < num_locais_todos; i++) {
        // Verifica se o POI 'i' está na aresta v1-v2
        if ((locais_todos[i].vert1 == v1 && locais_todos[i].vert2 == v2) ||
            (locais_todos[i].vert1 == v2 && locais_todos[i].vert2 == v1)) {
            
            // Verifica se NÃO é o destino do trecho atual
            if (strcmp(locais_todos[i].nome, nome_destino) != 0) {
                 printf("\n     (Passando por: %s)", locais_todos[i].nome);
            }
        }
    }
}


/*
 * =======================
 * FUNÇÃO MAIN
 * =======================
 */
int main(int argc, char *argv[]) {
    Vert *G;
    int ordemG = 50;
    criaGrafo(&G, ordemG);
    
    // --- PARTE 2: ATENÇÃO! ---
    // Os pesos agora são em METROS.
    // **VOCÊ DEVE SUBSTITUIR** estes valores de exemplo (100, 120, etc.)
    // pelas distâncias reais (em metros) do seu mapa.

    // Adicionando as arestas do mapa com distâncias em METROS
    // Arestas com POIs (valores de exemplo)
    acrescentaAresta(G, ordemG, 1, 2, 120);     // "Ecully Charbon"
    acrescentaAresta(G, ordemG, 2, 3, 110);     // "Tapeçaria Renova"
    acrescentaAresta(G, ordemG, 5, 6, 115);     // "Gelato Borelli"
    acrescentaAresta(G, ordemG, 4, 13, 170);    // "We Vets Veterinario"
    acrescentaAresta(G, ordemG, 10, 11, 110);   // "1900 Pizzeria"
    acrescentaAresta(G, ordemG, 15, 16, 155);   // "Bacio di Latte"
    acrescentaAresta(G, ordemG, 9, 18, 185);    // "Petiskin do Bob"
    acrescentaAresta(G, ordemG, 10, 19, 165);   // "Academia CPN"
    acrescentaAresta(G, ordemG, 13, 22, 165);   // "Degas Pompeia"
    acrescentaAresta(G, ordemG, 16, 25, 165);   // "Cabelereiro Edmilson Araujo"
    acrescentaAresta(G, ordemG, 20, 21, 115);   // "Pizzaria Nogueira"
    acrescentaAresta(G, ordemG, 25, 37, 165);   // "Colegio Sagrado Coracao de Jesus"
    acrescentaAresta(G, ordemG, 31, 32, 120);   // "Santiago Padaria"
    acrescentaAresta(G, ordemG, 32, 33, 135);   // "Cazeco Bar"
    acrescentaAresta(G, ordemG, 31, 41, 165);   // "Galpao da Pizza"
    acrescentaAresta(G, ordemG, 34, 44, 165);   // "Hospital Sao Camilo"
    acrescentaAresta(G, ordemG, 35, 45, 165);   // "St. Marche Perdizes"
    acrescentaAresta(G, ordemG, 36, 46, 165);   // "Minuto Pao de Acucar"
    acrescentaAresta(G, ordemG, 41, 42, 110);   // "Bar do Gomes"
    acrescentaAresta(G, ordemG, 47, 48, 150);   // "Academia Smart Fit"
    acrescentaAresta(G, ordemG, 22, 34, 150);   // "Minha Casa"
    
    // Arestas restantes (do seu código original) - Coloque distâncias reais!
    // Se você não colocar todas as arestas, o grafo será desconexo
    // e o Dijkstra falhará (causando o erro que você viu).
    
    // Horizontais
    acrescentaAresta(G, ordemG, 0, 1, 120); // Aresta 1 (modificada)
    acrescentaAresta(G, ordemG, 3, 4, 120); // Aresta 4
    acrescentaAresta(G, ordemG, 4, 5, 120); // Aresta 5
    acrescentaAresta(G, ordemG, 6, 7, 120); // Aresta 7
    acrescentaAresta(G, ordemG, 7, 8, 120); // Aresta 8
    
    acrescentaAresta(G, ordemG, 9, 10, 120); // Aresta 9
    acrescentaAresta(G, ordemG, 11, 12, 120); // Aresta 11
    acrescentaAresta(G, ordemG, 12, 13, 120); // Aresta 12
    acrescentaAresta(G, ordemG, 13, 14, 120); // Aresta 13
    acrescentaAresta(G, ordemG, 14, 15, 120); // Aresta 14
    acrescentaAresta(G, ordemG, 16, 17, 120); // Aresta 16
    
    acrescentaAresta(G, ordemG, 18, 19, 120); // Aresta 17
    acrescentaAresta(G, ordemG, 19, 20, 120); // Aresta 18
    acrescentaAresta(G, ordemG, 21, 22, 120); // Aresta 20
    acrescentaAresta(G, ordemG, 22, 23, 120); // Aresta 21
    acrescentaAresta(G, ordemG, 23, 24, 120); // Aresta 22
    acrescentaAresta(G, ordemG, 24, 25, 120); // Aresta 23
    acrescentaAresta(G, ordemG, 25, 26, 60); // Aresta 24
    acrescentaAresta(G, ordemG, 26, 27, 60); // Aresta 25
    
    acrescentaAresta(G, ordemG, 28, 29, 120); // Aresta 26
    
    acrescentaAresta(G, ordemG, 30, 31, 120); // Aresta 27
    acrescentaAresta(G, ordemG, 33, 34, 120); // Aresta 30
    acrescentaAresta(G, ordemG, 34, 35, 120); // Aresta 31
    acrescentaAresta(G, ordemG, 35, 36, 120); // Aresta 32
    acrescentaAresta(G, ordemG, 36, 37, 120); // Aresta 33
    acrescentaAresta(G, ordemG, 37, 38, 60); // Aresta 34
    acrescentaAresta(G, ordemG, 38, 39, 60); // Aresta 35
    
    acrescentaAresta(G, ordemG, 40, 41, 120); // Aresta 36
    acrescentaAresta(G, ordemG, 42, 43, 120); // Aresta 38
    acrescentaAresta(G, ordemG, 43, 44, 120); // Aresta 39
    acrescentaAresta(G, ordemG, 44, 45, 120); // Aresta 40
    acrescentaAresta(G, ordemG, 45, 46, 120); // Aresta 41
    acrescentaAresta(G, ordemG, 46, 47, 120); // Aresta 42
    
    // Verticais
    acrescentaAresta(G, ordemG, 0, 9, 165); // Aresta 44
    acrescentaAresta(G, ordemG, 18, 28, 80); // Aresta 46
    acrescentaAresta(G, ordemG, 28, 30, 80); // Aresta 47
    acrescentaAresta(G, ordemG, 30, 40, 165); // Aresta 48
    
    acrescentaAresta(G, ordemG, 1, 10, 165); // Aresta 49
    acrescentaAresta(G, ordemG, 19, 29, 80); // Aresta 51
    acrescentaAresta(G, ordemG, 29, 31, 80); // Aresta 52
    
    acrescentaAresta(G, ordemG, 2, 11, 165); // Aresta 54
    acrescentaAresta(G, ordemG, 11, 20, 165); // Aresta 55
    acrescentaAresta(G, ordemG, 20, 32, 165); // Aresta 56
    acrescentaAresta(G, ordemG, 32, 42, 165); // Aresta 57
    
    acrescentaAresta(G, ordemG, 3, 12, 165); // Aresta 58
    acrescentaAresta(G, ordemG, 12, 21, 165); // Aresta 59
    acrescentaAresta(G, ordemG, 21, 33, 165); // Aresta 60
    acrescentaAresta(G, ordemG, 33, 43, 165); // Aresta 61
    
    acrescentaAresta(G, ordemG, 5, 14, 165); // Aresta 66
    acrescentaAresta(G, ordemG, 14, 23, 165); // Aresta 67
    acrescentaAresta(G, ordemG, 23, 35, 165); // Aresta 68
    
    acrescentaAresta(G, ordemG, 6, 15, 165); // Aresta 70
    acrescentaAresta(G, ordemG, 15, 24, 165); // Aresta 71
    acrescentaAresta(G, ordemG, 24, 36, 165); // Aresta 72
    
    acrescentaAresta(G, ordemG, 7, 16, 165); // Aresta 74
    
    acrescentaAresta(G, ordemG, 26, 38, 165); // Aresta 78
    
    acrescentaAresta(G, ordemG, 8, 17, 165); // Aresta 79
    acrescentaAresta(G, ordemG, 17, 27, 165); // Aresta 80
    acrescentaAresta(G, ordemG, 27, 39, 165); // Aresta 81
    acrescentaAresta(G, ordemG, 39, 48, 165); // Aresta 82
    

    // --- PARTE 2: Pontos de Interesse (POIs) com distâncias parciais ---
    // A soma de dist_vert1 e dist_vert2 DEVE ser igual ao 'peso' da aresta.
    // **VOCÊ DEVE ATUALIZAR** estes valores de exemplo.

    // Ex: Rua v22-v34. Distância total = 150m (definido acima).
    // Coloquei 50m de v22 e 100m de v34.
      // Rua v22-v34. Distância total = 165m.
    Localidades minha_casa = {"Minha Casa", 22, 34, 15, 150}; 
    
    Localidades locais[] = {
    {"Ecully Charbon", 1, 2, 30, 90},         // Aresta 1-2 (peso 120)
    {"Tapeçaria Renova", 2, 3, 80, 30},       // Aresta 2-3 (peso 110)
    {"Gelato Borelli", 5, 6, 100, 15},         // Aresta 5-6 (peso 115)
    {"We Vets Veterinario", 4, 13, 20, 150},   // Aresta 4-13 (peso 170)
    {"1900 Pizzeria", 10, 11, 70, 40},        // Aresta 10-11 (peso 110)
    {"Bacio di Latte", 15, 16, 115, 40},       // Aresta 15-16 (peso 155)
    {"Petiskin do Bob", 9, 18, 165, 20},        // Aresta 9-18 (peso 185)
    {"Academia CPN", 10, 19, 75, 90},         // Aresta 10-19 (peso 165)
    {"Degas Pompeia", 13, 22, 15, 150},        // Aresta 13-22 (peso 165)
    {"Cabelereiro Edmilson Araujo", 16, 25, 115, 50}, // Aresta 16-25 (peso 165)
    {"Pizzaria Nogueira", 20, 21, 15, 100},    // Aresta 20-21 (peso 115)
    {"Colegio Sagrado Coracao de Jesus", 25, 37, 75, 90}, // Aresta 25-37 (peso 165)
    {"Santiago Padaria", 31, 32, 100, 20},     // Aresta 31-32 (peso 120)
    {"Cazeco Bar", 32, 33, 25, 90},           // Aresta 32-33 (peso 135)
    {"Galpao da Pizza", 31, 41, 125, 40},     // Aresta 31-41 (peso 165)
    {"Hospital Sao Camilo", 34, 44, 100, 65},  // Aresta 34-44 (peso 165)
    {"St. Marche Perdizes", 35, 45, 95, 70},  // Aresta 35-45 (peso 165)
    {"Minuto Pao de Acucar", 36, 46, 75, 90},  // Aresta 36-46 (peso 165)
    {"Bar do Gomes", 41, 42, 20, 90},         // Aresta 41-42 (peso 110)
    {"Academia Smart Fit", 47, 48, 70, 80}    // Aresta 47-48 (peso 150)
    };
    
    int num_locais = sizeof(locais) / sizeof(locais[0]);
    int escolha;
    
    // Loop principal do menu
    while (1) {
        
        printf("\n      ++++++++++++++++++++++++++++++++++++++++\n");
        printf("      |                  MENU                |\n");
        printf("      ++++++++++++++++++++++++++++++++++++++++\n\n");
        
        printf("(1) Localizacao de minha casa\n");
        printf("(2) Calcular passeio (PARTE 2)\n");
        printf("(0) Fechar programa\n\n");
        
        printf("Digite uma opcao: ");
        scanf("%d", &escolha);
        
        if (escolha == 1){
            printf("\nMinha casa esta entre os vertices V%d e V%d\n", minha_casa.vert1, minha_casa.vert2);
            printf("A %d metros de V%d e %d metros de V%d.\n", minha_casa.dist_vert1, minha_casa.vert1, minha_casa.dist_vert2, minha_casa.vert2);
            printf("\n--------------------------------------------------------\n\n");
        }
        
        else if(escolha == 2){
            // Chama a função principal da Parte 2
            calcularPasseio(G, ordemG, minha_casa, locais, num_locais);
            
            printf("\nPressione enter para voltar ao menu principal...");
            while (getchar() != '\n'); // Limpa o buffer de entrada
            getchar(); // Espera o enter
        }
        
        else if (escolha == 0){
            printf("\nObrigado por utilizar o programa, fechando programa...\n");
            break;
        }
        
        else {
            printf("\nEscolha invalida, por favor digite novamente\n\n");
        }
        
        // Limpa o buffer de entrada em caso de digitação inválida (ex: 'a')
        while (getchar() != '\n'); 
    }
    
    // Libera toda memória do grafo antes de encerrar
    destroiGrafo(&G, ordemG);

    return(0);
}