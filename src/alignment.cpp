#include "alignment.hpp"

#include <algorithm>

int GlobalAligner::AlignTwoSeq(std::vector<std::vector<Cell>> &align_matrix, const char *query, unsigned int query_len, const char *target,
                               unsigned int target_len, int match, int mismatch, int gap) {
    for (int i = 1; i < query_len + 1; i++) {
        align_matrix[i][0] = {i * gap, Direction::Vertical};
    }
    for (int j = 1; j < target_len + 1; j++) {
        align_matrix[0][j] = {j * gap, Direction::Horizontal};
    }

    for (int i = 1; i < query_len + 1; i++) {
        for (int j = 1; j < target_len + 1; j++) {
            Cell diagonal_cell;
            if (query[i - 1] == target[j - 1]) {  //Match
                diagonal_cell = {align_matrix[i - 1][j - 1].value + match,
                                 Direction::Diagonal};
            } else {
                diagonal_cell = {align_matrix[i - 1][j - 1].value + mismatch,
                                 Direction::Diagonal};
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

int GlobalAligner::AlignSeqAndGraph(const char *sequence, unsigned int sequence_len, Graph &graph, int match, int mismatch, int gap) {
    std::vector<Node *> top_graph = graph.TopologicalSort();

    std::vector<std::vector<int>> align_matrix(top_graph.size() + 1, std::vector<int>(sequence_len + 1));
    for (int i = 1; i < top_graph.size() + 1; i++) {
        align_matrix[i][0] = i * gap;
    }
    for (int j = 1; j < sequence_len + 1; j++) {
        align_matrix[0][j] = j * gap;
    }

    for (int i = 1; i < top_graph.size() + 1; i++) {
        for (int j = 1; j < sequence_len + 1; j++) {
            std::vector<int> diagonal_values;
            std::vector<int> up_values;
            int left_value;

            //provjera jeli trenutni čvor početni čvor grafa
            if (top_graph[i - 1]->incoming_edges.empty()) {
                if (top_graph[i - 1]->letter == sequence[j - 1]) {  //Match
                    diagonal_values.push_back(align_matrix[0][j - 1] + match);
                } else {
                    diagonal_values.push_back(align_matrix[0][j - 1] + mismatch);
                }
                up_values.push_back(align_matrix[0][j] + gap);

            } else {
                if (top_graph[i - 1]->letter == sequence[j - 1]) {  //Match
                    for (Edge *edge : top_graph[i - 1]->incoming_edges) {
                        diagonal_values.push_back(align_matrix[edge->origin->index][j - 1] + match);
                    }
                } else {
                    for (Edge *edge : top_graph[i - 1]->incoming_edges) {
                        diagonal_values.push_back(align_matrix[edge->origin->index][j - 1] + mismatch);
                    }
                }
                for (Edge *edge : top_graph[i - 1]->incoming_edges) {
                    up_values.push_back(align_matrix[edge->origin->index][j] + gap);
                }
            }

            left_value = align_matrix[i][j - 1] + gap;
            int dig_value = *std::max_element(diagonal_values.begin(), diagonal_values.end());
            int up_value = *std::max_element(up_values.begin(), up_values.end());

            if (dig_value >= up_value && dig_value >= left_value) {
                align_matrix[i][j] = dig_value;
            } else if (up_value >= left_value) {
                align_matrix[i][j] = up_value;
            } else {
                align_matrix[i][j] = left_value;
            }
        }
    }
    return align_matrix[top_graph.size()][sequence_len];
}

int GlobalAligner::AlignTwoGraph(Graph &query, Graph &target, int match, int mismatch, int gap) {
    //TO DO
    return -1;
}