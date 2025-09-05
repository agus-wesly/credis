#include "tree.h"
#include "sorted_set.h"
#include <time.h>


static int cb(AVLNode *left) {
    TEntry *ent = container_of(left, TEntry, node);
    return ent->value;

}

// void test_individual() {
//     if(true){
//         add_tree_entry(root, 50);
//         display_tree(root, cb);
//         add_tree_entry(root, 30);
//         display_tree(root, cb);
//         add_tree_entry(root, 20);
//         display_tree(root, cb);
//     }
//     if(false) {
//         add_tree_entry(root, 50);
//         display_tree(root, cb);
//         add_tree_entry(root, 100);
//         display_tree(root, cb);
//         add_tree_entry(root, 20);
//         display_tree(root, cb);
//         remove_tree_entry(50);
//         display_tree(root, cb);
//     }
// 
//     if(false){
//         add_tree_entry(root, 50);
//         display_tree(root, cb);
//         add_tree_entry(root, 30);
//         display_tree(root, cb);
//         add_tree_entry(root, 35);
//         display_tree(root, cb);
//     }
//     if(false){
//         add_tree_entry(root, 50);
//         display_tree(root, cb);
//         add_tree_entry(root, 60);
//         display_tree(root, cb);
//         add_tree_entry(root, 75);
//         display_tree(root, cb);
// 
//     }
//     if(false){
//         add_tree_entry(root, 50);
//         display_tree(root, cb);
//         add_tree_entry(root, 60);
//         display_tree(root, cb);
//         add_tree_entry(root, 55);
//         display_tree(root, cb);
//         remove_tree_entry(60);
//         display_tree(root, cb);
//         add_tree_entry(root, 51);
//         display_tree(root, cb);
//     }
//     if(false){
//         add_tree_entry(root, 40);
//         display_tree(root, cb);
//         add_tree_entry(root, 20);
//         display_tree(root, cb);
//         add_tree_entry(root, 10);
//         display_tree(root, cb);
//         add_tree_entry(root, 25);
//         display_tree(root, cb);
//         add_tree_entry(root, 30);
//         display_tree(root, cb);
//         add_tree_entry(root, 22);
//         display_tree(root, cb);
//         add_tree_entry(root, 50);
//         display_tree(root, cb);
//     }
// 
// }
// 
// void test_one() {
//     add_tree_entry(root, 10);
//     display_tree(root, cb);
//     add_tree_entry(root, 9);
//     display_tree(root, cb);
//     add_tree_entry(root, 8);
//     display_tree(root, cb);
//     add_tree_entry(root, 20);
//     display_tree(root, cb);
//     add_tree_entry(root, 19);
//     display_tree(root, cb);
//     add_tree_entry(root, 18);
//     display_tree(root, cb);
// }


// void test_all() {
//     // 1. Add elements
//     printf("\nAdding elements: 50, 30, 70, 20, 40, 60, 80\n");
//     add_tree_entry(root, 50);
//     add_tree_entry(root, 30);
//     add_tree_entry(root, 70);
//     printf("Tree after insertion:\n");
//     display_tree(root, cb);
//     printf("Height : %d\n", node_height(root));
// 
//     add_tree_entry(root, 20);
//     add_tree_entry(root, 40);
//     add_tree_entry(root, 60);
//     add_tree_entry(root, 80);
//     add_tree_entry(root, 50);
// 
//     // 2. Remove a leaf node
//     printf("\nRemoving leaf node 50...\n");
//     remove_tree_entry(50);
//     display_tree(root, cb);
//     printf("Height : %d\n", node_height(root));
// 
//     // 2. Remove a leaf node
//     printf("\nRemoving leaf node 20...\n");
//     remove_tree_entry(20);
//     display_tree(root, cb);
//     printf("Height : %d\n", node_height(root));
// 
//     // 3. Remove a node with one child
//     printf("\nRemoving node 30 (one child)...\n");
//     remove_tree_entry(30);
//     display_tree(root, cb);
//     printf("Height : %d\n", node_height(root));
// 
//     // 4. Remove a node with two children
//     printf("\nRemoving node 50 (two children)...\n");
//     remove_tree_entry(50);
//     display_tree(root, cb);
//     printf("Height : %d\n", node_height(root));
// 
//     // 5. Try removing a node that doesn’t exist
//     printf("\nRemoving non-existent node 999...\n");
//     remove_tree_entry(999);
//     display_tree(root, cb);
//     printf("Height : %d\n", node_height(root));
// 
//     // 6. Add again after deletion
//     printf("\nAdding 25 back to the tree...\n");
//     add_tree_entry(25);
//     display_tree(root, cb);
//     printf("Height : %d\n", node_height(root));
// 
// }
// 
void test_insert_patterns(AVLNode **root) {
    if (true) {
        printf("\n=== Ascending Insert Test ===\n");
        for (int i = 1; i <= 10; i++) {
            add_tree_entry(root, i);
        }
        for (int i = 1; i <= 10; i++) {
            add_tree_entry(root, i);
        }
    }

    printf("\n=== Descending Insert Test ===\n");
    for (int i = 20; i >= 11; i--) {
        add_tree_entry(root, i);
    }

    printf("\n=== Random Insert Test ===\n");
    int nums[] = {15, 25, 5, 18, 30, 2, 8, 12, 19};
    for (int i = 0; i < (int)sizeof(nums) / (int)sizeof(nums[0]); i++) {
        add_tree_entry(root, nums[i]);
    }

    display_tree(*root, cb);
}

void test_removal_patterns(AVLNode **root) {
    printf("\n=== Remove => %d ===\n", 2);
    remove_tree_entry(root, 2); // leaf
    display_tree(*root, cb);

    printf("\n=== Remove %d ===\n", 8);
    remove_tree_entry(root, 8); // should have one child in many AVL setups
    display_tree(*root, cb);

    printf("\n=== Remove %d ===\n", 15);
    remove_tree_entry(root, 15); 
    display_tree(*root, cb);

    printf("\n=== Remove %d ===\n", 999);
    remove_tree_entry(root, 999); 
    display_tree(*root, cb);

    printf("\n=== Remove From Empty Tree ===\n");
    // Remove all
    for (int i = 1; i <= 30; i++)
        remove_tree_entry(root, i); // try again on empty tree

    display_tree(*root, cb);
}

void stress_random_ops(AVLNode **root, int count) {
    printf("\n=== Random Stress Test (%d ops) ===\n", count);
    srand(time(NULL));
    for (int i = 0; i < count; i++) {
        int op = rand() % 2;
        int val = rand() % 100;
        if (op == 0) {
            printf("Insert %d\n", val);
            add_tree_entry(root, val);
        } else {
            printf("Remove %d\n", val);
            remove_tree_entry(root, val);
        }
        display_tree(*root, cb);
    }
}

void run_test() {
    SortedSet *s = new_sorted_set();
    test_insert_patterns(&s->by_score);
    test_removal_patterns(&s->by_score);
    stress_random_ops(&s->by_score, 6969);
}
