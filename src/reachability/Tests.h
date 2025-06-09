#ifndef VDSPROJECT_REACHABILITY_TESTS_H
#define VDSPROJECT_REACHABILITY_TESTS_H

#include <gtest/gtest.h>
#include "Reachability.h"

using namespace ClassProject;

struct ReachabilityTest : testing::Test {
    std::unique_ptr<ClassProject::ReachabilityInterface> fsm2;
    std::vector<BDD_ID> stateVars2;
    std::vector<BDD_ID> transitionFunctions;

    void SetUp() override {
        fsm2 = std::make_unique<ClassProject::Reachability>(2);
        stateVars2 = fsm2->getStates();
    }
};

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

    BDD_ID t0 = fsm2->xor2(stateVars2[0], stateVars2[1]);
    BDD_ID t1 = fsm2->xor2(stateVars2[1], stateVars2[2]);
    BDD_ID t2 = stateVars2[2];

    fsm2->setTransitionFunctions({t0, t1, t2});
    fsm2->setInitState({false, false, false});

    ASSERT_TRUE(fsm2->isReachable({false, false, false}));
    ASSERT_TRUE(fsm2->isReachable({true, false, false}));
    ASSERT_TRUE(fsm2->isReachable({true, true, false}));
    ASSERT_TRUE(fsm2->isReachable({false, true, false}));

    ASSERT_EQ(fsm2->stateDistance({false, false, false}), 0);
    ASSERT_EQ(fsm2->stateDistance({true, false, false}), 1);
    ASSERT_EQ(fsm2->stateDistance({true, true, false}), 2);
    ASSERT_EQ(fsm2->stateDistance({false, true, false}), 3);
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

#endif
