#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <stdexcept>

enum TokenType {
    END, NUM, ID, WHAT, COLON, TYPE, ASSIGN,
    PLUS, MINUS, MUL, DIV, MOD, AND, OR, LPAREN, RPAREN, SEMICOLON
};

struct Context {
    std::unordered_map<std::string, int> variables;
};

struct Expr {
    virtual ~Expr() = default;
    virtual int eval(Context& ctx) const = 0;
    virtual bool is_logical() const { return false; }
};

struct Number : Expr {
    int value;
    explicit Number(int v) : value(v) {}
    int eval(Context&) const override { return value; }
};

struct Variable : Expr {
    std::string name;
    explicit Variable(const std::string& n) : name(n) {}
    int eval(Context& ctx) const override {
        auto it = ctx.variables.find(name);
        if (it == ctx.variables.end()) throw std::runtime_error("Undefined variable: " + name);
        return it->second;
    }
};

struct BinOp : Expr {
    TokenType op;
    std::unique_ptr<Expr> left, right;
    BinOp(TokenType op, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
        : op(op), left(std::move(l)), right(std::move(r)) {}
    int eval(Context& ctx) const override {
        int a = left->eval(ctx), b = right->eval(ctx);
        switch(op) {
        case PLUS:
            return a + b;
        case MINUS:
            return a - b;
        case MUL:
            return a * b;
        case DIV:
            if (b == 0) throw std::runtime_error("Division by zero");
            return a / b;
        case MOD:
            if (b == 0) throw std::runtime_error("Modulo by zero");
            return a % b;
        case AND:
            return a && b;
        case OR:
            return a || b;
        default:
            throw std::runtime_error("Unknown operator");
        }
    }
    bool is_logical() const override {
        return op == AND || op == OR;
    }
};

struct VarDeclaration : Expr {
    std::string name;
    std::unique_ptr<Expr> expr;
    VarDeclaration(const std::string& n, std::unique_ptr<Expr> e)
        : name(n), expr(std::move(e)) {}
    int eval(Context& ctx) const override {
        int val = expr->eval(ctx);
        ctx.variables[name] = val;
        return val;
    }
};

class Parser {
public:
    static std::unique_ptr<Expr> parse(const std::string& line);
};
