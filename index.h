#ifndef INDEX_H
#define INDEX_H

#include <QString>
#include <QVector>
#include <QPair>

// 定义一个结构体来存储关键字和指针
struct IndexEntry {
    QString key; // 关键字，即 order_date 的值
    int pointer; // 指针，指向数据表中的记录
};

// 定义 B+ 树的节点
struct BPlusTreeNode {
    QVector<QString> keys; // 存储关键字
    QVector<qintptr> pointers; // 存储指针或子节点指针(qintptr保证指针精度)
    bool isLeaf; // 是否是叶子节点
    BPlusTreeNode* next; // 叶子节点的下一个节点(用于范围查询)
    BPlusTreeNode* parent; // 父节点指针
    int degree; // 节点的度

    BPlusTreeNode(int t, bool leaf) {
        degree = t;
        isLeaf = leaf;
        next = nullptr;
        parent = nullptr;
    }
};

// 定义 Index 类
class Index {
public:
    Index(int degree = 3);
    ~Index();

    void buildIndex(const QVector<IndexEntry>& entries);
    int search(const QString& key) const;
    QVector<int> searchRange(const QString& start, const QString& end) const;
    QVector<int> searchCondition(const QString& condition);
    void insert(const QString& key, int pointer);
    void remove(const QString& key);
    void printTree() const;
    void clear();

private:
    BPlusTreeNode* root;
    int degree; // B+树的度

    BPlusTreeNode* findLeaf(const QString& key) const;
    void insertInternal(BPlusTreeNode* node, const QString& key, BPlusTreeNode* child);
    void removeInternal(BPlusTreeNode* node, const QString& key);
    void borrowFromLeft(BPlusTreeNode* node, int idx);
    void borrowFromRight(BPlusTreeNode* node, int idx);
    void merge(BPlusTreeNode* node, int idx);
    void updateParent(BPlusTreeNode* node, const QString& oldKey, const QString& newKey);
    void clearHelper(BPlusTreeNode* node);
    void printTreeHelper(BPlusTreeNode* node, int level) const;
};

#endif // INDEX_H
