#include <iostream>
#include <string>
#include "SkipDatabase.h"

int main() {
    // 跳表最大层级, 默认超时时间(单位为秒), 删除过期键周期时长, 周期性存盘时长, LRU存储数量
    SkipDatabase<std::string, std::string> sk(32, 10, 5, 60, 10);
    sk.run();
}