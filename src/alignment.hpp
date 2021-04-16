#include <vector>

enum Direction { None,
                 Diagonal,
                 Vertical,
                 Horizontal };

struct Cell {
    int value;
    Direction direction;
};

class Aligner {
   public:
    virtual int align(std::vector<std::vector<Cell>> &align_matrix, const char *query, unsigned int query_len, const char *target,
                      unsigned int target_len, int match, int mismatch, int gap) = 0;
};

class GlobalAligner : public Aligner {
   public:
    virtual int align(std::vector<std::vector<Cell>> &align_matrix, const char *query, unsigned int query_len, const char *target,
                      unsigned int target_len, int match, int mismatch, int gap);
};
