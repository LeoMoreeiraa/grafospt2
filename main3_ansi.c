/*
 * Projeto de Programacao - Locais no Meu Bairro - PARTE 2
 * Teoria dos Grafos - 2025-2
 * 
 Nome: Eduardo Figueira Losco
RA: 10416650
Nome: Guilherme Rainho Geraldo
RA: 10418251
Nome: Leonardo Moreira dos Santos
RA: 10417555
Nome: Matheus Alonso Varjao
RA: 10417888

 * 
 * Descricao: Calcula passeios fechados mais curtos que iniciam em "Minha Casa",
 * visitam locais pre-definidos e retornam a "Minha Casa".
 * 
 * Estrategia adotada:
 * - Usa algoritmo de Dijkstra para calcular distancias entre vertices
 * - Calcula distancias entre locais no meio de arestas considerando distancias reais
 * - Modo 1: Visita um local especifico e retorna
 * - Modo 2: Visita varios locais (TSP) usando Vizinho Mais Proximo + 2-opt
 * 
 * Compilar: gcc -O2 -o bairro bairro.c -lm
 * Executar: ./bairro
 */

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <math.h>

/* ========== ESTRUTURAS DE DADOS ========== */

typedef struct a {
    int extremo2;
    float peso;
    struct a *prox;
} Arest;

typedef struct v {
    int nome;
    Arest *prim;
} Vert;

typedef struct {
    const char* nome;
    int v1;
    int v2;
    float distV1;
    float distV2;
} PontoDeInteresse;

/* ========== PROTOTIPOS ========== */

void criaGrafo(Vert **G, int ordem);
void destroiGrafo(Vert **G, int ordem);
int acrescentaAresta(Vert G[], int ordem, int v1, int v2, float peso);
float calculaDistancia(Vert G[], int ordem, int origem, int destino, int *anterior);
float pesoAresta(Vert G[], int ordem, int v1, int v2);
float distanciaEntrePontos(Vert G[], int ordem, PontoDeInteresse a, PontoDeInteresse b, 
                           int *path_a, int *len_a, int *path_b, int *len_b);
void calcularMatrizDistancias(Vert G[], int ordem, PontoDeInteresse casa, 
                               PontoDeInteresse locais[], int num_locais, float **matriz);
void resolverTSP_VizinhoMaisProximo(float **matriz, int n, int *rota);
void melhorar2opt(float **matriz, int n, int *rota);
void exibirResultadosSimples(Vert G[], int ordem, PontoDeInteresse casa, PontoDeInteresse destino);
void exibirResultadosCompleto(Vert G[], int ordem, PontoDeInteresse casa, 
                              PontoDeInteresse locais[], int *indices_visitar, 
                              int num_visitar, int *rota, float **matriz);
void reconstruirCaminho(int origem, int destino, int *anterior, int *caminho, int *len);
void exibirMenu();
void listarLocais(PontoDeInteresse todos_locais[], int total);
int selecionarLocal(PontoDeInteresse todos_locais[], int total);
void selecionarMultiplosLocais(PontoDeInteresse todos_locais[], int total, int *indices, int *num);
void imprimeGrafo(Vert G[], int ordem);

/* ========== IMPLEMENTACAO DAS FUNCOES ========== */

void criaGrafo(Vert **G, int ordem) {
    int i;
    *G = (Vert*) malloc(sizeof(Vert) * ordem);
    if (!*G) {
        fprintf(stderr, "Erro de memoria ao criar grafo\n");
        exit(1);
    }
    for (i = 0; i < ordem; ++i) {
        (*G)[i].nome = i;
        (*G)[i].prim = NULL;
    }
}

void destroiGrafo(Vert **G, int ordem) {
    int i;
    Arest *a, *n;
    if (!*G) return;
    for (i = 0; i < ordem; ++i) {
        a = (*G)[i].prim;
        while (a) {
            n = a->prox;
            free(a);
            a = n;
        }
    }
    free(*G);
    *G = NULL;
}

int acrescentaAresta(Vert G[], int ordem, int v1, int v2, float peso) {
    Arest *A1, *A2;
    if (v1 < 0 || v1 >= ordem || v2 < 0 || v2 >= ordem) return 0;

    A1 = (Arest*) malloc(sizeof(Arest));
    if (!A1) return 0;
    A1->extremo2 = v2;
    A1->peso = peso;
    A1->prox = G[v1].prim;
    G[v1].prim = A1;

    if (v1 == v2) return 1;

    A2 = (Arest*) malloc(sizeof(Arest));
    if (!A2) return 1;
    A2->extremo2 = v1;
    A2->peso = peso;
    A2->prox = G[v2].prim;
    G[v2].prim = A2;
    return 1;
}

void reconstruirCaminho(int origem, int destino, int *anterior, int *caminho, int *len) {
    int temp[100];
    int count = 0;
    int atual = destino;
    int i;
    
    while (atual != -1 && atual != origem) {
        temp[count++] = atual;
        atual = anterior[atual];
    }
    temp[count++] = origem;
    
    *len = count;
    for (i = 0; i < count; i++) {
        caminho[i] = temp[count - 1 - i];
    }
}

float calculaDistancia(Vert G[], int ordem, int origem, int destino, int *anterior) {
    float *dist;
    int *visitado;
    int i, count, u, v;
    float min, w, resultado;
    Arest *adj;
    
    dist = (float*) malloc(sizeof(float) * ordem);
    visitado = (int*) malloc(sizeof(int) * ordem);
    
    if (!dist || !visitado) {
        fprintf(stderr, "Erro de memoria em calculaDistancia\n");
        exit(1);
    }
    
    for (i = 0; i < ordem; ++i) {
        dist[i] = FLT_MAX;
        visitado[i] = 0;
        if (anterior) anterior[i] = -1;
    }
    dist[origem] = 0.0f;
    
    for (count = 0; count < ordem - 1; ++count) {
        min = FLT_MAX;
        u = -1;
        for (i = 0; i < ordem; ++i) {
            if (!visitado[i] && dist[i] <= min) {
                min = dist[i];
                u = i;
            }
        }
        if (u == -1) break;
        visitado[u] = 1;
        if (u == destino) break;
        
        for (adj = G[u].prim; adj; adj = adj->prox) {
            v = adj->extremo2;
            w = adj->peso;
            if (!visitado[v] && dist[u] != FLT_MAX && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                if (anterior) anterior[v] = u;
            }
        }
    }
    
    resultado = dist[destino];
    free(dist);
    free(visitado);
    return resultado;
}

float pesoAresta(Vert G[], int ordem, int v1, int v2) {
    Arest *a;
    if (v1 < 0 || v1 >= ordem || v2 < 0 || v2 >= ordem) return FLT_MAX;
    for (a = G[v1].prim; a; a = a->prox) {
        if (a->extremo2 == v2) return a->peso;
    }
    return FLT_MAX;
}

float distanciaEntrePontos(Vert G[], int ordem, PontoDeInteresse a, PontoDeInteresse b,
                           int *path_a, int *len_a, int *path_b, int *len_b) {
    int *ant;
    float d1, d2, d3, d4, opt1, opt2, opt3, opt4, minDist;
    int melhor, va_ini, vb_ini;
    
    ant = (int*) malloc(sizeof(int) * ordem);
    
    if ((a.v1 == b.v1 && a.v2 == b.v2) || (a.v1 == b.v2 && a.v2 == b.v1)) {
        free(ant);
        if (path_a) {
            path_a[0] = a.v1;
            path_a[1] = a.v2;
            *len_a = 2;
        }
        if (path_b) {
            path_b[0] = b.v1;
            *len_b = 1;
        }
        return fabs(a.distV1 - b.distV1);
    }
    
    d1 = calculaDistancia(G, ordem, a.v1, b.v1, ant);
    opt1 = d1 + a.distV1 + b.distV1;
    
    d2 = calculaDistancia(G, ordem, a.v1, b.v2, NULL);
    opt2 = d2 + a.distV1 + b.distV2;
    
    d3 = calculaDistancia(G, ordem, a.v2, b.v1, NULL);
    opt3 = d3 + a.distV2 + b.distV1;
    
    d4 = calculaDistancia(G, ordem, a.v2, b.v2, NULL);
    opt4 = d4 + a.distV2 + b.distV2;
    
    minDist = opt1;
    melhor = 1;
    
    if (opt2 < minDist) { minDist = opt2; melhor = 2; }
    if (opt3 < minDist) { minDist = opt3; melhor = 3; }
    if (opt4 < minDist) { minDist = opt4; melhor = 4; }
    
    if (path_a && path_b && len_a && len_b) {
        int vb_fim;
        
        if (melhor == 1) { va_ini = a.v1; vb_ini = b.v1; vb_fim = b.v2; }
        else if (melhor == 2) { va_ini = a.v1; vb_ini = b.v2; vb_fim = b.v1; }
        else if (melhor == 3) { va_ini = a.v2; vb_ini = b.v1; vb_fim = b.v2; }
        else { va_ini = a.v2; vb_ini = b.v2; vb_fim = b.v1; }
        
        calculaDistancia(G, ordem, va_ini, vb_ini, ant);
        reconstruirCaminho(va_ini, vb_ini, ant, path_a, len_a);
        path_b[0] = vb_fim;
        *len_b = 1;
    }
    
    free(ant);
    return minDist;
}

void calcularMatrizDistancias(Vert G[], int ordem, PontoDeInteresse casa, 
                               PontoDeInteresse locais[], int num_locais, float **matriz) {
    int i, j;
    PontoDeInteresse pi, pj;
    for (i = 0; i <= num_locais; ++i) {
        for (j = 0; j <= num_locais; ++j) {
            if (i == j) {
                matriz[i][j] = 0.0f;
            } else {
                pi = (i == 0) ? casa : locais[i-1];
                pj = (j == 0) ? casa : locais[j-1];
                matriz[i][j] = distanciaEntrePontos(G, ordem, pi, pj, NULL, NULL, NULL, NULL);
            }
        }
    }
}

void resolverTSP_VizinhoMaisProximo(float **matriz, int n, int *rota) {
    int *visitado;
    int i, j, atual, proximo;
    float min_dist;
    
    visitado = (int*) calloc(n, sizeof(int));
    
    rota[0] = 0;
    visitado[0] = 1;
    
    for (i = 1; i < n; ++i) {
        atual = rota[i-1];
        min_dist = FLT_MAX;
        proximo = -1;
        
        for (j = 0; j < n; ++j) {
            if (!visitado[j] && matriz[atual][j] < min_dist) {
                min_dist = matriz[atual][j];
                proximo = j;
            }
        }
        
        rota[i] = proximo;
        visitado[proximo] = 1;
    }
    
    free(visitado);
}

void melhorar2opt(float **matriz, int n, int *rota) {
    int melhorou = 1;
    int i, j, esq, dir, temp;
    float dist_antes, dist_depois;
    
    while (melhorou) {
        melhorou = 0;
        for (i = 1; i < n - 2; ++i) {
            for (j = i + 1; j < n - 1; ++j) {
                dist_antes = matriz[rota[i-1]][rota[i]] + matriz[rota[j]][rota[j+1]];
                dist_depois = matriz[rota[i-1]][rota[j]] + matriz[rota[i]][rota[j+1]];
                
                if (dist_depois < dist_antes - 0.01f) {
                    esq = i;
                    dir = j;
                    while (esq < dir) {
                        temp = rota[esq];
                        rota[esq] = rota[dir];
                        rota[dir] = temp;
                        esq++;
                        dir--;
                    }
                    melhorou = 1;
                }
            }
        }
    }
}

void exibirResultadosSimples(Vert G[], int ordem, PontoDeInteresse casa, PontoDeInteresse destino) {
    int path_ida[100], len_ida = 0;
    int path_volta[100], len_volta = 0;
    int i;
    float dist_ida, dist_volta, total;
    
    dist_ida = distanciaEntrePontos(G, ordem, casa, destino, path_ida, &len_ida, path_volta, &len_volta);
    dist_volta = dist_ida;
    total = dist_ida + dist_volta;
    
    printf("\n");
    printf("-----------------------------------------------------------\n");
    printf("                RESULTADO: IDA E VOLTA A UM LOCAL\n");
    printf("-----------------------------------------------------------\n\n");
    
    printf("TRAJETO DE IDA: Minha Casa -> %s\n", destino.nome);
    printf("--------------------------------------------------------------------------------\n");
    printf("  Vertices percorridos: ");
    for (i = 0; i < len_ida; i++) {
        printf("V%d", path_ida[i] + 1);
        if (i < len_ida - 1) printf(" -> ");
    }
    printf("\n  Distancia: %.2f metros\n\n", dist_ida);
    
    printf("TRAJETO DE VOLTA: %s -> Minha Casa\n", destino.nome);
    printf("--------------------------------------------------------------------------------\n");
    printf("  Vertices percorridos: ");
    for (i = len_ida - 1; i >= 0; i--) {
        printf("V%d", path_ida[i] + 1);
        if (i > 0) printf(" -> ");
    }
    printf("\n  Distancia: %.2f metros\n\n", dist_volta);
    
    printf("DISTANCIA TOTAL PERCORRIDA: %.2f metros (%.2f km)\n", total, total/1000.0f);
    printf("-----------------------------------------------------------\n\n");
}

void exibirResultadosCompleto(Vert G[], int ordem, PontoDeInteresse casa, 
                              PontoDeInteresse locais[], int *indices_visitar, 
                              int num_visitar, int *rota, float **matriz) {
    int i, j, orig_idx, dest_idx;
    int path[100], len_path, path_b[100], len_b;
    float dist, total, dist_acumulada;
    PontoDeInteresse origem, destino;
    
    printf("\n");
    printf("-----------------------------------------------------------\n");
    printf("            RESULTADO: PASSEIO VISITANDO MULTIPLOS LOCAIS\n");
    printf("-----------------------------------------------------------\n\n");
    
    printf("SEQUENCIA DE LOCAIS:\n");
    printf("--------------------------------------------------------------------------------\n");
    printf(" 1. Minha Casa (V%d-V%d)\n", casa.v1+1, casa.v2+1);
    for (i = 1; i < num_visitar + 1; ++i) {
        int idx = rota[i];
        PontoDeInteresse local = locais[idx-1];
        printf(" %d. %s (V%d-V%d)\n", i+1, local.nome, local.v1+1, local.v2+1);
    }
    printf(" %d. Minha Casa (retorno)\n\n", num_visitar+2);
    
    printf("TRECHOS DO PASSEIO:\n");
    printf("================================================================================\n");
    total = 0.0f;
    dist_acumulada = 0.0f;
    
    for (i = 0; i <= num_visitar; ++i) {
        orig_idx = rota[i];
        dest_idx = (i < num_visitar) ? rota[i+1] : rota[0];
        
        origem = (orig_idx == 0) ? casa : locais[orig_idx-1];
        destino = (dest_idx == 0) ? casa : locais[dest_idx-1];
        
        dist = distanciaEntrePontos(G, ordem, origem, destino, path, &len_path, path_b, &len_b);
        
        printf("\nTrecho %d: ", i+1);
        if (orig_idx == 0) printf("Minha Casa");
        else printf("%s", locais[orig_idx-1].nome);
        printf(" -> ");
        if (dest_idx == 0) printf("Minha Casa\n");
        else printf("%s\n", locais[dest_idx-1].nome);
        
        printf("  Vertices percorridos: ");
        
        /* Imprime o caminho completo de vértices */
        for (j = 0; j < len_path; j++) {
            printf("V%d", path[j] + 1);
            if (j < len_path - 1) printf(" -> ");
        }
        
        printf("\n");
        
        dist_acumulada += dist;
        printf("  Distancia deste trecho: %.2f metros\n", dist);
        printf("  Distancia acumulada: %.2f metros\n", dist_acumulada);
        
        total += dist;
    }
    
    printf("\n");
    printf("-----------------------------------------------------------\n");
    printf("DISTANCIA TOTAL DO PASSEIO: %.2f metros (%.2f km)\n", total, total/1000.0f);
    printf("-----------------------------------------------------------\n\n");
}

void exibirMenu() {
    printf("\n");
    printf("-----------------------------------------------------------\n");
    printf("           SISTEMA DE ROTAS - LOCAIS NO MEU BAIRRO\n");
    printf("-----------------------------------------------------------\n\n");
    printf("Escolha uma opcao:\n\n");
    printf("  [1] Calcular ida e volta a um unico local\n");
    printf("  [2] Calcular passeio visitando multiplos locais (TSP)\n");
    printf("  [3] Exibir grafo modelado\n");
    printf("  [0] Sair\n\n");
    printf("-----------------------------------------------------------\n");
    printf("Opcao: ");
}

void listarLocais(PontoDeInteresse todos_locais[], int total) {
    int i;
    printf("\n");
    printf("-----------------------------------------------------------\n");
    printf("                    LOCAIS DISPONIVEIS\n");
    printf("-----------------------------------------------------------\n\n");
    for (i = 0; i < total; i++) {
        printf("  [%2d] %s (V%d-V%d)\n", i+1, todos_locais[i].nome, 
               todos_locais[i].v1+1, todos_locais[i].v2+1);
    }
    printf("\n-----------------------------------------------------------\n");
}

int selecionarLocal(PontoDeInteresse todos_locais[], int total) {
    int escolha;
    listarLocais(todos_locais, total);
    printf("\nEscolha o numero do local (1-%d): ", total);
    scanf("%d", &escolha);
    
    if (escolha < 1 || escolha > total) {
        printf("Opcao invalida! Usando local padrao (1).\n");
        return 0;
    }
    return escolha - 1;
}

void selecionarMultiplosLocais(PontoDeInteresse todos_locais[], int total, int *indices, int *num) {
    int i, escolha;
    listarLocais(todos_locais, total);
    
    printf("\nQuantos locais deseja visitar? (1-%d): ", total);
    scanf("%d", num);
    
    if (*num < 1 || *num > total) {
        printf("Numero invalido! Usando 3 locais padrao.\n");
        *num = 3;
        indices[0] = 0;
        indices[1] = 7;
        indices[2] = 19;
        return;
    }
    
    printf("\nDigite os numeros dos locais (um por vez):\n");
    for (i = 0; i < *num; i++) {
        printf("  Local %d: ", i+1);
        scanf("%d", &escolha);
        
        if (escolha < 1 || escolha > total) {
            printf("    Opcao invalida! Usando local %d.\n", i+1);
            indices[i] = i;
        } else {
            indices[i] = escolha - 1;
        }
    }
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
            printf("  v%d(dist:%.2fm)", aux->extremo2, aux->peso);
    }
    printf("\n\n");
}

int main(void) {
    /* Declaracao de TODAS as variaveis no inicio */
    Vert *G;
    const int ordemG = 39;
    PontoDeInteresse minha_casa;
    PontoDeInteresse todos_locais[20];
    int total_locais;
    int opcao;
    int indice_destino;
    int indices_visitar[20];
    int num_visitar;
    int n;
    int i;
    float **matriz;
    PontoDeInteresse *locais_visitar;
    int *rota;
    
    /* Agora sim, codigo executavel */
    criaGrafo(&G, ordemG);

    /* Construindo o grafo com todas as arestas */
   /* --- INICIO DAS ARESTAS (Ordem Original) --- */
    acrescentaAresta(G, ordemG, 0, 1, 213.2f);     /* A1:  V1 -- V2 */
    acrescentaAresta(G, ordemG, 1, 2, 119.79f);    /* A2:  V2 -- V3 */
    acrescentaAresta(G, ordemG, 2, 3, 46.35f);     /* A3:  V3 -- V4 */
    acrescentaAresta(G, ordemG, 3, 4, 48.27f);     /* A4:  V4 -- V5 */
    acrescentaAresta(G, ordemG, 0, 5, 111.17f);    /* A5:  V1 -- V6 */
    acrescentaAresta(G, ordemG, 1, 6, 110.55f);    /* A6:  V2 -- V7 */
    acrescentaAresta(G, ordemG, 2, 7, 110.94f);    /* A7:  V3 -- V8 */
    acrescentaAresta(G, ordemG, 3, 8, 111.14f);    /* A8:  V4 -- V9 */
    acrescentaAresta(G, ordemG, 4, 9, 111.27f);    /* A9:  V5 -- V10 */
    acrescentaAresta(G, ordemG, 5, 6, 209.99f);    /* A10: V6 -- V7 */
    acrescentaAresta(G, ordemG, 6, 7, 119.42f);    /* A11: V7 -- V8 */
    acrescentaAresta(G, ordemG, 7, 8, 45.99f);     /* A12: V8 -- V9 */
    acrescentaAresta(G, ordemG, 8, 9, 46.92f);     /* A13: V9 -- V10 */
    acrescentaAresta(G, ordemG, 5, 11, 116.47f);   /* A14: V6 -- V12 */
    acrescentaAresta(G, ordemG, 6, 12, 114.34f);   /* A15: V7 -- V13 (Great Kids School) */
    acrescentaAresta(G, ordemG, 9, 14, 114.57f);   /* A16: V10 -- V15 */
    acrescentaAresta(G, ordemG, 10, 11, 150.16f);  /* A17: V11 -- V12 (UBS Campestre) */
    acrescentaAresta(G, ordemG, 11, 12, 211.10f);  /* A18: V12 -- V13 (Ponto de onibus 1) */
    acrescentaAresta(G, ordemG, 12, 13, 96.89f);   /* A19: V13 -- V14 */
    acrescentaAresta(G, ordemG, 13, 14, 118.29f);  /* A20: V14 -- V15 */
    acrescentaAresta(G, ordemG, 14, 15, 212.88f);  /* A21: V15 -- V16 (Green Residence) */
    acrescentaAresta(G, ordemG, 11, 16, 123.31f);  /* A22: V12 -- V17 (Auto Eletrica/Chaveiro) */
    acrescentaAresta(G, ordemG, 12, 17, 116.35f);  /* A23: V13 -- V18 */
    acrescentaAresta(G, ordemG, 13, 18, 114.87f);  /* A24: V14 -- V19 */
    acrescentaAresta(G, ordemG, 14, 20, 121.40f);  /* A25: V15 -- V21 (Verzani e Sandrini) */
    acrescentaAresta(G, ordemG, 15, 21, 127.81f);  /* A26: V16 -- V22 */
    acrescentaAresta(G, ordemG, 16, 17, 211.79f);  /* A27: V17 -- V18 */
    acrescentaAresta(G, ordemG, 17, 18, 93.76f);   /* A28: V18 -- V19 */
    acrescentaAresta(G, ordemG, 18, 19, 41.71f);   /* A29: V19 -- V20 */
    acrescentaAresta(G, ordemG, 19, 20, 79.83f);   /* A30: V20 -- V21 */
    acrescentaAresta(G, ordemG, 20, 21, 212.83f);  /* A31: V21 -- V22 */
    acrescentaAresta(G, ordemG, 16, 22, 112.52f);  /* A32: V17 -- V23 */
    acrescentaAresta(G, ordemG, 17, 23, 111.62f);  /* A33: V18 -- V24 */
    acrescentaAresta(G, ordemG, 19, 24, 107.50f);  /* A34: V20 -- V25 */
    acrescentaAresta(G, ordemG, 20, 25, 110.43f);  /* A35: V21 -- V26 */
    acrescentaAresta(G, ordemG, 21, 26, 112.87f);  /* A38: V22 -- V27 (Horizontal) */
    acrescentaAresta(G, ordemG, 22, 23, 207.67f);  /* A37: V23 -- V24 (Paroquia Sao Judas) */
    acrescentaAresta(G, ordemG, 23, 24, 135.43f);  /* A38: V24 -- V25 (Minha Casa - Vertical) */
    acrescentaAresta(G, ordemG, 24, 25, 78.97f);   /* A39: V25 -- V26 */
    acrescentaAresta(G, ordemG, 25, 26, 212.41f);  /* A40: V26 -- V27 */
    acrescentaAresta(G, ordemG, 22, 27, 114.91f);  /* A41: V23 -- V28 */
    acrescentaAresta(G, ordemG, 23, 28, 117.53f);  /* A42: V24 -- V29 */
    acrescentaAresta(G, ordemG, 25, 29, 115.58f);  /* A43: V26 -- V30 (Ponto de onibus 2) */
    acrescentaAresta(G, ordemG, 26, 30, 115.71f);  /* A44: V27 -- V31 (Smartfit) */
    acrescentaAresta(G, ordemG, 27, 28, 206.12f);  /* A45: V28 -- V29 (Uooba, Armando Nissan) */
    acrescentaAresta(G, ordemG, 28, 29, 219.46f);  /* A46: V29 -- V30 (Vetnasa, Vila Safari) */
    acrescentaAresta(G, ordemG, 29, 30, 212.37f);  /* A47: V30 -- V31 (Alo Bebe) */
    acrescentaAresta(G, ordemG, 27, 31, 118.35f);  /* A48: V28 -- V32 */
    acrescentaAresta(G, ordemG, 28, 32, 118.79f);  /* A49: V29 -- V33 */
    acrescentaAresta(G, ordemG, 29, 33, 116.25f);  /* A50: V30 -- V34 */
    acrescentaAresta(G, ordemG, 30, 34, 117.29f);  /* A51: V31 -- V35 */
    acrescentaAresta(G, ordemG, 31, 32, 207.44f);  /* A52: V32 -- V33 */
    acrescentaAresta(G, ordemG, 32, 33, 214.40f);  /* A53: V33 -- V34 */
    acrescentaAresta(G, ordemG, 33, 34, 213.80f);  /* A54: V34 -- V35 */
    acrescentaAresta(G, ordemG, 31, 35, 116.70f);  /* A55: V32 -- V36 */
    acrescentaAresta(G, ordemG, 32, 36, 114.20f);  /* A56: V33 -- V37 */
    acrescentaAresta(G, ordemG, 33, 37, 114.73f);  /* A57: V34 -- V38 (Ponto de onibus 3) */
    acrescentaAresta(G, ordemG, 34, 38, 116.68f);  /* A58: V35 -- V39 (Coop) */
    acrescentaAresta(G, ordemG, 35, 36, 211.53f);  /* A59: V36 -- V37 (Actos) */
    acrescentaAresta(G, ordemG, 36, 37, 219.10f);  /* A60: V37 -- V38 (Central IBC) */
    acrescentaAresta(G, ordemG, 37, 38, 214.51f);  /* A61: V38 -- V39 */

    /* Inicializacao das estruturas */
    minha_casa.nome = "Minha Casa";
    minha_casa.v1 = 23;
    minha_casa.v2 = 24;
    minha_casa.distV1 = 93.35f;
    minha_casa.distV2 = 42.08f;
    
    /* Inicializacao do array de locais */
    todos_locais[0].nome = "UBS Campestre";
    todos_locais[0].v1 = 10;
    todos_locais[0].v2 = 11;
    todos_locais[0].distV1 = 31.14f;
    todos_locais[0].distV2 = 119.02f;
    
    todos_locais[1].nome = "Auto Eletrica";
    todos_locais[1].v1 = 11;
    todos_locais[1].v2 = 16;
    todos_locais[1].distV1 = 91.67f;
    todos_locais[1].distV2 = 31.64f;
    
    todos_locais[2].nome = "Chaveiro Garrido";
    todos_locais[2].v1 = 11;
    todos_locais[2].v2 = 16;
    todos_locais[2].distV1 = 68.84f;
    todos_locais[2].distV2 = 54.47f;
    
    todos_locais[3].nome = "Ponto de onibus 1";
    todos_locais[3].v1 = 11;
    todos_locais[3].v2 = 12;
    todos_locais[3].distV1 = 111.91f;
    todos_locais[3].distV2 = 99.19f;
    
    todos_locais[4].nome = "Great Kids School";
    todos_locais[4].v1 = 6;
    todos_locais[4].v2 = 12;
    todos_locais[4].distV1 = 62.08f;
    todos_locais[4].distV2 = 52.26f;
    
    todos_locais[5].nome = "Verzani e Sandrini";
    todos_locais[5].v1 = 14;
    todos_locais[5].v2 = 20;
    todos_locais[5].distV1 = 68.21f;
    todos_locais[5].distV2 = 53.19f;
    
    todos_locais[6].nome = "Green Residence";
    todos_locais[6].v1 = 14;
    todos_locais[6].v2 = 15;
    todos_locais[6].distV1 = 134.47f;
    todos_locais[6].distV2 = 78.41f;
    
    todos_locais[7].nome = "Smartfit";
    todos_locais[7].v1 = 26;
    todos_locais[7].v2 = 30;
    todos_locais[7].distV1 = 96.59f;
    todos_locais[7].distV2 = 16.28f;
    
    todos_locais[8].nome = "Paroquia Sao Judas";
    todos_locais[8].v1 = 22;
    todos_locais[8].v2 = 23;
    todos_locais[8].distV1 = 68.77f;
    todos_locais[8].distV2 = 138.90f;
    
    todos_locais[9].nome = "Ponto de onibus 2";
    todos_locais[9].v1 = 25;
    todos_locais[9].v2 = 29;
    todos_locais[9].distV1 = 50.55f;
    todos_locais[9].distV2 = 61.88f;
    
    todos_locais[10].nome = "Uooba Buffet";
    todos_locais[10].v1 = 27;
    todos_locais[10].v2 = 28;
    todos_locais[10].distV1 = 64.73f;
    todos_locais[10].distV2 = 141.39f;
    
    todos_locais[11].nome = "Armando Nissan";
    todos_locais[11].v1 = 27;
    todos_locais[11].v2 = 28;
    todos_locais[11].distV1 = 52.22f;
    todos_locais[11].distV2 = 153.90f;
    
    todos_locais[12].nome = "Vetnasa Clinica";
    todos_locais[12].v1 = 28;
    todos_locais[12].v2 = 32;
    todos_locais[12].distV1 = 96.14f;
    todos_locais[12].distV2 = 22.65f;
    
    todos_locais[13].nome = "Vila Safari";
    todos_locais[13].v1 = 28;
    todos_locais[13].v2 = 29;
    todos_locais[13].distV1 = 114.61f;
    todos_locais[13].distV2 = 104.85f;
    
    todos_locais[14].nome = "Clinica Veterinaria";
    todos_locais[14].v1 = 28;
    todos_locais[14].v2 = 29;
    todos_locais[14].distV1 = 202.01f;
    todos_locais[14].distV2 = 17.45f;
    
    todos_locais[15].nome = "Alo Bebe";
    todos_locais[15].v1 = 29;
    todos_locais[15].v2 = 30;
    todos_locais[15].distV1 = 80.69f;
    todos_locais[15].distV2 = 131.68f;
    
    todos_locais[16].nome = "Actos";
    todos_locais[16].v1 = 36;
    todos_locais[16].v2 = 37;
    todos_locais[16].distV1 = 205.54f;
    todos_locais[16].distV2 = 13.56f;
    
    todos_locais[17].nome = "Central IBC";
    todos_locais[17].v1 = 36;
    todos_locais[17].v2 = 37;
    todos_locais[17].distV1 = 20.02f;
    todos_locais[17].distV2 = 199.08f;
    
    todos_locais[18].nome = "Ponto de onibus 3";
    todos_locais[18].v1 = 33;
    todos_locais[18].v2 = 37;
    todos_locais[18].distV1 = 60.81f;
    todos_locais[18].distV2 = 53.92f;
    
    todos_locais[19].nome = "Coop";
    todos_locais[19].v1 = 34;
    todos_locais[19].v2 = 38;
    todos_locais[19].distV1 = 39.10f;
    todos_locais[19].distV2 = 77.58f;
    
    total_locais = 20;
    
    /* Loop principal do menu */
    do {
        exibirMenu();
        scanf("%d", &opcao);
        
        switch(opcao) {
            case 1:
                /* MODO 1: Visitar um unico local */
                printf("\n=== MODO 1: IDA E VOLTA A UM LOCAL ===\n");
                indice_destino = selecionarLocal(todos_locais, total_locais);
                
                printf("\nCalculando rota...\n");
                exibirResultadosSimples(G, ordemG, minha_casa, todos_locais[indice_destino]);
                
                printf("\nPressione ENTER para continuar...");
                getchar(); /* Limpa buffer */
                getchar(); /* Aguarda ENTER */
                break;
            
            case 2:
                /* MODO 2: Visitar multiplos locais (TSP) */
                printf("\n=== MODO 2: VISITAR MULTIPLOS LOCAIS ===\n");
                
                selecionarMultiplosLocais(todos_locais, total_locais, indices_visitar, &num_visitar);
                
                printf("\nLocais selecionados:\n");
                for (i = 0; i < num_visitar; i++) {
                    printf("  %d. %s\n", i+1, todos_locais[indices_visitar[i]].nome);
                }
                
                printf("\nCalculando distancias...\n");
                n = num_visitar + 1;
                matriz = (float**) malloc(sizeof(float*) * n);
                for (i = 0; i < n; ++i) {
                    matriz[i] = (float*) malloc(sizeof(float) * n);
                }
                
                locais_visitar = (PontoDeInteresse*) malloc(sizeof(PontoDeInteresse) * num_visitar);
                for (i = 0; i < num_visitar; ++i) {
                    locais_visitar[i] = todos_locais[indices_visitar[i]];
                }
                
                calcularMatrizDistancias(G, ordemG, minha_casa, locais_visitar, num_visitar, matriz);
                
                printf("Calculando melhor rota (Vizinho Mais Proximo + 2-opt)...\n");
                rota = (int*) malloc(sizeof(int) * n);
                resolverTSP_VizinhoMaisProximo(matriz, n, rota);
                melhorar2opt(matriz, n, rota);
                
                exibirResultadosCompleto(G, ordemG, minha_casa, locais_visitar, indices_visitar, num_visitar, rota, matriz);
                
                /* Liberar memoria */
                for (i = 0; i < n; ++i) {
                    free(matriz[i]);
                }
                free(matriz);
                free(locais_visitar);
                free(rota);
                
                printf("\nPressione ENTER para continuar...");
                getchar(); /* Limpa buffer */
                getchar(); /* Aguarda ENTER */
                break;
            
            case 3:
                /* MODO 3: Exibir grafo modelado */
                printf("\n=== MODO 3: EXIBIR GRAFO MODELADO ===\n");
                imprimeGrafo(G, ordemG);
                
                printf("\nPressione ENTER para continuar...");
                getchar(); /* Limpa buffer */
                getchar(); /* Aguarda ENTER */
                break;
            
            case 0:
                printf("\nEncerrando programa...\n");
                break;
                
            default:
                printf("\nOpcao invalida! Tente novamente.\n");
                printf("\nPressione ENTER para continuar...");
                getchar(); /* Limpa buffer */
                getchar(); /* Aguarda ENTER */
                break;
        }
        
    } while (opcao != 0);
    
    /* Destruir grafo e liberar memoria */
    destroiGrafo(&G, ordemG);
    
    printf("\nPrograma finalizado com sucesso.\n\n");
    
    return 0;
}