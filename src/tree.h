#include "common.h"
#ifndef TREE_H 
#define TREE_H

typedef struct AVLNode AVLNode;

struct AVLNode {
    int32 height;
    AVLNode *left;
    AVLNode *right;
};

#define withoffset(node, display, p, offset) \
    do { \
        if ((*offset) <= 0) \
            display((node), (p)); \
        else  \
            (*offset) = (*offset) - 1; \
    } while(0);\

void init_tree_node(AVLNode *node);
void add_tree_node(AVLNode **root, AVLNode *new_node, int (*compare)(AVLNode *, AVLNode *));
void display_tree(AVLNode *root, int (cb)(AVLNode *));
AVLNode* remove_tree_node(AVLNode **root, AVLNode *key, int (*compare)(AVLNode *, AVLNode *));
int node_height(AVLNode *node);
void dfs_tree(AVLNode *node, void (cb) (AVLNode *, void *userdata), void *userdata, int *offset);
void dfs_tree_with_boundary(AVLNode *node, AVLNode *lower, void (display)(AVLNode *node, void *userdata), void *p, int *offset);

#endif // TREE_H
