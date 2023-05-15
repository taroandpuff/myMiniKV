## miniKV 数据库
项目以跳表,B+树,红黑树为基础构成,可以分别使用三种数据结构
### 基于跳表的键值型数据库
   - 数据库定时存盘策略
   - 键值对超时时间的设置
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

### 基于B+树的键值型数据库
   - B+树的简易实现,完成增删查改功能
   - 完成增加时的拆分处理
   - 完成删除时的合并和借入节点处理
   - 完成修改节点后向上修改索引处理
   - 使用泛型编程完成
![bPlusTree](https://github.com/taroandpuff/myMiniKV/assets/109141550/540c5776-ad1a-4c43-907b-c9ca69c91b17)


   
### 基于红黑树的键值型数据库
   - 基于`2-3树` 实现左倾红黑树,完成增删查改功能
   - 完成增加和删除时的平衡处理
   - 使用泛型编程完成

`2-3树` 情况下:
![9](https://github.com/taroandpuff/myMiniKV/assets/109141550/56d55434-1163-4d1d-b84f-093802ceac80)
1. 增加时的拆分操作:
![1](https://github.com/taroandpuff/myMiniKV/assets/109141550/7961c26e-0e27-4cc7-8c14-0e998e3644e1)
2. 删除最小值:
![2](https://github.com/taroandpuff/myMiniKV/assets/109141550/e9710c56-ff06-4e6c-8e0a-81c2ccfe3719)
- 当前节点的左子节点不是2-节点。不用处理
- 当前节点的左子节点是2-节点而它的兄弟节点不是2-节点。处理：从兄弟节点借一个节点。
![4](https://github.com/taroandpuff/myMiniKV/assets/109141550/1f435a22-04b4-4c25-baa9-8a806e372b52)
- 当前节点的左子节点和它的兄弟节点都是2-节点。处理：将左子节点、当前结点中的最小节点和左子节点最近的兄弟节点合并成一个4-节点。
![5](https://github.com/taroandpuff/myMiniKV/assets/109141550/73034226-3a29-487c-8a5b-1bdcd9cc0d2d)
3. 删除任意, 找到节点后, 替换成右子树中的最小值, 然后删除最小值
![6](https://github.com/taroandpuff/myMiniKV/assets/109141550/b5541e28-0911-4a74-9e42-ce4c467e324b)

红黑树情况下:
1. 左旋
![10](https://github.com/taroandpuff/myMiniKV/assets/109141550/c44ae783-06f8-42c6-87a8-814e151b7047)
2. 右旋:
![11](https://github.com/taroandpuff/myMiniKV/assets/109141550/8b0bca01-be8c-48f8-88bb-2f314d519faa)
3. 增加的几种情况:
- 插入情形一(向2-节点中插入新值)
插入一个新值在2-节点，有两种可能：
   - 新值小于老值，那么就新增了一个红色节点在左子树，和单个3-节点等价。
   - 新值大于老值，那么就新增了一个红色节点在右子树，这个时候就要用到上面的LL单旋转，将其旋转为红色左链接。
![12](https://github.com/taroandpuff/myMiniKV/assets/109141550/b0dd4c04-3001-4517-9dfd-c334601a971c)
- 插入情形二(向3-节点中插入新值)
插入一个新值在3-节点，有三中可能：

   - 新值大于原树中的两个值。
![13](https://github.com/taroandpuff/myMiniKV/assets/109141550/bdc9b1f5-5565-4432-b3d0-67025fa9861e)

   - 新值小于用树中的两个值。
  ![14](https://github.com/taroandpuff/myMiniKV/assets/109141550/5076ddf3-a82a-4ed4-ada3-f1b3c852ef69)

   - 新值在原树中的两个值之间。
![15](https://github.com/taroandpuff/myMiniKV/assets/109141550/fc9b5aa1-fddb-4c82-a0d4-04b9c867315a)
- 删除最小值
根据2-3树中的删除最小值，我们一直沿着左节点递归处理节点，我们需要转换的代码有下面这些：

   - 当前节点左子树为空时，表示当前节点就是最小节点。处理：删除该节点。
   - 当前节点的左子节点是2-节点而它的兄弟节点不是2-节点。处理：从兄弟节点借一个节点。
  ![16](https://github.com/taroandpuff/myMiniKV/assets/109141550/0d444de5-a5ba-47a9-a76b-d92fb30e30db)
![17](https://github.com/taroandpuff/myMiniKV/assets/109141550/f84a9d44-0736-4109-9549-5a9364aed1b3)
   - 当前节点的左子节点和它的兄弟节点都是2-节点。处理：将左子节点、当前结点和左子节点最近的兄弟节点合并成一个4-节点。
![18](https://github.com/taroandpuff/myMiniKV/assets/109141550/fe6455ae-b209-455d-8f3b-0191cd11e079)
![19](https://github.com/taroandpuff/myMiniKV/assets/109141550/0e8f3fc5-cb81-4df6-8290-75432447aee9)
- 删除最小值的优化:
   - 我们可以通过观察, 情况2和情况3统一起来处理:
![24](https://github.com/taroandpuff/myMiniKV/assets/109141550/adaff76b-8805-4bbd-b35b-6132a0df413a)
![25](https://github.com/taroandpuff/myMiniKV/assets/109141550/a486bb91-7c4b-448d-b58f-c4bfab33d11e)
- 删除最大值
根据2-3树中的删除最大值，我们一直沿着右节点递归处理节点，我们需要转换的代码有下面这些：
   - 需要把左连接改成右连接:
![21](https://github.com/taroandpuff/myMiniKV/assets/109141550/8a0d9d43-cd81-45c7-b653-c0b3490f6b69)
   - 当前节点右子树为空时，表示当前节点就是最小节点。处理：删除该节点。
   - 当前节点的右子节点是2-节点而它的兄弟节点不是2-节点。处理：从兄弟节点借一个节点。
   ![22](https://github.com/taroandpuff/myMiniKV/assets/109141550/5a38587f-0e5a-4349-b12c-73291f176e06)
![23](https://github.com/taroandpuff/myMiniKV/assets/109141550/2a2990f7-ea2c-4df9-9cf4-da74b6aca494)
   - 当前节点的右子节点和它的兄弟节点都是2-节点。处理：将右子节点、当前结点和右子节点最近的兄弟节点合并成一个4-节点。和删除最小值相同
- 删除最大值优化:
   - 通过观察, 我们可以发现, 可以和删除最小值一样, 统一起来

- 删除任意值, 找到节点后, 替换成右子树中的最小值, 然后删除最小值
![26](https://github.com/taroandpuff/myMiniKV/assets/109141550/4efbf010-28ee-4981-8bc5-8a0cf7923d9d)
