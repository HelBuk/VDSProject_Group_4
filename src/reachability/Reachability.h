#ifndef VDSPROJECT_REACHABILITY_H
#define VDSPROJECT_REACHABILITY_H

#include "ReachabilityInterface.h"

namespace ClassProject {

    class Reachability : public ReachabilityInterface {
    public:
        Reachability(unsigned int stateSize, unsigned int inputSize = 0);
        const std::vector<BDD_ID> &getStates() const override;
        const std::vector<BDD_ID> &getInputs() const override;
        bool isReachable(const std::vector<bool> &stateVector) override;
        int stateDistance(const std::vector<bool> &stateVector) override;
        void setTransitionFunctions(const std::vector<BDD_ID> &transitionFunctions) override;
        void setInitState(const std::vector<bool> &stateVector) override;

    private:
        std::vector<BDD_ID> stateVars;
        std::vector<BDD_ID> inputVars;
        std::vector<BDD_ID> nextStateVars;
        std::vector<BDD_ID> transitions;
        BDD_ID reachableSet;

        bool reachableComputed = false;
        std::map<std::vector<bool>, int> distanceMap;

        BDD_ID buildTransitionRelation();
        BDD_ID buildCharacteristic(const std::vector<bool> &assignments, const std::vector<BDD_ID> &vars);
        BDD_ID existentialQuantify(BDD_ID f, const std::vector<BDD_ID> &vars);
        void computeReachableSet();
    };

}

#endif
