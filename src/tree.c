#include "tree.h"
#include "math.h"

void init_tree_node(AVLNode *node) {
    node->left = NULL;
    node->right = NULL;
    node->height = 0;
}

// compare => 0 same, 1 right larger, otherwise -1
AVLNode **find_tree_node(AVLNode **base, AVLNode *node, int (*compare)(AVLNode *, AVLNode *)) {
    AVLNode *curr = *base;
    if (curr == NULL)
        return base;

    int result = compare(curr, node);

    if (result == 0)
        return base;

    if (result == -1) {
        return find_tree_node(&curr->left, node, compare);
    }
    if (result == 1) {
        return find_tree_node(&curr->right, node, compare);
    }

    assert(0 && "Unreachable");
}

// TODO
static void update_height(AVLNode *node) {
    if (node == NULL) return;
    node->height = 1 + fmax(node_height(node->left), node_height(node->right));
}

static int node_balance_factor(AVLNode *node) {
    return (node_height(node->left) - node_height(node->right));
}

static AVLNode *rot_left(AVLNode *node) {
    AVLNode *new_node = node->right;
    AVLNode *inner = new_node->left;
    new_node->left = node;
    node->right = inner;

    update_height(node);
    update_height(new_node);
    return new_node;
}

static AVLNode *rot_right(AVLNode *node) {
    AVLNode *new_node = node->left;
    AVLNode *inner = new_node->right;
    new_node->right = node;
    node->left = inner;

    update_height(node);
    update_height(new_node);
    return new_node;
}

AVLNode* rot_left_right(AVLNode *node){
    node->left = rot_left(node->left);
    return rot_right(node);
}

AVLNode* rot_right_left(AVLNode *node){
    node->right = rot_right(node->right);
    return rot_left(node);
}



static AVLNode* avl_rebalance(AVLNode *node) {
    if (node == NULL) return NULL;

    if (node->left != NULL) {
        node->left = avl_rebalance(node->left);
    }
    if (node->right != NULL) {
        node->right = avl_rebalance(node->right);
    }

    // LL => Balance factor : > 1, left : >= 0 => rot_right(node)
    // LR => Balance factor : > 1, left : < 0 => rot_left(node.left); rot_right(node)
    // RR => Balance factor : < -1, right : >= 0
    // RL => Balance factor : < -1, right : > 0

    if (node_balance_factor(node) > 1) {
        if(node_balance_factor(node->left) >= 0){
            printf("ll rotation...\n"); 
            AVLNode *new_node = rot_right(node);
            return new_node;
        } else {
            printf("lr rotation...\n"); 
            return rot_left_right(node);
        }
    }
    if(node_balance_factor(node) < -1) {
        if(node_balance_factor(node->right) <= 0) {
            printf("rr rotation...\n"); 
            AVLNode *new_node = rot_left(node);
            return new_node;
        } else {
            printf("rl rotation...\n"); 
            return rot_right_left(node);
        }
    }
    return node;
}

void add_tree_node(AVLNode **p_root, AVLNode *new_node, int (*compare)(AVLNode *, AVLNode *)) {
    AVLNode **find = find_tree_node(p_root, new_node, compare);
    if((*find) == NULL) {
        *find = new_node;
        update_height(*p_root);
        *p_root = avl_rebalance(*p_root);
    }
}

AVLNode **find_smallest(AVLNode **target) {
    AVLNode *node = *target;
    assert(node != NULL);

    if (node->left != NULL) {
        return find_smallest(&node->left);
    }
    return target;
}

void node_detach(AVLNode **p_node) {
    AVLNode *node = *p_node;
    assert(node != NULL);

    if (node->left != NULL && node->right != NULL){
        AVLNode **to_victim = &node->right;
        while((*to_victim)->left != NULL) to_victim = &((*to_victim)->left);

        AVLNode *victim = *to_victim;
        assert(victim->left == NULL);

        *to_victim = victim->right;
        *victim = *node;
        *p_node = victim;
    }
    else if (node->left != NULL) {
        *p_node = node->left;
    }
    else if (node->right != NULL) {
        *p_node = node->right;
    } else {
        *p_node = NULL;
    }
}

AVLNode* remove_tree_node(AVLNode **p_root, AVLNode *key, int (*compare)(AVLNode *, AVLNode *)) {
    AVLNode **find = find_tree_node(p_root, key, compare);
    AVLNode *founded = *find;
    if (founded == NULL) return NULL;

    node_detach(find);
    update_height(*p_root);
    *p_root = avl_rebalance(*p_root);

    return founded;
}

static void print_node(AVLNode *node, int level, int(cb)(AVLNode *)) {
    if (node == NULL)
        return;
    print_node(node->right, level + 1, cb);
    for (int i = 0; i < level; ++i)
        printf("    ");
    printf("%d\n", cb(node));
    print_node(node->left, level + 1, cb);
}

void display_tree(AVLNode *root, int(cb)(AVLNode *)) { print_node(root, 0, cb); printf("===================================\n"); }

int node_height(AVLNode *node){
    if(node == NULL) return 0;
    return 1 + fmax(node_height(node->left), node_height(node->right));
}
