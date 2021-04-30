#include "graph.hpp"

enum Direction { None,
                 DiagonalMatch,
                 DiagonalMismatch,
                 Vertical,
                 Horizontal };

struct Cell {
    int value;
    Direction direction;
    std::tuple<int, int> parent_index;
};

class Aligner {
   public:
    virtual int AlignTwoSeq(std::vector<std::vector<Cell>> &align_matrix, const char *query, unsigned int query_len, const char *target,
                            unsigned int target_len, int match, int mismatch, int gap) = 0;

    virtual int AlignSeqAndGraph(const char *sequence, unsigned int sequence_len, Graph &graph, int match, int mismatch, int gap) = 0;

    virtual int AlignTwoGraph(Graph &query, Graph &target, int match, int mismatch, int gap) = 0;
};

class GlobalAligner : public Aligner {
   public:
    virtual int AlignTwoSeq(std::vector<std::vector<Cell>> &align_matrix, const char *query, unsigned int query_len, const char *target,
                            unsigned int target_len, int match, int mismatch, int gap);

    virtual int AlignSeqAndGraph(const char *sequence, unsigned int sequence_len, Graph &graph, int match, int mismatch, int gap);

    virtual int AlignTwoGraph(Graph &query, Graph &target, int match, int mismatch, int gap);
};
