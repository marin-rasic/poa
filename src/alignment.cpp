#include "alignment.hpp"

#include <algorithm>

//određuje koja je Cell veća, priotizirajući one koje imaju manji index reda
bool max_cell(Cell a, Cell b) {
    return a.value < b.value || a.value == b.value && std::get<0>(a.parent_index) > std::get<0>(b.parent_index);
}

//funckije za dva niza
int GlobalAligner::AlignTwoSeq(std::vector<std::vector<Cell>> &align_matrix, const char *query, unsigned int query_len, const char *target,
                               unsigned int target_len, int match, int mismatch, int gap) {
    for (int i = 1; i < query_len + 1; i++) {
        align_matrix[i][0] = {i * gap, Direction::Vertical};
    }
    for (int j = 1; j < target_len + 1; j++) {
        align_matrix[0][j] = {j * gap, Direction::Horizontal};
    }
    align_matrix[0][0] = {0, Direction::None};

    for (int i = 1; i < query_len + 1; i++) {
        for (int j = 1; j < target_len + 1; j++) {
            Cell diagonal_cell;
            if (query[i - 1] == target[j - 1]) {  //Match
                diagonal_cell = {align_matrix[i - 1][j - 1].value + match,
                                 Direction::DiagonalMatch};
            } else {
                diagonal_cell = {align_matrix[i - 1][j - 1].value + mismatch,
                                 Direction::DiagonalMismatch};
            }
            Cell up_cell = {align_matrix[i - 1][j].value + gap, Direction::Vertical};
            Cell left_cell = {align_matrix[i][j - 1].value + gap, Direction::Horizontal};

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

void GlobalAligner::GraphTwoSeq(Graph &empty_graph, std::vector<std::vector<Cell>> &align_matrix, const char *query, unsigned int query_len,
                                const char *query_id, const char *target, unsigned int target_len, const char *target_id) {
    Node *prev_query_node = nullptr, *prev_target_node = nullptr;
    unsigned int query_index = query_len, target_index = target_len;

    bool finished = false;
    while (!finished) {
        switch (align_matrix[query_index][target_index].direction) {
            case Direction::DiagonalMatch: {
                Node *query_node = Graph::addNewNode(query[query_index - 1], query_id, query_index, prev_query_node);
                Node *target_node = query_node;
                std::tuple<const char *, unsigned int> origin(target_id, target_index);
                query_node->origin_of_letter.push_back(origin);

                //dodaje edge koji nedostaje u slučaju da su prethodno bila dva različita noda
                if (prev_target_node != prev_query_node && prev_target_node != nullptr) {
                    Edge *edge = new Edge(query_node, prev_target_node);
                    query_node->outgoing_edges.push_back(edge);
                    prev_target_node->incoming_edges.push_back(edge);
                }
                prev_target_node = target_node;
                prev_query_node = query_node;
                query_index--;
                target_index--;
                break;
            }

            case Direction::DiagonalMismatch: {
                prev_query_node = Graph::addNewNode(query[query_index - 1], query_id, query_index, prev_query_node);
                prev_target_node = Graph::addNewNode(target[target_index - 1], target_id, target_index, prev_target_node);
                query_index--;
                target_index--;
                break;
            }

            case Direction::Horizontal: {
                prev_target_node = Graph::addNewNode(target[target_index - 1], target_id, target_index, prev_target_node);
                target_index--;
                break;
            }

            case Direction::Vertical: {
                prev_query_node = Graph::addNewNode(query[query_index - 1], query_id, query_index, prev_query_node);
                query_index--;
                break;
            }

            case Direction::None: {
                finished = true;
                if (prev_query_node == prev_target_node) {
                    empty_graph.start_nodes.push_back(prev_target_node);
                } else {
                    empty_graph.start_nodes.push_back(prev_target_node);
                    empty_graph.start_nodes.push_back(prev_query_node);
                }
                break;
            }
        }
    }
}

void GlobalAligner::AlignAndGraphTwoSeq(Graph &empty_graph, const char *query, unsigned int query_len, const char *query_id, const char *target,
                                        unsigned int target_len, const char *target_id, int match, int mismatch, int gap) {
    std::vector<std::vector<Cell>> align_matrix(query_len + 1, std::vector<Cell>(target_len + 1));
    GlobalAligner::AlignTwoSeq(align_matrix, query, query_len, target, target_len, match, mismatch, gap);
    GlobalAligner::GraphTwoSeq(empty_graph, align_matrix, query, query_len, query_id, target, target_len, target_id);
}

//funckije za niz i graf
int GlobalAligner::AlignSeqAndGraph(std::vector<std::vector<Cell>> &align_matrix, const char *sequence, unsigned int sequence_len,
                                    Graph &graph, int match, int mismatch, int gap) {
    std::vector<Node *> top_graph = graph.TopologicalSort();

    for (int i = 1; i < top_graph.size() + 1; i++) {
        align_matrix[i][0] = {i * gap, Direction::Vertical, std::tuple<int, int>(i - 1, 0)};
    }
    for (int j = 1; j < sequence_len + 1; j++) {
        align_matrix[0][j] = {j * gap, Direction::Horizontal, std::tuple<int, int>(0, j - 1)};
    }
    align_matrix[0][0] = {0, Direction::None, std::tuple<int, int>(-1, -1)};

    for (int i = 1; i < top_graph.size() + 1; i++) {
        for (int j = 1; j < sequence_len + 1; j++) {
            std::vector<Cell> diagonal_values;
            std::vector<Cell> up_values;

            //provjera jeli trenutni čvor početni čvor grafa
            if (top_graph[i - 1]->incoming_edges.empty()) {
                if (top_graph[i - 1]->letter == sequence[j - 1]) {  //Match
                    diagonal_values.push_back({align_matrix[0][j - 1].value + match, Direction::DiagonalMatch, std::tuple<int, int>(0, j - 1)});
                } else {
                    diagonal_values.push_back({align_matrix[0][j - 1].value + mismatch, Direction::DiagonalMismatch, std::tuple<int, int>(0, j - 1)});
                }
                up_values.push_back({align_matrix[0][j].value + gap, Direction::Vertical, std::tuple<int, int>(0, j)});

            } else {
                if (top_graph[i - 1]->letter == sequence[j - 1]) {  //Match
                    for (Edge *edge : top_graph[i - 1]->incoming_edges) {
                        diagonal_values.push_back({align_matrix[edge->origin->index][j - 1].value + match, Direction::DiagonalMatch, std::tuple<int, int>(edge->origin->index, j - 1)});
                        up_values.push_back({align_matrix[edge->origin->index][j].value + gap, Direction::Vertical, std::tuple<int, int>(edge->origin->index, j)});
                    }
                } else {
                    for (Edge *edge : top_graph[i - 1]->incoming_edges) {
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
    return align_matrix[top_graph.size()][sequence_len].value;
}

void GlobalAligner::GraphSeqAndGraph(std::vector<std::vector<Cell>> &align_matrix, Graph &query, const char *target, unsigned int target_len,
                                     const char *target_id) {
    std::vector<Node *> top_graph = query.TopologicalSort();

    Node *prev_query_node = nullptr, *prev_target_node = nullptr;
    unsigned int query_index = top_graph.size(), target_index = target_len;

    bool finished = false;
    while (!finished) {
        switch (align_matrix[query_index][target_index].direction) {
            case Direction::DiagonalMatch: {
                std::tuple<const char *, unsigned int> origin(target_id, target_index);
                top_graph[query_index - 1]->origin_of_letter.push_back(origin);

                //dodaje edge koji nedostaje u slučaju da su prethodno bila dva različita noda
                if (prev_query_node != prev_target_node && prev_target_node != nullptr) {
                    Edge *edge = new Edge(top_graph[query_index - 1], prev_target_node);
                    top_graph[query_index - 1]->outgoing_edges.push_back(edge);
                    prev_target_node->incoming_edges.push_back(edge);
                }

                prev_target_node = top_graph[query_index - 1];
                prev_query_node = prev_target_node;

                int new_query_index = std::get<0>(align_matrix[query_index][target_index].parent_index);
                target_index = std::get<1>(align_matrix[query_index][target_index].parent_index);
                query_index = new_query_index;

                break;
            }

            case Direction::DiagonalMismatch: {
                prev_target_node = Graph::addNewNode(target[target_index - 1], target_id, target_index, prev_target_node);
                int new_query_index = std::get<0>(align_matrix[query_index][target_index].parent_index);
                target_index = std::get<1>(align_matrix[query_index][target_index].parent_index);
                query_index = new_query_index;
                break;
            }

            case Direction::Horizontal: {
                prev_target_node = Graph::addNewNode(target[target_index - 1], target_id, target_index, prev_target_node);
                target_index = std::get<1>(align_matrix[query_index][target_index].parent_index);
                break;
            }

            case Direction::Vertical: {
                prev_query_node = top_graph[query_index - 1];
                query_index = std::get<0>(align_matrix[query_index][target_index].parent_index);
                break;
            }

            case Direction::None: {
                finished = true;
                if (prev_query_node != prev_target_node) {
                    query.start_nodes.push_back(prev_target_node);
                }
                break;
            }
        }
    }
}

void GlobalAligner::AlignAndGraphSeqAndGraph(Graph &query, const char *target, unsigned int target_len,
                                             const char *target_id, int match, int mismatch, int gap) {
    std::vector<Node *> top_graph = query.TopologicalSort();
    std::vector<std::vector<Cell>> align_matrix(top_graph.size() + 1, std::vector<Cell>(target_len + 1));
    GlobalAligner::AlignSeqAndGraph(align_matrix, target, target_len, query, match, mismatch, gap);
    GlobalAligner::GraphSeqAndGraph(align_matrix, query, target, target_len, target_id);
}

//funckije za dva grafa
int GlobalAligner::AlignTwoGraph(std::vector<std::vector<Cell>> &align_matrix, Graph &query, Graph &target, int match, int mismatch, int gap) {
    std::vector<Node *> query_top_graph = query.TopologicalSort();
    std::vector<Node *> target_top_graph = target.TopologicalSort();

    for (int i = 1; i < query_top_graph.size() + 1; i++) {
        align_matrix[i][0] = {i * gap, Direction::Vertical, std::tuple<int, int>(i - 1, 0)};
    }
    for (int j = 1; j < target_top_graph.size() + 1; j++) {
        align_matrix[0][j] = {j * gap, Direction::Horizontal, std::tuple<int, int>(j - 1, 0)};
    }
    align_matrix[0][0] = {0, Direction::None, std::tuple<int, int>(-1, -1)};

    for (int i = 1; i < query_top_graph.size() + 1; i++) {
        for (int j = 1; j < target_top_graph.size() + 1; j++) {
            std::vector<Cell> diagonal_values;
            std::vector<Cell> up_values;
            std::vector<Cell> left_values;

            //pregledava trenutni query čvor
            if (query_top_graph[i - 1]->incoming_edges.empty()) {
                if (query_top_graph[i - 1]->letter == target_top_graph[j - 1]->letter) {  //Match
                    diagonal_values.push_back({align_matrix[0][j - 1].value + match, Direction::DiagonalMatch, std::tuple<int, int>(0, j - 1)});
                } else {
                    diagonal_values.push_back({align_matrix[0][j - 1].value + mismatch, Direction::DiagonalMismatch, std::tuple<int, int>(0, j - 1)});
                }
                up_values.push_back({align_matrix[0][j].value + gap, Direction::Vertical, std::tuple<int, int>(0, j)});
            } else {
                if (query_top_graph[i - 1]->letter == target_top_graph[j - 1]->letter) {  //Match
                    for (Edge *edge : query_top_graph[i - 1]->incoming_edges) {
                        diagonal_values.push_back({align_matrix[edge->origin->index][j - 1].value + match, Direction::DiagonalMatch, std::tuple<int, int>(edge->origin->index, j - 1)});
                        up_values.push_back({align_matrix[edge->origin->index][j].value + gap, Direction::Vertical, std::tuple<int, int>(edge->origin->index, j)});
                    }
                } else {
                    for (Edge *edge : query_top_graph[i - 1]->incoming_edges) {
                        diagonal_values.push_back({align_matrix[edge->origin->index][j - 1].value + mismatch, Direction::DiagonalMismatch, std::tuple<int, int>(edge->origin->index, j - 1)});
                        up_values.push_back({align_matrix[edge->origin->index][j].value + gap, Direction::Vertical, std::tuple<int, int>(edge->origin->index, j)});
                    }
                }
            }

            //pregledava trenutni target čvor
            if (target_top_graph[j - 1]->incoming_edges.empty()) {
                if (query_top_graph[i - 1]->letter == target_top_graph[j - 1]->letter) {  //Match
                    diagonal_values.push_back({align_matrix[i - 1][0].value + match, Direction::DiagonalMatch, std::tuple<int, int>(i - 1, 0)});
                } else {
                    diagonal_values.push_back({align_matrix[i - 1][0].value + mismatch, Direction::DiagonalMismatch, std::tuple<int, int>(i - 1, 0)});
                }
                left_values.push_back({align_matrix[i][0].value + gap, Direction::Horizontal, std::tuple<int, int>(i, 0)});
            } else {
                if (query_top_graph[i - 1]->letter == target_top_graph[j - 1]->letter) {  //Match
                    for (Edge *edge : target_top_graph[j - 1]->incoming_edges) {
                        diagonal_values.push_back({align_matrix[i - 1][edge->origin->index].value + match, Direction::DiagonalMatch, std::tuple<int, int>(i - 1, edge->origin->index)});
                        left_values.push_back({align_matrix[i][edge->origin->index].value + gap, Direction::Horizontal, std::tuple<int, int>(i, edge->origin->index)});
                    }
                } else {
                    for (Edge *edge : target_top_graph[j - 1]->incoming_edges) {
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

    return align_matrix[query_top_graph.size()][target_top_graph.size()].value;
}

void GlobalAligner::GraphTwoGraph(std::vector<std::vector<Cell>> &align_matrix, Graph &query, Graph &target) {
    std::vector<Node *> query_graph = query.TopologicalSort();
    std::vector<Node *> target_graph = target.TopologicalSort();
    Node *prev_query_node = nullptr, *prev_target_node = nullptr;
    unsigned int query_index = query_graph.size(), target_index = target_graph.size();

    bool finished = false;
    while (!finished) {
        switch (align_matrix[query_index][target_index].direction) {
            case Direction::DiagonalMatch: {
                for (std::tuple<const char *, unsigned int> origin : target_graph[target_index - 1]->origin_of_letter) {
                    query_graph[query_index - 1]->origin_of_letter.push_back(origin);
                }

                //dodaje edge koji nedostaje u slučaju da su prethodno bila dva različita noda
                if (prev_query_node != prev_target_node && prev_target_node != nullptr) {
                    Edge *edge = new Edge(query_graph[query_index - 1], prev_target_node);
                    query_graph[query_index - 1]->outgoing_edges.push_back(edge);
                    prev_target_node->incoming_edges.push_back(edge);
                }

                prev_target_node = query_graph[query_index - 1];
                prev_query_node = prev_target_node;

                int new_query_index = std::get<0>(align_matrix[query_index][target_index].parent_index);
                target_index = std::get<1>(align_matrix[query_index][target_index].parent_index);
                query_index = new_query_index;

                break;
            }

            case Direction::DiagonalMismatch: {
                prev_target_node = Graph::addNewNode(target_graph[target_index - 1]->letter,
                                                     std::get<0>(target_graph[target_index - 1]->origin_of_letter[0]),
                                                     std::get<1>(target_graph[target_index - 1]->origin_of_letter[0]),
                                                     prev_target_node);

                int new_query_index = std::get<0>(align_matrix[query_index][target_index].parent_index);
                target_index = std::get<1>(align_matrix[query_index][target_index].parent_index);
                query_index = new_query_index;
                break;
            }

            case Direction::Horizontal: {
                prev_target_node = Graph::addNewNode(target_graph[target_index - 1]->letter,
                                                     std::get<0>(target_graph[target_index - 1]->origin_of_letter[0]),
                                                     std::get<1>(target_graph[target_index - 1]->origin_of_letter[0]),
                                                     prev_target_node);

                target_index = std::get<1>(align_matrix[query_index][target_index].parent_index);
                break;
            }

            case Direction::Vertical: {
                prev_query_node = query_graph[query_index - 1];
                query_index = std::get<0>(align_matrix[query_index][target_index].parent_index);
                break;
            }

            case Direction::None: {
                finished = true;
                if (prev_query_node != prev_target_node) {
                    query.start_nodes.push_back(prev_target_node);
                }
                break;
            }
        }
    }
}

void GlobalAligner::AlignAndGraphTwoGraph(Graph &query, Graph &target, int match, int mismatch, int gap) {
    std::vector<Node *> query_graph = query.TopologicalSort();
    std::vector<Node *> target_graph = target.TopologicalSort();
    std::vector<std::vector<Cell>> align_matrix(query_graph.size() + 1, std::vector<Cell>(target_graph.size() + 1));

    GlobalAligner::AlignTwoGraph(align_matrix, query, target, match, mismatch, gap);
    GlobalAligner::GraphTwoGraph(align_matrix, query, target);
}