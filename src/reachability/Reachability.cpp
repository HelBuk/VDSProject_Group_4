#include "Reachability.h"

namespace ClassProject {

    Reachability::Reachability(unsigned int stateSize, unsigned int inputSize)
    : ReachabilityInterface(stateSize, inputSize)
    {
        if (stateSize == 0) {
            throw std::runtime_error("stateSize must be greater than zero.");
        }

        for (unsigned int i = 0; i < stateSize; ++i) {
            stateVars.push_back(createVar("s" + std::to_string(i)));
            nextStateVars.push_back(createVar("s'" + std::to_string(i)));
        }
        for (unsigned int i = 0; i < inputSize; ++i) {
            inputVars.push_back(createVar("i" + std::to_string(i)));
        }

        transitions.reserve(stateSize);
        for (unsigned int i = 0; i < stateSize; ++i) {
            transitions.push_back(stateVars[i]);
        }

        std::vector<bool> init(stateSize, false);
        setInitState(init);
    }

    const std::vector<BDD_ID>& Reachability::getStates() const {
        return stateVars;
    }

    const std::vector<BDD_ID>& Reachability::getInputs() const {
        return inputVars;
    }

    void Reachability::setTransitionFunctions(const std::vector<BDD_ID> &transitionFunctions) {
        if (transitionFunctions.size() != stateVars.size()) {
            throw std::runtime_error("Transition function size mismatch.");
        }
        BDD_ID maxValidID = lastValidID();

        for (const BDD_ID &id : transitionFunctions) {
            if (id > maxValidID) {
                throw std::runtime_error("Invalid BDD_ID provided in transition functions.");
            }
        }

        transitions = transitionFunctions;
        reachableComputed = false;
        reachableSet = False();
        state_space.clear();
        distanceMap.clear();
    }

    void Reachability::setInitState(const std::vector<bool>& stateVector) {
        if (stateVector.size() != stateVars.size()) {
            throw std::runtime_error("Init state size mismatch.");
        }

        reachableSet = buildCharacteristic(stateVector, stateVars);
        state_space.clear();
        state_space.push_back(reachableSet);
        reachableComputed = false;
    }

    BDD_ID Reachability::buildCharacteristic(const std::vector<bool>& values, const std::vector<BDD_ID>& vars) {
        BDD_ID result = True();
        for (size_t i = 0; i < vars.size(); ++i) {
            BDD_ID term = values[i] ? vars[i] : neg(vars[i]);
            result = and2(result, term);
        }
        return result;
    }

    BDD_ID Reachability::existentialQuantify(BDD_ID f, const std::vector<BDD_ID>& vars) {
        for (BDD_ID v : vars) {
            f = or2(coFactorTrue(f, v), coFactorFalse(f, v));
        }
        return f;
    }

    BDD_ID Reachability::buildTransitionRelation() {
        BDD_ID result = True();
        for (size_t i = 0; i < transitions.size(); ++i) {
            BDD_ID delta = transitions[i];
            BDD_ID s_p = nextStateVars[i];

            BDD_ID term = or2(
                and2(s_p, delta),
                and2(neg(s_p), neg(delta))
            );
            result = and2(result, term);
        }
        return result;
    }

    void Reachability::computeReachableSet() {
        if (reachableComputed) return;

        BDD_ID tau = buildTransitionRelation();          // transition relation
        BDD_ID charReachable = reachableSet;             // characteristic function

        do {
            reachableSet = charReachable;

            BDD_ID tmp = and2(charReachable, tau);
            tmp = existentialQuantify(tmp, inputVars);   // ∃x
            tmp = existentialQuantify(tmp, stateVars);   // ∃s

            for (size_t i = 0; i < stateVars.size(); ++i) {
                tmp = and2(tmp, xnor2(stateVars[i], nextStateVars[i])); // s_i = s'_i survive
            }

            BDD_ID img = existentialQuantify(tmp, nextStateVars); // ∃s' - If s′ ≠ s, it’s removed

            state_space.push_back(img);
            charReachable = or2(reachableSet, img);
        } while (charReachable != reachableSet);

        reachableSet = charReachable;
        reachableComputed = true;
    }


    bool Reachability::isReachable(const std::vector<bool>& stateVector) {
        if (stateVector.size() != stateVars.size()) {
            throw std::runtime_error("State vector size mismatch.");
        }
        computeReachableSet();
        BDD_ID test = buildCharacteristic(stateVector, stateVars);
        return and2(test, reachableSet) != False();
    }

    int Reachability::stateDistance(const std::vector<bool>& stateVector) {
        if (stateVector.size() != stateVars.size()) {
            throw std::runtime_error("State vector size mismatch.");
        }

        computeReachableSet();
        if (!isReachable(stateVector)) return -1;

        BDD_ID test = buildCharacteristic(stateVector, stateVars);
        for (size_t dist = 0; dist < state_space.size(); ++dist) {
            if (and2(state_space[dist], test) != False()) {
                return static_cast<int>(dist);
            }
        }
        return -1;
    }

}
