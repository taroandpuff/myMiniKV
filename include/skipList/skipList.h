#pragma once
/* ************************************************************************
> Description:   
    由于是模板类，因此声明和定义在同一个头文件
    假设原始链表有 n 个结点，那么索引的层级就是 log(n)-1(不包括原始链表)
    因为在每一层的访问次数是常量，因此查找结点的平均时间复杂度是O（logn）
    比起常规的查找方式，也就是线性依次访问链表节点的方式，效率要高得多
    基于链表的优化增加了额外的空间开销
    假设原始链表有 n 个结点，那么各层索引的结点总数是 n/2 + n/4 + n/8 + n/16 + ... + 2，约等于n。
    因此优化之后的数据结构所占空间，是原来的2倍。这是典型的以空间换时间的做法
 ************************************************************************/
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <stdlib.h>
#include <list>
#include <unistd.h>

#define STORE_FILE "../store/dumpFile.txt"
#define AOF_FILE "../store/append_on_file.txt"

std::mutex mtx;
std::string delimiter = ":";
typedef std::chrono::seconds SECOND;
typedef std::chrono::high_resolution_clock Clock;
typedef Clock::time_point TimeStamp;

// 跳表节点模板
template <typename K, typename V>
class Node {
public:
    // 节点定义
    Node() {}
    Node(K k, V v, int, int);
    ~Node();

    // 节点操作
    K getKey() const;
    V getValue() const;
    void setValue(V);

    // 超时设置
    bool isSetExpire() const {
        return this->setExpire;
    }
    bool isExpired() const;
    time_t getExpireSystime() const;
    TimeStamp getExpireTimeStamp() const;
    void setExpireTime(int);
    void expandExpireTime(int);

    // 输出当前节点情况
    void print() const;

    // 指向每层(所在层)所连接的下一个节点
    Node<K, V> **forward;

    // 节点层级(所在层级)
    int nodeLevel;

private:
    K key;
    V value;
    TimeStamp expireTime;
    bool setExpire;
};

// 根据 timeout 设置超时时间
template <typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level, int timeout): key(k), value(v), nodeLevel(level) {
    if (timeout > 0) {
        this->expireTime = Clock::now() + SECOND(timeout);
        this->setExpire = true;
    } else {
        this->expireTime = Clock::now();
        this->setExpire = false;
    }

    // level加1, 因为index从0到level
    this->forward = new Node<K, V>* [level + 1];

    // 初始化forward为0
    memset(this->forward, 0, sizeof(Node<K, V>*) * (level + 1));
} 

//析构函数
template <typename K, typename V>
Node<K, V>::~Node() {
    delete[] forward;
}

// 获取key
template <typename K, typename V>
K Node<K, V>::getKey() const {
    return key;
}

// 获取value
template <typename K, typename V>
V Node<K, V>::getValue() const {
    return value;
}


// 判断是否超时
template <typename K, typename V>
bool Node<K, V>::isExpired() const {
    if (this->setExpire) {
        if (std::chrono::duration_cast<SECOND>(this->getExpireTimeStamp() - Clock::now()).count() <= 0) {
            return true;
        }
    }
    return false;
} 

// 更新value
template<typename K, typename V>
void Node<K, V>::setValue(V value) {
    this->value = value;
}

// 获取超时时间
template<typename K, typename V>
TimeStamp Node<K, V>::getExpireTimeStamp() const {
    return this->expireTime;
}

template<typename K, typename V>
time_t Node<K, V>::getExpireSystime() const {
    time_t time = std::chrono::system_clock::to_time_t(this->expireTime);
    return time;
}

// 添加超时时间设置
template<typename K, typename V>
void Node<K, V>::setExpireTime(int timeout) {
    this->setExpire = true;
    this->expireTime += SECOND(timeout);
}

// 延长超时时间
template <typename K, typename V>
void Node<K, V>::expandExpireTime(int timeout) {
    if (this->setExpire) {
        this->expireTime += SECOND(timeout);
    }
}

// 打印节点情况
template<typename K, typename V>
void Node<K, V>::print() const {
    std::string outputs("The Node is ");
    outputs += this->getKey() + " : " + this->getValue();
    if (this->isSetExpire()) {
        char buf[50] = "-> expireTime : ";
        time_t expireTime = this->getExpireSystime();
        strftime(buf + strlen(buf), sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&expireTime));
        outputs += buf;
    }
    std::cout << outputs << std::endl;
}

// 跳表类模板
template <typename K, typename V>
class SkipList {
public:
    SkipList(int, int);
    ~SkipList();

    int getRandomLevel();

    Node<K, V> *createNode(K, V, int, bool);

    // 展示节点
    void displayList();

    // 节点操作
    int insertElement(K, V);
    bool is_exist_element(K);
    bool search_element(K);
    void delete_element(K);
    Node<K, V>* set_expire_key(K, int);

    // 加载文件
    bool dump_file();
    void load_file();
    int size();

private:
    // 通过字符串读取节点值
    void getKey_value_from_string(const std::string &str, std::string *key, std::string *value);
    bool is_valid_string(const std::string &str);
    // 默认超时时间
    int timeout;
    // 最大 level限制
    int _max_level;
    // 当前的level
    int _skip_list_level;

    // 指向头节点指针
    Node<K, V>* _header;

    // 用于读取和保存数据库
    std::ofstream _file_writer;
    std::ifstream _file_reader;


    // 跳表元素数量
    int _element_count;
};

// 创建新节点
template <typename K, typename V>
Node<K, V>* SkipList<K, V>::createNode(const K k, const V v, int level, bool setExpire) {
    int timeout = -1;
    if (setExpire) {
        timeout = this->timeout + rand() % 1800;
    }
    Node<K, V> *node = new Node<K, V>(k, v, level, timeout);
    return node;
}

// 插入节点
/* 
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                        100
                 |
                 |                        insert +----+
level 3         1+---------->10+---------------> | 50 |          70       100
                                                 |    |
                                                 |    |
level 2         1            10         30       | 50 |          70       100
                                                 |    |
                                                 |    |
level 1         1    4       10         30       | 50 |          70       100
                                                 |    |
                                                 |    |
level 0         1    4    9  10         30   40  | 50 |  60      70       100
                                                 +----+

*/
template <typename K, typename V>
int SkipList<K, V>::insertElement(const K key, const V value) {
    mtx.lock();
    // 获取头节点
    Node<K, V>* current = this->_header;
    // update用于储存将要用于操作的节点指针数组, 每层最多一个指针节点,所以最大值为_max_level+1
    Node<K, V>* update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

    // 从最高层开始寻找
    for (int i = _skip_list_level; i >= 0; i--) {
        // 当下一节点为空或者key大于插入值的key时,进入下一层,继续寻找
        while (current->forward[i] != NULL && current->forward[i]->getKey() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    current = current->forward[0];
    // 如果成功找到节点
    if (current != NULL && current->getKey() == key) {
        current->setValue(value);
        std::cout << "The node is exists, but has been changed" << std::endl;
        mtx.unlock();
        return 1;
    }

    // 如果没有找到值相同的节点, 进行插入操作
    if (current == NULL || current->getKey() != key) {
        // 获取随机层级高度
        int random_level = getRandomLevel();

        // 如果random_level大于当前最大层级高度
        if (random_level > _skip_list_level) {
            for (int i = _skip_list_level + 1; i <= random_level; i++) {
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }

        // 创建节点
        Node<K, V> *inserted_node = createNode(key, value, random_level, false);

        // 插入节点
        for (int i = 0; i <= random_level; i++) {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }        
        std::cout << "Successfully inserted key" << std::endl;
        _element_count++;
    }
    mtx.unlock();
    return 0;
}

// 展示跳表
template <typename K, typename V>
void SkipList<K, V>::displayList() {
        std::cout << "\n*****Skip List*****"
              << "\n";
        for (int i = 0; i <= _skip_list_level; i++) {
            Node<K, V> *node = this->_header->forward[i];
            if (node == nullptr && i == 0) {
                std::cout << "empty\n";
                break;
            }
            std::cout << "level" << i << ": ";
            while (node != NULL) {
                node->print();
                node = node->forward[i];
            }
            std::cout << std::endl;
        }
}

// 保存跳表数据(只保存最低层节点)
template <typename K, typename V>
bool SkipList<K, V>::dump_file() {
    std::cout << "dump_file-------------------------" << std::endl;
    _file_writer.open(STORE_FILE);

    if (_file_writer) {
        Node<K, V> *node = this->_header->forward[0];

        while (node != NULL) {
            // 如果节点未超时
            if (!node->isExpired()) {
                _file_writer << node->getKey() << " : " << node->getValue() << '\n';
            } 
            node->print();
            node = node->forward[0];
        }
        _file_writer.flush();
        _file_writer.close();
        return true;
    } else {
        char* path;
        std::cout << "The file open failed\n";
        path = get_current_dir_name();
        std::cout << "the work path is" << path << std::endl;
        return false;
    }
}

// 加载数据
template <typename K, typename V>
void SkipList<K, V>::load_file() {
    _file_reader.open(STORE_FILE);
    std::cout << "load_file----------------------------" << std::endl;
    std::string line;
    std::string *key = new std::string();
    std::string *value = new std::string();
    while (getline(_file_reader, line)) {
        getKey_value_from_string(line, key, value);
        if (key->empty() || value->empty()) {
            continue;
        }
        insertElement(*key, *value);
        std::cout << "key: " << *key << " value: " << *value << std::endl;
    }
    _file_reader.close();    
}

// 获取当前跳表的节点数
template <typename K, typename V>
int SkipList<K, V>::size() {
    return _element_count;
}

template <typename K, typename V>
void SkipList<K, V>::getKey_value_from_string(const std::string &str, std::string *key, std::string *value) {
    if (!is_valid_string(str)) return;
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.length());
}

template <typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string &str) {
    if (str.empty()) {
        return false;
    }
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
}

// 删除节点
template <typename K, typename V>
void SkipList<K, V>::delete_element(K key) {
    mtx.lock();
    Node<K, V> *current = this->_header;
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

    // 从最高点开始查找
    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] != NULL && current->forward[i]->getKey() < key) {
            current = current->forward[i];            
        }
        update[i] = current;
    }

    current = current->forward[0];
    if (current != NULL && current->getKey() == key) {
        // 从最低点开始删除
        for (int i = 0; i <= _skip_list_level; i++) {
            // 如果该层没有目标节点,则已经删除完
            if (update[i]->forward[i] != current) {
                break;
            }
            update[i]->forward[i] = current->forward[i];
        }
        delete current;
        current = nullptr;
        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == 0) {
            --_skip_list_level;            
        }
        std::cout << "Successfully deleted key " << key << std::endl;
        _element_count--;
    }
    mtx.unlock();
    return;
}

// 查找节点
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/
template <typename K, typename V>
bool SkipList<K, V>::search_element(K key) {
    Node<K, V> *current = _header;
    
    // 从最高层开始找
    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] && current->forward[i]->getKey() < key) {
            current = current->forward[i];        
        }
    }

    current = current->forward[0];

    if (current != NULL && current->getKey() == key) {
        current->print();
        return true;
    }

    std::cout << "Not Found Key: " << key << std::endl;
    return false;
} 

// 查看键是否存在
template <typename K, typename V>
bool SkipList<K, V>::is_exist_element(K key) {
    Node<K, V> *current = _header;

    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] != NULL && current->forward[i]->getKey() < key) {
            current = current->forward[i];
        }
    }

    current = current->forward[0];

    if (current != NULL && current->getKey() == key) {
        return true;
    }
    return false;
}

// 设置超时
template <typename K, typename V>
Node<K, V>* SkipList<K, V>::set_expire_key(K key, int time) {
    Node<K, V> *current = _header;

    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] && current->forward[i]->getKey() < key) {
            current = current->forward[i];
        }
    }
    current = current->forward[0];
    
    if (current != NULL && current->getKey() == key) {
        current->setExpireTime(this->timeout);
        current->expandExpireTime(time);
        current->print();
        return current;
    }
    std::cout << "Not Found Key: " << key << std::endl;
    return nullptr;
}

// 构造函数
template <typename K, typename V>
SkipList<K, V>::SkipList(int max_level, int timeout_) {
    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;
    this->timeout =  timeout_;

    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level, -1);
}
// 析构函数
template <typename K, typename V>
SkipList<K, V>::~SkipList()
{

    if (_file_writer.is_open())
    {
        _file_writer.close();
    }
    if (_file_reader.is_open())
    {
        _file_reader.close();
    }
    delete _header;
    _header = nullptr;
}

// 获取随机层数
template <typename K, typename V>
int SkipList<K, V>::getRandomLevel()
{

    int k = 1;
    while (rand() % 2)
    {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
}
