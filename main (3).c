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

/* ========== IMPLEMENTACAO DAS FUNCOES ========== */

void criaGrafo(Vert **G, int ordem) {
    *G = (Vert*) malloc(sizeof(Vert) * ordem);
    if (!*G) {
        fprintf(stderr, "Erro de memoria ao criar grafo\n");
        exit(1);
    }
    for (int i = 0; i < ordem; ++i) {
        (*G)[i].nome = i;
        (*G)[i].prim = NULL;
    }
}

void destroiGrafo(Vert **G, int ordem) {
    if (!*G) return;
    for (int i = 0; i < ordem; ++i) {
        Arest *a = (*G)[i].prim;
        while (a) {
            Arest *n = a->prox;
            free(a);
            a = n;
        }
    }
    free(*G);
    *G = NULL;
}

int acrescentaAresta(Vert G[], int ordem, int v1, int v2, float peso) {
    if (v1 < 0 || v1 >= ordem || v2 < 0 || v2 >= ordem) return 0;

    Arest *A1 = (Arest*) malloc(sizeof(Arest));
    if (!A1) return 0;
    A1->extremo2 = v2;
    A1->peso = peso;
    A1->prox = G[v1].prim;
    G[v1].prim = A1;

    if (v1 == v2) return 1;

    Arest *A2 = (Arest*) malloc(sizeof(Arest));
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
    
    while (atual != -1 && atual != origem) {
        temp[count++] = atual;
        atual = anterior[atual];
    }
    temp[count++] = origem;
    
    *len = count;
    for (int i = 0; i < count; i++) {
        caminho[i] = temp[count - 1 - i];
    }
}

float calculaDistancia(Vert G[], int ordem, int origem, int destino, int *anterior) {
    float *dist = (float*) malloc(sizeof(float) * ordem);
    int *visitado = (int*) malloc(sizeof(int) * ordem);
    
    if (!dist || !visitado) {
        fprintf(stderr, "Erro de memoria em calculaDistancia\n");
        exit(1);
    }
    
    for (int i = 0; i < ordem; ++i) {
        dist[i] = FLT_MAX;
        visitado[i] = 0;
        if (anterior) anterior[i] = -1;
    }
    dist[origem] = 0.0f;
    
    for (int count = 0; count < ordem - 1; ++count) {
        float min = FLT_MAX;
        int u = -1;
        for (int i = 0; i < ordem; ++i) {
            if (!visitado[i] && dist[i] <= min) {
                min = dist[i];
                u = i;
            }
        }
        if (u == -1) break;
        visitado[u] = 1;
        if (u == destino) break;
        
        for (Arest *adj = G[u].prim; adj; adj = adj->prox) {
            int v = adj->extremo2;
            float w = adj->peso;
            if (!visitado[v] && dist[u] != FLT_MAX && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                if (anterior) anterior[v] = u;
            }
        }
    }
    
    float resultado = dist[destino];
    free(dist);
    free(visitado);
    return resultado;
}

float pesoAresta(Vert G[], int ordem, int v1, int v2) {
    if (v1 < 0 || v1 >= ordem || v2 < 0 || v2 >= ordem) return FLT_MAX;
    for (Arest *a = G[v1].prim; a; a = a->prox) {
        if (a->extremo2 == v2) return a->peso;
    }
    return FLT_MAX;
}

float distanciaEntrePontos(Vert G[], int ordem, PontoDeInteresse a, PontoDeInteresse b,
                           int *path_a, int *len_a, int *path_b, int *len_b) {
    int *ant = (int*) malloc(sizeof(int) * ordem);
    
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
    
    float d1 = calculaDistancia(G, ordem, a.v1, b.v1, ant);
    float opt1 = d1 + a.distV1 + b.distV1;
    
    float d2 = calculaDistancia(G, ordem, a.v1, b.v2, NULL);
    float opt2 = d2 + a.distV1 + b.distV2;
    
    float d3 = calculaDistancia(G, ordem, a.v2, b.v1, NULL);
    float opt3 = d3 + a.distV2 + b.distV1;
    
    float d4 = calculaDistancia(G, ordem, a.v2, b.v2, NULL);
    float opt4 = d4 + a.distV2 + b.distV2;
    
    float minDist = opt1;
    int melhor = 1;
    
    if (opt2 < minDist) { minDist = opt2; melhor = 2; }
    if (opt3 < minDist) { minDist = opt3; melhor = 3; }
    if (opt4 < minDist) { minDist = opt4; melhor = 4; }
    
    if (path_a && path_b && len_a && len_b) {
        int va_ini, va_fim, vb_ini, vb_fim;
        
        if (melhor == 1) { va_ini = a.v1; va_fim = a.v2; vb_ini = b.v1; vb_fim = b.v2; }
        else if (melhor == 2) { va_ini = a.v1; va_fim = a.v2; vb_ini = b.v2; vb_fim = b.v1; }
        else if (melhor == 3) { va_ini = a.v2; va_fim = a.v1; vb_ini = b.v1; vb_fim = b.v2; }
        else { va_ini = a.v2; va_fim = a.v1; vb_ini = b.v2; vb_fim = b.v1; }
        
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
    for (int i = 0; i <= num_locais; ++i) {
        for (int j = 0; j <= num_locais; ++j) {
            if (i == j) {
                matriz[i][j] = 0.0f;
            } else {
                PontoDeInteresse pi = (i == 0) ? casa : locais[i-1];
                PontoDeInteresse pj = (j == 0) ? casa : locais[j-1];
                matriz[i][j] = distanciaEntrePontos(G, ordem, pi, pj, NULL, NULL, NULL, NULL);
            }
        }
    }
}

void resolverTSP_VizinhoMaisProximo(float **matriz, int n, int *rota) {
    int *visitado = (int*) calloc(n, sizeof(int));
    
    rota[0] = 0;
    visitado[0] = 1;
    
    for (int i = 1; i < n; ++i) {
        int atual = rota[i-1];
        float min_dist = FLT_MAX;
        int proximo = -1;
        
        for (int j = 0; j < n; ++j) {
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
    
    while (melhorou) {
        melhorou = 0;
        for (int i = 1; i < n - 2; ++i) {
            for (int j = i + 1; j < n - 1; ++j) {
                float dist_antes = matriz[rota[i-1]][rota[i]] + matriz[rota[j]][rota[j+1]];
                float dist_depois = matriz[rota[i-1]][rota[j]] + matriz[rota[i]][rota[j+1]];
                
                if (dist_depois < dist_antes - 0.01f) {
                    int esq = i, dir = j;
                    while (esq < dir) {
                        int temp = rota[esq];
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
    
    float dist_ida = distanciaEntrePontos(G, ordem, casa, destino, path_ida, &len_ida, path_volta, &len_volta);
    float dist_volta = dist_ida;
    float total = dist_ida + dist_volta;
    
    printf("\n");
    printf("-----------------------------------------------------------\n");
    printf("                RESULTADO: IDA E VOLTA A UM LOCAL\n");
    printf("-----------------------------------------------------------\n\n");
    
    printf("TRAJETO DE IDA: Minha Casa -> %s\n", destino.nome);
    printf("--------------------------------------------------------------------------------\n");
    printf("  Vertices percorridos: ");
    for (int i = 0; i < len_ida; i++) {
        printf("V%d", path_ida[i] + 1);
        if (i < len_ida - 1) printf(" -> ");
    }
    printf("\n  Distancia: %.2f metros\n\n", dist_ida);
    
    printf("TRAJETO DE VOLTA: %s -> Minha Casa\n", destino.nome);
    printf("--------------------------------------------------------------------------------\n");
    printf("  Vertices percorridos: ");
    for (int i = len_ida - 1; i >= 0; i--) {
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
    printf("\n");
    printf("-----------------------------------------------------------\n");
    printf("            RESULTADO: PASSEIO VISITANDO MULTIPLOS LOCAIS\n");
    printf("-----------------------------------------------------------\n\n");
    
    printf("SEQUENCIA DE LOCAIS:\n");
    printf("--------------------------------------------------------------------------------\n");
    printf(" 1. Minha Casa (V%d-V%d)\n", casa.v1+1, casa.v2+1);
    for (int i = 1; i < num_visitar + 1; ++i) {
        int idx = rota[i];
        PontoDeInteresse local = locais[idx-1];
        printf(" %d. %s (V%d-V%d)\n", i+1, local.nome, local.v1+1, local.v2+1);
    }
    printf(" %d. Minha Casa (retorno)\n\n", num_visitar+2);
    
    printf("TRECHOS DO PASSEIO:\n");
    printf("================================================================================\n");
    float total = 0.0f;
    float dist_acumulada = 0.0f;
    
    for (int i = 0; i <= num_visitar; ++i) {
        int orig_idx = rota[i];
        int dest_idx = (i < num_visitar) ? rota[i+1] : rota[0];
        
        PontoDeInteresse origem = (orig_idx == 0) ? casa : locais[orig_idx-1];
        PontoDeInteresse destino = (dest_idx == 0) ? casa : locais[dest_idx-1];
        
        int path[100], len_path = 0, path_b[100], len_b = 0;
        float dist = distanciaEntrePontos(G, ordem, origem, destino, path, &len_path, path_b, &len_b);
        
        printf("\nTrecho %d: ", i+1);
        if (orig_idx == 0) printf("Minha Casa");
        else printf("%s", locais[orig_idx-1].nome);
        printf(" -> ");
        if (dest_idx == 0) printf("Minha Casa\n");
        else printf("%s\n", locais[dest_idx-1].nome);
        
        printf("  Vertices: ");
        for (int j = 0; j < len_path; j++) {
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
    printf("  [0] Sair\n\n");
    printf("-----------------------------------------------------------\n");
    printf("Opcao: ");
}

void listarLocais(PontoDeInteresse todos_locais[], int total) {
    printf("\n");
    printf("-----------------------------------------------------------\n");
    printf("                    LOCAIS DISPONIVEIS\n");
    printf("-----------------------------------------------------------\n\n");
    for (int i = 0; i < total; i++) {
        printf("  [%2d] %s (V%d-V%d)\n", i+1, todos_locais[i].nome, 
               todos_locais[i].v1+1, todos_locais[i].v2+1);
    }
    printf("\n-----------------------------------------------------------\n");
}

int selecionarLocal(PontoDeInteresse todos_locais[], int total) {
    listarLocais(todos_locais, total);
    int escolha;
    printf("\nEscolha o numero do local (1-%d): ", total);
    scanf("%d", &escolha);
    
    if (escolha < 1 || escolha > total) {
        printf("Opcao invalida! Usando local padrao (1).\n");
        return 0;
    }
    return escolha - 1;
}

void selecionarMultiplosLocais(PontoDeInteresse todos_locais[], int total, int *indices, int *num) {
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
    for (int i = 0; i < *num; i++) {
        int escolha;
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

int main(void) {
    Vert *G;
    const int ordemG = 39;
    criaGrafo(&G, ordemG);

    // Construindo o grafo com todas as arestas
    acrescentaAresta(G, ordemG, 0, 1, 213.2f);
    acrescentaAresta(G, ordemG, 1, 2, 119.79f);
    acrescentaAresta(G, ordemG, 2, 3, 46.35f);
    acrescentaAresta(G, ordemG, 3, 4, 48.27f);
    acrescentaAresta(G, ordemG, 0, 5, 111.17f);
    acrescentaAresta(G, ordemG, 1, 6, 110.55f);
    acrescentaAresta(G, ordemG, 2, 7, 110.94f);
    acrescentaAresta(G, ordemG, 3, 8, 111.14f);
    acrescentaAresta(G, ordemG, 4, 9, 111.27f);
    acrescentaAresta(G, ordemG, 5, 6, 209.99f);
    acrescentaAresta(G, ordemG, 6, 7, 119.42f);
    acrescentaAresta(G, ordemG, 7, 8, 45.99f);
    acrescentaAresta(G, ordemG, 8, 9, 46.92f);
    acrescentaAresta(G, ordemG, 5, 11, 116.47f);
    acrescentaAresta(G, ordemG, 6, 12, 114.34f);
    acrescentaAresta(G, ordemG, 9, 14, 114.57f);
    acrescentaAresta(G, ordemG, 10, 11, 150.16f);
    acrescentaAresta(G, ordemG, 11, 12, 211.10f);
    acrescentaAresta(G, ordemG, 12, 13, 96.89f);
    acrescentaAresta(G, ordemG, 13, 14, 118.29f);
    acrescentaAresta(G, ordemG, 14, 15, 212.88f);
    acrescentaAresta(G, ordemG, 11, 16, 123.31f);
    acrescentaAresta(G, ordemG, 12, 17, 116.35f);
    acrescentaAresta(G, ordemG, 13, 18, 114.87f);
    acrescentaAresta(G, ordemG, 14, 20, 121.40f);
    acrescentaAresta(G, ordemG, 15, 21, 127.81f);
    acrescentaAresta(G, ordemG, 16, 17, 211.79f);
    acrescentaAresta(G, ordemG, 17, 18, 93.76f);
    acrescentaAresta(G, ordemG, 18, 19, 41.71f);
    acrescentaAresta(G, ordemG, 19, 20, 79.83f);
    acrescentaAresta(G, ordemG, 20, 21, 212.83f);
    acrescentaAresta(G, ordemG, 16, 22, 112.52f);
    acrescentaAresta(G, ordemG, 17, 23, 111.62f);
    acrescentaAresta(G, ordemG, 19, 24, 107.50f);
    acrescentaAresta(G, ordemG, 20, 25, 110.43f);
    acrescentaAresta(G, ordemG, 21, 26, 112.87f);
    acrescentaAresta(G, ordemG, 22, 23, 207.67f);
    acrescentaAresta(G, ordemG, 23, 24, 135.43f);
    acrescentaAresta(G, ordemG, 24, 28, 78.97f);
    acrescentaAresta(G, ordemG, 25, 26, 212.41f);
    acrescentaAresta(G, ordemG, 22, 27, 114.91f);
    acrescentaAresta(G, ordemG, 23, 28, 117.53f);
    acrescentaAresta(G, ordemG, 25, 29, 115.58f);
    acrescentaAresta(G, ordemG, 26, 30, 115.71f);
    acrescentaAresta(G, ordemG, 27, 28, 206.12f);
    acrescentaAresta(G, ordemG, 28, 29, 219.46f);
    acrescentaAresta(G, ordemG, 29, 30, 212.37f);
    acrescentaAresta(G, ordemG, 27, 31, 118.35f);
    acrescentaAresta(G, ordemG, 28, 32, 118.79f);
    acrescentaAresta(G, ordemG, 29, 33, 116.25f);
    acrescentaAresta(G, ordemG, 30, 34, 117.29f);
    acrescentaAresta(G, ordemG, 31, 32, 207.44f);
    acrescentaAresta(G, ordemG, 32, 33, 214.40f);
    acrescentaAresta(G, ordemG, 33, 34, 213.80f);
    acrescentaAresta(G, ordemG, 31, 35, 116.70f);
    acrescentaAresta(G, ordemG, 32, 36, 114.20f);
    acrescentaAresta(G, ordemG, 33, 37, 114.73f);
    acrescentaAresta(G, ordemG, 34, 38, 116.68f);
    acrescentaAresta(G, ordemG, 35, 36, 211.53f);
    acrescentaAresta(G, ordemG, 36, 37, 219.10f);
    acrescentaAresta(G, ordemG, 37, 38, 214.51f);

    PontoDeInteresse minha_casa = {"Minha Casa", 23, 24, 93.35f, 42.08f};
    
    PontoDeInteresse todos_locais[] = {
        {"UBS Campestre", 10, 11, 31.14f, 119.02f},
        {"Auto Eletrica", 11, 16, 91.67f, 31.64f},
        {"Chaveiro Garrido", 11, 16, 68.84f, 54.47f},
        {"Ponto de onibus 1", 11, 12, 111.91f, 99.19f},
        {"Great Kids School", 6, 12, 62.08f, 52.26f},
        {"Verzani e Sandrini", 14, 20, 68.21f, 53.19f},
        {"Green Residence", 14, 15, 134.47f, 78.41f},
        {"Smartfit", 26, 30, 96.59f, 16.28f},
        {"Paroquia Sao Judas", 22, 23, 68.77f, 138.90f},
        {"Ponto de onibus 2", 25, 29, 50.55f, 61.88f},
        {"Uooba Buffet", 27, 28, 64.73f, 141.39f},
        {"Armando Nissan", 27, 28, 52.22f, 153.90f},
        {"Vetnasa Clinica", 28, 32, 96.14f, 22.65f},
        {"Vila Safari", 28, 29, 114.61f, 104.85f},
        {"Clinica Veterinaria", 28, 29, 202.01f, 17.45f},
        {"Alo Bebe", 29, 30, 80.69f, 131.68f},
        {"Actos", 36, 37, 205.54f, 13.56f},
        {"Central IBC", 36, 37, 20.02f, 199.08f},
        {"Ponto de onibus 3", 33, 37, 60.81f, 53.92f},
        {"Coop", 34, 38, 39.10f, 77.58f}
    };
    
    int total_locais = sizeof(todos_locais) / sizeof(todos_locais[0]);
    int opcao;
    
    do {
        exibirMenu();
        scanf("%d", &opcao);
        
        switch(opcao) {
            case 1: {
                // MODO 1: Visitar um único local
                printf("\n=== MODO 1: IDA E VOLTA A UM LOCAL ===\n");
                int indice_destino = selecionarLocal(todos_locais, total_locais);
                
                printf("\nCalculando rota...\n");
                exibirResultadosSimples(G, ordemG, minha_casa, todos_locais[indice_destino]);
                
                printf("\nPressione ENTER para continuar...");
                getchar(); // Limpa buffer
                getchar(); // Aguarda ENTER
                break;
            }
            
            case 2: {
                // MODO 2: Visitar múltiplos locais (TSP)
                printf("\n=== MODO 2: VISITAR MULTIPLOS LOCAIS ===\n");
                
                int indices_visitar[20];
                int num_visitar;
                
                selecionarMultiplosLocais(todos_locais, total_locais, indices_visitar, &num_visitar);
                
                printf("\nLocais selecionados:\n");
                for (int i = 0; i < num_visitar; i++) {
                    printf("  %d. %s\n", i+1, todos_locais[indices_visitar[i]].nome);
                }
                
                printf("\nCalculando distancias...\n");
                int n = num_visitar + 1;
                float **matriz = (float**) malloc(sizeof(float*) * n);
                for (int i = 0; i < n; ++i) {
                    matriz[i] = (float*) malloc(sizeof(float) * n);
                }
                
                PontoDeInteresse *locais_visitar = (PontoDeInteresse*) malloc(sizeof(PontoDeInteresse) * num_visitar);
                for (int i = 0; i < num_visitar; ++i) {
                    locais_visitar[i] = todos_locais[indices_visitar[i]];
                }
                
                calcularMatrizDistancias(G, ordemG, minha_casa, locais_visitar, num_visitar, matriz);
                
                printf("Calculando melhor rota (Vizinho Mais Proximo + 2-opt)...\n");
                int *rota = (int*) malloc(sizeof(int) * n);
                resolverTSP_VizinhoMaisProximo(matriz, n, rota);
                melhorar2opt(matriz, n, rota);
                
                exibirResultadosCompleto(G, ordemG, minha_casa, locais_visitar, indices_visitar, num_visitar, rota, matriz);
                
                // Liberar memória
                for (int i = 0; i < n; ++i) {
                    free(matriz[i]);
                }
                free(matriz);
                free(locais_visitar);
                free(rota);
                
                printf("\nPressione ENTER para continuar...");
                getchar(); // Limpa buffer
                getchar(); // Aguarda ENTER
                break;
            }
            
            case 0:
                printf("\nEncerrando programa...\n");
                break;
                
            default:
                printf("\nOpcao invalida! Tente novamente.\n");
                printf("\nPressione ENTER para continuar...");
                getchar(); // Limpa buffer
                getchar(); // Aguarda ENTER
                break;
        }
        
    } while (opcao != 0);
    
    // Destruir grafo e liberar memória
    destroiGrafo(&G, ordemG);
    
    printf("\nPrograma finalizado com sucesso.\n\n");
    
    return 0;
}