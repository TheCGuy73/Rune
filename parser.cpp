#include "parser.h"
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <memory>

struct Token {
    TokenType type;
    int value;
    std::string text;
};

class Lexer {
    std::istringstream in;
    char curr;
    void next_char() { curr = in.get(); }
public:
    explicit Lexer(const std::string& s) : in(s), curr(0) { next_char(); }
    Token next() {
        while (curr != EOF && std::isspace(curr)) next_char();
        if (curr == EOF || curr == -1) return {END, 0, ""};
        if (std::isdigit(curr)) {
            int v = 0;
            while (std::isdigit(curr)) {
                v = v * 10 + (curr - '0');
                next_char();
            }
            return {NUM, v, ""};
        }
        if (std::isalpha(curr)) {
            std::string id;
            while (std::isalnum(curr)) {
                id += curr;
                next_char();
            }
            if (id == "what") return {WHAT, 0, ""};
            if (id == "int") return {TYPE, 0, ""};
            return {ID, 0, id};
        }
        switch (curr) {
        case ':': next_char(); return {COLON, 0, ""};
        case '=': next_char(); return {ASSIGN, 0, ""};
        case '+': next_char(); return {PLUS, 0, ""};
        case '-': next_char(); return {MINUS, 0, ""};
        case '*': next_char(); return {MUL, 0, ""};
        case '/': next_char(); return {DIV, 0, ""};
        case '%': next_char(); return {MOD, 0, ""};
        case '&': next_char(); return {AND, 0, ""};
        case '|': next_char(); return {OR, 0, ""};
        case '(': next_char(); return {LPAREN, 0, ""};
        case ')': next_char(); return {RPAREN, 0, ""};
        case ';': next_char(); return {SEMICOLON, 0, ""};
        default:
            throw std::runtime_error("Invalid character");
        }
    }
};

class ParserImpl {
    Lexer lex;
    Token curr;
    void next() { curr = lex.next(); }
public:
    explicit ParserImpl(const std::string& s) : lex(s) { next(); }
    std::unique_ptr<Expr> parse() {
        auto node = statement();
        if (curr.type != SEMICOLON)
            throw std::runtime_error("Expected ';' at end of statement");
        next();
        if (curr.type != END)
            throw std::runtime_error("Unexpected input after ';'");
        return node;
    }
private:
    std::unique_ptr<Expr> statement() {
        if (curr.type == WHAT) {
            next();
            if (curr.type != ID) throw std::runtime_error("Expected variable name");
            std::string var = curr.text;
            next();
            if (curr.type != COLON) throw std::runtime_error("Expected ':' after variable name");
            next();
            if (curr.type != TYPE) throw std::runtime_error("Expected type 'int'");
            next();
            if (curr.type != ASSIGN) throw std::runtime_error("Expected '=' after type");
            next();
            auto value = expr();
            return std::make_unique<VarDeclaration>(var, std::move(value));
        }
        return expr();
    }

    std::unique_ptr<Expr> expr() { return or_expr(); }

    std::unique_ptr<Expr> or_expr() {
        auto node = and_expr();
        while (curr.type == OR) {
            next();
            node = std::make_unique<BinOp>(OR, std::move(node), and_expr());
        }
        return node;
    }
    std::unique_ptr<Expr> and_expr() {
        auto node = add_expr();
        while (curr.type == AND) {
            next();
            node = std::make_unique<BinOp>(AND, std::move(node), add_expr());
        }
        return node;
    }
    std::unique_ptr<Expr> add_expr() {
        auto node = mul_expr();
        while (curr.type == PLUS || curr.type == MINUS) {
            TokenType op = curr.type; next();
            node = std::make_unique<BinOp>(op, std::move(node), mul_expr());
        }
        return node;
    }
    std::unique_ptr<Expr> mul_expr() {
        auto node = unary_expr();
        while (curr.type == MUL || curr.type == DIV || curr.type == MOD) {
            TokenType op = curr.type; next();
            node = std::make_unique<BinOp>(op, std::move(node), unary_expr());
        }
        return node;
    }
    std::unique_ptr<Expr> unary_expr() {
        if (curr.type == MINUS) {
            next();
            return std::make_unique<BinOp>(MINUS, std::make_unique<Number>(0), unary_expr());
        }
        return primary();
    }
    std::unique_ptr<Expr> primary() {
        if (curr.type == NUM) {
            int v = curr.value; next();
            return std::make_unique<Number>(v);
        }
        if (curr.type == ID) {
            std::string name = curr.text; next();
            return std::make_unique<Variable>(name);
        }
        if (curr.type == LPAREN) {
            next();
            auto node = expr();
            if (curr.type != RPAREN) throw std::runtime_error("Expected ')'");
            next();
            return node;
        }
        throw std::runtime_error("Expected number, variable or '('");
    }
};

std::unique_ptr<Expr> Parser::parse(const std::string& line) {
    return ParserImpl(line).parse();
}
