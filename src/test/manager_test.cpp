/**
 * @file manager_test.cpp
 * @brief Unit tests for the BDD Manager class using Google Test.
 *
 * Tests all core logic operations, structure traversal, and utility functions
 * of the ClassProject::Manager class, following TDD principles.
 */

#include "Tests.h"
#include <fstream>
#include <string>
#include <filesystem>

using namespace ClassProject;

// ======== Variable and Constants ========
TEST_F(ManagerTest, CreateVarReturnsUniqueIds) {
    BDD_ID a = manager->createVar("a");
    BDD_ID b = manager->createVar("b");
    BDD_ID a2 = manager->createVar("a");
    EXPECT_NE(a, b);
    EXPECT_EQ(a, a2);
}

TEST_F(ManagerTest, ReturnsCorrectConstants) {
    EXPECT_EQ(manager->True(), 1);
    EXPECT_EQ(manager->False(), 0);
}

TEST_F(ManagerTest, IdentifiesConstantsCorrectly) {
    EXPECT_TRUE(manager->isConstant(manager->True()));
    EXPECT_TRUE(manager->isConstant(manager->False()));
}

TEST_F(ManagerTest, IdentifiesVariablesCorrectly) {
    BDD_ID x = manager->createVar("x");
    EXPECT_TRUE(manager->isVariable(x));
    EXPECT_FALSE(manager->isVariable(manager->True()));
}

TEST_F(ManagerTest, TopVarIsCorrect) {
    BDD_ID x = manager->createVar("x");
    EXPECT_EQ(manager->topVar(x), x);
}

// ======== ITE, CoFactor and Logic Operations ========
TEST_F(ManagerTest, IteShortCircuits) {
    BDD_ID x = manager->createVar("x");
    BDD_ID t = manager->createVar("t");
    BDD_ID e = manager->createVar("e");
    EXPECT_EQ(manager->ite(manager->True(), t, e), t);
    EXPECT_EQ(manager->ite(manager->False(), t, e), e);
    EXPECT_EQ(manager->ite(x, t, t), t);
}

TEST_F(ManagerTest, CoFactorFunctionsCorrectly) {
    BDD_ID x = manager->createVar("x");
    EXPECT_EQ(manager->coFactorTrue(x, x), manager->True());
    EXPECT_EQ(manager->coFactorFalse(x, x), manager->False());
}

TEST_F(ManagerTest, LogicGatesWork) {
    BDD_ID a = manager->createVar("a");
    BDD_ID b = manager->createVar("b");
    EXPECT_EQ(manager->neg(manager->True()), manager->False());
    EXPECT_EQ(manager->and2(manager->True(), b), b);
    EXPECT_EQ(manager->or2(manager->False(), b), b);
    EXPECT_EQ(manager->xor2(b, manager->False()), b);
    EXPECT_EQ(manager->nand2(a, b), manager->neg(manager->and2(a, b)));
    EXPECT_EQ(manager->nor2(a, b), manager->neg(manager->or2(a, b)));
    EXPECT_EQ(manager->xnor2(a, b), manager->neg(manager->xor2(a, b)));
}

// ======== Structure and Utilities ========
TEST_F(ManagerTest, GetTopVarNameReturnsLabel) {
    BDD_ID x = manager->createVar("x");
    EXPECT_EQ(manager->getTopVarName(x), "x");
}

TEST_F(ManagerTest, VisualizeBDDCreatesFile) {
    BDD_ID x = manager->createVar("x");
    BDD_ID y = manager->createVar("y");
    BDD_ID f = manager->and2(x, y);
    std::string filename = "test_output.dot";
    manager->visualizeBDD(filename, f);
    std::ifstream in(filename);
    ASSERT_TRUE(in.good());
    in.close();
    std::remove(filename.c_str());
}

TEST_F(ManagerTest, FindNodesCollectsReachableNodes) {
    BDD_ID x = manager->createVar("x");
    BDD_ID y = manager->createVar("y");
    BDD_ID expr = manager->and2(x, y);

    std::set<BDD_ID> nodes;
    manager->findNodes(expr, nodes);

    EXPECT_GE(nodes.size(), 4);
    EXPECT_TRUE(nodes.count(expr));
    EXPECT_TRUE(nodes.count(y));
    EXPECT_TRUE(nodes.count(manager->True()));
    EXPECT_TRUE(nodes.count(manager->False()));
}

TEST_F(ManagerTest, FindVarsFindsAllVariables) {
    BDD_ID x = manager->createVar("x");
    BDD_ID y = manager->createVar("y");
    BDD_ID expr = manager->or2(x, y);

    std::set<BDD_ID> vars;
    manager->findVars(expr, vars);

    EXPECT_TRUE(vars.count(y));
    EXPECT_GE(vars.size(), 1);
}

TEST_F(ManagerTest, UniqueTableSizeTracksNodeCount) {
    size_t initialSize = manager->uniqueTableSize();
    BDD_ID x = manager->createVar("x");
    BDD_ID y = manager->createVar("y");
    BDD_ID z = manager->and2(x, y);

    size_t newSize = manager->uniqueTableSize();
    EXPECT_GT(newSize, initialSize);
}

TEST_F(ManagerTest, VisualizeBDDFunctionExample) { // (a+b)(c+d)
    BDD_ID a = manager->createVar("a");
    BDD_ID b = manager->createVar("b");
    BDD_ID c = manager->createVar("c");
    BDD_ID d = manager->createVar("d");

    BDD_ID ab = manager->or2(a, b);
    BDD_ID cd = manager->or2(c, d);
    BDD_ID f = manager->and2(ab, cd);

    std::string filename = "f_expr.dot";
    manager->visualizeBDD(filename, f);

    auto abs_path = std::filesystem::absolute(filename);
    std::ifstream infile(filename);
    ASSERT_TRUE(infile.good());

    std::cout << "DOT file created: " << abs_path << std::endl;
    std::cout << "To view it, run:\n";
    std::cout << "dot -Tpng " << abs_path << " -o f_expr.png && xdg-open f_expr.png" << std::endl;
}

struct FreshManagerTest : testing::Test {
    ClassProject::Manager manager;
};

TEST_F(FreshManagerTest, UniqueTableMatches) {
    // Expect only False and True initially
    size_t initSize = manager.uniqueTableSize();
    EXPECT_EQ(initSize, 2);

    // Add 4 variables
    auto a = manager.createVar("a");
    auto b = manager.createVar("b");
    auto c = manager.createVar("c");
    auto d = manager.createVar("d");
    EXPECT_EQ(manager.uniqueTableSize(), 6); // 4 variables + 2 constants

    // a + b
    auto ab = manager.or2(a, b);
    EXPECT_GE(manager.uniqueTableSize(), 7);

    // c * d
    auto cd = manager.and2(c, d);
    EXPECT_GE(manager.uniqueTableSize(), 8);

    // (a + b) * (c * d)
    auto f = manager.and2(ab, cd);
    EXPECT_GE(manager.uniqueTableSize(), 10);

    size_t finalSize = manager.uniqueTableSize();
    EXPECT_EQ(finalSize, 10); // 2 constants + 4 vars + 4 expressions
}

struct FreshManagerTest2 : testing::Test {
    ClassProject::Manager manager;
};

TEST(FreshManagerTest2, InspectReachableNodes) {
    ClassProject::Manager manager;

    BDD_ID a = manager.createVar("a");
    BDD_ID b = manager.createVar("b");
    BDD_ID c = manager.createVar("c");
    BDD_ID d = manager.createVar("d");

    BDD_ID ab = manager.or2(a, b);
    BDD_ID cd = manager.and2(c, d);
    BDD_ID f = manager.and2(ab, cd);

    std::set<BDD_ID> nodes;
    manager.findNodes(f, nodes);

    std::cout << "\n--- Reachable Nodes from f ---" << std::endl;
    for (BDD_ID id : nodes) {
        std::string label = manager.getTopVarName(id);
        std::cout << "Node ID: " << id
                  << " | topVar: " << manager.topVar(id)
                  << " (" << label << ")"
                  << " | isConst: " << manager.isConstant(id)
                  << " | isVar: " << manager.isVariable(id)
                  << std::endl;
    }
    std::cout << "Total reachable nodes: " << nodes.size() << std::endl;

    // Sanity check
    EXPECT_GE(nodes.size(), 6);  // 4 vars + 2 ops + 2 constants (some overlap)
}


TEST_F(ManagerTest, IteUsesComputedTableForCaching) {
    BDD_ID a = manager->createVar("a");
    BDD_ID b = manager->createVar("b");
    BDD_ID ite1 = manager->ite(a, b, manager->False());
    BDD_ID ite2 = manager->ite(a, b, manager->False());
    EXPECT_EQ(ite1, ite2);  // Cached result should return same node
}

TEST_F(ManagerTest, AddNodeAvoidsRedundantNodes) {
    BDD_ID a = manager->createVar("a");
    BDD_ID hi = manager->True();
    BDD_ID lo = manager->False();

    BDD_ID node1 = manager->and2(a, manager->True());  // Should build (a, T, F)
    BDD_ID node2 = manager->and2(a, manager->True());  // Should reuse same structure
    EXPECT_EQ(node1, node2);
}

TEST_F(ManagerTest, CoFactorTrueAndFalsePreserveSemantics) {
    BDD_ID a = manager->createVar("a");
    BDD_ID b = manager->createVar("b");
    BDD_ID expr = manager->or2(a, b);

    BDD_ID coT = manager->coFactorTrue(expr, a);   // Should be b
    BDD_ID coF = manager->coFactorFalse(expr, a);  // Should be b

    EXPECT_EQ(coT, manager->True());
    EXPECT_EQ(coF, b);
}

