//
// Created by tobias on 21.11.16.
//

#ifndef VDSPROJECT_TESTS_H
#define VDSPROJECT_TESTS_H

#include <gtest/gtest.h>
#include "../Manager.h"

struct ManagerTest : testing::Test {
    std::unique_ptr<ClassProject::Manager> manager = std::make_unique<ClassProject::Manager>();

    ClassProject::BDD_ID false_id = manager->False();
    ClassProject::BDD_ID true_id = manager->True();

    ClassProject::BDD_ID a_id = manager->createVar("a");
    ClassProject::BDD_ID b_id = manager->createVar("b");
    ClassProject::BDD_ID c_id = manager->createVar("c");
    ClassProject::BDD_ID d_id = manager->createVar("d");

    ClassProject::BDD_ID neg_a_id = manager->neg(a_id);
    ClassProject::BDD_ID neg_b_id = manager->neg(b_id);

    ClassProject::BDD_ID a_and_b_id = manager->and2(a_id, b_id);
    ClassProject::BDD_ID a_or_b_id = manager->or2(a_id, b_id);
    ClassProject::BDD_ID a_xor_b_id = manager->xor2(a_id, b_id);
    ClassProject::BDD_ID a_nand_b_id = manager->nand2(a_id, b_id);
    ClassProject::BDD_ID a_nor_b_id = manager->nor2(a_id, b_id);
    ClassProject::BDD_ID a_xnor_b_id = manager->xnor2(a_id, b_id);

    ClassProject::BDD_ID c_or_d_id = manager->or2(c_id, d_id);

    // f1 = a*b+c+d
    ClassProject::BDD_ID f1_id = manager->or2(a_and_b_id, c_or_d_id);
};

TEST_F(ManagerTest, CoFactorTrueTest) /* NOLINT */
{
    EXPECT_EQ(manager->coFactorTrue(false_id), false_id);
    EXPECT_EQ(manager->coFactorTrue(true_id), true_id);
    EXPECT_EQ(manager->coFactorTrue(a_id), true_id);
    EXPECT_EQ(manager->coFactorTrue(b_id), true_id);
    EXPECT_EQ(manager->coFactorTrue(a_and_b_id), b_id);
    EXPECT_EQ(manager->coFactorTrue(c_or_d_id), true_id);
    EXPECT_EQ(manager->coFactorTrue(f1_id), manager->or2(b_id, c_or_d_id));

    EXPECT_EQ(manager->coFactorTrue(false_id, false_id), false_id);
    EXPECT_EQ(manager->coFactorTrue(false_id, true_id), false_id);
    EXPECT_EQ(manager->coFactorTrue(false_id, a_id), false_id);
    EXPECT_EQ(manager->coFactorTrue(false_id, b_id), false_id);

    EXPECT_EQ(manager->coFactorTrue(true_id, false_id), true_id);
    EXPECT_EQ(manager->coFactorTrue(true_id, true_id), true_id);
    EXPECT_EQ(manager->coFactorTrue(true_id, a_id), true_id);
    EXPECT_EQ(manager->coFactorTrue(true_id, b_id), true_id);

    EXPECT_EQ(manager->coFactorTrue(a_id, false_id), a_id);
    EXPECT_EQ(manager->coFactorTrue(a_id, true_id), a_id);
    EXPECT_EQ(manager->coFactorTrue(a_id, a_id), true_id);
    EXPECT_EQ(manager->coFactorTrue(a_id, b_id), a_id);

    EXPECT_EQ(manager->coFactorTrue(a_and_b_id, false_id), a_and_b_id);
    EXPECT_EQ(manager->coFactorTrue(a_and_b_id, true_id), a_and_b_id);
    EXPECT_EQ(manager->coFactorTrue(a_and_b_id, a_id), b_id);
    EXPECT_EQ(manager->coFactorTrue(a_and_b_id, b_id), a_id);
    EXPECT_EQ(manager->coFactorTrue(a_and_b_id, c_id), a_and_b_id);
    EXPECT_EQ(manager->coFactorTrue(a_and_b_id, a_and_b_id), a_and_b_id);

    EXPECT_EQ(manager->coFactorTrue(c_or_d_id, c_id), true_id);
    EXPECT_EQ(manager->coFactorTrue(c_or_d_id, d_id), true_id);

    EXPECT_EQ(manager->coFactorTrue(f1_id, a_id), manager->or2(b_id, c_or_d_id));
    EXPECT_EQ(manager->coFactorTrue(f1_id, b_id), manager->or2(a_id, c_or_d_id));
    EXPECT_EQ(manager->coFactorTrue(f1_id, c_id), true_id);
    EXPECT_EQ(manager->coFactorTrue(f1_id, d_id), true_id);
}

TEST_F(ManagerTest, CoFactorFalseTest) /* NOLINT */
{

    EXPECT_EQ(manager->coFactorFalse(false_id), false_id);
    EXPECT_EQ(manager->coFactorFalse(true_id), true_id);
    EXPECT_EQ(manager->coFactorFalse(a_id), false_id);
    EXPECT_EQ(manager->coFactorFalse(b_id), false_id);
    EXPECT_EQ(manager->coFactorFalse(a_and_b_id), false_id);
    EXPECT_EQ(manager->coFactorFalse(c_or_d_id), d_id);
    EXPECT_EQ(manager->coFactorFalse(f1_id), c_or_d_id);

    EXPECT_EQ(manager->coFactorFalse(false_id, false_id), false_id);
    EXPECT_EQ(manager->coFactorFalse(false_id, true_id), false_id);
    EXPECT_EQ(manager->coFactorFalse(false_id, a_id), false_id);
    EXPECT_EQ(manager->coFactorFalse(false_id, b_id), false_id);

    EXPECT_EQ(manager->coFactorFalse(true_id, false_id), true_id);
    EXPECT_EQ(manager->coFactorFalse(true_id, true_id), true_id);
    EXPECT_EQ(manager->coFactorFalse(true_id, a_id), true_id);
    EXPECT_EQ(manager->coFactorFalse(true_id, b_id), true_id);

    EXPECT_EQ(manager->coFactorFalse(a_id, false_id), a_id);
    EXPECT_EQ(manager->coFactorFalse(a_id, true_id), a_id);
    EXPECT_EQ(manager->coFactorFalse(a_id, a_id), false_id);
    EXPECT_EQ(manager->coFactorFalse(a_id, b_id), a_id);

    EXPECT_EQ(manager->coFactorFalse(a_and_b_id, false_id), a_and_b_id);
    EXPECT_EQ(manager->coFactorFalse(a_and_b_id, true_id), a_and_b_id);
    EXPECT_EQ(manager->coFactorFalse(a_and_b_id, a_id), false_id);
    EXPECT_EQ(manager->coFactorFalse(a_and_b_id, b_id), false_id);
    EXPECT_EQ(manager->coFactorFalse(a_and_b_id, c_id), a_and_b_id);
    EXPECT_EQ(manager->coFactorFalse(a_and_b_id, a_and_b_id), a_and_b_id);

    EXPECT_EQ(manager->coFactorFalse(c_or_d_id, c_id), d_id);
    EXPECT_EQ(manager->coFactorFalse(c_or_d_id, d_id), c_id);

    EXPECT_EQ(manager->coFactorFalse(f1_id, a_id), c_or_d_id);
    EXPECT_EQ(manager->coFactorFalse(f1_id, b_id), c_or_d_id);
    EXPECT_EQ(manager->coFactorFalse(f1_id, c_id), manager->or2(a_and_b_id, d_id));
    EXPECT_EQ(manager->coFactorFalse(f1_id, d_id), manager->or2(a_and_b_id, c_id));
}

#endif