#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <stdexcept>

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
    int op;
    std::unique_ptr<Expr> left, right;
    BinOp(int op, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
        : op(op), left(std::move(l)), right(std::move(r)) {}
    int eval(Context& ctx) const override {
        int a = left->eval(ctx), b = right->eval(ctx);
        switch(op) {
        case 6:  // PLUS
            return a + b;
        case 7:  // MINUS
            return a - b;
        case 8:  // MUL
            return a * b;
        case 9:  // DIV
            if (b == 0) throw std::runtime_error("Division by zero");
            return a / b;
        case 10: // MOD
            if (b == 0) throw std::runtime_error("Modulo by zero");
            return a % b;
        case 11: // AND
            return a && b;
        case 12: // OR
            return a || b;
        default:
            throw std::runtime_error("Unknown operator");
        }
    }
    bool is_logical() const override {
        return op == 11 || op == 12; // AND or OR
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