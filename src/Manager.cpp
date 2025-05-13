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

namespace ClassProject {

    /**
     * @brief Constructor initializes the unique table with constant nodes (False and True).
     */
    Manager::Manager() : currentID(2) {
        uniqueTable[0] = {0, 0, 0, 0}; // False
        uniqueTable[1] = {1, 1, 1, 1}; // True
        falseID = 0;
        trueID = 1;
    }

    const BDD_ID &Manager::True() { return trueID; }
    const BDD_ID &Manager::False() { return falseID; }

    bool Manager::isConstant(BDD_ID f) { return f == falseID || f == trueID; }

    bool Manager::isVariable(BDD_ID x) {
        return uniqueTable.count(x) && //checking whether the key x exists in the map uniqueTable
               uniqueTable[x].high == trueID && //Not sure what is isVarible
               uniqueTable[x].low == falseID;
    }

    BDD_ID Manager::createVar(const std::string &label) {
        if (labelToID.count(label)) return labelToID[label];
        BDD_ID id = currentID++;
        labelToID[label] = id;
        idToLabel[id] = label;
        uniqueTable[id] = {id, id, falseID, trueID};
        return id;
    }

    BDD_ID Manager::topVar(BDD_ID f) {
        return uniqueTable[f].topVar;
    }

    BDD_ID Manager::coFactorTrue(BDD_ID f, BDD_ID x) {
        if (uniqueTable.count(f) == 0) {
            std::cout << "Invalid BDD_ID passed to coFactorTrue: " << f << std::endl;
            std::abort();
        }
        if (x == 0) x = topVar(f); // use top variable if not given
        return (topVar(f) == x) ? uniqueTable[f].high : f; // if x is not topVar of node f, follow high branch else return f unchanged as f doesn't depend on x
    }

    BDD_ID Manager::coFactorFalse(BDD_ID f, BDD_ID x) {
        if (uniqueTable.count(f) == 0) {
            std::cout << "Invalid BDD_ID passed to coFactorTrue: " << f << std::endl;
            std::abort();
        }
        if (x == 0) x = topVar(f);
        return (topVar(f) == x) ? uniqueTable[f].low : f; // if x is not topVar of node f, follow low branch else return f unchanged as f doesn't depend on x
    }

    BDD_ID Manager::coFactorTrue(BDD_ID f) {
        return coFactorTrue(f, 0);  // default to x = topVar(f)
    }

    BDD_ID Manager::coFactorFalse(BDD_ID f) {
        return coFactorFalse(f, 0); // default to x = topVar(f)
    }

    /**
     * @brief Adds a unique node to the table or returns existing one.
     */
    BDD_ID Manager::addNode(BDD_ID v, BDD_ID h, BDD_ID l) {
        for (const auto &[id, node] : uniqueTable)
            if (node.high == h && node.low == l && node.topVar == v) return id;
        BDD_ID id = currentID++;
        uniqueTable[id] = {id, v, l, h};
        return id;
    }

    /**
     * @brief Implements the ITE (if-then-else) operator.
     */
    BDD_ID Manager::ite(BDD_ID f, BDD_ID g, BDD_ID h) {
        if (f == trueID) return g;
        if (f == falseID) return h;
        if (g == h) return g;
        auto key = std::make_tuple(f, g, h);
        if (computedTable.count(key)) return computedTable[key];
        BDD_ID top = std::numeric_limits<BDD_ID>::max();
        if (!isConstant(f)) top = std::min(top, topVar(f));
        if (!isConstant(g)) top = std::min(top, topVar(g));
        if (!isConstant(h)) top = std::min(top, topVar(h));
        BDD_ID hi = ite(coFactorTrue(f, top), coFactorTrue(g, top), coFactorTrue(h, top));
        BDD_ID lo = ite(coFactorFalse(f, top), coFactorFalse(g, top), coFactorFalse(h, top));
        BDD_ID res = (hi == lo) ? hi : addNode(top, hi, lo); //a bit confusing order, but left to follow the lecture
        computedTable[key] = res;
        return res;
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
        return idToLabel.count(id) ? idToLabel[id] : "n" + std::to_string(id);
    }
    /**
     * @brief Outputs the BDD to a .dot graph file.
     */
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
                out << "n" << id << " [label=\"" << getTopVarName(topVar(id)) << "\"];\n";
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

    /**
     * @brief Recursively finds all nodes reachable from a root.
     */
    void Manager::findNodes(const BDD_ID &r, std::set<BDD_ID> &n) {
        if (n.count(r)) return;
        n.insert(r);
        if (!isConstant(r)) {
            findNodes(uniqueTable[r].high, n);
            findNodes(uniqueTable[r].low, n);
        }
    }

    /**
     * @brief Finds all variable IDs used in a BDD.
     */
    void Manager::findVars(const BDD_ID &r, std::set<BDD_ID> &v) {
        std::set<BDD_ID> n;
        findNodes(r, n);
        for (auto id : n)
            if (isVariable(id)) v.insert(id);
    }

    size_t Manager::uniqueTableSize() {
        return uniqueTable.size();
    }


}
