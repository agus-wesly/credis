#include "common.h"
#ifndef TREE_H 
#define TREE_H

typedef struct AVLNode AVLNode;

struct AVLNode {
    AVLNode *left;
    AVLNode *right;
    AVLNode *parent;
    int32 height;
    int32 cnt;
};

#define with_offset_and_limit(node, display, p, offset, limit) \
    do { \
        if ((*offset) <= 0) { \
            if ((*limit) == -1) { /* No limit */ \
                display((node), (p)); \
            }\
            else if ((*limit) > 0) { \
                display((node), (p)); \
                (*limit) = (*limit) - 1;\
            }\
        } \
        else { \
            (*offset) = (*offset) - 1; \
        }\
    } while(0);\

void init_tree_node(AVLNode *node);
AVLNode* search(AVLNode **root, AVLNode *target, int (*compare)(AVLNode *, AVLNode *));
void search_and_insert(AVLNode **root, AVLNode *new_node, int (*compare)(AVLNode *, AVLNode *));
void display_tree(AVLNode *root, int (cb)(AVLNode *));
AVLNode* search_and_delete(AVLNode **root, AVLNode *key, int (*compare)(AVLNode *, AVLNode *));
// int node_height(AVLNode *node);
void dfs_tree(AVLNode *node, void (cb) (AVLNode *, void *userdata), void *userdata, int *offset, int *limit);
void dfs_tree_with_boundary(AVLNode *node, AVLNode *lower, void (display)(AVLNode *node, void *userdata), void *p, int *offset, int *limit);
AVLNode *avl_detach(AVLNode *node);
AVLNode *avl_offset(AVLNode *node, int offset);
AVLNode *avl_find_ge(AVLNode **root, AVLNode *target, int (*compare)(AVLNode *, AVLNode *));
#endif // TREE_H
