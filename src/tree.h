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
void add_tree_node(AVLNode **root, AVLNode *new_node, int (*compare)(AVLNode *, AVLNode *));
void display_tree(AVLNode *root, int (cb)(AVLNode *));
AVLNode* remove_tree_node(AVLNode **root, AVLNode *key, int (*compare)(AVLNode *, AVLNode *));
int node_height(AVLNode *node);

// static AVLNode* avl_rebalance(AVLNode *node);

#endif // TREE_H
