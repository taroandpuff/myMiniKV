#include "BPlusTree.h"


void runFrame() {
    int k = 0;
    std::cout << "0，输入b+树的阶数k\n" << std::endl;
	std::cout << "k:";
	std::cin >> k;
	BPlusTree* B = new BPlusTree(k);
	while (true) {
		std::cout << "************************\n" << std::endl;
		std::cout << "1，向b+树添加一个元素，key为字符串，data为数字\n" << std::endl;
		std::cout << "2，向b+树删除指定的key关键字\n" << std::endl;
		std::cout << "3，向b+树查找指定key对应的数据\n" << std::endl;
		std::cout << "4，展示b+树\n" << std::endl;
		std::cout << "5，退出程序\n" << std::endl;
		std::cout << "************************\n" << std::endl;
		int opt;
		int data;
		bool succ;
		std::string key;
		std::cin >> opt;
		switch (opt) {
		case 1:
			std::cout << "key:";
			std::cin >> key;
			int data;
			std::cout << "data:";
			std::cin >> data;
			succ = B->add(key, data);
			if (succ) {
				std::cout << "添加成功\n";
			}
			else {
				std::cout << "添加失败\n";
			}
			break;
		case 2:
			std::cout << "key:";
			std::cin >> key;
			succ = B->remove(key);
			if (succ) {
				std::cout << "删除成功\n";
			}
			else {
				std::cout << "删除失败\n";
			}
			break;
		case 3:

			std::cout << "key:";
			std::cin >> key;
			succ = B->find(key, data);
			if (succ) {
				std::cout << data << "\n";
				std::cout << "查找成功\n";
			}
			else {
				std::cout << "查找失败\n";
			}
			break;
		case 4:
			B->showBTree();
			break;
		default:
			return;
		}
	}
}

int main() {
    runFrame();
    return 0;
}