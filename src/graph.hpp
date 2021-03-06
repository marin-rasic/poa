#include <algorithm>
#include <tuple>
#include <vector>

class Graph;

class Edge;

class Node {
   public:
    char letter;

    //id of sequences from which the letter comes from and the index of the letter within those sequences
    std::vector<std::tuple<const char *, unsigned int>> origin_of_letter;

    std::vector<Edge *> incoming_edges;
    std::vector<Edge *> outgoing_edges;

    //other nodes this node is aligned to
    std::vector<Node *> aligned_nodes;

    //number of remaining incoming edges - used for topological sort
    int num_remain_edges = -1;
    //index in topological sort
    int index;

    //used for searching consensus paths
    int consensus_score = 0;
    Edge *consensus_edge = nullptr;

    Node(char letter) : letter(letter){};
    Node(char letter, const char *sequence_id, unsigned int index) : letter(letter) {
        std::tuple<const char *, unsigned int> origin(sequence_id, index);
        origin_of_letter.push_back(origin);
    };

    // aligns two nodes
    static void AlignTwoNodes(Node *a, Node *b, bool fuse, Graph &target);
    // fuses two nodes
    static void FuseTwoNodes(Node *a, Node *b, bool align, Graph &target);
};

class Edge {
   public:
    Node *origin;
    Node *destination;

    int CalculateConsensusScore();

    Edge(Node *origin, Node *destination) : origin(origin), destination(destination){};
};

class Graph {
   public:
    std::vector<Node *> start_nodes;

    std::vector<Node *> TopologicalSort();

    //creates a POA graph from sequence
    static void LinearGraph(Graph &empty_graph, const char *sequence, unsigned int sequence_len, const char *sequence_id);

    std::string FindConsensus();
};
