#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <sstream>
#include <vector>

enum class VoltType { INT, FLOAT, DOUBLE };

inline std::string voltTypeToLLVM(VoltType t) {
    switch (t) {
        case VoltType::INT: return "i32";
        case VoltType::FLOAT: return "float";
        case VoltType::DOUBLE: return "double";
    }
    return "i32";
}

struct Context {
    std::unordered_map<std::string, VoltType> var_types;
    int temp_index = 0;
};

struct Expr {
    virtual ~Expr() = default;
    virtual VoltType getType(Context& ctx) const = 0;
    virtual std::string toLLVMIR(Context& ctx, std::string& result_var) const = 0;
};

struct Number : Expr {
    std::string text;
    VoltType type;
    Number(const std::string& t, VoltType tp) : text(t), type(tp) {}
    VoltType getType(Context&) const override { return type; }
    std::string toLLVMIR(Context&, std::string& result_var) const override {
        result_var = "%t" + std::to_string(rand() % 10000);
        std::ostringstream oss;
        if (type == VoltType::INT)
            oss << result_var << " = add i32 0, " << text;
        else if (type == VoltType::FLOAT)
            oss << result_var << " = fadd float 0.0, " << text << " ; float";
        else if (type == VoltType::DOUBLE)
            oss << result_var << " = fadd double 0.0, " << text << " ; double";
        return oss.str();
    }
};

struct Variable : Expr {
    std::string name;
    Variable(const std::string& n) : name(n) {}
    VoltType getType(Context& ctx) const override {
        auto it = ctx.var_types.find(name);
        if (it == ctx.var_types.end()) throw std::runtime_error("Undefined variable: " + name);
        return it->second;
    }
    std::string toLLVMIR(Context&, std::string& result_var) const override {
        result_var = "%" + name;
        return "; uso variabile " + name;
    }
};

struct BinOp : Expr {
    int op;
    std::unique_ptr<Expr> left, right;
    BinOp(int op, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
        : op(op), left(std::move(l)), right(std::move(r)) {}
    VoltType getType(Context& ctx) const override {
        VoltType lt = left->getType(ctx), rt = right->getType(ctx);
        if (lt == VoltType::DOUBLE || rt == VoltType::DOUBLE) return VoltType::DOUBLE;
        if (lt == VoltType::FLOAT || rt == VoltType::FLOAT) return VoltType::FLOAT;
        return VoltType::INT;
    }
    std::string toLLVMIR(Context& ctx, std::string& result_var) const override {
        std::string lvar, rvar;
        std::ostringstream oss;
        oss << left->toLLVMIR(ctx, lvar) << "\n";
        oss << right->toLLVMIR(ctx, rvar) << "\n";
        VoltType t = getType(ctx);
        std::string llvmT = voltTypeToLLVM(t);
        result_var = "%t" + std::to_string(++ctx.temp_index);

        // Floating point or integer operation
        bool isFloat = (t == VoltType::FLOAT || t == VoltType::DOUBLE);
        std::string opstr;
        switch(op) {
        case 6: opstr = isFloat ? "fadd" : "add"; break;
        case 7: opstr = isFloat ? "fsub" : "sub"; break;
        case 8: opstr = isFloat ? "fmul" : "mul"; break;
        case 9: opstr = isFloat ? "fdiv" : "sdiv"; break;
        case 10: opstr = isFloat ? "frem" : "srem"; break;
        case 11: { // AND logico (&&)
            std::string lbool = "%t" + std::to_string(++ctx.temp_index);
            std::string rbool = "%t" + std::to_string(++ctx.temp_index);
            std::string andres = "%t" + std::to_string(++ctx.temp_index);
            result_var = "%t" + std::to_string(++ctx.temp_index);
            oss << lbool << " = icmp ne " << llvmT << " " << lvar << ", 0\n";
            oss << rbool << " = icmp ne " << llvmT << " " << rvar << ", 0\n";
            oss << andres << " = and i1 " << lbool << ", " << rbool << "\n";
            oss << result_var << " = zext i1 " << andres << " to i32";
            return oss.str();
        }
        case 12: { // OR logico (||)
            std::string lbool = "%t" + std::to_string(++ctx.temp_index);
            std::string rbool = "%t" + std::to_string(++ctx.temp_index);
            std::string orres = "%t" + std::to_string(++ctx.temp_index);
            result_var = "%t" + std::to_string(++ctx.temp_index);
            oss << lbool << " = icmp ne " << llvmT << " " << lvar << ", 0\n";
            oss << rbool << " = icmp ne " << llvmT << " " << rvar << ", 0\n";
            oss << orres << " = or i1 " << lbool << ", " << rbool << "\n";
            oss << result_var << " = zext i1 " << orres << " to i32";
            return oss.str();
        }
        default: opstr = isFloat ? "fadd" : "add";
        }
        oss << result_var << " = " << opstr << " " << llvmT << " " << lvar << ", " << rvar;
        return oss.str();
    }
};

struct VarDeclaration : Expr {
    std::string name;
    VoltType type;
    std::unique_ptr<Expr> expr;
    VarDeclaration(const std::string& n, VoltType t, std::unique_ptr<Expr> e)
        : name(n), type(t), expr(std::move(e)) {}
    VoltType getType(Context&) const override { return type; }
    std::string toLLVMIR(Context& ctx, std::string& result_var) const override {
        std::string expr_var;
        std::ostringstream oss;
        oss << expr->toLLVMIR(ctx, expr_var) << "\n";
        std::string llvmT = voltTypeToLLVM(type);
        oss << "%" << name << " = alloca " << llvmT << "\n";
        oss << "store " << llvmT << " " << expr_var << ", " << llvmT << "* %" << name;
        ctx.var_types[name] = type;
        result_var = "%" + name;
        return oss.str();
    }
};

struct IfStatement : Expr {
    struct Branch {
        std::unique_ptr<Expr> condition; // nullptr for 'else'
        std::vector<std::unique_ptr<Expr>> statements;
    };
    std::vector<Branch> branches;
    VoltType getType(Context&) const override { return VoltType::INT; }
    std::string toLLVMIR(Context&, std::string&) const override {
        return "; IF/ELSEIF/ELSE not yet implemented in IR";
    }
};

class Parser {
public:
    static std::unique_ptr<Expr> parse(const std::string& line);
};