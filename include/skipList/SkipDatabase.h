#pragma once

#include "skipList.h"
#include "LRU.h"
#include <memory>
#include <atomic>
#include <thread>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <stdexcept>

std::string trim(std::string &str) {
    // 去掉头部空格
    str.erase(0, str.find_first_not_of(" \t")); 
    // 去掉尾部空格
    str.erase(str.find_last_not_of(" \t") + 1);
    return str;
}

enum KEY_STATE {
    SET = 0,
    KEYS,
    DEL,
    EXPIRE,
    SHOW,
    CLOSE,
    DUMP
};

// 对stoi中的参数异常进行捕获,避免程序崩溃跳出 
int validStoi(const std::string& str) {
    int time;
    try {
        time = stoi(str);
    } catch(std::invalid_argument) {
        std::cerr << "Invalid_argument\n";
    } catch(std::out_of_range) {
        std::cerr << "Out of range\n";
    } catch(...) {
        std::cerr << "Please input valid expire_time\n";
    }
    return time;
}

// 跳表数据库类
template <typename K, typename V>
class SkipDatabase {
    friend std::string trim(std::string& str);
    typedef Node<K, V> PAGE;
    typedef SkipDatabase<K, V> Skipdb;
    typedef SkipList<K, V> Skip;

public:
    // 构造函数
    SkipDatabase(int maxLevel_, int timeout_, int cycleDeletetime_, int cycleDumptime_, int cap) : 
            maxLevel(maxLevel_), timeout(timeout_), cycleDeletetime(cycleDeletetime_), 
            cycleDumptime(cycleDumptime_),capacity(cap), expiredKey(0), isClose(false), 
            isDump(false), skipList(new Skip(maxLevel_, timeout_)), lru(new LRUCache<K, PAGE*>(capacity)) {
            skipList->loadFile();
            }
    // 析构函数
    ~SkipDatabase() {
        isClose = true;
        Skipdb::dumpFile(this);
    }

    void run();
private:
    void handler(std::vector<std::string>&);
    static void dumpFile(void*);
    static void cycleToDelete(void*);
    void lazyToDelete(K);
    bool setFunc(std::vector<std::string>&);
    bool expireFunc(std::vector<std::string>&);
    bool keysFunc(std::vector<std::string>&);
    bool delFunc(std::vector<std::string>&);
    bool showFunc(std::vector<std::string>&);
    bool dumpFunc(std::vector<std::string>&);
    bool closeFunc(std::vector<std::string>&);
private:
    int maxLevel;
    // 跳表默认超时时间
    int timeout;
    // 周期性删除过期键时间
    int cycleDeletetime;
    // 周期存盘时间
    int cycleDumptime;
    int capacity;
    int expiredKey;
    std::atomic<bool> isClose;
    std::atomic<bool> isDump;
    std::mutex mMutex;
    std::unique_ptr<Skip> skipList;
    // 只存放设置了过期时间的键，指向跳表中节点的指针
    std::unique_ptr<LRUCache<K, PAGE*>> lru;
    static std::unordered_map<std::string, KEY_STATE> ALLSTATE;
};   

// 定义指令哈希表
template<typename K, typename V>
std::unordered_map<std::string, KEY_STATE> SkipDatabase<K, V>::ALLSTATE{
    {"SET", SET}, {"DEL", DEL}, {"EXPIRE", EXPIRE}, {"KEYS", KEYS}, 
    {"SHOW", SHOW}, {"DUMP", DUMP}, {"CLOSE", CLOSE}
};

// 总运行函数
template<typename K, typename V>
void SkipDatabase<K, V>::run() {
    std::thread dumpThread(Skipdb::dumpFile, this);
    std::thread deleteThread(Skipdb::cycleToDelete, this);
    dumpThread.detach();
    deleteThread.detach();
    std::stringstream sstream;
    while (!isClose) {
        std::string line;
        std::getline(std::cin, line);
        sstream << line;
        std::vector<std::string> commands;
        std::string s;
        while (sstream >> s) {
            s = trim(s);
            commands.push_back(s);
        }
        if (commands.size() > 0) {
            // 处理命令
            handler(commands);
        }
        // 清除命令等待下一命令
        sstream.clear();
    }
}


//处理命令
template<typename K, typename V>
void SkipDatabase<K, V>::handler(std::vector<std::string>& commands) {
    std::string command = commands[0];
    std::transform(command.begin(), command.end(), command.begin(), ::toupper);
        if (this->ALLSTATE.find(command) != this->ALLSTATE.end()) {
            auto state = ALLSTATE[command];
            switch (state)
            {
            case SET:
                if (!setFunc(commands)) {
                    std::cout << "Please input SET key value [timeout]\n";
                }
                break;
            case KEYS:
                if (!keysFunc(commands)) {
                    std::cout << "Please input KEYS key\n";
                }
                break;
            case EXPIRE:
                if (!expireFunc(commands)) {
                    std::cout << "Please input EXPIRE key timeout\n";
                }
                break;
            case DEL:
                if (!delFunc(commands)) {
                    std::cout << "Please input DEL key";
                }
                break;
            case SHOW:
                if (!showFunc(commands)) {
                    std::cout << "Please input SHOW ALL/CAP\n";
                }
                break;
            case DUMP:
                if (!dumpFunc(commands)) {
                    std::cout << "Please only input DUMP\n";
                }
                break;
            case CLOSE:
                if (!closeFunc(commands)) {
                    std::cout << "Please only input CLOSE\n";
                }
                break;
            default:
                std::cout << "Invalid command\n";
                break;
            }
        }
        else {
            std::cout << "Invalid command\n";
        }
}

// 周期性存盘
template<typename K, typename V>
void SkipDatabase<K, V>::dumpFile(void* arg) {
    Skipdb* ptr = (Skipdb*)arg;
        while (!ptr->isClose) {
            std::this_thread::sleep_for(std::chrono::seconds(ptr->cycleDumptime));
            ptr->skipList->dumpFile();
        }    
}

// 周期删除过期键
template<typename K, typename V>
void SkipDatabase<K, V>::cycleToDelete(void* arg) {
    Skipdb* ptr = (Skipdb*)arg;
    while (!ptr->isClose) {
        std::this_thread::sleep_for(std::chrono::seconds(ptr->cycleDeletetime));
        // 获得设置了过期时间的键数量
        size_t num = ptr->lru->getCurrentCapacity();
        if (num <= 0) continue;
        // 随机获得删除过期键数量
        int waitToDelete = rand() % num;
        while (waitToDelete--) {
            auto key = ptr->lru->getHeadKey();
            // 如果已经过期
            if (ptr->lru->isExist(key) && ptr->lru->isExpired(key)) {
                std::cout << "cycle_delete" << std::endl;
                ptr->expiredKey++;
                ptr->skipList->deleteElement(key);
                ptr->lru->deleteKey(key);
            }
        }
    }
}

// 惰性删除
template<typename K, typename V>
void SkipDatabase<K, V>::lazyToDelete(K key) {
    // 键已过期
    if (lru->isExpired(key)) {
        this->expiredKey++;
        skipList->deleteElement(key);
        if (lru->isExist(key)) {
            lru->deleteKey(key);
        }
    }
    return;
}

// 添加键
template<typename K, typename V>
bool SkipDatabase<K, V>::setFunc(std::vector<std::string>& commands) {
    if (commands.size() < 3) return false;
    K key(commands[1]);
    V value(commands[2]);
    // 默认不设置超时时间
    skipList->insertElement(key, value);
    if (commands.size() == 4) {
        int time = validStoi(commands[3]);
        if (time > 0) {
            auto node = skipList->setExpireKey(key, time);
            lru->setKey(key, node);
            return true;
        }
    }
    return true;
}

// 设置超时时间
template<typename K, typename V>
bool SkipDatabase<K, V>::expireFunc(std::vector<std::string>& commands) {
        if (commands.size() != 3) return false;
        K key(commands[1]);
        if (skipList->isExistElement(key)) {
            int time = validStoi(commands[2]);
            if (time > 0) {
                // 第三个参数控制是否打印
                auto node = skipList->setExpireKey(key, time);
                lru->setKey(key, node);
                return true;
            }
        }
        return false;
    }

// 查询键
template<typename K, typename V>
bool SkipDatabase<K, V>::keysFunc(std::vector<std::string>& commands) {
    if (commands.size() != 2) return false;
    K key(commands[1]);
    if (skipList->searchElement(key)) {
        lazyToDelete(key);
        return true;
    }
    return false;
}

// 删除键
template<typename K, typename V>
bool SkipDatabase<K, V>::delFunc(std::vector<std::string>& commands) {
        if (commands.size() != 2) return false;
        K key(commands[1]);
        // 若跳表中存在该节点则删除
        if (skipList->isExistElement(key)) {
            skipList->deleteElement(key);
            // 若在 LRU 中也存在则删除该节点
            if (lru->isExist(key)) {
                lru->deleteKey(key);
            }
            return true;
        }
        return false;    
}

// 查询全部
template<typename K, typename V>
bool SkipDatabase<K, V>::showFunc(std::vector<std::string>& commands) {
        if (commands.size() != 2) return false;
        std::string s = commands[1];
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        if (s == "ALL") {
            skipList->displayList();
            return true;
        }
        else if (s == "CAP") {
            int skipdbCap = skipList->size();
            int expireKeys = lru->getCurrentCapacity();
            std::cout << "The Skipdb keys' number is : " << skipdbCap 
                      << " , The expired_map keys' number is: " << expireKeys << std::endl;
            return true;
        }
        return false;        
}

template<typename K, typename V>
bool SkipDatabase<K, V>::dumpFunc(std::vector<std::string>& commands) {
    if (commands.size() != 1) return false;
    return skipList->dumpFile();
}

template<typename K, typename V>
bool SkipDatabase<K, V>::closeFunc(std::vector<std::string>& commands) {
    if (commands.size() != 1)   return false;
    isClose = true;
    return true;
}