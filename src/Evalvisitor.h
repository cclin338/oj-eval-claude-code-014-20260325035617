#pragma once
#ifndef PYTHON_INTERPRETER_EVALVISITOR_H
#define PYTHON_INTERPRETER_EVALVISITOR_H

#include "Python3ParserBaseVisitor.h"
#include <any>
#include <string>
#include <unordered_map>
#include <iostream>

// Forward declarations for helper functions
std::any addValues(const std::any& a, const std::any& b);
std::any subtractValues(const std::any& a, const std::any& b);
std::any multiplyValues(const std::any& a, const std::any& b);

class EvalVisitor : public Python3ParserBaseVisitor {
private:
    std::unordered_map<std::string, std::any> variables;

public:
    // Override key methods
    std::any visitFile_input(Python3Parser::File_inputContext *ctx) override;
    std::any visitStmt(Python3Parser::StmtContext *ctx) override;
    std::any visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) override;
    std::any visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) override;
    std::any visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) override;
    std::any visitAtom(Python3Parser::AtomContext *ctx) override;
    std::any visitTest(Python3Parser::TestContext *ctx) override;
    std::any visitOr_test(Python3Parser::Or_testContext *ctx) override;
    std::any visitAnd_test(Python3Parser::And_testContext *ctx) override;
    std::any visitNot_test(Python3Parser::Not_testContext *ctx) override;
    std::any visitComparison(Python3Parser::ComparisonContext *ctx) override;
    std::any visitArith_expr(Python3Parser::Arith_exprContext *ctx) override;
    std::any visitTerm(Python3Parser::TermContext *ctx) override;
    std::any visitFactor(Python3Parser::FactorContext *ctx) override;
    std::any visitAtom_expr(Python3Parser::Atom_exprContext *ctx) override;
    std::any visitTrailer(Python3Parser::TrailerContext *ctx) override;

    // Helper methods
    std::any getValue(const std::any& value);
    void printValue(const std::any& value);

private:
    // Helper function to extract variable name
    std::string extractVariableName(Python3Parser::TestContext* test);
};


#endif//PYTHON_INTERPRETER_EVALVISITOR_H
