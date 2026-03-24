#include "Evalvisitor.h"
#include "Python3Parser.h"
#include <sstream>
#include <iomanip>
#include <vector>

using namespace antlr4;

// Helper functions for arithmetic operations
std::any addValues(const std::any& a, const std::any& b) {
    // Try int + int
    try {
        long long a_val = std::any_cast<long long>(a);
        long long b_val = std::any_cast<long long>(b);
        return a_val + b_val;
    } catch (...) {
        // Not both ints
    }

    // Try double + double or double + int or int + double
    try {
        double a_val, b_val;

        try {
            a_val = std::any_cast<long long>(a);
        } catch (...) {
            a_val = std::any_cast<double>(a);
        }

        try {
            b_val = std::any_cast<long long>(b);
        } catch (...) {
            b_val = std::any_cast<double>(b);
        }

        return a_val + b_val;
    } catch (...) {
        // Not numeric types
    }

    // Try string concatenation
    try {
        std::string a_str = std::any_cast<std::string>(a);
        std::string b_str = std::any_cast<std::string>(b);
        return a_str + b_str;
    } catch (...) {
        // Not strings
    }

    return nullptr;
}

std::any subtractValues(const std::any& a, const std::any& b) {
    // Try int - int
    try {
        long long a_val = std::any_cast<long long>(a);
        long long b_val = std::any_cast<long long>(b);
        return a_val - b_val;
    } catch (...) {
        // Not both ints
    }

    // Try double - double or double - int or int - double
    try {
        double a_val, b_val;

        try {
            a_val = std::any_cast<long long>(a);
        } catch (...) {
            a_val = std::any_cast<double>(a);
        }

        try {
            b_val = std::any_cast<long long>(b);
        } catch (...) {
            b_val = std::any_cast<double>(b);
        }

        return a_val - b_val;
    } catch (...) {
        // Not numeric types
    }

    return nullptr;
}

std::any multiplyValues(const std::any& a, const std::any& b) {
    // Try int * int
    try {
        long long a_val = std::any_cast<long long>(a);
        long long b_val = std::any_cast<long long>(b);
        return a_val * b_val;
    } catch (...) {
        // Not both ints
    }

    // Try double * double or double * int or int * double
    try {
        double a_val, b_val;

        try {
            a_val = std::any_cast<long long>(a);
        } catch (...) {
            a_val = std::any_cast<double>(a);
        }

        try {
            b_val = std::any_cast<long long>(b);
        } catch (...) {
            b_val = std::any_cast<double>(b);
        }

        return a_val * b_val;
    } catch (...) {
        // Not numeric types
    }

    return nullptr;
}

// Helper function to convert any to string for printing
std::string anyToString(const std::any& value) {
    if (!value.has_value()) {
        return "None";
    } else if (value.type() == typeid(std::string)) {
        return std::any_cast<std::string>(value);
    } else if (value.type() == typeid(long long)) {
        return std::to_string(std::any_cast<long long>(value));
    } else if (value.type() == typeid(double)) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(6) << std::any_cast<double>(value);
        std::string s = ss.str();
        // Remove trailing zeros
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (s.back() == '.') {
            s.push_back('0');
        }
        return s;
    } else if (value.type() == typeid(bool)) {
        return std::any_cast<bool>(value) ? "True" : "False";
    } else if (value.type() == typeid(std::nullptr_t)) {
        return "None";
    } else {
        return "<?>";
    }
}

std::any EvalVisitor::visitFile_input(Python3Parser::File_inputContext *ctx) {
    // Visit all statements in the file
    return visitChildren(ctx);
}

std::any EvalVisitor::visitStmt(Python3Parser::StmtContext *ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) {
    // Handle assignment: testlist (ASSIGN testlist)*
    auto assigns = ctx->ASSIGN();
    if (!assigns.empty()) {
        // This is an assignment statement
        auto testlists = ctx->testlist();
        if (testlists.size() >= 2) {
            // Simple case: single variable assignment like x = 5
            // Get the value from the right side
            auto value = visit(testlists.back());

            // The left side should be a testlist containing a test
            auto leftTestlist = testlists[0];
            auto leftTests = leftTestlist->test();
            if (!leftTests.empty()) {
                // For simple variable assignment, the test should be visitable
                // and will return the variable name when we visit it
                // Actually, we need to extract the variable name from the parse tree
                auto leftTest = leftTests[0];
                // Try to get the variable name by traversing the parse tree
                // A simple variable name would be: test -> or_test -> and_test -> not_test -> comparison -> arith_expr -> term -> factor -> atom_expr -> atom -> NAME
                // Let's write a helper or traverse
                std::string varName = extractVariableName(leftTest);
                if (!varName.empty()) {
                    variables[varName] = value;
                }
            }
        }
        return nullptr;
    }

    // Not an assignment, just evaluate the expression
    if (!ctx->testlist().empty()) {
        return visit(ctx->testlist(0));
    }

    return visitChildren(ctx);
}

// Helper function to extract variable name from a test context
std::string EvalVisitor::extractVariableName(Python3Parser::TestContext* test) {
    if (!test) return "";

    auto or_test = test->or_test();
    if (!or_test) return "";

    auto and_tests = or_test->and_test();
    if (and_tests.empty()) return "";

    auto and_test = and_tests[0];
    auto not_tests = and_test->not_test();
    if (not_tests.empty()) return "";

    auto not_test = not_tests[0];
    auto comparison = not_test->comparison();
    if (!comparison) return "";

    auto arith_exprs = comparison->arith_expr();
    if (arith_exprs.empty()) return "";

    auto arith_expr = arith_exprs[0];
    auto terms = arith_expr->term();
    if (terms.empty()) return "";

    auto term = terms[0];
    auto factors = term->factor();
    if (factors.empty()) return "";

    auto factor = factors[0];
    auto atom_expr = factor->atom_expr();
    if (!atom_expr) return "";

    auto atom = atom_expr->atom();
    if (!atom) return "";

    if (atom->NAME()) {
        return atom->NAME()->getText();
    }

    return "";
}

std::any EvalVisitor::visitAtom(Python3Parser::AtomContext *ctx) {
    if (ctx->NAME()) {
        // Variable reference
        std::string varName = ctx->NAME()->getText();
        auto it = variables.find(varName);
        if (it != variables.end()) {
            return it->second;
        }
        // Variable not found
        return nullptr;
    } else if (ctx->NUMBER()) {
        std::string numStr = ctx->NUMBER()->getText();
        // Check if it's a float
        if (numStr.find('.') != std::string::npos) {
            return std::stod(numStr);
        } else {
            // Integer
            try {
                return std::stoll(numStr);
            } catch (...) {
                // For very large numbers, return as string for now
                return numStr;
            }
        }
    } else if (!ctx->STRING().empty()) {
        // String literal - STRING() returns a vector
        std::string str = ctx->STRING(0)->getText();
        // Remove quotes
        if (str.length() >= 2) {
            str = str.substr(1, str.length() - 2);
        }
        return str;
    } else if (ctx->TRUE()) {
        return true;
    } else if (ctx->FALSE()) {
        return false;
    } else if (ctx->NONE()) {
        return nullptr;
    }

    return visitChildren(ctx);
}

std::any EvalVisitor::visitTest(Python3Parser::TestContext *ctx) {
    // For simple expressions, just visit the first child
    if (ctx->or_test()) {
        return visit(ctx->or_test());
    }
    return nullptr;
}

std::any EvalVisitor::visitOr_test(Python3Parser::Or_testContext *ctx) {
    if (ctx->and_test().size() == 1) {
        return visit(ctx->and_test(0));
    }
    return nullptr;
}

std::any EvalVisitor::visitAnd_test(Python3Parser::And_testContext *ctx) {
    if (ctx->not_test().size() == 1) {
        return visit(ctx->not_test(0));
    }
    return nullptr;
}

std::any EvalVisitor::visitNot_test(Python3Parser::Not_testContext *ctx) {
    if (ctx->comparison()) {
        return visit(ctx->comparison());
    }
    return nullptr;
}

std::any EvalVisitor::visitComparison(Python3Parser::ComparisonContext *ctx) {
    if (ctx->arith_expr().size() == 1) {
        return visit(ctx->arith_expr(0));
    }
    return nullptr;
}

std::any EvalVisitor::visitArith_expr(Python3Parser::Arith_exprContext *ctx) {
    // arith_expr: term (addorsub_op term)*
    auto terms = ctx->term();
    if (terms.empty()) {
        return nullptr;
    }

    // Start with first term
    auto result = visit(terms[0]);

    // Process remaining terms with operators
    auto ops = ctx->addorsub_op();
    for (size_t i = 1; i < terms.size(); i++) {
        auto term = terms[i];
        auto termValue = visit(term);

        if (i - 1 < ops.size()) {
            auto op = ops[i - 1];
            if (op->ADD()) {
                // Addition
                result = addValues(result, termValue);
            } else if (op->MINUS()) {
                // Subtraction
                result = subtractValues(result, termValue);
            }
        }
    }

    return result;
}

std::any EvalVisitor::visitTerm(Python3Parser::TermContext *ctx) {
    // term: factor (muldivmod_op factor)*
    auto factors = ctx->factor();
    if (factors.empty()) {
        return nullptr;
    }

    // Start with first factor
    auto result = visit(factors[0]);

    // Process remaining factors with operators
    auto ops = ctx->muldivmod_op();
    for (size_t i = 1; i < factors.size(); i++) {
        auto factor = factors[i];
        auto factorValue = visit(factor);

        if (i - 1 < ops.size()) {
            auto op = ops[i - 1];
            if (op->STAR()) {
                // Multiplication
                result = multiplyValues(result, factorValue);
            } else if (op->DIV() || op->IDIV() || op->MOD()) {
                // Division, integer division, modulo - not implemented yet
                // For now, just return the first value
            }
        }
    }

    return result;
}

std::any EvalVisitor::visitFactor(Python3Parser::FactorContext *ctx) {
    if (ctx->atom_expr()) {
        return visit(ctx->atom_expr());
    }
    return nullptr;
}

std::any EvalVisitor::visitAtom_expr(Python3Parser::Atom_exprContext *ctx) {
    if (ctx->atom()) {
        auto atom = ctx->atom();

        // Check if there's a trailer (function call)
        if (ctx->trailer()) {
            auto trailer = ctx->trailer();
            if (trailer->OPEN_PAREN()) {
                // Function call - check if atom is a NAME
                if (atom->NAME()) {
                    std::string funcName = atom->NAME()->getText();
                    if (funcName == "print") {
                        // Handle print function
                        if (trailer->arglist()) {
                            auto args = trailer->arglist()->argument();
                            for (size_t i = 0; i < args.size(); i++) {
                                auto arg = args[i];
                                auto tests = arg->test();
                                if (!tests.empty()) {
                                    auto value = visit(tests[0]);
                                    std::cout << anyToString(value);
                                    if (i != args.size() - 1) {
                                        std::cout << " ";
                                    }
                                }
                            }
                        }
                        std::cout << std::endl;
                        return nullptr;
                    }
                }
            }
        }

        // Not a function call or not print, visit the atom normally
        return visit(atom);
    }
    return nullptr;
}

std::any EvalVisitor::visitTrailer(Python3Parser::TrailerContext *ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::getValue(const std::any& value) {
    return value;
}

void EvalVisitor::printValue(const std::any& value) {
    std::cout << anyToString(value);
}