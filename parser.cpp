#include "parser.h"
#include <cctype>
#include <stdexcept>
#include <string>
#include <memory>
#include <sstream>

namespace {

enum TokenType { NUM, ID, WHAT, COLON, TYPE, ASSIGN, PLUS, MINUS, MUL, DIV, MOD, AND, OR, LPAREN, RPAREN, END, EQ };

struct Token {
    TokenType type;
    std::string text;
    int value;
};

class Lexer {
    const std::string& text;
    size_t pos = 0;
public:
    Lexer(const std::string& s) : text(s) {}
    Token next() {
        while (pos < text.size() && std::isspace(text[pos])) ++pos;
        if (pos >= text.size() || text[pos] == ';') {
            if (pos < text.size() && text[pos] == ';') ++pos;
            return {END, "", 0};
        }

        if (std::isdigit(text[pos])) {
            int n = 0;
            while (pos < text.size() && std::isdigit(text[pos]))
                n = n * 10 + (text[pos++] - '0');
            return {NUM, "", n};
        }

        if (std::isalpha(text[pos]) || text[pos] == '_') {
            size_t start = pos;
            while (pos < text.size() && (std::isalnum(text[pos]) || text[pos] == '_'))
                ++pos;
            std::string word = text.substr(start, pos - start);
            if (word == "what") return {WHAT, "", 0};
            if (word == "int") return {TYPE, "", 0};
            return {ID, word, 0};
        }

        if (text[pos] == ':') { ++pos; return {COLON, "", 0}; }
        if (text[pos] == '=') { ++pos; return {ASSIGN, "", 0}; }
        if (text[pos] == '+') { ++pos; return {PLUS, "", 0}; }
        if (text[pos] == '-') { ++pos; return {MINUS, "", 0}; }
        if (text[pos] == '*') { ++pos; return {MUL, "", 0}; }
        if (text[pos] == '/') { ++pos; return {DIV, "", 0}; }
        if (text[pos] == '%') { ++pos; return {MOD, "", 0}; }
        if (text.substr(pos,2) == "&&") { pos += 2; return {AND, "", 0}; }
        if (text.substr(pos,2) == "||") { pos += 2; return {OR, "", 0}; }
        if (text[pos] == '(') { ++pos; return {LPAREN, "", 0}; }
        if (text[pos] == ')') { ++pos; return {RPAREN, "", 0}; }
        throw std::runtime_error("Invalid character");
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
        if (curr.type != END)
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

} // end anonymous namespace

std::unique_ptr<Expr> Parser::parse(const std::string& line) {
    ParserImpl p(line);
    return p.parse();
}