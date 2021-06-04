#include "graph.hpp"

#include <queue>
#include <unordered_map>

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

void Node::AlignTwoNodes(Node *a, Node *b, bool fuse, Graph &target) {
    std::vector<Node *> a_aligned;

    for (Node *node : b->aligned_nodes) {
        bool align_to_a = true;

        for (Node *node_a : a->aligned_nodes) {
            if (node->letter == node_a->letter) {
                FuseTwoNodes(node_a, node, false, target);
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

void Node::FuseTwoNodes(Node *a, Node *b, bool align, Graph &target) {
    // add all origins from target node to query node
    for (std::tuple<const char *, unsigned int> origin : b->origin_of_letter) {
        a->origin_of_letter.push_back(origin);
    }

    if (align) {
        Node::AlignTwoNodes(a, b, true, target);
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
}

std::string BuildConsensus(Node *node) {
    std::string consensus = "";
    consensus = node->letter + consensus;
    do {
        node = node->consensus_edge->origin;
        consensus = node->letter + consensus;
    } while (!node->incoming_edges.empty());

    return consensus;
}

int Edge::CalculateConsensusScore() {
    std::unordered_map<const char *, bool> origins;
    int number_of_seq = 0;
    for (auto orig : this->origin->origin_of_letter) {
        origins[std::get<0>(orig)] = true;
    }
    for (auto orig : this->destination->origin_of_letter) {
        if (origins.find(std::get<0>(orig)) != origins.end()) {
            number_of_seq++;
        }
    }
    return number_of_seq;
}

int TraverseGraph(std::vector<Node *> &graph, int starting_index = 0) {
    int best_node_index = -1;
    int best_node_score = 0;

    for (int i = starting_index + 1; i < graph.size(); i++) {
        Edge *best_edge = nullptr;
        int best_edge_score = 0;

        for (Edge *incoming_edge : graph[i]->incoming_edges) {
            int edge_score = incoming_edge->CalculateConsensusScore();

            if (best_edge) {
                if (best_edge->origin->consensus_score + best_edge_score <=
                    incoming_edge->origin->consensus_score + edge_score) {
                    best_edge = incoming_edge;
                    best_edge_score = edge_score;
                }
            } else {
                best_edge = incoming_edge;
                best_edge_score = edge_score;
            }
        }

        if (best_edge) {
            graph[i]->consensus_score += best_edge_score + best_edge->origin->consensus_score;
            graph[i]->consensus_edge = best_edge;
            if (best_node_index == -1) {
                best_node_index = i;
                best_node_score = graph[i]->consensus_score;
            } else {
                best_node_index = graph[best_node_index]->consensus_score > graph[i]->consensus_score
                                      ? best_node_index
                                      : i;
            }
        }
    }

    return best_node_index;
}

std::string Graph::FindConsensus() {
    std::vector<Node *> top_sort = this->TopologicalSort();
    int best_node_index = TraverseGraph(top_sort, -1);

    if (!top_sort[best_node_index]->outgoing_edges.empty()) {
        for (Node *node : top_sort) {
            if (node != top_sort[best_node_index]) {
                node->consensus_score = -1;
            }
        }
        best_node_index = TraverseGraph(top_sort, best_node_index);
    }

    return BuildConsensus(top_sort[best_node_index]);
}