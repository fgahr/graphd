#include <graphd/graph.hpp>
#include <graphd/input/parse.hpp>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include <getopt.h>

void usage(std::string progname) {
    std::cerr << "usage: " << progname << " [-f file.dot] from-node to-node\n"
              << "  if no input file is specified, stdin is assumed.\n";
}

int run(std::istream &in, int argc, char **argv) {
    if (optind > argc - 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    graphd::NodeName from_node = argv[optind];
    graphd::NodeName to_node = argv[optind + 1];

    auto parser = graphd::input::Parser::of(in);

    try {
        graphd::Graph g;
        std::unique_ptr<graphd::input::Expression> e{parser.parse()};
        e->apply_to_graph(g);

        graphd::Path p = g.shortest_path(from_node, to_node);
        std::cout << "total distance: " << p.total_distance << "\n";
        std::cout << p.nodes[0];
        for (size_t i = 1; i < p.nodes.size(); i++) {
            std::cout << " -> " << p.nodes[i];
        }
        std::cout << "\n";

        return EXIT_SUCCESS;
    } catch (const std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}

int main(int argc, char **argv) {
    if (int opt = getopt(argc, argv, "f:"); opt == 'f') {
        std::fstream f{optarg};
        return run(f, argc, argv);
        f.close();
    } else {
        return run(std::cin, argc, argv);
    }
}
