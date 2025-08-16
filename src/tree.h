#include "common.h"
#ifndef TREE_H 
#define TREE_H

typedef struct AVLNode AVLNode;

struct AVLNode {
    int32 height;
    AVLNode *left;
    AVLNode *right;
};

void init_tree_node(AVLNode *node);
void add_tree_node(AVLNode *new_node, int (*compare)(AVLNode *, AVLNode *));
void display_tree(int (cb)(AVLNode *));
AVLNode* remove_tree_node(AVLNode *node, AVLNode *key, int (*compare)(AVLNode *, AVLNode *));

AVLNode *rot_left(AVLNode *node);
AVLNode *rot_right(AVLNode *node);
int node_height(AVLNode *node);
int node_balance_factor(AVLNode *node);
AVLNode* avl_rebalance(AVLNode *node);
#endif // TREE_H
