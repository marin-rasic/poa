#include "graph.hpp"

#include <queue>

Node *Graph::addNewNode(char letter, const char *sequence_id, unsigned int index, Node *prev_node) {
    Node *new_node = new Node(letter, sequence_id, index);
    if (prev_node != nullptr) {
        Edge *edge = new Edge(new_node, prev_node);
        new_node->outgoing_edges.push_back(edge);
        prev_node->incoming_edges.push_back(edge);
    }
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

    //resetiranje varijable num_remain_edges za moguće buduće topološko sortiranje
    for (Node *node : sorted_nodes) {
        node->num_remain_edges = -1;
    }

    return sorted_nodes;
}

void LinearGraph(Graph &empty_graph, const char *sequence, unsigned int sequence_len, const char *sequence_id) {
    Node *new_node;
    Node *prev_node = nullptr;
    for (int i = 0; i < sequence_len; i++) {
        new_node = new Node(sequence[i], sequence_id, i + 1);
        if (prev_node) {
            Edge *edge = new Edge(prev_node, new_node);
            prev_node->outgoing_edges.push_back(edge);
            new_node->incoming_edges.push_back(edge);
        } else {
            empty_graph.start_nodes.push_back(new_node);
        }
        prev_node = new_node;
    }
}