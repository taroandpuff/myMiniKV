#include "redBlackTree.h"
#include <vector>

int main() {
    RedBlockTree<int, int> tree;
    std::vector<int> arr = {2, 4, 8, 66, 5, 21, 3, 6, 53, 9, 232, 1, 454, 10, 43, 23, 42};
    for (int i = 0; i < arr.size(); i++) {
        tree.insert(arr[i], i);
        tree.inorderTraversal();
    }
    for (int i = arr.size() - 1; i >= 0; i--) {
        //std:: cout << arr[i] << std::endl;
        tree.searchKey(arr[i]);
        tree.deleted(arr[i]);
        tree.inorderTraversal();
    }
    return 0;
}
