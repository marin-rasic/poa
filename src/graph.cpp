#include "graph.hpp"

#include <queue>

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
                //resetiranje varijable num_remain_edges za moguće buduće topološko sortiranje
                node->num_remain_edges = -1;
            }
        }
    }

    return sorted_nodes;
}

void Graph::LinearGraph(Graph &empty_graph, const char *sequence, unsigned int sequence_len, const char *sequence_id) {
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

void Node::align_two_nodes(Node *a, Node *b, bool fuse, Graph &target) {
    std::vector<Node *> a_aligned;

    for (Node *node : b->aligned_nodes) {
        bool align_to_a = true;

        for (Node *node_a : a->aligned_nodes) {
            if (node->letter == node_a->letter) {
                fuse_two_nodes(node_a, node, false, target);
                align_to_a = false;
            } else {
                node_a->aligned_nodes.push_back(node);
                node->aligned_nodes.push_back(node_a);
            }
        }

        if (align_to_a) {
            a_aligned.push_back(node);
            node->aligned_nodes.push_back(a);

            if (fuse) {
                node->aligned_nodes.erase(std::remove(node->aligned_nodes.begin(),
                                                      node->aligned_nodes.end(),
                                                      b),
                                          node->aligned_nodes.end());
            }
        }
    }

    if (!fuse) {
        b->aligned_nodes.push_back(a);

        for (Node *node : a->aligned_nodes) {
            node->aligned_nodes.push_back(b);
            b->aligned_nodes.push_back(node);
        }
        a->aligned_nodes.push_back(b);
    }

    for (Node *node : a_aligned) {
        a->aligned_nodes.push_back(node);
    }
}

void Node::fuse_two_nodes(Node *a, Node *b, bool align, Graph &target) {
    // add all origins from target node to query node
    for (std::tuple<const char *, unsigned int> origin : b->origin_of_letter) {
        a->origin_of_letter.push_back(origin);
    }

    if (align) {
        Node::align_two_nodes(a, b, true, target);
    }

    // change destination of all incoming edges of target node to query node
    for (Edge *edge : b->incoming_edges) {
        edge->destination = a;
        a->incoming_edges.push_back(edge);
    }

    for (Edge *edge : b->outgoing_edges) {
        // prevents creation of duplicate edges
        bool add_edge = true;
        for (Edge *dest_edge : edge->destination->incoming_edges) {
            if (dest_edge->origin == a) {
                add_edge = false;
                break;
            }
        }
        if (!add_edge) {
            for (auto it = edge->destination->incoming_edges.begin(); it != edge->destination->incoming_edges.end(); it++) {
                if ((*it)->origin == b) {
                    edge->destination->incoming_edges.erase(it);
                    break;
                }
            }
        } else {
            edge->origin = a;
            a->outgoing_edges.push_back(edge);
        }
    }

    //if b is starting node in target graph, remove it from target graphs starting nodes
    if ((b->incoming_edges).empty()) {
        target.start_nodes.erase(std::remove(target.start_nodes.begin(),
                                             target.start_nodes.end(),
                                             b),
                                 target.start_nodes.end());
    }
    // frees target node from memory
    delete b;
}