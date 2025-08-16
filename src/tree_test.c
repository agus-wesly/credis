#include "tree.h"
#include <time.h>

extern AVLNode *root;
void add_tree_entry(int value);
int cb(AVLNode *left);
void remove_tree_entry(int value);

void test_individual() {
    if(true){
        add_tree_entry(50);
        display_tree(root, cb);
        add_tree_entry(30);
        display_tree(root, cb);
        add_tree_entry(20);
        display_tree(root, cb);
    }
    if(true) {
        add_tree_entry(50);
        display_tree(root, cb);
        add_tree_entry(100);
        display_tree(root, cb);
        add_tree_entry(20);
        display_tree(root, cb);
        remove_tree_entry(50);
        display_tree(root, cb);
    }

    if(true){
        add_tree_entry(50);
        display_tree(root, cb);
        add_tree_entry(30);
        display_tree(root, cb);
        add_tree_entry(35);
        display_tree(root, cb);
    }
    if(true){
        add_tree_entry(50);
        display_tree(root, cb);
        add_tree_entry(60);
        display_tree(root, cb);
        add_tree_entry(75);
        display_tree(root, cb);

    }
    if(true){
        add_tree_entry(50);
        display_tree(root, cb);
        add_tree_entry(60);
        display_tree(root, cb);
        add_tree_entry(55);
        display_tree(root, cb);
        remove_tree_entry(60);
        display_tree(root, cb);
        add_tree_entry(51);
        display_tree(root, cb);
    }
    if(true){
        add_tree_entry(40);
        display_tree(root, cb);
        add_tree_entry(20);
        display_tree(root, cb);
        add_tree_entry(10);
        display_tree(root, cb);
        add_tree_entry(25);
        display_tree(root, cb);
        add_tree_entry(30);
        display_tree(root, cb);
        add_tree_entry(22);
        display_tree(root, cb);
        add_tree_entry(50);
        display_tree(root, cb);
    }

}

void test_one() {
    add_tree_entry(10);
    display_tree(root, cb);
    add_tree_entry(9);
    display_tree(root, cb);
    add_tree_entry(8);
    display_tree(root, cb);
    add_tree_entry(20);
    display_tree(root, cb);
    add_tree_entry(19);
    display_tree(root, cb);
    add_tree_entry(18);
    display_tree(root, cb);
}


void test_all() {
    // 1. Add elements
    printf("\nAdding elements: 50, 30, 70, 20, 40, 60, 80\n");
    add_tree_entry(50);
    add_tree_entry(30);
    add_tree_entry(70);
    printf("Tree after insertion:\n");
    display_tree(root, cb);
    printf("Height : %d\n", node_height(root));

    add_tree_entry(20);
    add_tree_entry(40);
    add_tree_entry(60);
    add_tree_entry(80);
    add_tree_entry(50);

    // 2. Remove a leaf node
    printf("\nRemoving leaf node 50...\n");
    remove_tree_entry(50);
    display_tree(root, cb);
    printf("Height : %d\n", node_height(root));

    // 2. Remove a leaf node
    printf("\nRemoving leaf node 20...\n");
    remove_tree_entry(20);
    display_tree(root, cb);
    printf("Height : %d\n", node_height(root));

    // 3. Remove a node with one child
    printf("\nRemoving node 30 (one child)...\n");
    remove_tree_entry(30);
    display_tree(root, cb);
    printf("Height : %d\n", node_height(root));

    // 4. Remove a node with two children
    printf("\nRemoving node 50 (two children)...\n");
    remove_tree_entry(50);
    display_tree(root, cb);
    printf("Height : %d\n", node_height(root));

    // 5. Try removing a node that doesn’t exist
    printf("\nRemoving non-existent node 999...\n");
    remove_tree_entry(999);
    display_tree(root, cb);
    printf("Height : %d\n", node_height(root));

    // 6. Add again after deletion
    printf("\nAdding 25 back to the tree...\n");
    add_tree_entry(25);
    display_tree(root, cb);
    printf("Height : %d\n", node_height(root));

}

void test_insert_patterns() {
    if(true) {
        printf("\n=== Ascending Insert Test ===\n");
        for (int i = 1; i <= 10; i++) {
            add_tree_entry(i);
            // display_tree(root, cb);
        }
    }

    printf("\n=== Descending Insert Test ===\n");
    for (int i = 20; i >= 11; i--) {
        add_tree_entry(i);
        // display_tree(root, cb);
    }

    printf("\n=== Random Insert Test ===\n");
    int nums[] = {15, 25, 5, 18, 30, 2, 8, 12, 19};
    for (int i = 0; i < sizeof(nums) / sizeof(nums[0]); i++) {
        add_tree_entry(nums[i]);
        // display_tree(root, cb);
    }
}

void test_removal_patterns() {
    printf("\n=== Remove => %d ===\n", 2);
    remove_tree_entry(2); // leaf
    display_tree(root, cb);

    printf("\n=== Remove %d ===\n", 8);
    remove_tree_entry(8); // should have one child in many AVL setups
    display_tree(root, cb);

    printf("\n=== Remove %d ===\n", 15);
    remove_tree_entry(15); 
    display_tree(root, cb);

    printf("\n=== Remove %d ===\n", 999);
    remove_tree_entry(999); 
    display_tree(root, cb);

    printf("\n=== Remove From Empty Tree ===\n");
    // Remove all
    for (int i = 1; i <= 30; i++)
        remove_tree_entry(i); // try again on empty tree
    display_tree(root, cb);
}


void stress_random_ops(int count) {
    printf("\n=== Random Stress Test (%d ops) ===\n", count);
    srand(time(NULL));
    for (int i = 0; i < count; i++) {
        int op = rand() % 2;
        int val = rand() % 100;
        if (op == 0) {
            printf("Insert %d\n", val);
            add_tree_entry(val);
        } else {
            printf("Remove %d\n", val);
            remove_tree_entry(val);
        }
        display_tree(root, cb);
    }
}

void run_test() {
    test_insert_patterns();
    display_tree(root, cb);
    test_removal_patterns();
    stress_random_ops(6969);
}
