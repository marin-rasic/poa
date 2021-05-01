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
    //funkcije za dva niza
    virtual int AlignTwoSeq(std::vector<std::vector<Cell>> &align_matrix, const char *query, unsigned int query_len, const char *target,
                            unsigned int target_len, int match, int mismatch, int gap) = 0;

    virtual void GraphTwoSeq(Graph &empty_graph, std::vector<std::vector<Cell>> &align_matrix, const char *query, unsigned int query_len, const char *query_id, const char *target,
                             unsigned int target_len, const char *target_id) = 0;

    virtual void AlignAndGraphTwoSeq(Graph &empty_graph, const char *query, unsigned int query_len, const char *query_id, const char *target,
                                     unsigned int target_len, const char *target_id, int match, int mismatch, int gap) = 0;

    //funkcije za niz i graf
    virtual int AlignSeqAndGraph(std::vector<std::vector<Cell>> &align_matrix, const char *sequence, unsigned int sequence_len, Graph &graph, int match, int mismatch, int gap) = 0;

    virtual void GraphSeqAndGraph(std::vector<std::vector<Cell>> &align_matrix, Graph &query, const char *target, unsigned int target_len, const char *target_id) = 0;

    virtual void AlignAndGraphSeqAndGraph(Graph &query, const char *target, unsigned int target_len,
                                          const char *target_id, int match, int mismatch, int gap) = 0;

    //funkcije za dva grafa
    virtual int AlignTwoGraph(std::vector<std::vector<Cell>> &align_matrix, Graph &query, Graph &target, int match, int mismatch, int gap) = 0;

    virtual void GraphTwoGraph(std::vector<std::vector<Cell>> &align_matrix, Graph &query, Graph &target) = 0;

    virtual void AlignAndGraphTwoGraph(Graph &query, Graph &target, int match, int mismatch, int gap) = 0;
};

class GlobalAligner : public Aligner {
   public:
    //funckije za dva niza
    virtual int AlignTwoSeq(std::vector<std::vector<Cell>> &align_matrix, const char *query, unsigned int query_len, const char *target,
                            unsigned int target_len, int match, int mismatch, int gap);

    virtual void GraphTwoSeq(Graph &empty_graph, std::vector<std::vector<Cell>> &align_matrix, const char *query, unsigned int query_len, const char *query_id, const char *target,
                             unsigned int target_len, const char *target_id);

    virtual void AlignAndGraphTwoSeq(Graph &empty_graph, const char *query, unsigned int query_len, const char *query_id, const char *target,
                                     unsigned int target_len, const char *target_id, int match, int mismatch, int gap);

    //funkcije za graf i niz
    virtual int AlignSeqAndGraph(std::vector<std::vector<Cell>> &align_matrix, const char *sequence, unsigned int sequence_len, Graph &graph, int match, int mismatch, int gap);

    virtual void GraphSeqAndGraph(std::vector<std::vector<Cell>> &align_matrix, Graph &query, const char *target, unsigned int target_len, const char *target_id);

    virtual void AlignAndGraphSeqAndGraph(Graph &query, const char *target, unsigned int target_len,
                                          const char *target_id, int match, int mismatch, int gap);

    //funkcije za dva grafa
    virtual int AlignTwoGraph(std::vector<std::vector<Cell>> &align_matrix, Graph &query, Graph &target, int match, int mismatch, int gap);

    virtual void GraphTwoGraph(std::vector<std::vector<Cell>> &align_matrix, Graph &query, Graph &target);

    virtual void AlignAndGraphTwoGraph(Graph &query, Graph &target, int match, int mismatch, int gap);
};
