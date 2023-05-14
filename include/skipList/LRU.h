#pragma once

#include <list>
#include <unordered_map>
#include <iostream>
#include<mutex>

std::mutex lruMtx;

// 双向链表节点
template<typename K, typename V>
struct LinkNode {
    K key;
    V value;
    LinkNode* pre;
    LinkNode* next;

    LinkNode() = default;
    LinkNode(K _key, V _value): key(_key), value(_value), pre(nullptr), next(nullptr) {}
    ~LinkNode() {
        std::cout << "析构中..." << key << "->" << value << std::endl;
    }

    bool isExpired() {
        return value->isExpired();
    }
};

// LRU算法模板类
template<typename K, typename V>
class LRUCache {
    typedef LinkNode<K, V> Node;
public:
    LRUCache(int);
    ~LRUCache();
    void setKey(const K, const V);
    void deleteKey(const K);
    V getValue(const K);
    K getHeadKey() const {
        return head->next->key;
    };
    void print() const;
    bool isExist(K);
    bool isExpired(K);
    int getCurrentCapacity();
private:
    void removeNode(Node*);
    void pushNode(Node*);
private:
    std::unordered_map<K, Node*> cache;
    int capacity;
    Node* head;
    Node* tail;
};

// 构造函数
template<typename K, typename V>
LRUCache<K, V>::LRUCache(int cap) : capacity(cap) {
    head = new Node();
    tail = new Node();
    head->pre = nullptr;
    tail->next = nullptr;
    head->next = tail;
    tail->pre = head;
}

// 析构函数
template<typename K, typename V>
LRUCache<K, V>::~LRUCache() {
    if (head != nullptr) {
        delete head;
        head = nullptr;
    }
    if (tail != nullptr) {
        delete tail;
        tail = nullptr;
    }
    for (auto &a : cache) {
        if (a.second != nullptr) {
            delete a.second;
            a.second = nullptr;
        }
    }
}

// 更新节点值
template<typename K, typename V>
void LRUCache<K, V>::setKey(const K _key, const V _val) {
    lruMtx.lock();
    // 存在节点
    if (cache.find(_key) != cache.end()) {
        Node* node = cache[_key];
        removeNode(node);
        node->value = _val;
        pushNode(node);
        lruMtx.unlock();
        return;
    }
    // 不存在节点,则先检查节点数是否达到最大值
    if (cache.size() == this->capacity) {
        K top_key = head->next->key;
        removeNode(head->next);
        Node* node = cache[top_key];
        delete node;
        node = nullptr;
        cache.erase(top_key);
    }
    Node* node = new Node(_key, _val);
    pushNode(node);
    cache[_key] = node;
    lruMtx.unlock();
}

// 删除节点
template<typename K, typename V>
void LRUCache<K, V>::deleteKey(const K key) {
    lruMtx.lock();
    Node* node = cache[key];
    removeNode(node);
    delete node;
    node = nullptr;
    cache.erase(key);
    lruMtx.unlock();
}

// 取出节点值并更新顺序
template<typename K, typename V>
V LRUCache<K, V>::getValue(const K _key) {
    lruMtx.lock();
    if (cache.find(_key) != cache.end()) {
        Node* node = cache[_key];
        removeNode(node);
        pushNode(node);
        lruMtx.unlock();
        return node->value;
    }
    lruMtx.unlock();
    return nullptr;
}

// 依此打印双向链表节点值
template<typename K, typename V>
void LRUCache<K, V>::print() const {
    if (this->head->next == this->tail) {
        std::cout << "empty\n";
        return;
    }
    Node* node = this->head->next;
    while (node != this->tail->pre) {
        std::cout << node->value->getValue() << "->";
        node = node->next;
    }
    std::cout << node->value->getValue() << std::endl;
}

// 检查是否存在节点
template<typename K, typename V>
bool LRUCache<K, V>::isExist(K key) {
    auto it = cache.find(key);
    return it != cache.end();
}

// 检查节点是否已经过期
template<typename K, typename V>
bool LRUCache<K, V>::isExpired(K key) {
    if (isExist(key)) {
        return cache[key]->isExpired();
    }
    return false;
}

// 获得设置了过期时间的节点数量
template<typename K, typename V>
int LRUCache<K, V>::getCurrentCapacity() {
    return cache.size();
}

// 删除链表中节点
template<typename K, typename V>
void LRUCache<K, V>::removeNode(Node* node) {
    node->pre->next = node->next;
    node->next->pre = node->pre;
}

// 将节点加入双向链表末尾
template<typename K, typename V>
void LRUCache<K, V>::pushNode(Node* node) {
    tail->pre->next = node;
    node->pre = tail->pre;
    node->next = tail;
    tail->pre = node;
}