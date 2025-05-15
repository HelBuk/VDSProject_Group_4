/**
* @file manager_test.cpp
 * @brief Unit tests for the BDD Manager class using Google Test.
 *
 * Tests all core logic operations, structure traversal, and utility functions
 * of the ClassProject::Manager class, following TDD principles.
 */

#include "Tests.h"
#include <fstream> // For std::ifstream and std::remove
#include <string>
#include <filesystem>

using namespace ClassProject;

class ManagerTest : public ::testing::Test {
protected:
    Manager mgr;
};

// ======== Variable and Constants ========
TEST_F(ManagerTest, CreateVarReturnsUniqueIds) {
    BDD_ID a = mgr.createVar("a");
    BDD_ID b = mgr.createVar("b");
    BDD_ID a2 = mgr.createVar("a");
    EXPECT_NE(a, b);
    EXPECT_EQ(a, a2);
}

TEST_F(ManagerTest, ReturnsCorrectConstants) {
    EXPECT_EQ(mgr.True(), 1);
    EXPECT_EQ(mgr.False(), 0);
}

TEST_F(ManagerTest, IdentifiesConstantsCorrectly) {
    EXPECT_TRUE(mgr.isConstant(mgr.True()));
    EXPECT_TRUE(mgr.isConstant(mgr.False()));
}

TEST_F(ManagerTest, IdentifiesVariablesCorrectly) {
    BDD_ID x = mgr.createVar("x");
    EXPECT_TRUE(mgr.isVariable(x));
    EXPECT_FALSE(mgr.isVariable(mgr.True()));
}

TEST_F(ManagerTest, TopVarIsCorrect) {
    BDD_ID x = mgr.createVar("x");
    EXPECT_EQ(mgr.topVar(x), x);
}

// ======== ITE, CoFactor and Logic Operations ========
TEST_F(ManagerTest, IteShortCircuits) {
    BDD_ID x = mgr.createVar("x");
    BDD_ID t = mgr.createVar("t");
    BDD_ID e = mgr.createVar("e");
    EXPECT_EQ(mgr.ite(mgr.True(), t, e), t);
    EXPECT_EQ(mgr.ite(mgr.False(), t, e), e);
    EXPECT_EQ(mgr.ite(x, t, t), t);
}

TEST_F(ManagerTest, CoFactorFunctionsCorrectly) {
    BDD_ID x = mgr.createVar("x");
    EXPECT_EQ(mgr.coFactorTrue(x, x), mgr.True());
    EXPECT_EQ(mgr.coFactorFalse(x, x), mgr.False());
}

TEST_F(ManagerTest, LogicGatesWork) {
    BDD_ID a = mgr.createVar("a");
    BDD_ID b = mgr.createVar("b");
    EXPECT_EQ(mgr.neg(mgr.True()), mgr.False());
    EXPECT_EQ(mgr.and2(mgr.True(), b), b);
    EXPECT_EQ(mgr.or2(mgr.False(), b), b);
    EXPECT_EQ(mgr.xor2(b, mgr.False()), b);
    EXPECT_EQ(mgr.nand2(a, b), mgr.neg(mgr.and2(a, b)));
    EXPECT_EQ(mgr.nor2(a, b), mgr.neg(mgr.or2(a, b)));
    EXPECT_EQ(mgr.xnor2(a, b), mgr.neg(mgr.xor2(a, b)));
}

// ======== Structure and Utilities ========
TEST_F(ManagerTest, GetTopVarNameReturnsLabel) {
    BDD_ID x = mgr.createVar("x");
    EXPECT_EQ(mgr.getTopVarName(x), "x");
}

TEST_F(ManagerTest, VisualizeBDDCreatesFile) {
    BDD_ID x = mgr.createVar("x");
    BDD_ID y = mgr.createVar("y");
    BDD_ID f = mgr.and2(x, y);
    std::string filename = "test_output.dot";
    mgr.visualizeBDD(filename, f);
    std::ifstream in(filename);
    ASSERT_TRUE(in.good());
    in.close();
    std::remove(filename.c_str());
}

TEST_F(ManagerTest, FindNodesCollectsReachableNodes) {
    BDD_ID x = mgr.createVar("x");
    BDD_ID y = mgr.createVar("y");
    BDD_ID expr = mgr.and2(x, y);

    std::set<BDD_ID> nodes;
    mgr.findNodes(expr, nodes);

    EXPECT_GE(nodes.size(), 4);
    EXPECT_TRUE(nodes.count(expr));
    EXPECT_TRUE(nodes.count(y));
    EXPECT_TRUE(nodes.count(mgr.True()));
    EXPECT_TRUE(nodes.count(mgr.False()));
}

TEST_F(ManagerTest, FindVarsFindsAllVariables) {
    BDD_ID x = mgr.createVar("x");
    BDD_ID y = mgr.createVar("y");
    BDD_ID expr = mgr.or2(x, y);

    std::set<BDD_ID> vars;
    mgr.findVars(expr, vars);

    EXPECT_TRUE(vars.count(y));
    EXPECT_GE(vars.size(), 1);
}

TEST_F(ManagerTest, UniqueTableSizeTracksNodeCount) {
    size_t initialSize = mgr.uniqueTableSize();
    BDD_ID x = mgr.createVar("x");
    BDD_ID y = mgr.createVar("y");
    BDD_ID z = mgr.and2(x, y);

    size_t newSize = mgr.uniqueTableSize();
    EXPECT_GT(newSize, initialSize);
}

TEST_F(ManagerTest, VisualizeBDDFunctionExample) { // (a+b)(c+d)
    BDD_ID a = mgr.createVar("a");
    BDD_ID b = mgr.createVar("b");
    BDD_ID c = mgr.createVar("c");
    BDD_ID d = mgr.createVar("d");

    BDD_ID ab = mgr.or2(a, b);
    BDD_ID cd = mgr.or2(c, d);
    BDD_ID f = mgr.and2(ab, cd);

    std::string filename = "f_expr.dot";
    mgr.visualizeBDD(filename, f);

    auto abs_path = std::filesystem::absolute(filename);
    std::ifstream infile(filename); //The file exists, it was successfully opened, no I/O errors occurred
    ASSERT_TRUE(infile.good());

    std::cout << "DOT file created: " << abs_path << std::endl;
    std::cout << "To view it, run:\n";
    std::cout << "dot -Tpng " << abs_path << " -o f_expr.png && xdg-open f_expr.png" << std::endl;
}

TEST_F(ManagerTest, UniqueTableMatches) {
    size_t initSize = mgr.uniqueTableSize();  // Should be 2: {False, True}
    EXPECT_EQ(initSize, 2);

    BDD_ID a = mgr.createVar("a");  // ID 2
    BDD_ID b = mgr.createVar("b");  // ID 3
    BDD_ID c = mgr.createVar("c");  // ID 4
    BDD_ID d = mgr.createVar("d");  // ID 5
    EXPECT_EQ(mgr.uniqueTableSize(), 6); // {0,1,a,b,c,d}

    // a + b → ID 6
    BDD_ID ab = mgr.or2(a, b);
    EXPECT_GE(mgr.uniqueTableSize(), 7);

    // c * d → ID 7
    BDD_ID cd = mgr.and2(c, d);
    EXPECT_GE(mgr.uniqueTableSize(), 8);

    // (a + b) * (c * d) → ID 9
    BDD_ID f = mgr.and2(ab, cd);
    EXPECT_GE(mgr.uniqueTableSize(), 10);

    size_t finalSize = mgr.uniqueTableSize();
    EXPECT_EQ(finalSize, 10); // Should match the example if no sharing missed
}