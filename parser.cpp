#include "parser.h"
#include <sstream>
#include <vector>

// Forward declarations for types used in this file
class Context;

namespace {

// Token types including logic symbols
enum TokenType {
    NUM, ID, WHAT, COLON, TYPE, ASSIGN, PLUS, MINUS, MUL, DIV, MOD,
    AND, OR, LPAREN, RPAREN, END, SEMICOLON,
    IF, THEN, ELSE, ELSEIF, ENDIF,
    GT, LT, GTE, LTE, DI // LOGIC SYMBOLS: gt, lt, gte, lte, di
};

struct Token {
    TokenType type;
    std::string text;
};

// Lexer with logic symbols
class Lexer {
    std::string text;
    size_t pos = 0;
    std::istringstream in;
    char curr;
    void next_char() { curr = in.get(); }
public:
    explicit Lexer(const std::string& s) : text(s), pos(0), in(s), curr(0) { next_char(); }
    Token next() {
        while (pos < text.size() && std::isspace(text[pos])) ++pos;
        if (pos >= text.size()) return {END, ""};

        if (text[pos] == ';') { ++pos; return {SEMICOLON, ";"}; }
        // numbers
        if (std::isdigit(text[pos]) || (text[pos] == '.' && pos + 1 < text.size() && std::isdigit(text[pos + 1]))) {
            size_t start = pos;
            bool has_dot = false;
            while (pos < text.size() && (std::isdigit(text[pos]) || text[pos] == '.')) {
                if (text[pos] == '.') {
                    if (has_dot) break;
                    has_dot = true;
                }
                ++pos;
            }
            return {NUM, text.substr(start, pos - start)};
        }
        // identifiers, keywords, and logic symbols
        if (std::isalpha(text[pos]) || text[pos] == '_') {
            size_t start = pos;
            while (pos < text.size() && (std::isalnum(text[pos]) || text[pos] == '_')) ++pos;
            std::string word = text.substr(start, pos - start);
            if (word == "what") return {WHAT, ""};
            if (word == "int" || word == "float" || word == "double") return {TYPE, word};
            if (word == "if") return {IF, ""};
            if (word == "then") return {THEN, ""};
            if (word == "else") return {ELSE, ""};
            if (word == "elseif") return {ELSEIF, ""};
            if (word == "endif") return {ENDIF, ""};
            // logic symbols
            if (word == "gt") return {GT, "gt"};
            if (word == "lt") return {LT, "lt"};
            if (word == "gte") return {GTE, "gte"};
            if (word == "lte") return {LTE, "lte"};
            if (word == "di") return {DI, "di"};
            return {ID, word};
        }
        if (text[pos] == ':') { ++pos; return {COLON, ":"}; }
        if (text[pos] == '=') {
            if (pos+1 < text.size() && text[pos+1] == '=') {
                pos += 2;
                return {DI, "=="}; // Not used in this grammar, but left for extensibility
            }
            ++pos; return {ASSIGN, "="};
        }
        if (text[pos] == '+') { ++pos; return {PLUS, "+"}; }
        if (text[pos] == '-') { ++pos; return {MINUS, "-"}; }
        if (text[pos] == '*') { ++pos; return {MUL, "*"}; }
        if (text[pos] == '/') { ++pos; return {DIV, "/"}; }
        if (text[pos] == '%') { ++pos; return {MOD, "%"}; }
        if (text.substr(pos,2) == "&&") { pos += 2; return {AND, "&&"}; }
        if (text.substr(pos,2) == "||") { pos += 2; return {OR, "||"}; }
        if (text[pos] == '(') { ++pos; return {LPAREN, "("}; }
        if (text[pos] == ')') { ++pos; return {RPAREN, ")"}; }
        // classic comparison symbols for completeness
        if (text[pos] == '>') {
            if (pos+1 < text.size() && text[pos+1] == '=') { pos+=2; return {GTE, ">="}; }
            ++pos; return {GT, ">"}; 
        }
        if (text[pos] == '<') {
            if (pos+1 < text.size() && text[pos+1] == '=') { pos+=2; return {LTE, "<="}; }
            ++pos; return {LT, "<"};
        }
        if (text[pos] == '!') {
            if (pos+1 < text.size() && text[pos+1] == '=') { pos+=2; return {DI, "!="}; }
        }
        throw std::runtime_error(std::string("Invalid character '") + text[pos] + "'");
    }
};

// Logic operator AST node
struct LogicOp : Expr {
    TokenType op;
    std::unique_ptr<Expr> left, right;
    LogicOp(TokenType op, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
        : op(op), left(std::move(l)), right(std::move(r)) {}
    VoltType getType(Context&) const override { return VoltType::INT; }
    std::string toLLVMIR(Context&, std::string&) const override {
        return "; logic op not yet implemented in IR";
    }
};

class ParserImpl {
    Lexer lex;
    Token curr;
    VoltType default_type = VoltType::INT;
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
        if (curr.type == IF) return if_statement();
        if (curr.type == WHAT) {
            next();
            if (curr.type != ID) throw std::runtime_error("Expected variable name");
            std::string var = curr.text;
            next();
            if (curr.type != COLON) throw std::runtime_error("Expected ':' after variable name");
            next();
            VoltType vtype = VoltType::INT;
            if (curr.type == TYPE) {
                if (curr.text == "int") vtype = VoltType::INT;
                else if (curr.text == "float") vtype = VoltType::FLOAT;
                else if (curr.text == "double") vtype = VoltType::DOUBLE;
                else throw std::runtime_error("Unknown type: " + curr.text);
            } else throw std::runtime_error("Expected type after ':'");
            next();
            if (curr.type != ASSIGN) throw std::runtime_error("Expected '=' after type");
            next();
            auto value = expr(vtype);
            return std::make_unique<VarDeclaration>(var, vtype, std::move(value));
        }
        return expr(default_type);
    }

    std::unique_ptr<Expr> if_statement() {
        next();
        auto if_cond = expr(VoltType::INT);
        if (curr.type != THEN) throw std::runtime_error("Expected 'then' after if condition");
        next();
        if (curr.type != SEMICOLON) throw std::runtime_error("Expected ';' after then");
        next();
        auto if_branch = parse_block_until({ELSEIF, ELSE, ENDIF});
        std::vector<std::unique_ptr<Expr>> stmts = std::move(if_branch);
        IfStatement::Branch main_branch{std::move(if_cond), std::move(stmts)};
        std::vector<IfStatement::Branch> branches;
        branches.push_back(std::move(main_branch));
        while (curr.type == ELSEIF) {
            next();
            auto elseif_cond = expr(VoltType::INT);
            if (curr.type != THEN) throw std::runtime_error("Expected 'then' after elseif condition");
            next();
            if (curr.type != SEMICOLON) throw std::runtime_error("Expected ';' after then");
            next();
            auto elseif_stmts = parse_block_until({ELSEIF, ELSE, ENDIF});
            branches.emplace_back(IfStatement::Branch{std::move(elseif_cond), std::move(elseif_stmts)});
        }
        if (curr.type == ELSE) {
            next();
            if (curr.type != SEMICOLON) throw std::runtime_error("Expected ';' after else");
            next();
            auto else_stmts = parse_block_until({ENDIF});
            branches.emplace_back(IfStatement::Branch{nullptr, std::move(else_stmts)});
        }
        if (curr.type != ENDIF) throw std::runtime_error("Expected 'endif' to close if statement");
        next();
        if (curr.type != SEMICOLON) throw std::runtime_error("Expected ';' after endif");
        next();
        auto ifs = std::make_unique<IfStatement>();
        ifs->branches = std::move(branches);
        return ifs;
    }

    std::vector<std::unique_ptr<Expr>> parse_block_until(std::initializer_list<TokenType> stopTokens) {
        std::vector<std::unique_ptr<Expr>> statements;
        while (true) {
            for (auto t : stopTokens) if (curr.type == t) return statements;
            // Allow empty statements (just a semicolon)
            if (curr.type == SEMICOLON) {
                next();
                continue;
            }
            auto stmt = statement();
            if (curr.type != SEMICOLON) throw std::runtime_error("Expected ';' at end of statement in block");
            next();
            statements.push_back(std::move(stmt));
        }
    }

    // Logic expression handling
    std::unique_ptr<Expr> expr(VoltType vtype) { return logic_expr(vtype); }

    std::unique_ptr<Expr> logic_expr(VoltType vtype) {
        auto node = or_expr(vtype);
        while (is_logic_op(curr.type)) {
            TokenType op = curr.type;
            next();
            node = std::make_unique<LogicOp>(op, std::move(node), or_expr(vtype));
        }
        return node;
    }
    bool is_logic_op(TokenType t) {
        return t == GT || t == LT || t == GTE || t == LTE || t == DI;
    }
    std::unique_ptr<Expr> or_expr(VoltType vtype) {
        auto node = and_expr(vtype);
        while (curr.type == OR) {
            next();
            node = std::make_unique<BinOp>(12, std::move(node), and_expr(vtype));
        }
        return node;
    }
    std::unique_ptr<Expr> and_expr(VoltType vtype) {
        auto node = add_expr(vtype);
        while (curr.type == AND) {
            next();
            node = std::make_unique<BinOp>(11, std::move(node), add_expr(vtype));
        }
        return node;
    }
    std::unique_ptr<Expr> add_expr(VoltType vtype) {
        auto node = mul_expr(vtype);
        while (curr.type == PLUS || curr.type == MINUS) {
            int op = curr.type;
            next();
            node = std::make_unique<BinOp>(op, std::move(node), mul_expr(vtype));
        }
        return node;
    }
    std::unique_ptr<Expr> mul_expr(VoltType vtype) {
        auto node = unary_expr(vtype);
        while (curr.type == MUL || curr.type == DIV || curr.type == MOD) {
            int op = curr.type;
            next();
            node = std::make_unique<BinOp>(op, std::move(node), unary_expr(vtype));
        }
        return node;
    }
    std::unique_ptr<Expr> unary_expr(VoltType vtype) {
        if (curr.type == MINUS) {
            next();
            if (vtype == VoltType::FLOAT || vtype == VoltType::DOUBLE) {
                return std::make_unique<BinOp>(
                    7,
                    std::make_unique<Number>("0.0", vtype),
                    unary_expr(vtype)
                );
            } else {
                return std::make_unique<BinOp>(
                    7,
                    std::make_unique<Number>("0", vtype),
                    unary_expr(vtype)
                );
            }
        }
        return primary(vtype);
    }
    std::unique_ptr<Expr> primary(VoltType vtype) {
        if (curr.type == NUM) {
            std::string numtxt = curr.text;
            if ((vtype == VoltType::FLOAT || vtype == VoltType::DOUBLE) && numtxt.find('.') == std::string::npos) {
                throw std::runtime_error("Errore: il valore assegnato a una variabile float/double deve contenere la virgola (es: 3.0)");
            }
            VoltType numtype = vtype;
            if (numtxt.find('.') != std::string::npos) {
                if (vtype == VoltType::DOUBLE) numtype = VoltType::DOUBLE;
                else if (vtype == VoltType::FLOAT) numtype = VoltType::FLOAT;
                else numtype = VoltType::FLOAT;
            } else {
                numtype = VoltType::INT;
            }
            next();
            return std::make_unique<Number>(numtxt, numtype);
        }
        if (curr.type == ID) {
            std::string name = curr.text;
            next();
            // If the next token is a logic operator, parse as logic expression
            if (is_logic_op(curr.type)) {
                auto left = std::make_unique<Variable>(name);
                TokenType op = curr.type;
                next();
                auto right = primary(vtype);
                return std::make_unique<LogicOp>(op, std::move(left), std::move(right));
            }
            return std::make_unique<Variable>(name);
        }
        if (curr.type == LPAREN) {
            next();
            auto node = expr(vtype);
            if (curr.type != RPAREN) throw std::runtime_error("Expected ')'");
            next();
            return node;
        }
        throw std::runtime_error("Expected number, variable or '('");
    }
} // end anonymous namespace

// Implementation of Parser::parse must be outside the anonymous namespace
std::unique_ptr<Expr> Parser::parse(const std::string& line) {
    return ParserImpl(line).parse();
}
