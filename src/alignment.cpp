#include "alignment.hpp"

int GlobalAligner::align(std::vector<std::vector<Cell>> &align_matrix, const char *query, unsigned int query_len, const char *target,
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
