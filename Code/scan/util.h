#include "defs.h"

Graph* read_graph_from_file(const char*);
void write_graph_to_file(const char*, Graph*);
Tree* read_tree_from_file(const char *);
void write_tree_to_file(const char *, Tree *);
void free_graph(Graph*);
void free_tree(Tree*);
