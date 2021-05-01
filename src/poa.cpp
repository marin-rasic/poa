#include <iostream>
#include <stack>

#include "alignment.hpp"

int main(void) {
    Aligner *aligner = new GlobalAligner();

    Graph graph1;
    aligner->AlignAndGraphTwoSeq(graph1, "ACGT", 4, "prva", "AGGT", 4, "druga", 1, -1, -1);
    std::vector<Node *> nodes = graph1.TopologicalSort();
    for (Node *node : nodes) {
        std::cout << node->letter << " ";
    }
    std::cout << std::endl;

    Graph graph2;
    aligner->AlignAndGraphTwoSeq(graph2, "ATT", 3, "cetvrta", "AGT", 3, "peta", 1, -1, -1);
    std::vector<Node *> nodes3 = graph2.TopologicalSort();
    for (Node *node : nodes3) {
        std::cout << node->letter << " ";
    }
    std::cout << std::endl;

    aligner->AlignAndGraphTwoGraph(graph1, graph2, 1, -1, -1);
    std::vector<Node *> nodes4 = graph1.TopologicalSort();
    for (Node *node : nodes4) {
        std::cout << node->letter << " ";
    }

    std::cout << std::endl;
    return 0;
}
