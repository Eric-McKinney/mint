#include <stdlib.h>
#include "graph.h"

Graph_t *create_graph(unsigned int num_nodes) {
    Graph_t *graph = malloc(sizeof(Graph_t));

    graph->num_nodes = num_nodes;
    graph->nodes = calloc(num_nodes, sizeof(Vertex_t));

    return graph;
}

void add_connection(Graph_t *graph, unsigned int from_idx, unsigned int to_idx) {
    if (from_idx >= graph->num_nodes || to_idx >= graph->num_nodes) {
        return;
    }

    Vertex_t *from = graph->nodes + from_idx;
    Vertex_t *to = graph->nodes + to_idx;

    from->degree++;
    void *ptr = realloc(from->connections, from->degree * sizeof(Vertex_t *));

    if (ptr != from->connections) {
        free(from->connections);
        from->connections = ptr;
    }

    from->connections[from->degree - 1] = to;
}

void free_graph(Graph_t *graph) {
    for (unsigned int i = 0; i < graph->num_nodes; i++) {
        Vertex_t *node = graph->nodes + i;
        free(node->connections);
    }

    free(graph->nodes);
    free(graph);
}

static int contains(Vertex_t **set, const Vertex_t *node, unsigned int num_nodes) {
    for (unsigned int i = 0; i < num_nodes; i++) {
        if (set[i] == node) {
            return 1;
        }
    }

    return 0;
}

static int subgraph_has_cycle(unsigned int num_nodes, Vertex_t *start) {
    Vertex_t *visited[num_nodes + 1];
    Vertex_t *to_visit[num_nodes + 1];
    unsigned int nodes_visited = 0;
    unsigned int to_visit_queue_end = 1;

    for (unsigned int i = 0; i < num_nodes + 1; i++) {
        visited[i] = NULL;
        to_visit[i] = NULL;
    }

    to_visit[0] = start;
    while (to_visit[nodes_visited] != NULL) {
        Vertex_t *curr_node = to_visit[nodes_visited];
        Vertex_t **neighbors = (Vertex_t **) curr_node->connections;
        visited[nodes_visited] = curr_node;

        for (unsigned int i = 0; i < curr_node->degree; i++) {
            if (contains(visited, neighbors[i], num_nodes)) {
                return 1;
            }

            to_visit[to_visit_queue_end++] = neighbors[i];
        }

        nodes_visited++;
    }

    return 0;
}

int contains_cycle(const Graph_t *graph) {
    if (graph->num_nodes <= 1) {
        return 0;
    }

    for (unsigned int i = 0; i < graph->num_nodes; i++) {
        if (subgraph_has_cycle(graph->num_nodes, graph->nodes + i)) {
            return 1;
        }
    }

    return 0;
}
