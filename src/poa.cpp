#include <iostream>
#include <stack>

#include "alignment.hpp"

void createAlignedGraph(Graph &graph, const char *query_id, const char *target_id, std::stack<std::tuple<char, char>> &align_vector) {
    Node *prev_query_node, *prev_target_node;
    Node *query_node, *target_node;
    unsigned int query_index = 1, target_index = 1;

    //početni čvorovi grafa
    char query = std::get<0>(align_vector.top());
    char target = std::get<1>(align_vector.top());
    if (query == '-') {
        prev_target_node = graph.addNode(target, target_id, target_index++);
        prev_query_node = prev_target_node;

    } else if (target == '-') {
        prev_query_node = graph.addNode(query, query_id, query_index++);
        prev_target_node = prev_query_node;

    } else if (query == target) {
        prev_query_node = graph.addNode(query, query_id, query_index++);
        prev_target_node = prev_query_node;
        std::tuple<const char *, unsigned int> origin(target_id, target_index++);
        prev_query_node->origin_of_letter.push_back(origin);

    } else {
        prev_query_node = graph.addNode(query, query_id, query_index++);
        prev_target_node = graph.addNode(target, target_id, target_index++);
    }

    align_vector.pop();

    // ostali čvorovi
    while (!align_vector.empty()) {
        query = std::get<0>(align_vector.top());
        target = std::get<1>(align_vector.top());
        if (query == '-') {
            target_node = graph.addNode(target, target_id, target_index++, prev_target_node);
            prev_target_node = target_node;

        } else if (target == '-') {
            query_node = graph.addNode(query, query_id, query_index++, prev_query_node);
            prev_query_node = query_node;

        } else if (query == target) {
            query_node = graph.addNode(query, query_id, query_index++, prev_query_node);
            std::tuple<const char *, unsigned int> origin(target_id, target_index++);
            query_node->origin_of_letter.push_back(origin);
            target_node = query_node;

            //spriječava stvaranje duplikatnih edgova
            if (prev_query_node != prev_target_node) {
                Edge *target_edge = new Edge(prev_target_node, query_node);
                prev_target_node->outgoing_edges.push_back(target_edge);
                target_node->incoming_edges.push_back(target_edge);
            }

            prev_target_node = target_node;
            prev_query_node = query_node;

        } else {
            query_node = graph.addNode(query, query_id, query_index++, prev_query_node);
            target_node = graph.addNode(target, target_id, target_index++, prev_target_node);

            prev_target_node = target_node;
            prev_query_node = query_node;
        }
        align_vector.pop();
    }
}

void AlignAndCreateGraph(Graph &graph, const char *query, unsigned int query_len, const char *query_id, const char *target,
                         unsigned int target_len, const char *target_id, int match, int mismatch, int gap, Aligner &aligner) {
    //matrica za poravnavanje dva niza
    std::vector<std::vector<Cell>> align_matrix(query_len + 1, std::vector<Cell>(target_len + 1));
    aligner.AlignTwoSeq(align_matrix, query, query_len, target, target_len, match, mismatch, gap);

    //stog čiji elementi opisuju izračunato poravnanje
    std::stack<std::tuple<char, char>> align_vector;
    int i = query_len, j = target_len;
    while (i > 0 || j > 0) {
        switch (align_matrix[i][j].direction) {
            case (Direction::Diagonal):
                align_vector.push(std::tuple<char, char>(query[i - 1], target[j - 1]));
                i--;
                j--;
                break;
            case (Direction::Horizontal):
                align_vector.push(std::tuple<char, char>('-', target[j - 1]));
                j--;
                break;
            case (Direction::Vertical):
                align_vector.push(std::tuple<char, char>(query[i - 1], '-'));
                i--;
                break;
        }
    }

    createAlignedGraph(graph, query_id, target_id, align_vector);
}

int main(void) {
    Graph graph;
    Aligner *aligner = new GlobalAligner();
    AlignAndCreateGraph(graph, "ACCT", 4, "prva", "AGGT", 4, "druga", 1, -1, -1, *aligner);
    int x = aligner->AlignSeqAndGraph("ATC", 3, graph, 1, -1, -1);
    std::cout << "rez = " << x << std::endl;
}
