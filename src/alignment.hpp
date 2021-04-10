#include <vector>

enum Direction { None,
                 Diagonal,
                 Vertical,
                 Horizontal };

struct Cell {
    int value;
    Direction direction;
};

int GlobalAlignment(std::vector<std::vector<Cell>> &alignMatrix, const char *query, unsigned int query_len, const char *target,
                    unsigned int target_len, int match, int mismatch, int gap);