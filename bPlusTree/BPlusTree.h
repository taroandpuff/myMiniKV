#pragma once

#include <stdio.h>
#include<iostream>
#include<algorithm>
#include<assert.h>
#include<stack>
#include<queue>
#include<math.h>
#include<vector>
#include<string>
#define McTree new BPlusTree
#define McNode new TreeNode



typedef struct TreeNode{
    // 存储的key索引
	std::vector<std::string> keys;
	//tag为0表示叶子节点
	bool tag;
    // 叶子节点储存数据,非叶子节点指向下一层节点
	struct {
		std::vector<int> data;
		std::vector<TreeNode*> sons;
	} values;
    // 叶子节点指向下一叶子节点的指针
	TreeNode* rLink;
}TreeNode,* TreePtr;

class BPlusTree {
private:
    // 根节点
    TreePtr root;
    // 指向叶子节点链表的头节点
    TreePtr linkHead;
    // 阶数(一个节点最多有K个key)
    int K;
private:
    // 二分查找位置
    size_t bSearch(std::vector<std::string> indexs, std::string key) {
        int l = 0;
        int r = indexs.size() - 1;
        while (l < r) {
            int mid = (l + r) >> 1;
            if (indexs[mid] >= key) {
                r = mid;
            } else {
                l = mid + 1;
            }
        }
        if (l == indexs.size() - 1 && indexs[l] != key) 
            return indexs.size();
        return l;
    }

    // 往上更新索引
    void up_updata(TreeNode *p, std::vector<TreeNode*> Stack, int i) {
        for (; i >= 0; i--) {
            // Stack记录p的上层节点
            TreeNode *parent = Stack[i];
            // pos记录parent中第几个value指向p
            size_t pos = bSearch(parent->keys, p->keys[0]);
            if (parent->keys[pos] == p->keys[p->keys.size() - 1]) {
                break;
            }
            // keys中记录的是子节点的keys中最大值
            parent->keys[pos] = p->keys[p->keys.size() - 1];
            p = parent;
        }
    }
    // 查找节点
    TreeNode* findNode(std::string key,std::vector<TreeNode*> &s) {
        // 如果为树为空,返回空指针
        if (linkHead == nullptr && root == nullptr) return nullptr;
        assert(linkHead && root);
    
        TreeNode* p = root;
        // 如果不是叶子节点,则不断往下找,并记录路径用于后续更新索引
        while (p->tag) {
            s.push_back(p);
            size_t pos = bSearch(p->keys, key);
            // 如果二分查找找到pos超界,则在最后一个子节点继续找
            if (pos == p->values.sons.size()) {
                pos--;
            }
            p = p->values.sons[pos];
        }
        return p;
    }

    // 根节点拆分
    void rootDivide(TreeNode* parent) {
        // 创建新的根节点
        TreeNode* newRoot = McNode;
        // 创建原根节点拆分后的右边节点
        TreeNode* rRoot = McNode;
        // 同一层节点的tag一致
        rRoot->tag = parent->tag;
        newRoot->tag = 1;
        // 更新指针
        if (rRoot->tag == 0) {
            rRoot->rLink = parent->rLink;
            parent->rLink = rRoot;
        }
        // 拆分数据
        for (int i = (parent->keys.size()) / 2; i != parent->keys.size();) {
            rRoot->keys.push_back(parent->keys[i]);
            if (rRoot->tag) {
                rRoot->values.sons.push_back(parent->values.sons[i]);
                parent->values.sons.erase(parent->values.sons.begin() + i);
            } else {
                rRoot->values.data.push_back(parent->values.data[i]);
                rRoot->values.data.erase(parent->values.data.begin() + i);
            }
            parent->keys.erase(parent->keys.begin() + i);
        }
        // 更新新节点数据
        root = newRoot;
        newRoot->keys.push_back(parent->keys[parent->keys.size() - 1]);
        newRoot->values.sons.push_back(parent);
        newRoot->keys.push_back(rRoot->keys[rRoot->keys.size() - 1]);
        newRoot->values.sons.push_back(rRoot);
    }

public:
    // 构造函数
    BPlusTree(int _k): root(nullptr), linkHead(nullptr), K(_k) {}
    // 查询
    bool find(std::string key, int &data) {
        // 栈记录路径节点
        std::vector<TreeNode*> Stack;
        // 查找到叶子节点
        TreeNode* leaf = findNode(key, Stack);
        if (leaf == nullptr) return false;
        assert(leaf && !leaf->tag);
        // 二分查找
        size_t pos = bSearch(leaf->keys, key);
        // 判断叶子节点中是否有key
        if (pos >= leaf->keys.size()) return false;
        data = leaf->values.data[pos];
        return leaf->keys[pos] == key ? true : false;
    }    
    
    // 增加节点
    bool add(std::string key, int data) {
        // 记录路径
        std::vector<TreeNode*> Stack;
        // 查找叶子节点
        TreeNode* leaf = findNode(key, Stack);
        // 空树
        if (leaf == nullptr) {
            assert(linkHead == nullptr && root == nullptr);
            // 创建根节点
            TreeNode* TNode = McNode;
            TNode->keys.push_back(key);
            TNode->tag = 0;
            TNode->values.data.push_back(data);
            // 更新根节点
            linkHead = TNode;
            TNode->rLink = nullptr;
            root = TNode;
            return true;
        }
        assert(leaf->keys.size() == leaf->values.data.size());
        std::vector<std::string>::iterator it;
        //找到该关键字，插入失败
        if ((it = std::find(leaf->keys.begin(), leaf->keys.end(), key)) != leaf->keys.end()) {
		    return false;
	    }
        // 向上修改内部节点最大值
        size_t pos = bSearch(leaf->keys, key);
        leaf->values.data.insert(leaf->values.data.begin() + pos, data);
        leaf->keys.insert(leaf->keys.begin() + pos, key);
        if (pos == leaf->keys.size() - 1 && !Stack.empty()) {
            // 往上修改
            up_updata(leaf, Stack, Stack.size() - 1);
        }
        // 如果keys数量小于阶数,不需要拆分
        if (leaf->keys.size() <= K) {
            return true;
        }
        assert(leaf->keys.size() == K + 1);
        // 如果插入的位置是根节点,拆分根节点
        if (Stack.empty()) {
            rootDivide(leaf);
            return true;
        }
        // i为栈顶指针
        int i = Stack.size() - 1;
        TreeNode* parent = Stack[i--];
        TreeNode* p = leaf;
        // 循环拆分
        while (p->keys.size() > K) {
            // q为p拆分后的右边节点
            TreeNode *q = McNode;
            q->tag = p->tag;
            // 如果是叶子节点,更新指针
            if (p->tag == 0) {
                q->rLink = p->rLink;
                p->rLink = q;
            }
            for (int i = p->keys.size() / 2; i != p->keys.size(); ) {
                q->keys.push_back(p->keys[i]);
                // 非叶子节点拆分指向子节点的指针,叶子节点拆分数据
                if (q->tag) {
                    q->values.sons.push_back(p->values.sons[i]);
                    p->values.sons.erase(p->values.sons.begin() + i);
                } else {
                    q->values.data.push_back(p->values.data[i]);
                    p->values.data.erase(p->values.data.begin() + i);
                }
                p->keys.erase(p->keys.begin() + i);                
            }
            // pos_p 为未拆分前节点的最大值在父节点的keys中的位置
            size_t pos_p = bSearch(parent->keys, q->keys[q->keys.size() - 1]);
            // 更新为新的最大值
            parent->keys[pos_p] = p->keys[p->keys.size() - 1];
            // 往parent的keys中添加q的最大值
            parent->keys.insert(parent->keys.begin() + pos_p + 1, q->keys[q->keys.size() - 1]);
            // 往parent的sons中添加指向q的指针
            parent->values.sons.insert(parent->values.sons.begin() + pos_p + 1, q);
            // 如果 parent是根节点
            if (parent == root) {
                if (parent->keys.size() > K) {
                    rootDivide(parent);
                }
                // 处理完,退出循环
                break;
            }
            // 更新p和parent
            p = parent;
            parent = Stack[i--];            
        }
        return true;
    }

    // 删除
    bool remove(std::string key) {
        if (root == nullptr) return false;

        std::vector<TreeNode*> Stack;
        // 查找叶子节点
        TreeNode* leaf = findNode(key, Stack);
        assert(leaf->keys.size() == leaf->values.data.size());
        std::vector<std::string>::iterator it;
        // 没找到节点,删除失败
        if ((it = std::find(leaf->keys.begin(), leaf->keys.end(), key)) == leaf->keys.end()) {
            return false;
        }
        // 向上修改内部节点最大值
        size_t pos = bSearch(leaf->keys, key);
        assert(leaf->keys[pos] == key);
        leaf->values.data.erase(leaf->values.data.begin() + pos);
        leaf->keys.erase(leaf->keys.begin() + pos);
        if (pos == leaf->keys.size()&&!Stack.empty()) {
            //往上修改
            up_updata(leaf, Stack,Stack.size() -1);
        }
        // leaf为根节点或节点keys数量大于等于最小值,直接删除
        if (leaf->keys.size() >= ceil(K / 2.0) || leaf == root) {
            return true;
        }

        // i为栈顶指针
        int i = Stack.size() - 1;
        TreeNode* parent = Stack[i--];
        TreeNode* p = leaf;
        // 循环从兄弟节点借值或合并兄弟节点
        while (p->keys.size() < ceil(K / 2.0)) {
            // 查找子节点
            size_t pos = bSearch(parent->keys, p->keys[p->keys.size() - 1]);
            // 有左兄弟,且可以借
            TreeNode* lBro = nullptr;
            if (pos > 0) {
                lBro = parent->values.sons[pos - 1];
            }
            if (lBro && lBro->keys.size() > ceil(K / 2.0)) {
                // count为借出的数量,尽量少借
                int count = ceil((lBro->keys.size() - ceil(K / 2.0)) / 2.0);
                p->keys.insert(p->keys.begin(), lBro->keys.end() - count, lBro->keys.end());
                // 叶子节点借出数据,非叶子节点借出指向子节点的指针
                if (p->tag) {
                    p->values.sons.insert(p->values.sons.begin(), lBro->values.sons.end() - count, lBro->values.sons.end());
                    lBro->values.sons.erase(lBro->values.sons.end() - count, lBro->values.sons.end());
                } else {
                    p->values.data.insert(p->values.data.begin(), lBro->values.data.end() - count, lBro->values.data.end());
                    lBro->values.data.erase(lBro->values.data.end() - count, lBro->values.data.end());                    
                }
                lBro->keys.erase(lBro->keys.end() - count, lBro->keys.end());
                // 向上更新
                up_updata(lBro, Stack, i+1);
                break;                
            }
            // 有右兄弟, 且可以借
            TreeNode * rBro = NULL;
            if (pos != parent->keys.size() - 1) {
                rBro = parent->values.sons[pos + 1];
            }
            if (rBro && rBro->keys.size() > ceil(K / 2.0)) {
                size_t Count = ceil((rBro->keys.size() - ceil(K / 2.0)) / 2.0);
                p->keys.insert(p->keys.end(), rBro->keys.begin(), rBro->keys.begin() + Count);
                rBro->keys.erase(rBro->keys.begin(), rBro->keys.begin() + Count);
                if (rBro->tag) {
                    p->values.sons.insert(p->values.sons.end(), rBro->values.sons.begin(), rBro->values.sons.begin()+Count);
                    rBro->values.sons.erase(rBro->values.sons.begin(), rBro->values.sons.begin() + Count);
                }
                else {
                    p->values.data.insert(p->values.data.end(), rBro->values.data.begin(), rBro->values.data.begin() + Count);
                    rBro->values.data.erase(rBro->values.data.begin(), rBro->values.data.begin() + Count);
                }
                // 向上更新
                up_updata(p, Stack, i + 1);
                break;
            }     
		// 与左兄弟合并, p数据转移到左兄弟上
		if (lBro) {
			lBro->keys.insert(lBro->keys.end(), p->keys.begin(), p->keys.end());
			if (lBro->tag) {
				lBro->values.sons.insert(lBro->values.sons.end(), p->values.sons.begin(), p->values.sons.end());
			}else {
				lBro->values.data.insert(lBro->values.data.end(), p->values.data.begin(), p->values.data.end());
			}
            //父节点关键字调整
			parent->keys[pos - 1] = lBro->keys[lBro->keys.size() - 1];
			parent->keys.erase(parent->keys.begin() + pos);
			parent->values.sons.erase(parent->values.sons.begin() + pos);
            // 如果是叶子节点,更新指针
			if (p->tag == 0) {
				lBro->rLink = p->rLink;
			}
            // 释放节点
			delete p;
            //根节点只有一个孩子时候，更新根节点
			if (parent == root && parent->keys.size() == 1) {
				root = lBro;
				delete parent;
				break;
			}
		} else {
		    //右兄弟数据转移到p
			p->keys.insert(p->keys.end(), rBro->keys.begin(), rBro->keys.end());
			if (p->tag) {
				p->values.sons.insert(p->values.sons.end(), rBro->values.sons.begin(), rBro->values.sons.end());
			}
			else {
				p->values.data.insert(p->values.data.end(), rBro->values.data.begin(), rBro->values.data.end());
			}
			parent->keys[pos] = p->keys[p->keys.size() - 1];
			parent->keys.erase(parent->keys.begin() + pos + 1);
			parent->values.sons.erase(parent->values.sons.begin() + pos + 1);
			if (p->tag == 0) {
				p->rLink = rBro->rLink;
			}
            // 释放节点
			delete rBro;
            //根节点只有一个孩子时候，删除
			if (parent == root && parent->keys.size() == 1) {
				root = p;
				delete parent;
				break;
			}
		}
        // 更新p和parent
		p = parent;
		if (i < 0) {
			break;
		}
		parent = Stack[i--];                   
        }
        return true;
    }
    void print(TreeNode * tNode) {
        std::cout << "[ ";
        for (int i = 0; i < tNode->keys.size(); i++) {
            if (tNode->tag) {
                std::cout << tNode->keys[i] << " ";
            }
            else {
                std::cout << tNode->keys[i] << ":" << tNode->values.data[i] << " ";
            }
            
        }
        std::cout << ']' << "\t";
    }    
    void showBTree() {
        printf("******************\n");
        std::queue<TreeNode*> q;
        q.push(root);
        int printed = 0;//已经打印
        int count = 1;//当前行最后一个是第几个
        int que_count = 1;//已经入队的个数
        while (!q.empty()) {
            if (printed == count) {
                count = que_count;
            }
            printed++;
            TreeNode* Node = q.front();
            q.pop();
            print(Node);
            if (printed == count) {
                std::cout << "\n";
            }
            if (Node->tag) {
                for (int i = 0; i < Node->keys.size(); i++) {
                    que_count++;
                    q.push(Node->values.sons[i]);
                }
            }
        }
        printf("******************\n");
    }    
};