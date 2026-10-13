//
// Created by Xuefeng Huang on 2020/1/30.
//

#ifndef TEXASSOLVER_ACTIONNODE_H
#define TEXASSOLVER_ACTIONNODE_H

#include <include/trainable/Trainable.h>
#include <thread>
#include <mutex>
#include <include/ranges/PrivateCards.h>
#include "include/nodes/GameTreeNode.h"
#include "include/nodes/GameActions.h"

class ActionNode : public GameTreeNode {
public:
    ActionNode(vector<GameActions> actions, vector<shared_ptr<GameTreeNode>> childrens, int player, GameRound round,
               double pot, shared_ptr<GameTreeNode> parent);
    ~ActionNode();
    vector<GameActions>& getActions();
    vector<shared_ptr<GameTreeNode>>& getChildrens();
    int getPlayer();
    shared_ptr<Trainable> getTrainable(int i, bool create_on_site = true, int use_halffloats = 0);
    void setTrainable(vector<shared_ptr<Trainable>> trainable, vector<PrivateCards>* player_privates);
    vector<PrivateCards>* player_privates;

private:
    GameTreeNodeType getType() override;

private:
    // TODO: this could be slimmed down
    vector<GameActions> actions;

public:
    void setActions(const vector<GameActions>& actions);

    void setChildrens(const vector<shared_ptr<GameTreeNode>>& childrens);

private:
    // TODO: this could be slimmed down too. The nodes following different chance nodes can all be shared, perhaps by tagging each branch with an id, reuse as much as possible
    vector<shared_ptr<GameTreeNode>> childrens;
    vector<shared_ptr<Trainable>> trainables;
    int player;
};

#endif //TEXASSOLVER_ACTIONNODE_H
