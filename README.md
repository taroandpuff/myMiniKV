## 基于跳表的键值型数据库
- 参考 github 上 [Skiplist-CPP](https://github.com/youngyangyang04/Skiplist-CPP) 的实现，在其基础上新增了几个新功能功能
   - 数据库定时存盘策略
   - 增加键值对超时时间的设置
   - 键值对惰性删除策略
   - 数据库周期删除策略，通过LRU算法根据当前设置过期键数量随机确定删除节点数，并从最近最少使用的节点开始删除
   - 使用智能指针优化内存管理
   
- 使用方法
   - `set K V` 插入/修改键值对
   - `show all` 展示跳表各层级节点
   - `del K` 删除键值对
   - `keys K` 查询键值对
   - `dump` 手动存盘
   - `expire K` 延长/设置超时时间
   - `close` 结束程序
