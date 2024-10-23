#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
using namespace std;

typedef struct vEB {
    bool isLeaf;
    int A[2]; 
    int universeSize; 
    struct vEB* summary;        // Summary vEB tree
    struct vEB** cluster;       // Cluster array
} vEB;


int high(int x, int sqrtU) {
    return x / sqrtU;
}

int low(int x, int sqrtU) {
    return x % sqrtU;
}

int index(int x, int y, int sqrtU) {
    return x * sqrtU + y;
}

vEB* createVEB(int universeSize) {
    vEB* tree = (vEB*)malloc(sizeof(vEB));
    tree->isLeaf = true;
    tree->A[0] = -1; 
    tree->A[1] = -1;
    tree->universeSize = universeSize;

    if (universeSize > 2) {
        int sqrtU = (int)sqrt(universeSize);
        tree->summary = createVEB(sqrtU);
        tree->cluster = (vEB**)malloc(sqrtU * sizeof(vEB*));
        for (int i = 0; i < sqrtU; i++) {
            tree->cluster[i] = createVEB(sqrtU);
        }
    } else {
        tree->summary = NULL;
        tree->cluster = NULL;
    }
    return tree;
}

void insert(vEB* tree, int x) {
if (x >= tree->universeSize) {
        return;  
    }

    if (tree->isLeaf) {
        if (tree->A[0] == -1) {
            tree->A[0] = x;
        } else if (tree->A[1] == -1) {
            if (x < tree->A[0]) {
                tree->A[1] = tree->A[0];
                tree->A[0] = x;
            } else if (x > tree->A[0]) {
                tree->A[1] = x;
            }
        } else if (x == tree->A[0] || x == tree->A[1]) {
            return;
        } else {
            // If the leaf node is full and x is a new value, 
            //a new non-leaf structure needs to be created
            int oldA0 = tree->A[0];
            int oldA1 = tree->A[1];
            
            // Convert leaf nodes to non-leaf nodes
            tree->isLeaf = false;
            int sqrtU = (int)sqrt(tree->universeSize);
            tree->summary = createVEB(sqrtU);
            tree->cluster = (vEB**)malloc(sqrtU * sizeof(vEB*));
            for (int i = 0; i < sqrtU; i++) {
                tree->cluster[i] = createVEB(sqrtU);
            }
            tree->A[0] = -1;
            tree->A[1] = -1;
            insert(tree, oldA0);
            insert(tree, oldA1);
            insert(tree, x);
        }
    } else {
        int sqrtU = (int)sqrt(tree->universeSize);
        int highX = high(x, sqrtU);
        int lowX = low(x, sqrtU);
        
        if (tree->cluster[highX]->A[0] == -1) {
            insert(tree->summary, highX);
        }
        insert(tree->cluster[highX], lowX);
    }
}


bool member(vEB* tree, int x) {
    if (tree->isLeaf) {
        return (tree->A[0] == x || tree->A[1] == x);
    } else {
        int sqrtU = (int)sqrt(tree->universeSize);
        int HX = high(x, sqrtU);
        int LX = low(x, sqrtU);
        
        if (tree->cluster[HX]->A[0] == -1) {
            return false;
        }
        return member(tree->cluster[HX], LX);
    }
}

/*Minimum 
For leaf nodes:
First check A[0], if not -1, return it
Otherwise check A[1], if not -1, return it
If both are -1, return -1 for empty tree
For non-leaf nodes:
Recursively find the smallest non-empty cluster
Recursively find the smallest value in that cluster
Combine the results using the idex function*/
int minimum(vEB* tree) {
    if (tree->isLeaf) {
        if (tree->A[0] != -1) return tree->A[0];
        if (tree->A[1] != -1) return tree->A[1];
        return -1;  
    } else {
        int minCluster = minimum(tree->summary);
        if (minCluster == -1) return -1;  
        return minimum(tree->cluster[minCluster]);
    }
}

int maximum(vEB* tree) {
    if (tree->isLeaf) {
        if (tree->A[1] != -1) return tree->A[1];
        if (tree->A[0] != -1) return tree->A[0];
        return -1;  
    } else {
        int maxCluster = maximum(tree->summary);
        if (maxCluster == -1) return -1; 
        return maximum(tree->cluster[maxCluster]);
    }
}

int successor(vEB* tree, int x) {
    if (tree->universeSize == 2) {
        if (x == 0 && tree->A[1] == 1) {//If x is 0 and the tree has 1, return 1 as the successor
            return 1; 
        }
        else return -1; 
    }
    if (x < tree->A[0]) {//If x is smaller than the minimum, the minimum is the successor
        return tree->A[0];
    }
    int sqrtU = (int)sqrt(tree->universeSize);
    int highX = high(x, sqrtU);
    int lowX = low(x, sqrtU);
    int succ = successor(tree->cluster[highX], lowX);
    if (succ != -1) {// If a successor is found in the current cluster
        return index(highX, succ, sqrtU);
    }
    // No successor in the current cluster, search for the next cluster in the summary
    int nextCluster = successor(tree->summary, highX);
    if (nextCluster != -1) {
        // If a successor cluster is found, return the index of the minimum element in that cluster
        return index(nextCluster, minimum(tree->cluster[nextCluster]), sqrtU);
    }
    return -1; 
}

void deletion(vEB* tree, int x) {
    if (tree->A[0] == -1) { // Tree is empty
        return;
    } else if (tree->A[0] == tree->A[1]) { 
        //If the tree only contains one element, delete that element if it matches x
        if (x == tree->A[0]) {
            tree->A[0] = tree->A[1] = -1; 
        }
    } else {// If x is outside the range of current
        if (x < tree->A[0] || x > tree->A[1]) {
            return; 
        }
        if (tree->universeSize > 2) {
            int sqrtU = (int)sqrt(tree->universeSize);
            int highX = high(x, sqrtU);
            int lowX = low(x, sqrtU);
            deletion(tree->cluster[highX], lowX );
            // If the cluster becomes empty after deletion, remove it from the summary and free its memory
            if (tree->cluster[highX]->A[0] == -1) { 
                deletion(tree->summary, highX);
                tree->cluster[highX] = NULL;
            }
            int newMin = tree->A[1];
            int newMax = tree->A[0];
            for (int i = 0; i < sqrtU; i++) {
                if (tree->cluster[i] != NULL) {
                    if (minimum(tree->cluster[i]) < newMin) newMin = minimum(tree->cluster[i]);
                    if (maximum(tree->cluster[i]) > newMax) newMax = maximum(tree->cluster[i]);
                }
            }
            tree->A[0] = newMin;
            tree->A[1] = newMax;
        }
    }
}

void freeVEB(vEB* tree) {
    if (tree == NULL) return;
    if (tree->universeSize > 2) {
        free(tree->summary);
        for (int i = 0; i < (int)sqrt(tree->universeSize); i++) {
            freeVEB(tree->cluster[i]);
        }
        free(tree->cluster);
    }
    free(tree);
}

int main() {
    int universeSize = 16; // Change as needed
    vEB* tree2 = createVEB(universeSize);
    insert(tree2, 3);
    insert(tree2, 5);
    insert(tree2, 1);
    insert(tree2, 7);

    printf("Minimum: %d\n", minimum(tree2));
    printf("Maximum: %d\n", maximum(tree2));
    // printf("Member 3: %d\n", member(tree2, 3));
    // printf("Member 8: %d\n", member(tree2, 8));

    printf("Member 1: %d\n", member(tree2, 1));
    printf("Member 2: %d\n", member(tree2, 2));
    printf("Member 3: %d\n", member(tree2, 3));
    printf("Member 4: %d\n", member(tree2, 4));
    printf("Member 5: %d\n", member(tree2, 5));
    printf("Member 6: %d\n", member(tree2, 6));
    printf("Member 7: %d\n", member(tree2, 7));
    printf("Member 8: %d\n", member(tree2, 8));
    printf("Member 9: %d\n", member(tree2, 9));
    printf("Member 10: %d\n", member(tree2, 10));

    printf("Successor to 1: %d\n", successor(tree2, 1));
    printf("Successor to 3: %d\n", successor(tree2, 3));
    printf("Successor to 5: %d\n", successor(tree2, 5));
    printf("Successor to 7: %d\n", successor(tree2, 7));

    deletion(tree2, 3);
    printf("Member 3 after deletion: %d\n", member(tree2, 3));

    freeVEB(tree2);
    universeSize = 256; // Change as needed
    vEB* tree = createVEB(universeSize);
    FILE *file = fopen("tree_ops.txt", "r");
    char *line = (char *)malloc(500 * sizeof(char));  
    char sz;
    int value;
    while (fgets(line, 256, file)) {
        sscanf(line, "%c %d", &sz, &value);
        printf("Operation: %c %d\n", sz, value);
        if (sz == 'I') {
            insert(tree, value);
        } else if (sz == 'D') {
            deletion(tree, value);
        } else if (sz == 'S') {
            int succ = successor(tree, value);
            if (succ != -1) {
                printf("Successor of %d is %d\n", value, succ);
            } else {
                printf("No successor for %d\n", value);
            }
        }
    }
    fclose(file);
    freeVEB(tree);
    return 0;

}
