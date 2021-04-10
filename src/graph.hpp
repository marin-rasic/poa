#include <string>
#include <tuple>
#include <vector>

class Edge;

class Node {
   public:
    char letter;

    //vektor koji sprema podatak o id niza iz kojeg slovo dolazi i poziciji tog slova u tom nizu
    std::vector<std::tuple<const char *, unsigned int>> origin_of_letter;

    std::vector<Edge *> incoming_edges;
    std::vector<Edge *> outgoing_edges;

    //number of remaining incoming edges - used for topological sort
    int num_remain_edges;

    Node(char letter) : letter(letter){};
    Node(char letter, const char *sequence_id, unsigned int index) : letter(letter) {
        std::tuple<const char *, unsigned int> origin(sequence_id, index);
        origin_of_letter.push_back(origin);
    };
};

class Edge {
   public:
    Node *origin;
    Node *destination;

    Edge(Node *origin, Node *destination) : origin(origin), destination(destination){};
};

class DAGGraph {
   public:
    std::vector<Node *> start_nodes;

    Node *addNode(char letter, const char *sequence_id, unsigned int index, Node *prevNode = nullptr);

    std::vector<Node *> TopologicalSort();
};

void AlignAndCreateGraph(DAGGraph &graph, const char *query, unsigned int query_len, const char *query_id, const char *target,
                         unsigned int target_len, const char *target_id, int match, int mismatch, int gap);
