#include "alignment.hpp"

#include <algorithm>

//određuje koja je Cell veća, priotizirajući one koje imaju veci index reda
bool max_cell(Cell a, Cell b) {
    return a.value < b.value || a.value == b.value && std::get<0>(a.parent_index) < std::get<0>(b.parent_index);
}

void Aligner::AlignAndGraphTwoSeq(Graph &empty_graph,
                                  const char *query, unsigned int query_len, const char *query_id,
                                  const char *target, unsigned int target_len, const char *target_id) {
    std::vector<std::vector<Cell>> align_matrix(query_len + 1, std::vector<Cell>(target_len + 1));
    AlignTwoSeq(align_matrix, query, query_len, target, target_len);

    LinearGraph(empty_graph, query, query_len, query_id);
    std::vector<Node *> top_query = empty_graph.TopologicalSort();

    Graph target_graph;
    LinearGraph(target_graph, target, target_len, target_id);
    std::vector<Node *> top_target = target_graph.TopologicalSort();

    CreateGraph(align_matrix, empty_graph, top_query, top_target);
}

void Aligner::AlignAndGraphSeqAndGraph(Graph &query,
                                       const char *target, unsigned int target_len, const char *target_id) {
    std::vector<Node *> top_graph = query.TopologicalSort();
    std::vector<std::vector<Cell>> align_matrix(top_graph.size() + 1, std::vector<Cell>(target_len + 1));
    AlignSeqAndGraph(align_matrix, top_graph, target, target_len);

    Graph target_graph;
    LinearGraph(target_graph, target, target_len, target_id);
    std::vector<Node *> top_target = target_graph.TopologicalSort();

    CreateGraph(align_matrix, query, top_graph, top_target);
}

void Aligner::AlignAndGraphTwoGraph(Graph &query,
                                    Graph &target) {
    std::vector<Node *> query_graph = query.TopologicalSort();
    std::vector<Node *> target_graph = target.TopologicalSort();
    std::vector<std::vector<Cell>> align_matrix(query_graph.size() + 1, std::vector<Cell>(target_graph.size() + 1));

    AlignTwoGraph(align_matrix, query_graph, target_graph);
    CreateGraph(align_matrix, query, query_graph, target_graph);
}

void Aligner::CreateGraph(std::vector<std::vector<Cell>> &align_matrix,
                          Graph &query, std::vector<Node *> &query_graph,
                          std::vector<Node *> &target_graph) {
    Node *prev_query_node = nullptr, *prev_target_node = nullptr;
    unsigned int query_index = query_graph.size(), target_index = target_graph.size();

    bool finished = false;
    while (!finished) {
        Direction direction = align_matrix[query_index][target_index].direction;
        switch (direction) {
            case DiagonalMatch: {
                for (std::tuple<const char *, unsigned int> origin : target_graph[target_index - 1]->origin_of_letter) {
                    query_graph[query_index - 1]->origin_of_letter.push_back(origin);
                }

                for (Edge *edge : target_graph[target_index - 1]->incoming_edges) {
                    edge->destination = query_graph[query_index - 1];
                    query_graph[query_index - 1]->incoming_edges.push_back(edge);
                }

                for (Edge *edge : target_graph[target_index - 1]->outgoing_edges) {
                    // spriječava stvaranje duplikatnih edgeva
                    bool add_edge = true;
                    for (auto it = edge->destination->incoming_edges.begin(); it != edge->destination->incoming_edges.end(); it++) {
                        if ((*it)->origin == query_graph[query_index - 1]) {
                            add_edge = false;
                            edge->destination->incoming_edges.erase(it);
                            break;
                        }
                    }
                    edge->origin = query_graph[query_index - 1];
                    if (add_edge)
                        query_graph[query_index - 1]->outgoing_edges.push_back(edge);
                }

                //oslobađa iz memorije node iz target_grapha koji smo spojili sa nodom iz query_grapha
                delete target_graph[target_index - 1];
                target_graph[target_index - 1] = query_graph[query_index - 1];

                prev_query_node = query_graph[query_index - 1];
                prev_target_node = prev_query_node;
                break;
            }
            case DiagonalMismatch: {
                std::vector<Node *> target_aligned;
                for (Node *node : query_graph[query_index - 1]->aligned_nodes) {
                    node->aligned_nodes.push_back(target_graph[target_index - 1]);
                    target_aligned.push_back(node);
                }
                query_graph[query_index - 1]->aligned_nodes.push_back(target_graph[target_index - 1]);

                for (Node *node : target_graph[target_index - 1]->aligned_nodes) {
                    node->aligned_nodes.push_back(query_graph[query_index - 1]);
                    query_graph[query_index - 1]->aligned_nodes.push_back(node);
                }

                target_graph[target_index - 1]->aligned_nodes.push_back(query_graph[query_index - 1]);
                for (Node *node : target_aligned) {
                    target_graph[target_index - 1]->aligned_nodes.push_back(node);
                }

                prev_query_node = query_graph[query_index - 1];
                prev_target_node = target_graph[target_index - 1];
                break;
            }
            case Horizontal: {
                prev_target_node = target_graph[target_index - 1];
                break;
            }
            case Vertical: {
                prev_query_node = query_graph[query_index - 1];
                break;
            }
            case None: {
                finished = true;
                if (prev_query_node != prev_target_node) {
                    query.start_nodes.push_back(prev_target_node);
                }
                break;
            }
        }

        int new_query_index = std::get<0>(align_matrix[query_index][target_index].parent_index);
        target_index = std::get<1>(align_matrix[query_index][target_index].parent_index);
        query_index = new_query_index;
    }
}

int GlobalAligner::AlignTwoSeq(std::vector<std::vector<Cell>> &align_matrix,
                               const char *query, unsigned int query_len,
                               const char *target, unsigned int target_len) {
    for (int i = 1; i < query_len + 1; i++) {
        align_matrix[i][0] = {i * gap, Direction::Vertical, std::tuple<int, int>(i - 1, 0)};
    }
    for (int j = 1; j < target_len + 1; j++) {
        align_matrix[0][j] = {j * gap, Direction::Horizontal, std::tuple<int, int>(0, j - 1)};
    }
    align_matrix[0][0] = {0, Direction::None, std::tuple<int, int>(-1, -1)};

    for (int i = 1; i < query_len + 1; i++) {
        for (int j = 1; j < target_len + 1; j++) {
            Cell diagonal_cell;
            if (query[i - 1] == target[j - 1]) {  //Match
                diagonal_cell = {align_matrix[i - 1][j - 1].value + match,
                                 Direction::DiagonalMatch, std::tuple<int, int>(i - 1, j - 1)};
            } else {  //Mismatch
                diagonal_cell = {align_matrix[i - 1][j - 1].value + mismatch,
                                 Direction::DiagonalMismatch, std::tuple<int, int>(i - 1, j - 1)};
            }
            Cell up_cell = {align_matrix[i - 1][j].value + gap, Direction::Vertical, std::tuple<int, int>(i - 1, j)};
            Cell left_cell = {align_matrix[i][j - 1].value + gap, Direction::Horizontal, std::tuple<int, int>(i, j - 1)};

            if (diagonal_cell.value >= up_cell.value && diagonal_cell.value >= left_cell.value) {
                align_matrix[i][j] = diagonal_cell;
            } else if (up_cell.value >= left_cell.value) {
                align_matrix[i][j] = up_cell;
            } else {
                align_matrix[i][j] = left_cell;
            }
        }
    }
    return align_matrix[query_len][target_len].value;
}

int GlobalAligner::AlignSeqAndGraph(std::vector<std::vector<Cell>> &align_matrix,
                                    std::vector<Node *> &graph,
                                    const char *target, unsigned int target_len) {
    for (int i = 1; i < graph.size() + 1; i++) {
        align_matrix[i][0] = {i * gap, Direction::Vertical, std::tuple<int, int>(i - 1, 0)};
    }
    for (int j = 1; j < target_len + 1; j++) {
        align_matrix[0][j] = {j * gap, Direction::Horizontal, std::tuple<int, int>(0, j - 1)};
    }
    align_matrix[0][0] = {0, Direction::None, std::tuple<int, int>(-1, -1)};

    for (int i = 1; i < graph.size() + 1; i++) {
        for (int j = 1; j < target_len + 1; j++) {
            std::vector<Cell> diagonal_values;
            std::vector<Cell> up_values;

            //provjera jeli trenutni čvor početni čvor grafa
            if (graph[i - 1]->incoming_edges.empty()) {
                if (graph[i - 1]->letter == target[j - 1]) {  //Match
                    diagonal_values.push_back({align_matrix[0][j - 1].value + match, Direction::DiagonalMatch, std::tuple<int, int>(0, j - 1)});
                } else {
                    diagonal_values.push_back({align_matrix[0][j - 1].value + mismatch, Direction::DiagonalMismatch, std::tuple<int, int>(0, j - 1)});
                }
                up_values.push_back({align_matrix[0][j].value + gap, Direction::Vertical, std::tuple<int, int>(0, j)});

            } else {
                if (graph[i - 1]->letter == target[j - 1]) {  //Match
                    for (Edge *edge : graph[i - 1]->incoming_edges) {
                        diagonal_values.push_back({align_matrix[edge->origin->index][j - 1].value + match, Direction::DiagonalMatch, std::tuple<int, int>(edge->origin->index, j - 1)});
                        up_values.push_back({align_matrix[edge->origin->index][j].value + gap, Direction::Vertical, std::tuple<int, int>(edge->origin->index, j)});
                    }
                } else {
                    for (Edge *edge : graph[i - 1]->incoming_edges) {
                        diagonal_values.push_back({align_matrix[edge->origin->index][j - 1].value + mismatch, Direction::DiagonalMismatch, std::tuple<int, int>(edge->origin->index, j - 1)});
                        up_values.push_back({align_matrix[edge->origin->index][j].value + gap, Direction::Vertical, std::tuple<int, int>(edge->origin->index, j)});
                    }
                }
            }

            Cell left_cell = {align_matrix[i][j - 1].value + gap, Direction::Horizontal, std::tuple<int, int>(i, j - 1)};
            Cell dig_cell = *std::max_element(diagonal_values.begin(), diagonal_values.end(), max_cell);
            Cell up_cell = *std::max_element(up_values.begin(), up_values.end(), max_cell);

            if (dig_cell.value >= up_cell.value && dig_cell.value >= left_cell.value) {
                align_matrix[i][j] = dig_cell;
            } else if (up_cell.value >= left_cell.value) {
                align_matrix[i][j] = up_cell;
            } else {
                align_matrix[i][j] = left_cell;
            }
        }
    }
    return align_matrix[graph.size()][target_len].value;
}

int GlobalAligner::AlignTwoGraph(std::vector<std::vector<Cell>> &align_matrix,
                                 std::vector<Node *> &query_graph,
                                 std::vector<Node *> &target_graph) {
    for (int i = 1; i < query_graph.size() + 1; i++) {
        align_matrix[i][0] = {i * gap, Direction::Vertical, std::tuple<int, int>(i - 1, 0)};
    }
    for (int j = 1; j < target_graph.size() + 1; j++) {
        align_matrix[0][j] = {j * gap, Direction::Horizontal, std::tuple<int, int>(j - 1, 0)};
    }
    align_matrix[0][0] = {0, Direction::None, std::tuple<int, int>(-1, -1)};

    for (int i = 1; i < query_graph.size() + 1; i++) {
        for (int j = 1; j < target_graph.size() + 1; j++) {
            std::vector<Cell> diagonal_values;
            std::vector<Cell> up_values;
            std::vector<Cell> left_values;

            //pregledava trenutni query čvor
            if (query_graph[i - 1]->incoming_edges.empty()) {
                if (query_graph[i - 1]->letter == target_graph[j - 1]->letter) {  //Match
                    diagonal_values.push_back({align_matrix[0][j - 1].value + match, Direction::DiagonalMatch, std::tuple<int, int>(0, j - 1)});
                } else {
                    diagonal_values.push_back({align_matrix[0][j - 1].value + mismatch, Direction::DiagonalMismatch, std::tuple<int, int>(0, j - 1)});
                }
                up_values.push_back({align_matrix[0][j].value + gap, Direction::Vertical, std::tuple<int, int>(0, j)});
            } else {
                if (query_graph[i - 1]->letter == target_graph[j - 1]->letter) {  //Match
                    for (Edge *edge : query_graph[i - 1]->incoming_edges) {
                        diagonal_values.push_back({align_matrix[edge->origin->index][j - 1].value + match, Direction::DiagonalMatch, std::tuple<int, int>(edge->origin->index, j - 1)});
                        up_values.push_back({align_matrix[edge->origin->index][j].value + gap, Direction::Vertical, std::tuple<int, int>(edge->origin->index, j)});
                    }
                } else {
                    for (Edge *edge : query_graph[i - 1]->incoming_edges) {
                        diagonal_values.push_back({align_matrix[edge->origin->index][j - 1].value + mismatch, Direction::DiagonalMismatch, std::tuple<int, int>(edge->origin->index, j - 1)});
                        up_values.push_back({align_matrix[edge->origin->index][j].value + gap, Direction::Vertical, std::tuple<int, int>(edge->origin->index, j)});
                    }
                }
            }

            //pregledava trenutni target čvor
            if (target_graph[j - 1]->incoming_edges.empty()) {
                if (query_graph[i - 1]->letter == target_graph[j - 1]->letter) {  //Match
                    diagonal_values.push_back({align_matrix[i - 1][0].value + match, Direction::DiagonalMatch, std::tuple<int, int>(i - 1, 0)});
                } else {
                    diagonal_values.push_back({align_matrix[i - 1][0].value + mismatch, Direction::DiagonalMismatch, std::tuple<int, int>(i - 1, 0)});
                }
                left_values.push_back({align_matrix[i][0].value + gap, Direction::Horizontal, std::tuple<int, int>(i, 0)});
            } else {
                if (query_graph[i - 1]->letter == target_graph[j - 1]->letter) {  //Match
                    for (Edge *edge : target_graph[j - 1]->incoming_edges) {
                        diagonal_values.push_back({align_matrix[i - 1][edge->origin->index].value + match, Direction::DiagonalMatch, std::tuple<int, int>(i - 1, edge->origin->index)});
                        left_values.push_back({align_matrix[i][edge->origin->index].value + gap, Direction::Horizontal, std::tuple<int, int>(i, edge->origin->index)});
                    }
                } else {
                    for (Edge *edge : target_graph[j - 1]->incoming_edges) {
                        diagonal_values.push_back({align_matrix[i - 1][edge->origin->index].value + mismatch, Direction::DiagonalMismatch, std::tuple<int, int>(i - 1, edge->origin->index)});
                        left_values.push_back({align_matrix[i][edge->origin->index].value + gap, Direction::Horizontal, std::tuple<int, int>(i, edge->origin->index)});
                    }
                }
            }

            Cell dig_cell = *std::max_element(diagonal_values.begin(), diagonal_values.end(), max_cell);
            Cell up_cell = *std::max_element(up_values.begin(), up_values.end(), max_cell);
            Cell left_cell = *std::max_element(left_values.begin(), left_values.end(), max_cell);

            if (dig_cell.value >= up_cell.value && dig_cell.value >= left_cell.value) {
                align_matrix[i][j] = dig_cell;
            } else if (up_cell.value >= left_cell.value) {
                align_matrix[i][j] = up_cell;
            } else {
                align_matrix[i][j] = left_cell;
            }
        }
    }

    return align_matrix[query_graph.size()][target_graph.size()].value;
}