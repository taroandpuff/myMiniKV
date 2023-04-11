#pragma once

#include <list>
#include <unordered_map>
#include <iostream>
#include<mutex>

std::mutex lru_mtx;

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

    bool is_expired() {
        return value->is_expired();
    }
};

// LRU算法模板类
template<typename K, typename V>
class LRU_Cache {
    typedef LinkNode<K, V> Node;
public:
    LRU_Cache(int);
    ~LRU_Cache();
    void set_key(const K, const V);
    void delete_key(const K);
    V get_value(const K);
    K get_head_key() const {
        return head->next->key;
    };
    void print() const;
    bool is_exist(K);
    bool is_expired(K);
    int get_current_capacity();
private:
    void remove_node(Node*);
    void push_node(Node*);
private:
    std::unordered_map<K, Node*> cache;
    int capacity;
    Node* head;
    Node* tail;
};

// 构造函数
template<typename K, typename V>
LRU_Cache<K, V>::LRU_Cache(int cap) : capacity(cap) {
    head = new Node();
    tail = new Node();
    head->pre = nullptr;
    tail->next = nullptr;
    head->next = tail;
    tail->pre = head;
}

// 析构函数
template<typename K, typename V>
LRU_Cache<K, V>::~LRU_Cache() {
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
void LRU_Cache<K, V>::set_key(const K _key, const V _val) {
    lru_mtx.lock();
    // 存在节点
    if (cache.find(_key) != cache.end()) {
        Node* node = cache[_key];
        remove_node(node);
        node->value = _val;
        push_node(node);
        lru_mtx.unlock();
        return;
    }
    // 不存在节点,则先检查节点数是否达到最大值
    if (cache.size() == this->capacity) {
        K top_key = head->next->key;
        remove_node(head->next);
        Node* node = cache[top_key];
        delete node;
        node = nullptr;
        cache.erase(top_key);
    }
    Node* node = new Node(_key, _val);
    push_node(node);
    cache[_key] = node;
    lru_mtx.unlock();
}

// 删除节点
template<typename K, typename V>
void LRU_Cache<K, V>::delete_key(const K key) {
    lru_mtx.lock();
    Node* node = cache[key];
    remove_node(node);
    delete node;
    node = nullptr;
    cache.erase(key);
    lru_mtx.unlock();
}

// 取出节点值并更新顺序
template<typename K, typename V>
V LRU_Cache<K, V>::get_value(const K _key) {
    lru_mtx.lock();
    if (cache.find(_key) != cache.end()) {
        Node* node = cache[_key];
        remove_node(node);
        push_node(node);
        lru_mtx.unlock();
        return node->value;
    }
    lru_mtx.unlock();
    return nullptr;
}

// 依此打印双向链表节点值
template<typename K, typename V>
void LRU_Cache<K, V>::print() const {
    if (this->head->next == this->tail) {
        std::cout << "empty\n";
        return;
    }
    Node* node = this->head->next;
    while (node != this->tail->pre) {
        std::cout << node->value->get_value() << "->";
        node = node->next;
    }
    std::cout << node->value->get_value() << std::endl;
}

// 检查是否存在节点
template<typename K, typename V>
bool LRU_Cache<K, V>::is_exist(K key) {
    auto it = cache.find(key);
    return it != cache.end();
}

// 检查节点是否已经过期
template<typename K, typename V>
bool LRU_Cache<K, V>::is_expired(K key) {
    if (is_exist(key)) {
        return cache[key]->is_expired();
    }
    return false;
}

// 获得设置了过期时间的节点数量
template<typename K, typename V>
int LRU_Cache<K, V>::get_current_capacity() {
    return cache.size();
}

// 删除链表中节点
template<typename K, typename V>
void LRU_Cache<K, V>::remove_node(Node* node) {
    node->pre->next = node->next;
    node->next->pre = node->pre;
}

// 将节点加入双向链表末尾
template<typename K, typename V>
void LRU_Cache<K, V>::push_node(Node* node) {
    tail->pre->next = node;
    node->pre = tail->pre;
    node->next = tail;
    tail->pre = node;
}