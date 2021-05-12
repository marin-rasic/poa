#include <iostream>
#include <stack>

#include "alignment.hpp"

int main(void) {
    Aligner *alig = new Aligner(1, -1, -1, Alignment::Global);
    Graph g;
    alig->AlignAndGraphTwoSeq(g, "CGGT", 4, "prva", "CCAT", 4, "druga");

    Graph f;
    alig->AlignAndGraphTwoSeq(f, "CGTT", 4, "treca", "CTTG", 4, "cetvrta");

    return 0;
}
