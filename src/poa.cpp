#include <iostream>
#include <stack>

#include "alignment.hpp"

int main(void) {
    Aligner *aligner = new GlobalAligner(1, -1, -1);
    Graph graph1;
    aligner->AlignAndGraphTwoSeq(graph1, "ACT", 3, "prva", "ACG", 3, "druga");

    aligner->AlignAndGraphSeqAndGraph(graph1, "ATT", 3, "treca");

    Graph graph2;
    aligner->AlignAndGraphTwoSeq(graph2, "GTC", 3, "cetvrta", "GGG", 3, "peta");

    aligner->AlignAndGraphTwoGraph(graph1, graph2);

    std::vector<Node *> nodes = graph1.TopologicalSort();
    for (Node *node : nodes) {
        std::cout << node->letter << " ";
        for (std::tuple<const char *, unsigned int> origin : node->origin_of_letter) {
            std::cout << std::get<0>(origin) << " ";
        }
        std::cout << std::endl;
    }
    return 0;
}
