#include "redBlackTree.h"


int main() {
    RedBlockTree<int, int> tree;
    int arr[] = {9,3,5,10,11,2,1,7,8};
    for (int i = 0; i < 8; i++) {
        tree.insert(arr[i], i);
    }
    tree.inorderTraversal();
    tree.searchKey(70);
    tree.deleted(90);
    tree.inorderTraversal();
    tree.deleteMin();
    tree.inorderTraversal();
    tree.deleteMax();
    tree.inorderTraversal();
    return 0;
}
