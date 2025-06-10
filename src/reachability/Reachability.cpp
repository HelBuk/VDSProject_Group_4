#include "Reachability.h"

namespace ClassProject {

    Reachability::Reachability(unsigned int stateSize, unsigned int inputSize)
    : ReachabilityInterface(stateSize, inputSize)
    {
        // Your original body remains unchanged
        for (unsigned int i = 0; i < stateSize; ++i) {
            stateVars.push_back(createVar("s" + std::to_string(i)));
            nextStateVars.push_back(createVar("s'" + std::to_string(i)));
        }
        for (unsigned int i = 0; i < inputSize; ++i) {
            inputVars.push_back(createVar("i" + std::to_string(i)));
        }
        transitions.resize(stateSize);
    }


    const std::vector<BDD_ID> &Reachability::getStates() const {
        return stateVars;
    }

    const std::vector<BDD_ID> &Reachability::getInputs() const {
        return inputVars;
    }

    void Reachability::setTransitionFunctions(const std::vector<BDD_ID> &transitionFunctions) {
        if (transitionFunctions.size() != stateVars.size()) {
            throw std::runtime_error("Transition function size mismatch.");
        }
        transitions = transitionFunctions;
        reachableComputed = false;
    }

    void Reachability::setInitState(const std::vector<bool> &stateVector) {
        if (stateVector.size() != stateVars.size()) {
            throw std::runtime_error("Init state size mismatch.");
        }

        reachableSet = buildCharacteristic(stateVector, stateVars);
        distanceMap.clear();
        distanceMap[stateVector] = 0;
        reachableComputed = false;
    }

    BDD_ID Reachability::buildCharacteristic(const std::vector<bool> &values, const std::vector<BDD_ID> &vars) {
        BDD_ID result = True();
        for (size_t i = 0; i < vars.size(); ++i) {
            BDD_ID term = values[i] ? vars[i] : neg(vars[i]);
            result = and2(result, term);
        }
        return result;
    }

    BDD_ID Reachability::existentialQuantify(BDD_ID f, const std::vector<BDD_ID> &vars) {
        for (BDD_ID v : vars) {
            f = or2(coFactorTrue(f, v), coFactorFalse(f, v));
        }
        return f;
    }

    BDD_ID Reachability::buildTransitionRelation() {
        BDD_ID result = True();
        for (size_t i = 0; i < transitions.size(); ++i) {
            BDD_ID δ = transitions[i];
            BDD_ID s_p = nextStateVars[i];

            BDD_ID term = or2(
                and2(s_p, δ),
                and2(neg(s_p), neg(δ))
            );

            result = and2(result, term);
        }
        return result;
    }

    void Reachability::computeReachableSet() {
        if (reachableComputed) return;

        BDD_ID τ = buildTransitionRelation();
        BDD_ID cR = reachableSet;

        bool fixpoint = false;
        int dist = 0;

        while (!fixpoint) {
            BDD_ID tmp = and2(cR, τ);

            tmp = existentialQuantify(tmp, inputVars);  // ∃x
            tmp = existentialQuantify(tmp, stateVars);  // ∃s

            // relabel s' to s
            BDD_ID relabeled = True();
            for (size_t i = 0; i < stateVars.size(); ++i) {
                BDD_ID eq = xnor2(stateVars[i], nextStateVars[i]);
                relabeled = and2(relabeled, eq);
            }

            BDD_ID img = and2(tmp, relabeled);
            img = existentialQuantify(img, nextStateVars);

            BDD_ID cR_next = or2(cR, img);

            if (cR_next == cR) {
                fixpoint = true;
            }

            cR = cR_next;
            dist++;
        }

        reachableSet = cR;
        reachableComputed = true;
    }

    bool Reachability::isReachable(const std::vector<bool> &stateVector) {
        if (stateVector.size() != stateVars.size()) {
            throw std::runtime_error("State vector size mismatch.");
        }

        computeReachableSet();
        BDD_ID test = buildCharacteristic(stateVector, stateVars);
        return and2(test, reachableSet) != False();
    }

    int Reachability::stateDistance(const std::vector<bool> &stateVector) {
        if (stateVector.size() != stateVars.size()) {
            throw std::runtime_error("State vector size mismatch.");
        }

        computeReachableSet();
        if (!isReachable(stateVector)) {
            return -1;
        }

        std::set<std::vector<bool>> frontier;
        std::set<std::vector<bool>> visited;

        std::vector<bool> initState(stateVars.size(), false);
        for (const auto &kv : distanceMap) {
            initState = kv.first; // the one initialized via setInitState
            break;
        }

        frontier.insert(initState);
        visited.insert(initState);

        int dist = 0;
        while (!frontier.empty()) {
            std::set<std::vector<bool>> nextFrontier;

            for (const auto &state : frontier) {
                if (state == stateVector) return dist;

                // 1. Build characteristic function for current state
                BDD_ID chi = buildCharacteristic(state, stateVars);

                // 2. Apply transition relation to it
                BDD_ID tmp = and2(chi, buildTransitionRelation());

                // 3. ∃x (input vars)
                tmp = existentialQuantify(tmp, inputVars);

                // 4. ∃s (current state vars)
                tmp = existentialQuantify(tmp, stateVars);

                // 5. Enumerate all possible valuations of nextStateVars
                size_t n = nextStateVars.size();
                for (size_t b = 0; b < (1 << n); ++b) {
                    std::vector<bool> next(n);
                    for (size_t i = 0; i < n; ++i) {
                        next[i] = (b >> i) & 1;
                    }

                    BDD_ID chi_next = buildCharacteristic(next, nextStateVars);
                    if (and2(tmp, chi_next) != False()) {
                        if (visited.find(next) == visited.end()) {
                            nextFrontier.insert(next);
                            visited.insert(next);
                            distanceMap[next] = dist + 1;
                        }
                    }
                }
            }

            frontier = nextFrontier;
            dist++;
        }

        return -1;
    }


}
