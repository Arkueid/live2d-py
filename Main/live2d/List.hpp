#pragma once
#include <iostream>

// 获取成员的地址偏移量
#define element_offset(type, member) (unsigned long long)(&((type *)0)->member)
//
#define element_entry(type, member, ptr) (type *)((unsigned long long)ptr - element_offset(type, member))
#define element_node_offset(type, node, key) ((long long)(&((type *)0)->key) - (long long)(&((type *)0)->node))
#define element_node_key(node, offset) *(long long *)((long long)node + offset)

// 链表节点
typedef struct list_node_t
{
    struct list_node_t *prev;
    struct list_node_t *next;
} list_node_t;

// 链表
// 头节点和尾节点不表示数据
typedef struct list_t
{
    list_node_t head; // 头节点
    list_node_t tail; // 尾节点
} list_t;

// 初始化链表
void list_init(list_t *list);

// 在anchor前插入节点node
void list_insert_before(list_node_t *anchor, list_node_t *node);

// 在anchor后插入节点node
void list_insert_after(list_node_t *anchor, list_node_t *node);

// 插入到头节点后
void list_push(list_t *list, list_node_t *node);

// 移除头节点后的节点
list_node_t *list_pop(list_t *list);

// 插入到尾节点前
void list_pushback(list_t *list, list_node_t *node);

// 移除尾节点前的节点
list_node_t *list_popback(list_t *list);

// 查找链表中节点是否存在
bool list_search(list_t *list, list_node_t* node);

// 从链表中删除节点
void list_remove(list_node_t *node);

// 判断链表是否为空
bool list_empty(list_t *list);

// 获得链表长度
size_t list_size(list_t *list);

// 插入并排序
void list_insert_sort(list_t *list, list_node_t *node, int offset);