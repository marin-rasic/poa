#include <algorithm>

#include "graph.hpp"

enum Direction { None,
                 DiagonalMatch,
                 DiagonalMismatch,
                 Vertical,
                 Horizontal };

enum Alignment { Global,
                 Local,
                 Semiglobal };

struct Cell {
    int value;
    Direction direction;
    std::tuple<int, int> parent_index;
};

class Aligner {
   public:
    Aligner(int match, int mismatch, int gap, Alignment alignment)
        : match(match), mismatch(mismatch), gap(gap), alignment(alignment){};

    //općenita funckija za poravnavanje dva niza i stvaranje novog grafa na temelju tog poravnanja
    virtual void AlignAndGraphTwoSeq(Graph &empty_graph,
                                     const char *query, unsigned int query_len, const char *query_id,
                                     const char *target, unsigned int target_len, const char *target_id);

    //općenita funkcija za poravnavanje niza i grafa i stvaranje novog grafa na temelju tog poravnanja
    virtual void AlignAndGraphSeqAndGraph(Graph &query,
                                          const char *target, unsigned int target_len, const char *target_id);

    //općenita funckija za poravnavanje dva grafa i stvaranje novog grafa na temelju tog poravnanja
    virtual void AlignAndGraphTwoGraph(Graph &query,
                                       Graph &target);

    //funckija za stvaranje grafa na temelju poravnanja
    virtual void CreateGraph(std::vector<std::vector<Cell>> &align_matrix,
                             Graph &query, std::vector<Node *> &query_graph,
                             Graph &target, std::vector<Node *> &target_graph,
                             std::tuple<int, int> starting_index);

    std::tuple<int, int> AlignTwoSeq(std::vector<std::vector<Cell>> &align_matrix,
                                     const char *query, unsigned int query_len,
                                     const char *target, unsigned int target_len,
                                     Cell min_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)});

    std::tuple<int, int> AlignSeqAndGraph(std::vector<std::vector<Cell>> &align_matrix,
                                          std::vector<Node *> &graph,
                                          const char *target, unsigned int target_len,
                                          Cell min_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)});

    std::tuple<int, int> AlignTwoGraph(std::vector<std::vector<Cell>> &align_matrix,
                                       std::vector<Node *> &query_graph,
                                       std::vector<Node *> &target_graph,
                                       Cell min_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)});

   protected:
    int match;
    int mismatch;
    int gap;
    Alignment alignment;
};