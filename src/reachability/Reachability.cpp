#include "Reachability.h"

namespace ClassProject {

    Reachability::Reachability(unsigned int stateSize, unsigned int inputSize)
    : ReachabilityInterface(stateSize, inputSize)
    {
        if (stateSize == 0) {
            throw std::runtime_error("stateSize must be greater than zero.");
        }

        // Create state and input variables
        for (unsigned int i = 0; i < stateSize; ++i) {
            stateVars.push_back(createVar("s" + std::to_string(i)));
            nextStateVars.push_back(createVar("s'" + std::to_string(i)));
        }
        for (unsigned int i = 0; i < inputSize; ++i) {
            inputVars.push_back(createVar("i" + std::to_string(i)));
        }

        // Default transition: identity for each state bit
        transitions.reserve(stateSize);
        for (unsigned int i = 0; i < stateSize; ++i) {
            transitions.push_back(stateVars[i]);
        }

        // Default initial state: all false
        std::vector<bool> init(stateSize, false);
        setInitState(init);
    }

    const std::vector<BDD_ID>& Reachability::getStates() const {
        return stateVars;
    }

    const std::vector<BDD_ID>& Reachability::getInputs() const {
        return inputVars;
    }

    void Reachability::setTransitionFunctions(const std::vector<BDD_ID>& transitionFunctions) {
        if (transitionFunctions.size() != stateVars.size()) {
            throw std::runtime_error("Transition function size mismatch.");
        }
        transitions = transitionFunctions;
        reachableComputed = false;
    }

    void Reachability::setInitState(const std::vector<bool>& stateVector) {
        if (stateVector.size() != stateVars.size()) {
            throw std::runtime_error("Init state size mismatch.");
        }

        reachableSet = buildCharacteristic(stateVector, stateVars);
        distanceMap.clear();
        distanceMap[stateVector] = 0;
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

        BDD_ID tau = buildTransitionRelation();
        BDD_ID cR = reachableSet; //charachteristic function

        while (true) {
            BDD_ID tmp = and2(cR, tau);
            tmp = existentialQuantify(tmp, inputVars);   // ∃x
            tmp = existentialQuantify(tmp, stateVars);   // ∃s

            // Relabel s' to s
            BDD_ID relabeled = True();
            for (size_t i = 0; i < stateVars.size(); ++i) {
                relabeled = and2(relabeled, xnor2(stateVars[i], nextStateVars[i])); // s_i = s'_i survive
            }

            BDD_ID img = and2(tmp, relabeled);
            img = existentialQuantify(img, nextStateVars); // ∃s' - If s′ ≠ s, it’s removed

            BDD_ID cR_next = or2(cR, img);
            if (cR_next == cR) break;

            cR = cR_next;
        }

        reachableSet = cR;
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

        std::set<std::vector<bool>> frontier;
        std::set<std::vector<bool>> visited;

        std::vector<bool> initState(stateVars.size(), false);
        for (const auto& kv : distanceMap) {
            initState = kv.first;
            break;
        }

        frontier.insert(initState);
        visited.insert(initState);
        int dist = 0;

        while (!frontier.empty()) {
            std::set<std::vector<bool>> nextFrontier;

            for (const auto& state : frontier) {
                if (state == stateVector) return dist;

                BDD_ID chi = buildCharacteristic(state, stateVars);
                BDD_ID tmp = and2(chi, buildTransitionRelation());

                tmp = existentialQuantify(tmp, inputVars);
                tmp = existentialQuantify(tmp, stateVars);

                size_t n = nextStateVars.size();
                for (size_t b = 0; b < (1 << n); ++b) {
                    std::vector<bool> next(n);
                    for (size_t i = 0; i < n; ++i) {
                        next[i] = (b >> i) & 1;
                    }

                    BDD_ID chi_next = buildCharacteristic(next, nextStateVars);
                    if (and2(tmp, chi_next) != False()) {
                        if (visited.insert(next).second) {
                            nextFrontier.insert(next);
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
