typedef struct _vertex_t Vertex;
typedef struct _edge_t Edge;
typedef struct _graph_t Graph;

typedef struct _node_t Node;
typedef struct _tree_t Tree;
typedef struct _euler_node_t ENode;
typedef struct _stack_t Stack;

typedef int item_t;

struct _vertex_t {
  unsigned int first; // Position of the first incident edge of a vertex in E
  unsigned int last; // Position of the last incident edge of a vertex in E
};

struct _graph_t {
  Vertex* V; // Array of vertices of the graph
  Edge* E; // Array of edges of the graph. It is the concatenation of the
           // adjacency lists of all vertices 
  unsigned int n; // Number of vertices in the graph
  unsigned int m; // Number of non-repeated edges in the graph
};

// Used to represent graphs and trees
struct _edge_t {
  unsigned int src; // Index of the source vertex of the edge
  unsigned int tgt; // Index of the target vertex of the edge
  int cmp; // Position of the complementary edge (in the adjacency list of
	   // tgt). cmp < 0 means that the field cmp is undefined
};

// Used to represent trees
struct _node_t {
  unsigned int first; // Position of the first incident edge of a node in E
  unsigned int last; // Position of the last incident edge of a node in E
};

struct _tree_t {
  Node* N; // Array of nodes of the tree
  Edge* E; // Array of edges of the tree. It is the concatenation of the adjacency lists of all nodes
  unsigned int n; // Number of nodes in the tree
  // The number of edges is n-1
};

struct _euler_node_t {
  unsigned int value;
  int next; // stores the index of the succesor in the array. Since
	    // the parallel_list_ranking algorithm uses this fields to
	    // store some negative values, it must be int instead of uint.
};

struct _stack_t {
  item_t *base;
  unsigned int top;
  unsigned int size;
};

Stack *stack_create(unsigned int size);
int stack_empty(Stack *);
int stack_full(Stack *);
int stack_push(Stack *, item_t);
item_t stack_pop(Stack *);
item_t stack_top(Stack *);
void stack_free(Stack *);

int tree_cycle(Tree *);
