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
int valid_stoi(const std::string& str) {
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
class Skip_Database {
    friend std::string trim(std::string& str);
    typedef Node<K, V> PAGE;
    typedef Skip_Database<K, V> Skipdb;
    typedef SkipList<K, V> Skip;

public:
    // 构造函数
    Skip_Database(int max_level_, int timeout_, int cycle_deletetime_, int cycle_dumptime_, int cap) : 
            max_level(max_level_), timeout(timeout_), cycle_deletetime(cycle_deletetime_), 
            cycle_dumptime(cycle_dumptime_),capacity(cap), expired_key(0), is_close(false), 
            is_dump(false), skip_list(new Skip(max_level, timeout)), lru(new LRU_Cache<K, PAGE*>(capacity)) {
            skip_list->load_file();
            }
    // 析构函数
    ~Skip_Database() {
        is_close = true;
        Skipdb::dump_file(this);
    }

    void run();
private:
    void handler(std::vector<std::string>&);
    static void dump_file(void*);
    static void cycle_to_delete(void*);
    void lazy_to_delete(K);
    bool set_func(std::vector<std::string>&);
    bool expire_func(std::vector<std::string>&);
    bool keys_func(std::vector<std::string>&);
    bool del_func(std::vector<std::string>&);
    bool show_func(std::vector<std::string>&);
    bool dump_func(std::vector<std::string>&);
    bool close_func(std::vector<std::string>&);
private:
    int max_level;
    // 跳表默认超时时间
    int timeout;
    // 周期性删除过期键时间
    int cycle_deletetime;
    // 周期存盘时间
    int cycle_dumptime;
    int capacity;
    int expired_key;
    std::atomic<bool> is_close;
    std::atomic<bool> is_dump;
    std::mutex m_mutex;
    std::unique_ptr<Skip> skip_list;
    // 只存放设置了过期时间的键，指向跳表中节点的指针
    std::unique_ptr<LRU_Cache<K, PAGE*>> lru;
    static std::unordered_map<std::string, KEY_STATE> ALLSTATE;
};   

// 定义指令哈希表
template<typename K, typename V>
std::unordered_map<std::string, KEY_STATE> Skip_Database<K, V>::ALLSTATE{
    {"SET", SET}, {"DEL", DEL}, {"EXPIRE", EXPIRE}, {"KEYS", KEYS}, 
    {"SHOW", SHOW}, {"DUMP", DUMP}, {"CLOSE", CLOSE}
};

// 总运行函数
template<typename K, typename V>
void Skip_Database<K, V>::run() {
    std::thread dump_thread(Skipdb::dump_file, this);
    std::thread delete_thread(Skipdb::cycle_to_delete, this);
    dump_thread.detach();
    delete_thread.detach();
    std::stringstream sstream;
    while (!is_close) {
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
void Skip_Database<K, V>::handler(std::vector<std::string>& commands) {
    std::string command = commands[0];
    std::transform(command.begin(), command.end(), command.begin(), ::toupper);
        if (this->ALLSTATE.find(command) != this->ALLSTATE.end()) {
            auto state = ALLSTATE[command];
            switch (state)
            {
            case SET:
                if (!set_func(commands)) {
                    std::cout << "Please input SET key value [timeout]\n";
                }
                break;
            case KEYS:
                if (!keys_func(commands)) {
                    std::cout << "Please input KEYS key\n";
                }
                break;
            case EXPIRE:
                if (!expire_func(commands)) {
                    std::cout << "Please input EXPIRE key timeout\n";
                }
                break;
            case DEL:
                if (!del_func(commands)) {
                    std::cout << "Please input DEL key";
                }
                break;
            case SHOW:
                if (!show_func(commands)) {
                    std::cout << "Please input SHOW ALL/CAP\n";
                }
                break;
            case DUMP:
                if (!dump_func(commands)) {
                    std::cout << "Please only input DUMP\n";
                }
                break;
            case CLOSE:
                if (!close_func(commands)) {
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
void Skip_Database<K, V>::dump_file(void* arg) {
    Skipdb* ptr = (Skipdb*)arg;
        while (!ptr->is_close) {
            std::this_thread::sleep_for(std::chrono::seconds(ptr->cycle_dumptime));
            ptr->skip_list->dump_file();
        }    
}

// 周期删除过期键
template<typename K, typename V>
void Skip_Database<K, V>::cycle_to_delete(void* arg) {
    Skipdb* ptr = (Skipdb*)arg;
    while (!ptr->is_close) {
        std::this_thread::sleep_for(std::chrono::seconds(ptr->cycle_deletetime));
        // 获得设置了过期时间的键数量
        size_t num = ptr->lru->get_current_capacity();
        if (num <= 0) continue;
        // 随机获得删除过期键数量
        int wait_to_delete = rand() % num;
        while (wait_to_delete--) {
            auto key = ptr->lru->get_head_key();
            // 如果已经过期
            if (ptr->lru->is_exist(key) && ptr->lru->is_expired(key)) {
                std::cout << "cycle_delete" << std::endl;
                ptr->expired_key++;
                ptr->skip_list->delete_element(key);
                ptr->lru->delete_key(key);
            }
        }
    }
}

// 惰性删除
template<typename K, typename V>
void Skip_Database<K, V>::lazy_to_delete(K key) {
    // 键已过期
    if (lru->is_expired(key)) {
        this->expired_key++;
        skip_list->delete_element(key);
        if (lru->is_exist(key)) {
            lru->delete_key(key);
        }
    }
    return;
}

// 添加键
template<typename K, typename V>
bool Skip_Database<K, V>::set_func(std::vector<std::string>& commands) {
    if (commands.size() < 3) return false;
    K key(commands[1]);
    V value(commands[2]);
    // 默认不设置超时时间
    skip_list->insert_element(key, value);
    if (commands.size() == 4) {
        int time = valid_stoi(commands[3]);
        if (time > 0) {
            auto node = skip_list->set_expire_key(key, time);
            lru->set_key(key, node);
            return true;
        }
    }
    return true;
}

// 设置超时时间
template<typename K, typename V>
bool Skip_Database<K, V>::expire_func(std::vector<std::string>& commands) {
        if (commands.size() != 3) return false;
        K key(commands[1]);
        if (skip_list->is_exist_element(key)) {
            int time = valid_stoi(commands[2]);
            if (time > 0) {
                // 第三个参数控制是否打印
                auto node = skip_list->set_expire_key(key, time);
                lru->set_key(key, node);
                return true;
            }
        }
        return false;
    }

// 查询键
template<typename K, typename V>
bool Skip_Database<K, V>::keys_func(std::vector<std::string>& commands) {
    if (commands.size() != 2) return false;
    K key(commands[1]);
    if (skip_list->search_element(key)) {
        lazy_to_delete(key);
        return true;
    }
    return false;
}

// 删除键
template<typename K, typename V>
bool Skip_Database<K, V>::del_func(std::vector<std::string>& commands) {
        if (commands.size() != 2) return false;
        K key(commands[1]);
        // 若跳表中存在该节点则删除
        if (skip_list->is_exist_element(key)) {
            skip_list->delete_element(key);
            // 若在 LRU 中也存在则删除该节点
            if (lru->is_exist(key)) {
                lru->delete_key(key);
            }
            return true;
        }
        return false;    
}

// 查询全部
template<typename K, typename V>
bool Skip_Database<K, V>::show_func(std::vector<std::string>& commands) {
        if (commands.size() != 2) return false;
        std::string s = commands[1];
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        if (s == "ALL") {
            skip_list->display_list();
            return true;
        }
        else if (s == "CAP") {
            int skipdb_cap = skip_list->size();
            int expire_keys = lru->get_current_capacity();
            std::cout << "The Skipdb keys' number is : " << skipdb_cap 
                      << " , The expired_map keys' number is: " << expire_keys << std::endl;
            return true;
        }
        return false;        
}

template<typename K, typename V>
bool Skip_Database<K, V>::dump_func(std::vector<std::string>& commands) {
    if (commands.size() != 1) return false;
    return skip_list->dump_file();
}

template<typename K, typename V>
bool Skip_Database<K, V>::close_func(std::vector<std::string>& commands) {
    if (commands.size() != 1)   return false;
    is_close = true;
    return true;
}