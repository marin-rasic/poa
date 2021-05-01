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

    aligner->AlignAndGraphSeqAndGraph(graph1, "ACCC", 4, "treca", 1, -1, -1);
    std::vector<Node *> nodes2 = graph1.TopologicalSort();
    for (Node *node : nodes2) {
        std::cout << node->letter << " ";
    }
    std::cout << std::endl;
}
