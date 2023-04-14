#pragma once

#include <iostream>
#include <cstdio>
#include <assert.h>

#define RED true
#define BLOCK false

template<typename K, typename V>
struct TreeNode {
    // key
    K key;
    // 值 
    V data;
    // 左子树
    TreeNode* left;
    // 右子树
    TreeNode* right;
    // 颜色
    bool color;

    size_t size;

    TreeNode() = default;
    TreeNode(K key, V data, bool color, size_t size) {
        this->data = data;
        this->color = color;
        this->key = key;
        this->size = size;
    }
};

template<typename K, typename V>
class RedBlockTree {
public:
    // 删除最小值
    void deleteMin() {
        if (isEmpty()) return;
        if (!isRed(root->left) && !isRed(root->right)) {
            root->color = RED;
        }
        root = deleteMin(root);
        if (!isEmpty()) root->color = BLOCK;
    }

    // 删除最大值
    void deleteMax() {
        if (isEmpty()) return;
        if (!isRed(root->left) && !isRed(root->right)) {
            root->color = RED;
        }                
        root = deleteMax(root);
        if (!isEmpty()) root->color = BLOCK;
     }

    // 删除任意节点
    void deleted(K key) {
        if (isEmpty()) return;
        if (!searchKey(root, key)) return;
        if (!isRed(root->left) && !isRed(root->right)) {
            root->color = RED;
        }     
        root = deleted(root, key);
        if (!isEmpty()) root->color = BLOCK;
    }

    // 递归的中序遍历
    void inorderTraversal() {
        if (root == nullptr) {
            std::cout << "empty!" << std::endl;
            return;
        }
        inorderTraversal(root);
        std::cout << std::endl;
    }

    // 插入数据到树中
    void insert(K key, V data) {    
        root = insert(root, key, data);
        root->color = BLOCK;
    }

    // 搜索节点
    void searchKey(K key) {
        TreeNode<K, V>* tmp = searchKey(root, key);
        if (tmp) {
            std::cout << tmp->key << " : " << tmp->data << std::endl;
        } else {
            std::cout << "Not Found" << std::endl;
        }
    }
private:
    TreeNode<K, V>* root = nullptr;
private:
    // 判断节点是否是红色
    bool isRed(TreeNode<K, V>* t) {
        if (t == nullptr) return false;
        return t->color == RED;
    }

    // 判断树是否为空
    bool isEmpty() {
        return root == nullptr;
    }

    // 寻找树中的最小节点
    TreeNode<K, V>* findMin(TreeNode<K, V>* t) {
        if (t->left == nullptr)
            return t;
        else
            return findMin(t->left); 
    }

    // 寻找树中最大值
    TreeNode<K, V>* findMax(TreeNode<K, V>* t) {
        if (t->right == nullptr) 
            return t;
        else 
            return findMax(t->right);
    }

    // LL单旋转
    TreeNode<K, V>* rotateWithLeft(TreeNode<K, V>* k2) {
        assert(k2 != nullptr && isRed(k2->left)); 
        TreeNode<K, V>* k1 = k2->left;
        k2->left =k1->right;
        k1->right = k2;
        k1->color = k2->color;
        k2->color = RED;
        //k1->size = k2->size;
       // k2->size = k2->left->size + k2->right->size + 1;
        return k1;
    }

    // RR单旋转
    TreeNode<K, V>* rotateWithRight(TreeNode<K, V>* k1) {
        assert(k1 != nullptr && isRed(k1->right)); 
        TreeNode<K, V>* k2 = k1->right;
        k1->right =k2->left;
        k2->left = k1;
        k2->color = k1->color;
        k1->color = RED;
        //k1->size = k1->left->size + k1->right->size + 1;
        return k2;
    }

    // 翻转节点及其两个子节点的颜色
    void colorConversion(TreeNode<K, V>* node) {
        node->color = !node->color;
        node->left->color = !node->left->color;
        node->right->color = !node->right->color;
    }


    // 插入数据到以t为根的树中
    TreeNode<K, V>* insert(TreeNode<K, V>* t, K key, V data) {
        if (t == nullptr) {
            TreeNode<K, V>* tmp = new TreeNode<K, V>(key, data, RED, 1);
            return tmp;
        }

        if (key < t->key) {
            t->left = insert(t->left, key, data);
        } else if (key > t->key) {
            t->right = insert(t->right, key, data);
        } else {
            t->data = data;
        }

        return balance(t);
    }

    // 恢复红黑树性质
    TreeNode<K, V>* balance(TreeNode<K, V>* t) {
        // 有红色右连接则RR旋转
        if (isRed(t->right) && !isRed(t->left)) t = rotateWithRight(t);
        // 连续两条红色连接则LL旋转
        if (isRed(t->left) && isRed(t->left->left)) t = rotateWithLeft(t);
        // 两个子节点都是红色连接则颜色转换
        if (isRed(t->left) && isRed(t->right)) colorConversion(t);
        //t->size = t->left->size + t->right->size + 1;
        return t;
    }

    // 左调整
    TreeNode<K, V>* moveRedLeft(TreeNode<K, V>* t) {
        colorConversion(t);
        // 当前节点的左子节点是2-节点而它的兄弟不是2-节点,若左子节点是2-节点,则一定有兄弟节点
        if (isRed(t->right->left)) {
            t->right = rotateWithLeft(t->right);
            t = rotateWithRight(t);
            colorConversion(t);
        }
        return t;
    }

    // 删除最小值
    TreeNode<K, V>* deleteMin(TreeNode<K, V>* t) {
        if (t->left == nullptr) return nullptr;
        // 当前节点的左子节点是2-节点
        if (!isRed(t->left) && !isRed(t->left->left)) {
            t = moveRedLeft(t);
        }
        t->left = deleteMin(t->left);

        return balance(t);
    }

    // 右调整
    TreeNode<K, V>* moveRedRight(TreeNode<K, V>* t) {
        colorConversion(t);
        if (isRed(t->left->left)) {
            t = rotateWithLeft(t);
            colorConversion(t);
        }
        return t;
    } 
    
    // 删除最大值
    TreeNode<K, V>* deleteMax(TreeNode<K, V>* t) {
        // 左连接是红连接,为了保证被处理节点的左节点是右节点的兄弟节点,转化成右连接
        if (isRed(t->left)) {
            t = rotateWithLeft(t);
        }
        if (t->right == nullptr) return nullptr;
        // 右节点为2-节点
        if (!isRed(t->right) && !isRed(t->right->left)) {
            t = moveRedRight(t);
        }
        t->right = deleteMax(t->right);
        return balance(t);
    }

    // 删除任意节点
    TreeNode<K, V>* deleted(TreeNode<K, V>* t, K key) {
        if (key < t->key) {
            // 左节点是2-节点
            if (!isRed(t->left) && !isRed(t->left->left)) {
                t = moveRedLeft(t);
            }
            t->left = deleted(t->left, key);
        } else {
            if (isRed(t->left)) {
                t = rotateWithLeft(t);
            }
            // 树底,删除
            if (t->key == key && t->right == nullptr) {
                return nullptr;
            }

            // 右节点是2-节点
            if (!isRed(t->right) && !isRed(t->right->left)) {
                t = moveRedRight(t);
            }
            if (t->key == key) {
                TreeNode<K, V>* x = findMin(t->right);
                t->key = x->key;
                t->data = x->data;
                t->right = deleteMin(t->right);
            } else {
                t->right = deleted(t->right, key);
            }
        }
        return balance(t);
    }

    // 中序遍历
    void inorderTraversal(TreeNode<K, V>* t) {
        if (t != nullptr) {
            inorderTraversal(t->left);
            printf("%d : %d  ", t->key, t->data);
            inorderTraversal(t->right);
        }
    }

    // 查找节点
    TreeNode<K, V>* searchKey(TreeNode<K, V>* t, K key) {
        if (!t) return nullptr;
        if (key == t->key) 
            return t;
        else if (key > t->key) 
            return searchKey(t->right, key);
        else if (key < t->key) 
            return searchKey(t->left, key);
        return nullptr;
    }
};