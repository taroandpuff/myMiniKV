#include <iostream>
#include <chrono>
#include <cstdlib>
#include <time.h>
#include "../include/redBlackTree/redBlackTree.h"

#define TEST_COUNT 100000
RedBlockTree<int, std::string> rbtree;

int main() {
    srand (time(NULL));  
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < TEST_COUNT; i++) {
        rbtree.insert(rand() % TEST_COUNT, "a");
    }

    auto finish = std::chrono::high_resolution_clock::now(); 
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "insert elapsed:" << elapsed.count() << std::endl;

}
