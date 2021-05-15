#include <getopt.h>

#include <iostream>
#include <memory>
#include <string>

#include "alignment.hpp"
#include "bioparser/fasta_parser.hpp"

static struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"match", required_argument, NULL, 'm'},
    {"alignment", required_argument, NULL, 'a'},
    {"mismatch", required_argument, NULL, 'n'},
    {"gap", required_argument, NULL, 'g'}};

class Sequence {
   public:
    std::string name;
    std::string data;
    Sequence(const char *name, std::uint32_t name_len, const char *data, std::uint32_t data_len)
        : name(name, name_len), data(data, data_len){};
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
                 "  -h, --help\n"
                 "      Print usage information\n";
}

int main(int argc, char *argv[]) {
    int opt;

    int8_t match = 1;
    int8_t mismatch = -1;
    int8_t gap = -1;
    int8_t alignment = 0;

    while ((opt = getopt_long(argc, argv, "a:g:hm:n:", long_options, NULL)) != -1) {
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

    for (int i = 0; i < sequences.size(); i++) {
        std::cout << sequences[i]->name << std::endl;
        std::cout << sequences[i]->data.size() << std::endl;
    }
    Aligner *a = new Aligner(match, mismatch, gap, (Alignment)alignment);
    Graph g;
    a->AlignAndGraphTwoSeq(g, sequences[0]->data.c_str(), sequences[0]->data.size(), sequences[0]->name.c_str(),
                           sequences[2]->data.c_str(), sequences[2]->data.size(), sequences[2]->name.c_str());

    for (Node *node : g.start_nodes) {
        std::cout << node->letter << ", ";
        for (auto origin : node->origin_of_letter) {
            std::cout << std::get<0>(origin) << ", ";
        }
        std::cout << std::endl;
    }
    return 0;
}
