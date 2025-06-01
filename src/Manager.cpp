/**
* @file Manager.cpp
 * @brief Implementation of the BDD Manager class for constructing and manipulating Binary Decision Diagrams.
 *
 * This class provides methods to create BDD variables, perform logical operations, manage unique and computed tables,
 * and visualize the resulting BDD graph.
 */
#include "Manager.h"

#include <iostream>
#include <ostream>
#include <limits>
#include <fstream>
#include <functional>
#include <unordered_map>  // Added for hash-based table


namespace ClassProject {
    Manager::Manager(): currentID(2),
                        trueID(1),
                        falseID(0) {
        uniqueTable[falseID] = {falseID, falseID, falseID, falseID};
        uniqueTable[trueID] = {trueID, trueID, trueID, trueID};
    }

    const BDD_ID &Manager::True() { return trueID; }
    const BDD_ID &Manager::False() { return falseID; }

    bool Manager::isConstant(BDD_ID f) { return f == falseID || f == trueID; }

    bool Manager::isVariable(BDD_ID x) {
        return variableIDs.count(x) > 0;
    }

    BDD_ID Manager::createVar(const std::string &label) {
        if (labelToID.count(label)) return labelToID[label];
        BDD_ID id = currentID++;
        labelToID[label] = id;
        idToLabel[id] = label;
        variableIDs.insert(id);
        uniqueTable[id] = {id, id, falseID, trueID};
        uniqueHashTable[std::make_tuple(id, falseID, trueID)] = id;
        return id;
    }

    BDD_ID Manager::topVar(BDD_ID f) {
        return uniqueTable[f].topVar;
    }

    BDD_ID Manager::coFactorTrue(BDD_ID f) {
        if (isConstant(f)) return f;
        return uniqueTable[f].high;
    }

    BDD_ID Manager::coFactorTrue(BDD_ID f, BDD_ID x) {
        auto key = std::make_pair(f, x);
        if (cofactorTrueCache.count(key)) return cofactorTrueCache[key];

        BDD_ID result;
        if (isConstant(f) || topVar(f) > x) {
            result = f;
        } else if (topVar(f) == x) {
            result = uniqueTable[f].high;
        } else {
            BDD_ID high = coFactorTrue(uniqueTable[f].high, x);
            BDD_ID low = coFactorTrue(uniqueTable[f].low, x);
            result = addNode(topVar(f), high, low);
        }

        cofactorTrueCache[key] = result;
        return result;
    }

    BDD_ID Manager::coFactorFalse(BDD_ID f) {
        if (isConstant(f)) return f;
        return uniqueTable[f].low;
    }

    BDD_ID Manager::coFactorFalse(BDD_ID f, BDD_ID x) {
        auto key = std::make_pair(f, x);
        if (cofactorFalseCache.count(key)) return cofactorFalseCache[key];

        BDD_ID result;
        if (isConstant(f) || topVar(f) > x) {
            result = f;
        } else if (topVar(f) == x) {
            result = uniqueTable[f].low;
        } else {
            BDD_ID high = coFactorFalse(uniqueTable[f].high, x);
            BDD_ID low = coFactorFalse(uniqueTable[f].low, x);
            result = addNode(topVar(f), high, low);  // Replace `ite(...)` here!
        }

        cofactorFalseCache[key] = result;
        return result;
    }


    BDD_ID Manager::addNode(BDD_ID v, BDD_ID h, BDD_ID l) {
        if (h == l) return h;
        auto key = std::make_tuple(v, l, h);
        if (uniqueHashTable.count(key)) return uniqueHashTable[key];
        BDD_ID id = currentID++;
        uniqueTable[id] = {id, v, l, h};
        uniqueHashTable[key] = id;
        return id;
    }

    BDD_ID Manager::ite(BDD_ID f, BDD_ID g, BDD_ID h) {
        if (f == trueID) return g;
        if (f == falseID) return h;
        if (g == h) return g;

        auto key = std::make_tuple(f, g, h);
        if (computedTable.count(key) > 0) {
            return computedTable[key];
        }

        BDD_ID top = std::numeric_limits<BDD_ID>::max();
        if (!isConstant(f)) top = std::min(top, topVar(f));
        if (!isConstant(g)) top = std::min(top, topVar(g));
        if (!isConstant(h)) top = std::min(top, topVar(h));

        BDD_ID f1 = coFactorTrue(f, top);
        BDD_ID g1 = coFactorTrue(g, top);
        BDD_ID h1 = coFactorTrue(h, top);
        BDD_ID f0 = coFactorFalse(f, top);
        BDD_ID g0 = coFactorFalse(g, top);
        BDD_ID h0 = coFactorFalse(h, top);

        BDD_ID hi = ite(f1, g1, h1);
        BDD_ID lo = ite(f0, g0, h0);

        BDD_ID result = (hi == lo) ? hi : addNode(top, hi, lo);
        computedTable[key] = result;
        return result;
    }

    BDD_ID Manager::neg(BDD_ID a) {
        return ite(a, falseID, trueID);
    }

    BDD_ID Manager::and2(BDD_ID a, BDD_ID b) {
        return ite(a, b, falseID);
    }

    BDD_ID Manager::or2(BDD_ID a, BDD_ID b) {
        return ite(a, trueID, b);
    }

    BDD_ID Manager::xor2(BDD_ID a, BDD_ID b) {
        return ite(a, neg(b), b);
    }

    BDD_ID Manager::nand2(BDD_ID a, BDD_ID b) {
        return neg(and2(a, b));
    }

    BDD_ID Manager::nor2(BDD_ID a, BDD_ID b) {
        return neg(or2(a, b));
    }

    BDD_ID Manager::xnor2(BDD_ID a, BDD_ID b) {
        return neg(xor2(a, b));
    }

    std::string Manager::getTopVarName(const BDD_ID &id) {
        if (isConstant(id)) return std::to_string(id);
        return idToLabel.count(topVar(id)) ? idToLabel[topVar(id)] : "n" + std::to_string(topVar(id));
    }

    void Manager::visualizeBDD(std::string filepath, BDD_ID &root) {
        std::ofstream out(filepath);
        out << "digraph BDD {\n";

        std::set<BDD_ID> visited;
        std::function<void(BDD_ID)> dfs = [&](BDD_ID id) {
            if (visited.count(id)) return;
            visited.insert(id);

            if (isConstant(id)) {
                out << "n" << id << " [label=\"" << (id == trueID ? "1" : "0") << "\", shape=box];\n";
            } else {
                out << "n" << id << " [label=\"" << getTopVarName(id) << "\"];\n";
                BDD_ID h = uniqueTable[id].high;
                BDD_ID l = uniqueTable[id].low;
                out << "n" << id << " -> n" << h << " [label=\"1\"];\n";
                out << "n" << id << " -> n" << l << " [label=\"0\", style=dashed];\n";
                dfs(h);
                dfs(l);
            }
        };

        dfs(root);
        out << "}\n";
    }

    void Manager::findNodes(const BDD_ID &r, std::set<BDD_ID> &n) {
        if (n.count(r)) return;
        n.insert(r);
        if (!isConstant(r)) {
            findNodes(uniqueTable[r].high, n);
            findNodes(uniqueTable[r].low, n);
        }
    }

    void Manager::findVars(const BDD_ID &r, std::set<BDD_ID> &v) {
        std::set<BDD_ID> n;
        findNodes(r, n);
        for (auto id: n) {
            if (isConstant(id)) continue;
            BDD_ID var = topVar(id);
            if (isVariable(var)) v.insert(var);
        }
    }

    size_t Manager::uniqueTableSize() {
        return uniqueTable.size();
    }
} // namespace ClassProject