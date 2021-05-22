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

    int8_t match = 1;
    int8_t mismatch = -1;
    int8_t gap = -1;
    int8_t alignment = 0;
    int8_t num_of_threads = 1;

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
                num_of_threads = atoi(optarg) > omp_get_max_threads() ? omp_get_max_threads() : atoi(optarg);
                break;
            default:
                return 1;
        }
    }

    if (argc != optind + 1) {
        std::cout << "Wrong number of arguments" << std::endl;
        return 1;
    }

    std::string sequence_file = argv[optind];
    auto sequences = bioparser::Parser<Sequence>::Create<bioparser::FastaParser>(sequence_file)->Parse(-1);

    Aligner *a = new Aligner(match, mismatch, gap, (Alignment)alignment);

    Graph graphs[num_of_threads];
    int index = sequences.size() / num_of_threads;

    std::vector<int> used_index(num_of_threads);

    for (int i = 0; i < num_of_threads; i++) {
        LinearGraph(graphs[i],
                    sequences[i * index]->data.c_str(),
                    sequences[i * index]->data.size(),
                    sequences[i * index]->name.c_str());
        used_index.push_back(i * index);
    }

#pragma omp parallel for schedule(static) num_threads(num_of_threads)
    for (int i = 0; i < sequences.size(); i++) {
        bool used = false;
        for (int index : used_index) {
            if (i == index) {
                used = true;
            }
        }

        if (used) {
            continue;
        }

        a->AlignAndGraphSeqAndGraph(graphs[omp_get_thread_num()],
                                    sequences[i]->data.c_str(),
                                    sequences[i]->data.size(),
                                    sequences[i]->name.c_str());
    }

    for (int i = 1; i < num_of_threads; i++) {
        a->AlignAndGraphTwoGraph(graphs[0], graphs[i]);
    }

    std::vector<Node *> graph = graphs[0].TopologicalSort();
    for (Node *node : graph) {
        std::cout << node->letter << " ";
        for (auto origin : node->origin_of_letter) {
            std::cout << std::get<0>(origin) << std::get<1>(origin) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    return 0;
}
