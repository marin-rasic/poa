#include <getopt.h>

#include <iostream>
#include <memory>
#include <string>

#include "alignment.hpp"
#include "bioparser/fasta_parser.hpp"
#include "omp.h"

static struct option long_options[] = {{"help", no_argument, NULL, 'h'},
                                       {"match", required_argument, NULL, 'm'},
                                       {"alignment", required_argument, NULL, 'a'},
                                       {"mismatch", required_argument, NULL, 'n'},
                                       {"gap", required_argument, NULL, 'g'},
                                       {"threads", required_argument, NULL, 't'}};

class Sequence {
   public:
    std::string name;
    std::string data;
    Sequence(const char *name, std::uint32_t name_len, const char *data, std::uint32_t data_len) : name(name, name_len), data(data, data_len){};
};

void Help() {
    std::cout << "usage: poa <sequence_file> [options]\n"
                 "\n"
                 " [options]\n"
                 "  -a, --alignment <int>\n"
                 "      default: 0\n"
                 "      alignment alogrithm:\n"
                 "          0: Global alignment\n"
                 "          1: Local alignment\n"
                 "          2: Semiglobal alignment\n"
                 "  -m, --match <int>\n"
                 "      default: 1\n"
                 "      score for matching bases\n"
                 "  -n, --mismatch <int>\n"
                 "      defualt: -1\n"
                 "      score for mismatching bases\n"
                 "  -g, --gap <int>\n"
                 "      default: -1\n"
                 "      linear gap penalty\n"
                 "  -t, --threads <int>\n"
                 "      default: 1\n"
                 "      number of threads"
                 "  -h, --help\n"
                 "      Print usage information\n";
}

int main(int argc, char *argv[]) {
    int opt;

    int8_t match = 5;
    int8_t mismatch = -4;
    int8_t gap = -8;
    int8_t alignment = 0;
    int num_of_threads = 1;

    while ((opt = getopt_long(argc, argv, "a:g:hm:n:t:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'a':
                alignment = atoi(optarg);
                break;
            case 'g':
                gap = atoi(optarg);
                break;
            case 'h':
                Help();
                return 0;
            case 'm':
                match = atoi(optarg);
                break;
            case 'n':
                mismatch = atoi(optarg);
                break;
            case 't':
                num_of_threads = atoi(optarg);
                break;
            default:
                return 1;
        }
    }

    Aligner *a = new Aligner(match, mismatch, gap, (Alignment)alignment);

    if (argc != optind + 1) {
        std::cout << "Wrong number of arguments" << std::endl;
        return 1;
    }

    std::string sequence_file = argv[optind];
    auto sequences = bioparser::Parser<Sequence>::Create<bioparser::FastaParser>(sequence_file)->Parse(-1);

    Graph g;
    std::vector<Graph> graphs(sequences.size() / 2, g);
    int max_index = sequences.size() % 2 == 0 ? sequences.size() : sequences.size() - 1;

#pragma omp parallel for num_threads(num_of_threads)
    for (int i = 0; i < sequences.size(); i = i + 2) {
        a->AlignAndGraphTwoSeq(graphs[i / 2],
                               sequences[i]->data.c_str(),
                               sequences[i]->data.size(),
                               sequences[i]->name.c_str(),
                               sequences[i + 1]->data.c_str(),
                               sequences[i + 1]->data.size(),
                               sequences[i + 1]->name.c_str());
    }

    if (max_index == sequences.size() - 1) {
        a->AlignAndGraphSeqAndGraph(graphs[0],
                                    sequences[max_index]->data.c_str(),
                                    sequences[max_index]->data.size(),
                                    sequences[max_index]->data.c_str());
    }

    do {
        std::vector<Graph> new_graphs;
        int max_index = graphs.size() % 2 == 0 ? graphs.size() : graphs.size() - 1;

#pragma omp parallel for num_threads(num_of_threads)
        for (int i = 0; i < max_index; i = i + 2) {
            a->AlignAndGraphTwoGraph(graphs[i], graphs[i + 1]);
            new_graphs.push_back(graphs[i]);
        }

        if (max_index == graphs.size() - 1) {
            a->AlignAndGraphTwoGraph(graphs[0], graphs[max_index]);
        }
        graphs = new_graphs;
    } while (graphs.size() / 2 >= 1);

    std::string consensus = graphs[0].FindConsensus();

    std::cout << "Consensus " << consensus.size() << std::endl;
    std::cout << consensus << std::endl;
    return 0;
}
