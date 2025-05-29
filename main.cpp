#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "parser.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Errore: devi specificare un file da interpretare.\n";
        std::cerr << "Esempio d'uso: " << argv[0] << " script.rn\n";
        return 1;
    }

    std::ifstream infile(argv[1]);
    if (!infile) {
        std::cerr << "Could not open file " << argv[1] << std::endl;
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
