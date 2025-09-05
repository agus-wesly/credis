#include "tree.h"

inline void init_tree_node(AVLNode *node) {
    node->left = node->right = node->parent = NULL;
    node->height = 1;
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

static int avl_height(AVLNode *node){
    if (node == NULL) return 0;
    return node->height;
}

static void avl_update(AVLNode *node) {
    if (node == NULL) return;
    node->height = 1 + fmax(avl_height(node->left), avl_height(node->right));
}


static AVLNode *rot_left(AVLNode *node) {
    AVLNode *parent = node->parent;
    AVLNode *new_node = node->right;
    AVLNode *inner = new_node->left;

    new_node->left = node;
    node->parent = new_node;

    node->right = inner;
    if (inner) {
        inner->parent = node;
    }
    new_node->parent = parent;

    avl_update(node);
    avl_update(new_node);
    return new_node;
}

static AVLNode *rot_right(AVLNode *node) {
    AVLNode *parent = node->parent;
    AVLNode *new_node = node->left;
    AVLNode *inner = new_node->right;

    new_node->right = node;
    node->parent = new_node;

    node->left = inner;
    if (inner) {
        inner->parent = node;
    }
    new_node->parent = parent;

    avl_update(node);
    avl_update(new_node);
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

static AVLNode* avl_fix_left(AVLNode *node) {
    if (avl_height(node->left->left) < avl_height(node->left->right)) {
        node->left = rot_left(node->left);
    }
    return rot_right(node);
}

static AVLNode *avl_fix_right(AVLNode *node) {
    if (avl_height(node->right->right) < avl_height(node->right->left)) {
        node->right = rot_right(node->right);
    }
    return rot_left(node);
}

static AVLNode* avl_rebalance(AVLNode *node) {
    while (true) {
        AVLNode **from = &node;
        AVLNode *parent = node->parent; // Can be NULL

        if (parent)  {
            from = parent->left == node ? &parent->left : &parent->right;
        }
        avl_update(node);
        int l = avl_height(node->left);
        int r = avl_height(node->right);

        if (l == r + 2) {
            *from = avl_fix_left(node);
        } else if (l + 2 == r) {
            *from = avl_fix_right(node);
        }
        if (parent == NULL) {
            return *from;
        }

        node = parent;
    }
}


void add_tree_node(AVLNode **root, AVLNode *new_node, int (*less_than)(AVLNode *, AVLNode *)) {
    AVLNode *parent = NULL;
    AVLNode **from = root;
    for (AVLNode *node = *from; node != NULL;) {
        from = less_than(new_node, node) ? &((*from)->left) : &((*from)->right);
        parent = node;
        node = *from;
    }
    *from = new_node;
    new_node->parent = parent;
    *root = avl_rebalance(new_node);
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

// TODO 
AVLNode* remove_tree_node(AVLNode **p_root, AVLNode *key, int (*compare)(AVLNode *, AVLNode *)) {
    return NULL;
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


void dfs_tree(AVLNode *node, void (cb) (AVLNode *, void *userdata), void *userdata, int *offset, int *limit) {
    if (node == NULL) return;

    dfs_tree(node->left, cb, userdata, offset, limit);
    with_offset_and_limit(node, cb, userdata, offset, limit);
    dfs_tree(node->right, cb, userdata, offset, limit);
}

void dfs_tree_with_boundary(AVLNode *node, AVLNode *lower, void (display)(AVLNode *node, void *userdata), void *p, int *offset, int *limit) {
    if (node == NULL) return;
    if (node == lower) {
        return;
        // dfs_tree(node->right, display, p, offset, limit);
    } else {
        dfs_tree_with_boundary(node->left, lower, display, p, offset, limit);
        with_offset_and_limit(node, display, p, offset, limit);
        dfs_tree_with_boundary(node->right, lower, display, p, offset, limit);
    }
}
