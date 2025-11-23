#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_VERTICES 100

// Estrutura para representar um nó na lista de adjacência
typedef struct Node {
    int vertex;
    struct Node* next;
} Node;

// Estrutura para representar um grafo
typedef struct Graph {
    int numVertices;
    Node** adjLists;
    bool* visited;
} Graph;

// Estrutura para fila (usada no BFS)
typedef struct Queue {
    int items[MAX_VERTICES];
    int front;
    int rear;
} Queue;

// Funções da fila
Queue* createQueue() {
    Queue* q = malloc(sizeof(Queue));
    q->front = -1;
    q->rear = -1;
    return q;
}

bool isEmpty(Queue* q) {
    return q->rear == -1;
}

void enqueue(Queue* q, int value) {
    if (q->rear == MAX_VERTICES - 1) {
        printf("Fila cheia!\n");
        return;
    }
    if (q->front == -1)
        q->front = 0;
    q->rear++;
    q->items[q->rear] = value;
}

int dequeue(Queue* q) {
    int item;
    if (isEmpty(q)) {
        printf("Fila vazia!\n");
        return -1;
    }
    item = q->items[q->front];
    q->front++;
    if (q->front > q->rear) {
        q->front = q->rear = -1;
    }
    return item;
}

// Criar um novo nó
Node* createNode(int v) {
    Node* newNode = malloc(sizeof(Node));
    newNode->vertex = v;
    newNode->next = NULL;
    return newNode;
}

// Criar um grafo com n vértices
Graph* createGraph(int vertices) {
    Graph* graph = malloc(sizeof(Graph));
    graph->numVertices = vertices;
    
    graph->adjLists = malloc(vertices * sizeof(Node*));
    graph->visited = malloc(vertices * sizeof(bool));
    
    for (int i = 0; i < vertices; i++) {
        graph->adjLists[i] = NULL;
        graph->visited[i] = false;
    }
    
    return graph;
}

// Adicionar aresta ao grafo (grafo não direcionado)
void addEdge(Graph* graph, int src, int dest) {
    // Adicionar aresta de src para dest
    Node* newNode = createNode(dest);
    newNode->next = graph->adjLists[src];
    graph->adjLists[src] = newNode;
    
    // Adicionar aresta de dest para src (não direcionado)
    newNode = createNode(src);
    newNode->next = graph->adjLists[dest];
    graph->adjLists[dest] = newNode;
}

// Adicionar aresta direcionada
void addDirectedEdge(Graph* graph, int src, int dest) {
    Node* newNode = createNode(dest);
    newNode->next = graph->adjLists[src];
    graph->adjLists[src] = newNode;
}

// Imprimir o grafo
void printGraph(Graph* graph) {
    printf("\nRepresentação do Grafo (Lista de Adjacência):\n");
    for (int v = 0; v < graph->numVertices; v++) {
        Node* temp = graph->adjLists[v];
        printf("Vértice %d: ", v);
        while (temp) {
            printf("%d -> ", temp->vertex);
            temp = temp->next;
        }
        printf("NULL\n");
    }
}

// Busca em Profundidade (DFS) - Recursiva
void DFS(Graph* graph, int vertex) {
    Node* adjList = graph->adjLists[vertex];
    
    graph->visited[vertex] = true;
    printf("%d ", vertex);
    
    while (adjList != NULL) {
        int connectedVertex = adjList->vertex;
        if (!graph->visited[connectedVertex]) {
            DFS(graph, connectedVertex);
        }
        adjList = adjList->next;
    }
}

// Resetar array de visitados
void resetVisited(Graph* graph) {
    for (int i = 0; i < graph->numVertices; i++) {
        graph->visited[i] = false;
    }
}

// Busca em Largura (BFS)
void BFS(Graph* graph, int startVertex) {
    Queue* q = createQueue();
    
    resetVisited(graph);
    graph->visited[startVertex] = true;
    enqueue(q, startVertex);
    
    printf("\nBusca em Largura (BFS) começando do vértice %d:\n", startVertex);
    
    while (!isEmpty(q)) {
        int currentVertex = dequeue(q);
        printf("%d ", currentVertex);
        
        Node* temp = graph->adjLists[currentVertex];
        
        while (temp) {
            int adjVertex = temp->vertex;
            
            if (!graph->visited[adjVertex]) {
                graph->visited[adjVertex] = true;
                enqueue(q, adjVertex);
            }
            temp = temp->next;
        }
    }
    printf("\n");
    free(q);
}

// Verificar se há caminho entre dois vértices
bool hasPath(Graph* graph, int src, int dest) {
    if (src == dest) return true;
    
    resetVisited(graph);
    Queue* q = createQueue();
    
    graph->visited[src] = true;
    enqueue(q, src);
    
    while (!isEmpty(q)) {
        int currentVertex = dequeue(q);
        
        if (currentVertex == dest) {
            free(q);
            return true;
        }
        
        Node* temp = graph->adjLists[currentVertex];
        
        while (temp) {
            int adjVertex = temp->vertex;
            
            if (!graph->visited[adjVertex]) {
                graph->visited[adjVertex] = true;
                enqueue(q, adjVertex);
            }
            temp = temp->next;
        }
    }
    
    free(q);
    return false;
}

// Liberar memória do grafo
void freeGraph(Graph* graph) {
    if (graph) {
        for (int v = 0; v < graph->numVertices; v++) {
            Node* temp = graph->adjLists[v];
            while (temp) {
                Node* toFree = temp;
                temp = temp->next;
                free(toFree);
            }
        }
        free(graph->adjLists);
        free(graph->visited);
        free(graph);
    }
}

// Função principal com exemplos de uso
int main() {
    printf("=== PROGRAMA DE GRAFOS ===\n\n");
    
    // Criar um grafo com 6 vértices
    Graph* graph = createGraph(6);
    
    // Adicionar arestas
    printf("Adicionando arestas ao grafo...\n");
    addEdge(graph, 0, 1);
    addEdge(graph, 0, 2);
    addEdge(graph, 1, 3);
    addEdge(graph, 1, 4);
    addEdge(graph, 2, 4);
    addEdge(graph, 3, 5);
    addEdge(graph, 4, 5);
    
    // Imprimir o grafo
    printGraph(graph);
    
    // Executar BFS
    BFS(graph, 0);
    
    // Executar DFS
    printf("\nBusca em Profundidade (DFS) começando do vértice 0:\n");
    resetVisited(graph);
    DFS(graph, 0);
    printf("\n");
    
    // Verificar se existe caminho entre vértices
    printf("\nVerificando caminhos:\n");
    printf("Caminho entre 0 e 5: %s\n", hasPath(graph, 0, 5) ? "Sim" : "Não");
    printf("Caminho entre 0 e 5: %s\n", hasPath(graph, 0, 5) ? "Sim" : "Não");
    
    // Criar um segundo grafo direcionado como exemplo
    printf("\n\n=== GRAFO DIRECIONADO ===\n");
    Graph* directedGraph = createGraph(4);
    addDirectedEdge(directedGraph, 0, 1);
    addDirectedEdge(directedGraph, 0, 2);
    addDirectedEdge(directedGraph, 1, 2);
    addDirectedEdge(directedGraph, 2, 0);
    addDirectedEdge(directedGraph, 2, 3);
    addDirectedEdge(directedGraph, 3, 3);
    
    printGraph(directedGraph);
    
    BFS(directedGraph, 2);
    
    // Liberar memória
    freeGraph(graph);
    freeGraph(directedGraph);
    
    printf("\nPrograma finalizado com sucesso!\n");
    
    return 0;
}
