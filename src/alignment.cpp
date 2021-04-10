#include "alignment.hpp"

int GlobalAlignment(std::vector<std::vector<Cell>> &alignMatrix, const char *query, unsigned int query_len, const char *target,
                    unsigned int target_len, int match, int mismatch, int gap) {
    for (int i = 1; i < query_len + 1; i++) {
        alignMatrix[i][0] = {i * gap, Direction::Vertical};
    }
    for (int j = 1; j < target_len + 1; j++) {
        alignMatrix[0][j] = {j * gap, Direction::Horizontal};
    }

    for (int i = 1; i < query_len + 1; i++) {
        for (int j = 1; j < target_len + 1; j++) {
            Cell diagonalCell;
            if (query[i - 1] == target[j - 1]) {  //Match
                diagonalCell = {alignMatrix[i - 1][j - 1].value + match,
                                Direction::Diagonal};
            } else {
                diagonalCell = {alignMatrix[i - 1][j - 1].value + mismatch,
                                Direction::Diagonal};
            }
            Cell upCell = {alignMatrix[i - 1][j].value + gap, Direction::Vertical};
            Cell leftCell = {alignMatrix[i][j - 1].value + gap, Direction::Horizontal};

            if (diagonalCell.value >= upCell.value && diagonalCell.value >= leftCell.value) {
                alignMatrix[i][j] = diagonalCell;
            } else if (upCell.value >= leftCell.value) {
                alignMatrix[i][j] = upCell;
            } else {
                alignMatrix[i][j] = leftCell;
            }
        }
    }
    return alignMatrix[query_len][target_len].value;
}