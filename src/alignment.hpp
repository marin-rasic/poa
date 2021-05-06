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
    Aligner(int match, int mismatch, int gap) : match(match), mismatch(mismatch), gap(gap){};

    //funkcije za poravnavanje dva niza
    virtual int AlignTwoSeq(std::vector<std::vector<Cell>> &align_matrix,
                            const char *query, unsigned int query_len,
                            const char *target, unsigned int target_len) = 0;

    //općenita funckija za poravnavanje dva niza i stvaranje novog grafa na temelju tog poravnanja
    virtual void AlignAndGraphTwoSeq(Graph &empty_graph,
                                     const char *query, unsigned int query_len, const char *query_id,
                                     const char *target, unsigned int target_len, const char *target_id);

    //funkcije za poravnavanje niza i grafa
    virtual int AlignSeqAndGraph(std::vector<std::vector<Cell>> &align_matrix,
                                 std::vector<Node *> &graph,  //topologicly sorted vector of nodes
                                 const char *target, unsigned int target_len) = 0;

    //općenita funkcija za poravnavanje niza i grafa i stvaranje novog grafa na temelju tog poravnanja
    virtual void AlignAndGraphSeqAndGraph(Graph &query,
                                          const char *target, unsigned int target_len, const char *target_id);

    //funkcije za poravnavanje dva grafa
    virtual int AlignTwoGraph(std::vector<std::vector<Cell>> &align_matrix,
                              std::vector<Node *> &query_graph,
                              std::vector<Node *> &target_graph) = 0;

    //općenita funckija za poravnavanje dva grafa i stvaranje novog grafa na temelju tog poravnanja
    virtual void AlignAndGraphTwoGraph(Graph &query,
                                       Graph &target);

    //funckija za stvaranje grafa na temelju poravnanja
    virtual void CreateGraph(std::vector<std::vector<Cell>> &align_matrix,
                             Graph &query, std::vector<Node *> &query_graph,
                             std::vector<Node *> &target_graph);

   protected:
    int match;
    int mismatch;
    int gap;
};

class GlobalAligner : public Aligner {
   public:
    GlobalAligner(int match, int mismatch, int gap) : Aligner(match, mismatch, gap){};

    virtual int AlignTwoSeq(std::vector<std::vector<Cell>> &align_matrix,
                            const char *query, unsigned int query_len,
                            const char *target, unsigned int target_len);

    virtual int AlignSeqAndGraph(std::vector<std::vector<Cell>> &align_matrix,
                                 std::vector<Node *> &graph,
                                 const char *target, unsigned int target_len);

    virtual int AlignTwoGraph(std::vector<std::vector<Cell>> &align_matrix,
                              std::vector<Node *> &query_graph,
                              std::vector<Node *> &target_graph);
};