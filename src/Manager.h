// A minimalistic BDD library, following Wolfgang Kunz lecture slides
//
// Created by Markus Wedler 2014

#ifndef VDSPROJECT_MANAGER_H
#define VDSPROJECT_MANAGER_H

#include "ManagerInterface.h"
#include <map>
#include <unordered_map>
#include <string>
#include <tuple>
#include <set>

namespace ClassProject {

    // Hash function specialization for std::tuple<BDD_ID, BDD_ID, BDD_ID>
    struct TupleHash {
        std::size_t operator()(const std::tuple<BDD_ID, BDD_ID, BDD_ID>& key) const {
            size_t h1 = std::hash<BDD_ID>{}(std::get<0>(key));
            size_t h2 = std::hash<BDD_ID>{}(std::get<1>(key));
            size_t h3 = std::hash<BDD_ID>{}(std::get<2>(key));
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    class Manager : public ManagerInterface {
    public:
        Manager();

        BDD_ID createVar(const std::string &label) override;
        const BDD_ID &True() override;
        const BDD_ID &False() override;
        bool isConstant(BDD_ID f) override;
        bool isVariable(BDD_ID x) override;
        BDD_ID topVar(BDD_ID f) override;
        BDD_ID ite(BDD_ID f, BDD_ID g, BDD_ID h) override;
        BDD_ID coFactorTrue(BDD_ID f, BDD_ID x) override;
        BDD_ID coFactorFalse(BDD_ID f, BDD_ID x) override;
        BDD_ID coFactorTrue(BDD_ID f) override;
        BDD_ID coFactorFalse(BDD_ID f) override;
        BDD_ID neg(BDD_ID a) override;
        BDD_ID and2(BDD_ID a, BDD_ID b) override;
        BDD_ID or2(BDD_ID a, BDD_ID b) override;
        BDD_ID xor2(BDD_ID a, BDD_ID b) override;
        BDD_ID nand2(BDD_ID a, BDD_ID b) override;
        BDD_ID nor2(BDD_ID a, BDD_ID b) override;
        BDD_ID xnor2(BDD_ID a, BDD_ID b) override;

        std::string getTopVarName(const BDD_ID &root) override;
        void findNodes(const BDD_ID &root, std::set<BDD_ID> &nodes_of_root) override;
        void findVars(const BDD_ID &root, std::set<BDD_ID> &vars_of_root) override;
        size_t uniqueTableSize() override;
        void visualizeBDD(std::string filepath, BDD_ID &root) override;

    private:
        struct Node {
            BDD_ID id, topVar, low, high;
        };

        BDD_ID currentID;
        BDD_ID trueID, falseID;

        std::map<std::string, BDD_ID> labelToID;
        std::map<BDD_ID, std::string> idToLabel;
        std::map<BDD_ID, Node> uniqueTable;
        std::unordered_map<std::tuple<BDD_ID, BDD_ID, BDD_ID>, BDD_ID, TupleHash> computedTable;
        std::unordered_map<std::tuple<BDD_ID, BDD_ID, BDD_ID>, BDD_ID, TupleHash> uniqueHashTable;
        std::set<BDD_ID> variableIDs;

        BDD_ID addNode(BDD_ID topVar, BDD_ID high, BDD_ID low);
    };

}

#endif