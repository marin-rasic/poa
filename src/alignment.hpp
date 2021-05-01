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

    virtual int AlignSeqAndGraph(std::vector<std::vector<Cell>> &align_matrix, const char *sequence, unsigned int sequence_len, Graph &graph, int match, int mismatch, int gap) = 0;

    virtual int AlignTwoGraph(Graph &query, Graph &target, int match, int mismatch, int gap) = 0;

    virtual void GraphTwoSeq(Graph &empty_graph, std::vector<std::vector<Cell>> &align_matrix, const char *query, unsigned int query_len, const char *query_id, const char *target,
                             unsigned int target_len, const char *target_id) = 0;

    virtual void AlignAndGraphTwoSeq(Graph &empty_graph, const char *query, unsigned int query_len, const char *query_id, const char *target,
                                     unsigned int target_len, const char *target_id, int match, int mismatch, int gap) = 0;

    virtual void GraphSeqAndGraph(std::vector<std::vector<Cell>> &align_matrix, Graph &query, const char *target, unsigned int target_len, const char *target_id) = 0;

    virtual void AlignAndGraphSeqAndGraph(Graph &query, const char *target, unsigned int target_len,
                                          const char *target_id, int match, int mismatch, int gap) = 0;
};

class GlobalAligner : public Aligner {
   public:
    virtual int AlignTwoSeq(std::vector<std::vector<Cell>> &align_matrix, const char *query, unsigned int query_len, const char *target,
                            unsigned int target_len, int match, int mismatch, int gap);

    virtual int AlignSeqAndGraph(std::vector<std::vector<Cell>> &align_matrix, const char *sequence, unsigned int sequence_len, Graph &graph, int match, int mismatch, int gap);

    virtual int AlignTwoGraph(Graph &query, Graph &target, int match, int mismatch, int gap);

    virtual void GraphTwoSeq(Graph &empty_graph, std::vector<std::vector<Cell>> &align_matrix, const char *query, unsigned int query_len, const char *query_id, const char *target,
                             unsigned int target_len, const char *target_id);

    virtual void AlignAndGraphTwoSeq(Graph &empty_graph, const char *query, unsigned int query_len, const char *query_id, const char *target,
                                     unsigned int target_len, const char *target_id, int match, int mismatch, int gap);

    virtual void GraphSeqAndGraph(std::vector<std::vector<Cell>> &align_matrix, Graph &query, const char *target, unsigned int target_len, const char *target_id);

    virtual void AlignAndGraphSeqAndGraph(Graph &query, const char *target, unsigned int target_len,
                                          const char *target_id, int match, int mismatch, int gap);
};
