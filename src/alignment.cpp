#include "alignment.hpp"

//uspoređuje ćelije i vraća veću
Cell compare_cells(Cell a, Cell b) {
    if (a.value >= b.value) {
        return a;
    }
    return b;
}

// popunjuje prvi redak i prvi stupac matrice za poravnanje
void InitilazeMatrix(std::vector<std::vector<Cell>> &align_matrix, int init_gap,
                     unsigned int query_len, unsigned int target_len, bool direction) {
    Direction vert = Vertical;
    Direction horiz = Horizontal;
    if (!direction) {
        vert = None;
        horiz = None;
    }

    for (int i = 1; i < query_len + 1; i++) {
        align_matrix[i][0] = {init_gap * i, vert, std::tuple<int, int>(i - 1, 0)};
    }
    for (int j = 1; j < target_len + 1; j++) {
        align_matrix[0][j] = {init_gap * j, horiz, std::tuple<int, int>(0, j - 1)};
    }
    align_matrix[0][0] = {0, Direction::None, std::tuple<int, int>(-1, -1)};
}

// izračunava matricu poravnanja
std::tuple<int, int> Aligner::AlignTwoSeq(std::vector<std::vector<Cell>> &align_matrix,
                                          const char *query, unsigned int query_len,
                                          const char *target, unsigned int target_len,
                                          Cell min_cell) {
    // for local alignment
    Cell local_max_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)};
    std::tuple<int, int> local_index;

    // for semi-global alignment
    Cell last_row_max_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)};
    std::tuple<int, int> last_row_index;
    Cell last_col_max_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)};
    std::tuple<int, int> last_col_index;

    for (int i = 1; i < query_len + 1; i++) {
        for (int j = 1; j < target_len + 1; j++) {
            Cell diagonal_cell;
            if (query[i - 1] == target[j - 1]) {  // Match
                diagonal_cell = {align_matrix[i - 1][j - 1].value + match,
                                 Direction::DiagonalMatch,
                                 std::tuple<int, int>(i - 1, j - 1)};
            } else {  // Mismatch
                diagonal_cell = {align_matrix[i - 1][j - 1].value + mismatch,
                                 Direction::DiagonalMismatch,
                                 std::tuple<int, int>(i - 1, j - 1)};
            }
            Cell up_cell = {align_matrix[i - 1][j].value + gap,
                            Direction::Vertical,
                            std::tuple<int, int>(i - 1, j)};
            Cell left_cell = {align_matrix[i][j - 1].value + gap,
                              Direction::Horizontal,
                              std::tuple<int, int>(i, j - 1)};

            align_matrix[i][j] = std::max({min_cell, diagonal_cell, up_cell, left_cell},
                                          [](Cell a, Cell b) { return a.value < b.value; });

            if (align_matrix[i][j].value >= local_max_cell.value) {
                local_max_cell = align_matrix[i][j];
                local_index = std::tuple<int, int>(i, j);
            }

            if (i == query_len && align_matrix[i][j].value >= last_row_max_cell.value) {
                last_row_max_cell = align_matrix[i][j];
                last_row_index = std::tuple<int, int>(i, j);
            }
            if (j == target_len && align_matrix[i][j].value >= last_col_max_cell.value) {
                last_col_max_cell = align_matrix[i][j];
                last_col_index = std::tuple<int, int>(i, j);
            }
        }
    }

    switch (alignment) {
        case Global:
            return std::tuple<int, int>(query_len, target_len);

        case Local:
            return local_index;

        case Semiglobal:
            return last_row_max_cell.value > last_col_max_cell.value ? last_row_index : last_col_index;
    }
    return std::tuple<int, int>(-1, -1);
}

std::tuple<int, int> Aligner::AlignSeqAndGraph(std::vector<std::vector<Cell>> &align_matrix,
                                               std::vector<Node *> &graph,
                                               const char *target, unsigned int target_len,
                                               Cell min_cell) {
    // for global alignment
    Cell global_max_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)};
    std::tuple<int, int> global_index;

    // for local alignment
    Cell local_max_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)};
    std::tuple<int, int> local_index;

    // for semi-global alignment
    Cell last_row_max_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)};
    std::tuple<int, int> last_row_index;
    Cell last_col_max_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)};
    std::tuple<int, int> last_col_index;

    for (int i = 1; i < graph.size() + 1; i++) {
        for (int j = 1; j < target_len + 1; j++) {
            Cell diagonal_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)};
            Cell up_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)};

            // provjera jeli trenutni čvor početni čvor grafa
            if (graph[i - 1]->incoming_edges.empty()) {
                if (graph[i - 1]->letter == target[j - 1]) {  // Match
                    diagonal_cell = compare_cells(diagonal_cell, {align_matrix[0][j - 1].value + match,
                                                                  Direction::DiagonalMatch,
                                                                  std::tuple<int, int>(0, j - 1)});
                } else {
                    diagonal_cell = compare_cells(diagonal_cell, {align_matrix[0][j - 1].value + mismatch,
                                                                  Direction::DiagonalMismatch,
                                                                  std::tuple<int, int>(0, j - 1)});
                }
                up_cell = compare_cells(up_cell, {align_matrix[0][j].value + gap,
                                                  Direction::Vertical,
                                                  std::tuple<int, int>(0, j)});

            } else {
                if (graph[i - 1]->letter == target[j - 1]) {  // Match
                    for (Edge *edge : graph[i - 1]->incoming_edges) {
                        diagonal_cell = compare_cells(diagonal_cell, {align_matrix[edge->origin->index][j - 1].value + match,
                                                                      Direction::DiagonalMatch,
                                                                      std::tuple<int, int>(edge->origin->index, j - 1)});
                        up_cell = compare_cells(up_cell, {align_matrix[edge->origin->index][j].value + gap,
                                                          Direction::Vertical,
                                                          std::tuple<int, int>(edge->origin->index, j)});
                    }
                } else {
                    for (Edge *edge : graph[i - 1]->incoming_edges) {
                        diagonal_cell = compare_cells(diagonal_cell, {align_matrix[edge->origin->index][j - 1].value + mismatch,
                                                                      Direction::DiagonalMismatch,
                                                                      std::tuple<int, int>(edge->origin->index, j - 1)});
                        up_cell = compare_cells(up_cell, {align_matrix[edge->origin->index][j].value + gap,
                                                          Direction::Vertical,
                                                          std::tuple<int, int>(edge->origin->index, j)});
                    }
                }
            }

            Cell left_cell = {align_matrix[i][j - 1].value + gap, Direction::Horizontal, std::tuple<int, int>(i, j - 1)};

            align_matrix[i][j] = std::max({min_cell, diagonal_cell, up_cell, left_cell},
                                          [](Cell a, Cell b) { return a.value < b.value; });

            if (graph[i - 1]->outgoing_edges.empty() && j == target_len &&
                align_matrix[i][j].value >= global_max_cell.value) {
                global_max_cell = align_matrix[i][j];
                global_index = std::tuple<int, int>(i, j);
            }

            if (align_matrix[i][j].value >= local_max_cell.value) {
                local_max_cell = align_matrix[i][j];
                local_index = std::tuple<int, int>(i, j);
            }

            if (graph[i - 1]->outgoing_edges.empty() && align_matrix[i][j].value >= last_row_max_cell.value) {
                last_row_max_cell = align_matrix[i][j];
                last_row_index = std::tuple<int, int>(i, j);
            }
            if (j == target_len && align_matrix[i][j].value >= last_col_max_cell.value) {
                last_col_max_cell = align_matrix[i][j];
                last_col_index = std::tuple<int, int>(i, j);
            }
        }
    }

    switch (alignment) {
        case Global:
            return global_index;

        case Local:
            return local_index;

        case Semiglobal:
            return last_row_max_cell.value > last_col_max_cell.value ? last_row_index : last_col_index;
    }
    return std::tuple<int, int>(-1, -1);
}

std::tuple<int, int> Aligner::AlignTwoGraph(std::vector<std::vector<Cell>> &align_matrix,
                                            std::vector<Node *> &query_graph,
                                            std::vector<Node *> &target_graph,
                                            Cell min_cell) {
    // for global alignment
    Cell global_max_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)};
    std::tuple<int, int> global_index;

    // for local alignment
    Cell local_max_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)};
    std::tuple<int, int> local_index;

    // for semi-global alignment
    Cell last_row_max_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)};
    std::tuple<int, int> last_row_index;
    Cell last_col_max_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)};
    std::tuple<int, int> last_col_index;

    for (int i = 1; i < query_graph.size() + 1; i++) {
        for (int j = 1; j < target_graph.size() + 1; j++) {
            Cell diagonal_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)};
            Cell up_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)};
            Cell left_cell = {std::numeric_limits<int>::min(), Direction::None, std::tuple<int, int>(-1, -1)};

            // pregledava trenutni target čvor
            if (target_graph[j - 1]->incoming_edges.empty()) {
                if (query_graph[i - 1]->letter == target_graph[j - 1]->letter) {  // Match
                    diagonal_cell = compare_cells(diagonal_cell, {align_matrix[i - 1][0].value + match,
                                                                  Direction::DiagonalMatch,
                                                                  std::tuple<int, int>(i - 1, 0)});
                } else {
                    diagonal_cell = compare_cells(diagonal_cell, {align_matrix[i - 1][0].value + mismatch,
                                                                  Direction::DiagonalMismatch,
                                                                  std::tuple<int, int>(i - 1, 0)});
                }
                left_cell = compare_cells(left_cell, {align_matrix[i][0].value + gap,
                                                      Direction::Horizontal,
                                                      std::tuple<int, int>(i, 0)});
            } else {
                if (query_graph[i - 1]->letter == target_graph[j - 1]->letter) {  // Match
                    for (Edge *edge : target_graph[j - 1]->incoming_edges) {
                        diagonal_cell = compare_cells(diagonal_cell, {align_matrix[i - 1][edge->origin->index].value + match,
                                                                      Direction::DiagonalMatch,
                                                                      std::tuple<int, int>(i - 1, edge->origin->index)});
                        left_cell = compare_cells(left_cell, {align_matrix[i][edge->origin->index].value + gap,
                                                              Direction::Horizontal,
                                                              std::tuple<int, int>(i, edge->origin->index)});
                    }
                } else {
                    for (Edge *edge : target_graph[j - 1]->incoming_edges) {
                        diagonal_cell = compare_cells(diagonal_cell, {align_matrix[i - 1][edge->origin->index].value + mismatch,
                                                                      Direction::DiagonalMismatch,
                                                                      std::tuple<int, int>(i - 1, edge->origin->index)});
                        left_cell = compare_cells(left_cell, {align_matrix[i][edge->origin->index].value + gap,
                                                              Direction::Horizontal,
                                                              std::tuple<int, int>(i, edge->origin->index)});
                    }
                }
            }

            // pregledava trenutni query čvor
            if (query_graph[i - 1]->incoming_edges.empty()) {
                if (query_graph[i - 1]->letter == target_graph[j - 1]->letter) {  // Match
                    diagonal_cell = compare_cells(diagonal_cell, {align_matrix[0][j - 1].value + match,
                                                                  Direction::DiagonalMatch,
                                                                  std::tuple<int, int>(0, j - 1)});
                } else {
                    diagonal_cell = compare_cells(diagonal_cell, {align_matrix[0][j - 1].value + mismatch,
                                                                  Direction::DiagonalMismatch,
                                                                  std::tuple<int, int>(0, j - 1)});
                }
                up_cell = compare_cells(up_cell, {align_matrix[0][j].value + gap,
                                                  Direction::Vertical,
                                                  std::tuple<int, int>(0, j)});
            } else {
                if (query_graph[i - 1]->letter == target_graph[j - 1]->letter) {  // Match
                    for (Edge *edge : query_graph[i - 1]->incoming_edges) {
                        diagonal_cell = compare_cells(diagonal_cell, {align_matrix[edge->origin->index][j - 1].value + match,
                                                                      Direction::DiagonalMatch,
                                                                      std::tuple<int, int>(edge->origin->index, j - 1)});
                        up_cell = compare_cells(up_cell, {align_matrix[edge->origin->index][j].value + gap,
                                                          Direction::Vertical,
                                                          std::tuple<int, int>(edge->origin->index, j)});
                    }
                } else {
                    for (Edge *edge : query_graph[i - 1]->incoming_edges) {
                        diagonal_cell = compare_cells(diagonal_cell, {align_matrix[edge->origin->index][j - 1].value + mismatch,
                                                                      Direction::DiagonalMismatch,
                                                                      std::tuple<int, int>(edge->origin->index, j - 1)});
                        up_cell = compare_cells(up_cell, {align_matrix[edge->origin->index][j].value + gap,
                                                          Direction::Vertical,
                                                          std::tuple<int, int>(edge->origin->index, j)});
                    }
                }
            }

            align_matrix[i][j] = std::max({min_cell, diagonal_cell, up_cell, left_cell},
                                          [](Cell a, Cell b) { return a.value < b.value; });

            if (query_graph[i - 1]->outgoing_edges.empty() && target_graph[j - 1]->outgoing_edges.empty() &&
                align_matrix[i][j].value >= global_max_cell.value) {
                global_max_cell = align_matrix[i][j];
                global_index = std::tuple<int, int>(i, j);
            }

            if (align_matrix[i][j].value >= local_max_cell.value) {
                local_max_cell = align_matrix[i][j];
                local_index = std::tuple<int, int>(i, j);
            }

            if (query_graph[i - 1]->outgoing_edges.empty() && align_matrix[i][j].value >= last_row_max_cell.value) {
                last_row_max_cell = align_matrix[i][j];
                last_row_index = std::tuple<int, int>(i, j);
            }
            if (target_graph[j - 1]->outgoing_edges.empty() && align_matrix[i][j].value >= last_col_max_cell.value) {
                last_col_max_cell = align_matrix[i][j];
                last_col_index = std::tuple<int, int>(i, j);
            }
        }
    }

    switch (alignment) {
        case Global:
            return global_index;

        case Local:
            return local_index;

        case Semiglobal:
            return last_row_max_cell.value > last_col_max_cell.value ? last_row_index : last_col_index;
    }
    return std::tuple<int, int>(-1, -1);
}

// stvara graf na temelju matrice poravnanja
void Aligner::CreateGraph(std::vector<std::vector<Cell>> &align_matrix,
                          Graph &query, std::vector<Node *> &query_graph,
                          Graph &target, std::vector<Node *> &target_graph,
                          std::tuple<int, int> starting_index) {
    unsigned int query_index = std::get<0>(starting_index), target_index = std::get<1>(starting_index);

    bool finished = false;
    while (!finished) {
        Direction direction = align_matrix[query_index][target_index].direction;
        switch (direction) {
            case DiagonalMatch: {
                Node::fuse_two_nodes(target, query_graph[query_index - 1], target_graph[target_index - 1]);
                break;
            }

            case DiagonalMismatch: {
                Node::align_two_nodes(target, query_graph[query_index - 1], target_graph[target_index - 1]);

                // if target node is the starting node in target_graph
                // make it a starting node in query_graph
                if (target_graph[target_index - 1]->incoming_edges.empty()) {
                    query.start_nodes.push_back(target_graph[target_index - 1]);
                    target.start_nodes.erase(std::remove(target.start_nodes.begin(),
                                                         target.start_nodes.end(),
                                                         target_graph[target_index - 1]),
                                             target.start_nodes.end());
                }

                break;
            }

            case Horizontal: {
                if (target_graph[target_index - 1]->incoming_edges.empty()) {
                    query.start_nodes.push_back(target_graph[target_index - 1]);
                    target.start_nodes.erase(std::remove(target.start_nodes.begin(),
                                                         target.start_nodes.end(),
                                                         target_graph[target_index - 1]),
                                             target.start_nodes.end());
                }
                break;
            }

            case Vertical: {
                break;
            }

            case None: {
                finished = true;
                for (Node *target_node : target.start_nodes) {
                    if (target_node == nullptr) {
                        continue;
                    }
                    bool add = true;
                    for (Node *query_node : query.start_nodes) {
                        if (target_node == query_node) {
                            add = false;
                            break;
                        }
                    }
                    if (add) {
                        query.start_nodes.push_back(target_node);
                    }
                }
                break;
            }
        }

        int new_query_index = std::get<0>(align_matrix[query_index][target_index].parent_index);
        target_index = std::get<1>(align_matrix[query_index][target_index].parent_index);
        query_index = new_query_index;
    }
}

int Aligner::AlignAndGraphTwoSeq(Graph &empty_graph,
                                 const char *query, unsigned int query_len, const char *query_id,
                                 const char *target, unsigned int target_len, const char *target_id) {
    std::vector<std::vector<Cell>> align_matrix(query_len + 1, std::vector<Cell>(target_len + 1));

    std::tuple<int, int> starting_index;
    switch (alignment) {
        case Global:
            InitilazeMatrix(align_matrix, gap, query_len, target_len, true);
            starting_index = AlignTwoSeq(align_matrix, query, query_len, target, target_len);
            break;

        case Local:
            InitilazeMatrix(align_matrix, 0, query_len, target_len, false);
            starting_index = AlignTwoSeq(align_matrix, query, query_len, target, target_len,
                                         {0, Direction::None, std::tuple<int, int>(-1, -1)});
            break;

        case Semiglobal:
            InitilazeMatrix(align_matrix, 0, query_len, target_len, false);
            starting_index = AlignTwoSeq(align_matrix, query, query_len, target, target_len);
            break;
    }

    Graph::LinearGraph(empty_graph, query, query_len, query_id);
    std::vector<Node *> top_query = empty_graph.TopologicalSort();

    Graph target_graph;
    Graph::LinearGraph(target_graph, target, target_len, target_id);
    std::vector<Node *> top_target = target_graph.TopologicalSort();

    CreateGraph(align_matrix, empty_graph, top_query, target_graph, top_target, starting_index);
    return align_matrix[std::get<0>(starting_index)][std::get<1>(starting_index)].value;
}

int Aligner::AlignAndGraphSeqAndGraph(Graph &query,
                                      const char *target, unsigned int target_len, const char *target_id) {
    std::vector<Node *> query_graph = query.TopologicalSort();
    std::vector<std::vector<Cell>> align_matrix(query_graph.size() + 1, std::vector<Cell>(target_len + 1));

    std::tuple<int, int> starting_index;
    switch (alignment) {
        case Global:
            InitilazeMatrix(align_matrix, gap, query_graph.size(), target_len, true);
            starting_index = AlignSeqAndGraph(align_matrix, query_graph, target, target_len);
            break;

        case Local:
            InitilazeMatrix(align_matrix, 0, query_graph.size(), target_len, false);
            starting_index = AlignSeqAndGraph(align_matrix, query_graph, target, target_len,
                                              {0, Direction::None, std::tuple<int, int>(-1, -1)});
            break;

        case Semiglobal:
            InitilazeMatrix(align_matrix, 0, query_graph.size(), target_len, false);
            starting_index = AlignSeqAndGraph(align_matrix, query_graph, target, target_len);
            break;
    }

    Graph target_graph;
    Graph::LinearGraph(target_graph, target, target_len, target_id);
    std::vector<Node *> top_target = target_graph.TopologicalSort();

    CreateGraph(align_matrix, query, query_graph, target_graph, top_target, starting_index);
    return align_matrix[std::get<0>(starting_index)][std::get<1>(starting_index)].value;
}

int Aligner::AlignAndGraphTwoGraph(Graph &query, Graph &target) {
    std::vector<Node *> query_graph = query.TopologicalSort();
    std::vector<Node *> target_graph = target.TopologicalSort();
    std::vector<std::vector<Cell>> align_matrix(query_graph.size() + 1, std::vector<Cell>(target_graph.size() + 1));

    std::tuple<int, int> starting_index;
    switch (alignment) {
        case Global:
            InitilazeMatrix(align_matrix, gap, query_graph.size(), target_graph.size(), true);
            starting_index = AlignTwoGraph(align_matrix, query_graph, target_graph);
            break;

        case Local:
            InitilazeMatrix(align_matrix, 0, query_graph.size(), target_graph.size(), false);
            starting_index = AlignTwoGraph(align_matrix, query_graph, target_graph,
                                           {0, Direction::None, std::tuple<int, int>(-1, -1)});
            break;

        case Semiglobal:
            InitilazeMatrix(align_matrix, 0, query_graph.size(), target_graph.size(), false);
            starting_index = AlignTwoGraph(align_matrix, query_graph, target_graph);
            break;
    }

    CreateGraph(align_matrix, query, query_graph, target, target_graph, starting_index);
    return align_matrix[std::get<0>(starting_index)][std::get<1>(starting_index)].value;
}
