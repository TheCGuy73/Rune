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
    std::ostringstream llvm_ir;
    llvm_ir << "; ModuleID = 'volt_module'\n";
    llvm_ir << "define i32 @main() {\n";

    int lineno = 0;
    while (std::getline(infile, line)) {
        lineno++;
        if (line.empty()) continue;
        // Rimuove eventuali spazi finali
        size_t end = line.find_last_not_of(" \t\r\n");
        if (end == std::string::npos) continue;
        line = line.substr(0, end + 1);

        if (line.empty()) continue;

        // Controllo rigido per il punto e virgola
        if (line.back() != ';') {
            std::cerr << "Errore: la riga " << lineno << " non termina con ';': " << line << std::endl;
            return 1;
        }

        try {
            auto expr = Parser::parse(line);
            std::string result_var;
            llvm_ir << "  " << expr->toLLVMIR(ctx, result_var) << "\n";
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << " in line " << lineno << ": " << line << std::endl;
            return 1;
        }
    }

    llvm_ir << "  ret i32 0\n";
    llvm_ir << "}\n";

    std::cout << "----- OUTPUT LLVM IR -----" << std::endl;
    std::cout << llvm_ir.str();

    return 0;
}