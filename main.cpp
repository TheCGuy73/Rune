#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "parser.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file.volt>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    // Verifica che il file abbia estensione .volt
    if (filename.size() < 5 || filename.substr(filename.size() - 5) != ".volt") {
        std::cerr << "Il file deve avere estensione .volt" << std::endl;
        return 1;
    }

    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "Could not open file " << filename << std::endl;
        return 1;
    }

    Context ctx;
    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty()) continue;
        try {
            auto expr = Parser::parse(line);
            int result = expr->eval(ctx);
            if (expr->is_logical()) {
                std::cout << (result ? "vero" : "falso") << std::endl;
            } else {
                std::cout << result << std::endl;
            }
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << " in line: " << line << std::endl;
        }
    }
    return 0;
}
