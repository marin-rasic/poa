#include "graph.hpp"

#include <queue>

Node *Graph::addNode(char letter, const char *sequence_id, unsigned int index, Node *prev_node) {
    Node *new_node = new Node(letter, sequence_id, index);
    if (prev_node == nullptr) {  //početni node
        this->start_nodes.push_back(new_node);
    } else {
        Edge *edge = new Edge(prev_node, new_node);
        prev_node->outgoing_edges.push_back(edge);
        new_node->incoming_edges.push_back(edge);
    }
    size++;
    return new_node;
}

std::vector<Node *> Graph::TopologicalSort() {
    std::vector<Node *> sorted_nodes;

    //čvorovi koji nemaju ulazne rubove
    std::queue<Node *> nodes;

    for (Node *n : this->start_nodes) {
        nodes.push(n);
    }
    int topological_index = 1;
    while (!nodes.empty()) {
        Node *node = nodes.front();
        nodes.pop();
        node->index = topological_index;
        topological_index++;

        sorted_nodes.push_back(node);
        for (Edge *e : node->outgoing_edges) {
            //ako prvi put prolazimo kroz čvor postavi mu broj preostalih ulazećih rubova
            if (e->destination->num_remain_edges == -1) {
                e->destination->num_remain_edges = e->destination->incoming_edges.size();
            }
            //ako cvor vise nema ulaznih rubova dodamo ga u sortiranu listu
            if (--e->destination->num_remain_edges == 0) {
                nodes.push(e->destination);
            }
        }
    }
    return sorted_nodes;
}
