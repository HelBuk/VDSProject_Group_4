#ifndef VDSPROJECT_REACHABILITY_TESTS_H
#define VDSPROJECT_REACHABILITY_TESTS_H

#include <gtest/gtest.h>
#include "Reachability.h"

using namespace ClassProject;

struct ReachabilityTest : testing::Test {
    std::unique_ptr<ClassProject::ReachabilityInterface> fsm2;
    std::unique_ptr<ClassProject::ReachabilityInterface> fsm4;
    std::vector<BDD_ID> stateVars2;
    std::vector<BDD_ID> stateVars4;
    std::vector<BDD_ID> transitionFunctions;

    void SetUp() override {
        fsm2 = std::make_unique<ClassProject::Reachability>(2);
        fsm4 = std::make_unique<ClassProject::Reachability>(4);
        stateVars2 = fsm2->getStates();
        stateVars4 = fsm4->getStates();
        transitionFunctions.clear();
    }
};

TEST_F(ReachabilityTest, FourStateBitExample) { /* NOLINT */

    auto s0 = stateVars4.at(0);
    auto s1 = stateVars4.at(1);
    auto s2 = stateVars4.at(2);
    auto s3 = stateVars4.at(3);

    //s0' = s2 xor (s0 and s1)
    transitionFunctions.push_back(fsm4->xor2(s2, fsm4->and2(s0, s2)));
    //s1' = not(s3) and (s3 or (s3 nand (s0 and s1)))
    transitionFunctions.push_back(fsm4->and2(fsm4->neg(s3), fsm4->or2(s3, fsm4->nand2(s3, fsm4->and2(s0,s1)))));
    //s2' = s0 nand (s1 and s3)
    transitionFunctions.push_back(fsm4->nand2(s0, fsm4->and2(s1, s3)));
    //s3' = (s2 xor (s0 and s1)) and (not(s3) and (s3 or (s3 nand (s0 and s1))))
    transitionFunctions.push_back(fsm4->and2(fsm4->xor2(s2, fsm4->and2(s0,s1)), fsm4->and2(fsm4->neg(s3), fsm4->or2(s3, fsm4->nand2(s3, fsm4->and2(s0,s1))))));
    fsm4->setTransitionFunctions(transitionFunctions);

    fsm4->setInitState({false,false,false,true});

    ASSERT_TRUE (fsm4->isReachable({false, false, false, false}));
    ASSERT_TRUE (fsm4->isReachable({false, false, false,  true}));
    ASSERT_TRUE (fsm4->isReachable({false, false,  true, false}));
    ASSERT_FALSE(fsm4->isReachable({false, false,  true,  true}));
    ASSERT_FALSE(fsm4->isReachable({false,  true, false, false}));
    ASSERT_FALSE(fsm4->isReachable({false,  true, false,  true}));
    ASSERT_TRUE (fsm4->isReachable({false,  true,  true, false}));
    ASSERT_FALSE(fsm4->isReachable({false,  true,  true,  true}));
    ASSERT_FALSE(fsm4->isReachable({ true, false, false, false}));
    ASSERT_FALSE(fsm4->isReachable({ true, false, false,  true}));
    ASSERT_FALSE(fsm4->isReachable({ true, false,  true, false}));
    ASSERT_FALSE(fsm4->isReachable({ true, false,  true,  true}));
    ASSERT_FALSE(fsm4->isReachable({ true,  true, false, false}));
    ASSERT_FALSE(fsm4->isReachable({ true,  true, false,  true}));
    ASSERT_FALSE(fsm4->isReachable({ true,  true,  true, false}));
    ASSERT_TRUE (fsm4->isReachable({ true,  true,  true,  true}));

    ASSERT_EQ (fsm4->stateDistance({false, false, false, true}), 0);
    ASSERT_EQ (fsm4->stateDistance({false, false,  true, false}), 1);
    ASSERT_EQ (fsm4->stateDistance({ true,  true,  true,  true}), 2);
    ASSERT_EQ (fsm4->stateDistance({false, false, false, false}), 3);
    ASSERT_EQ (fsm4->stateDistance({false,  true,  true, false}), 4);
    ASSERT_EQ (fsm4->stateDistance({false, false,  true,  true}), -1);
}

TEST_F(ReachabilityTest, HowTo_Example) {
    BDD_ID s0 = stateVars2.at(0);
    BDD_ID s1 = stateVars2.at(1);

    transitionFunctions.push_back(fsm2->neg(s0)); // s0' = not(s0)
    transitionFunctions.push_back(fsm2->neg(s1)); // s1' = not(s1)
    fsm2->setTransitionFunctions(transitionFunctions);

    fsm2->setInitState({false,false});

    ASSERT_TRUE(fsm2->isReachable({false, false}));
    ASSERT_FALSE(fsm2->isReachable({false, true}));
    ASSERT_FALSE(fsm2->isReachable({true, false}));
    ASSERT_TRUE(fsm2->isReachable({true, true}));
}

TEST_F(ReachabilityTest, NotTransitionsTest) {
    BDD_ID s0 = stateVars2.at(0);
    BDD_ID s1 = stateVars2.at(1);

    transitionFunctions.push_back(fsm2->neg(s0));
    transitionFunctions.push_back(fsm2->neg(s1));
    fsm2->setTransitionFunctions(transitionFunctions);
    fsm2->setInitState({false,false});

    ASSERT_TRUE(fsm2->isReachable({false, false}));
    ASSERT_TRUE(fsm2->isReachable({true, true}));
    ASSERT_FALSE(fsm2->isReachable({false, true}));
    ASSERT_FALSE(fsm2->isReachable({true, false}));

    ASSERT_EQ(fsm2->stateDistance({false, false}), 0);
    ASSERT_EQ(fsm2->stateDistance({true, true}), 1);
    ASSERT_EQ(fsm2->stateDistance({false, false}), 0); // cycle
    ASSERT_EQ(fsm2->stateDistance({true, false}), -1);
}

TEST_F(ReachabilityTest, IdentityTransitionsTest) {
    transitionFunctions = {stateVars2[0], stateVars2[1]};
    fsm2->setTransitionFunctions(transitionFunctions);
    fsm2->setInitState({true, false});

    ASSERT_TRUE(fsm2->isReachable({true, false}));
    ASSERT_FALSE(fsm2->isReachable({false, false}));
    ASSERT_EQ(fsm2->stateDistance({true, false}), 0);
    ASSERT_EQ(fsm2->stateDistance({false, false}), -1);
}

TEST_F(ReachabilityTest, ToggleS0KeepS1Test) {
    BDD_ID s0 = stateVars2.at(0);
    BDD_ID s1 = stateVars2.at(1);
    transitionFunctions = {fsm2->neg(s0), s1};

    fsm2->setTransitionFunctions(transitionFunctions);
    fsm2->setInitState({false, true});

    ASSERT_TRUE(fsm2->isReachable({false, true}));
    ASSERT_TRUE(fsm2->isReachable({true, true}));
    ASSERT_FALSE(fsm2->isReachable({true, false}));
    ASSERT_EQ(fsm2->stateDistance({false, true}), 0);
    ASSERT_EQ(fsm2->stateDistance({true, true}), 1);
    ASSERT_EQ(fsm2->stateDistance({true, false}), -1);
}

TEST_F(ReachabilityTest, WithInputs) {
    fsm2 = std::make_unique<Reachability>(2, 1);
    stateVars2 = fsm2->getStates();
    auto inputVars = fsm2->getInputs();

    BDD_ID t0 = fsm2->xor2(stateVars2[0], inputVars[0]);
    BDD_ID t1 = stateVars2[1];
    fsm2->setTransitionFunctions({t0, t1});
    fsm2->setInitState({false, false});

    ASSERT_TRUE(fsm2->isReachable({false, false}));
    ASSERT_TRUE(fsm2->isReachable({true, false}));
    ASSERT_EQ(fsm2->stateDistance({false, false}), 0);
    ASSERT_EQ(fsm2->stateDistance({true, false}), 1);
}

TEST_F(ReachabilityTest, ThreeStatesXorChain) {
    fsm2 = std::make_unique<Reachability>(3);
    stateVars2 = fsm2->getStates();

    BDD_ID t0 = fsm2->neg(stateVars2[0]);
    BDD_ID t1 = fsm2->xor2(stateVars2[1], stateVars2[0]);
    BDD_ID t2 = fsm2->xor2(stateVars2[2], stateVars2[1]);

    fsm2->setTransitionFunctions({t0, t1, t2});
    fsm2->setInitState({false, false, false});

    ASSERT_TRUE(fsm2->isReachable({false, false, false}));
    ASSERT_TRUE(fsm2->isReachable({true, true, true}));
    ASSERT_TRUE(fsm2->isReachable({false, false, false}));
    ASSERT_EQ(fsm2->stateDistance({true, true, true}), 3);
}


TEST_F(ReachabilityTest, NotS0NotS1_PDF_Example) {
    BDD_ID s0 = stateVars2.at(0);
    BDD_ID s1 = stateVars2.at(1);

    transitionFunctions = {fsm2->neg(s0), fsm2->neg(s1)};
    fsm2->setTransitionFunctions(transitionFunctions);
    fsm2->setInitState({false, false});

    ASSERT_TRUE(fsm2->isReachable({false, false}));
    ASSERT_TRUE(fsm2->isReachable({true, true}));
    ASSERT_FALSE(fsm2->isReachable({true, false}));
    ASSERT_FALSE(fsm2->isReachable({false, true}));

    ASSERT_EQ(fsm2->stateDistance({false, false}), 0);
    ASSERT_EQ(fsm2->stateDistance({true, true}), 1);
}

TEST_F(ReachabilityTest, InvalidStateVectorSize) {
    fsm2->setTransitionFunctions({fsm2->neg(stateVars2[0]), fsm2->neg(stateVars2[1])});
    fsm2->setInitState({false, false});

    EXPECT_THROW(fsm2->isReachable({true}), std::runtime_error);
    EXPECT_THROW(fsm2->stateDistance({true}), std::runtime_error);
    EXPECT_THROW(fsm2->setInitState({true}), std::runtime_error);
}

TEST_F(ReachabilityTest, NoTransitionsSetThrows) {
    fsm2->setInitState({false, false});
    EXPECT_NO_THROW(fsm2->isReachable({false, false}));
    EXPECT_NO_THROW(fsm2->stateDistance({false, false}));
}

TEST_F(ReachabilityTest, ThrowsOnZeroStateSize) {
    EXPECT_THROW({
        Reachability fsm(0, 1);
    }, std::runtime_error);
}


#endif
