#ifndef Graph_h
#define Graph_h

typedef struct vertex {
    unsigned int degree;  /* out-degree i.e. num edges starting from this node */
    struct vertex **connections;
} Vertex_t;

typedef struct {
    unsigned int num_nodes;
    Vertex_t *nodes;
} Graph_t;

Graph_t *create_graph(unsigned int num_nodes);
void free_graph(Graph_t *graph);
void add_connection(Graph_t *graph, unsigned int from_idx, unsigned int to_idx);
int contains_cycle(const Graph_t *graph);

#endif
