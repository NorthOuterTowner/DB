#include "index.h"
#include <algorithm>
#include <QDebug>

Index::Index(int t) : degree(t), root(nullptr) {}

Index::~Index() {
    clear();
}

void Index::clear() {
    clearHelper(root);
    root = nullptr;
}

void Index::clearHelper(BPlusTreeNode* node) {
    if (node) {
        if (!node->isLeaf) {
            for (auto ptr : node->pointers) {
                clearHelper(reinterpret_cast<BPlusTreeNode*>(ptr));
            }
        }
        delete node;
    }
}

void Index::buildIndex(const QVector<IndexEntry>& entries) {
    clear();  // 清除现有索引
    for (const auto& entry : entries) {
        insert(entry.key, entry.pointer);
    }
}

BPlusTreeNode* Index::findLeaf(const QString& key) const {
    if (!root) return nullptr;

    BPlusTreeNode* current = root;
    while (!current->isLeaf) {
        int i = 0;
        while (i < current->keys.size() && key >= current->keys[i]) {
            i++;
        }
        current = reinterpret_cast<BPlusTreeNode*>(current->pointers[i]);
    }
    return current;
}

int Index::search(const QString& key) const {
    if (!root) return -1;

    BPlusTreeNode* leaf = findLeaf(key);
    for (int i = 0; i < leaf->keys.size(); ++i) {
        if (leaf->keys[i] == key) {
            return leaf->pointers[i];
        }
    }
    return -1;
}

QVector<int> Index::searchRange(const QString& start, const QString& end) const {
    QVector<int> results;
    if (!root) return results;

    BPlusTreeNode* leaf = findLeaf(start);
    while (leaf) {
        for (int i = 0; i < leaf->keys.size(); ++i) {
            if (leaf->keys[i] >= start && leaf->keys[i] <= end) {
                results.push_back(leaf->pointers[i]);
            }
            if (leaf->keys[i] > end) {
                return results;
            }
        }
        leaf = leaf->next;
    }
    return results;
}

void Index::insert(const QString& key, int pointer) {
    if (!root) {
        root = new BPlusTreeNode(degree, true);
        root->keys.push_back(key);
        root->pointers.push_back(pointer);
        return;
    }

    BPlusTreeNode* leaf = findLeaf(key);
    auto pos = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    int index = pos - leaf->keys.begin();

    leaf->keys.insert(pos, key);
    leaf->pointers.insert(leaf->pointers.begin() + index, pointer);

    if (leaf->keys.size() > 2 * degree - 1) {
        BPlusTreeNode* newLeaf = new BPlusTreeNode(degree, true);
        int splitPos = degree;

        newLeaf->keys = QVector<QString>(leaf->keys.begin() + splitPos, leaf->keys.end());
        newLeaf->pointers = QVector<qintptr>(leaf->pointers.begin() + splitPos, leaf->pointers.end());

        leaf->keys.resize(splitPos);
        leaf->pointers.resize(splitPos);

        newLeaf->next = leaf->next;
        leaf->next = newLeaf;
        newLeaf->parent = leaf->parent;

        QString splitKey = newLeaf->keys[0];

        if (!leaf->parent) {
            BPlusTreeNode* newRoot = new BPlusTreeNode(degree, false);
            newRoot->keys.push_back(splitKey);
            newRoot->pointers.push_back(reinterpret_cast<qintptr>(leaf));
            newRoot->pointers.push_back(reinterpret_cast<qintptr>(newLeaf));
            root = newRoot;
            leaf->parent = newRoot;
            newLeaf->parent = newRoot;
        } else {
            insertInternal(leaf->parent, splitKey, newLeaf);
        }
    }
}

void Index::insertInternal(BPlusTreeNode* node, const QString& key, BPlusTreeNode* child) {
    auto pos = std::lower_bound(node->keys.begin(), node->keys.end(), key);
    int index = pos - node->keys.begin();

    node->keys.insert(pos, key);
    node->pointers.insert(node->pointers.begin() + index + 1, reinterpret_cast<qintptr>(child));
    child->parent = node;

    if (node->keys.size() > 2 * degree - 1) {
        BPlusTreeNode* newNode = new BPlusTreeNode(degree, false);
        int splitPos = degree - 1;

        QString splitKey = node->keys[splitPos];

        newNode->keys = QVector<QString>(node->keys.begin() + splitPos + 1, node->keys.end());
        newNode->pointers = QVector<qintptr>(node->pointers.begin() + splitPos + 1, node->pointers.end());

        node->keys.resize(splitPos);
        node->pointers.resize(splitPos + 1);

        for (int i = 0; i < newNode->pointers.size(); ++i) {
            BPlusTreeNode* childNode = reinterpret_cast<BPlusTreeNode*>(newNode->pointers[i]);
            childNode->parent = newNode;
        }

        if (!node->parent) {
            BPlusTreeNode* newRoot = new BPlusTreeNode(degree, false);
            newRoot->keys.push_back(splitKey);
            newRoot->pointers.push_back(reinterpret_cast<qintptr>(node));
            newRoot->pointers.push_back(reinterpret_cast<qintptr>(newNode));
            root = newRoot;
            node->parent = newRoot;
            newNode->parent = newRoot;
        } else {
            insertInternal(node->parent, splitKey, newNode);
        }
    }
}

void Index::remove(const QString& key) {
    if (!root) return;

    BPlusTreeNode* leaf = findLeaf(key);
    auto it = std::find(leaf->keys.begin(), leaf->keys.end(), key);
    if (it == leaf->keys.end()) return;

    int index = it - leaf->keys.begin();
    leaf->keys.remove(index);
    leaf->pointers.remove(index);

    if (leaf == root) {
        if (leaf->keys.isEmpty()) {
            delete root;
            root = nullptr;
        }
        return;
    }

    if (leaf->keys.size() >= degree - 1) return;

    BPlusTreeNode* parent = leaf->parent;
    int idx = 0;
    while (idx < parent->pointers.size() &&
           reinterpret_cast<BPlusTreeNode*>(parent->pointers[idx]) != leaf) {
        idx++;
    }

    if (idx > 0) {
        BPlusTreeNode* leftSibling = reinterpret_cast<BPlusTreeNode*>(parent->pointers[idx - 1]);
        if (leftSibling->keys.size() >= degree) {
            borrowFromLeft(leaf, idx);
            return;
        }
    }

    if (idx < parent->pointers.size() - 1) {
        BPlusTreeNode* rightSibling = reinterpret_cast<BPlusTreeNode*>(parent->pointers[idx + 1]);
        if (rightSibling->keys.size() >= degree) {
            borrowFromRight(leaf, idx);
            return;
        }
    }

    if (idx > 0) {
        merge(leaf, idx - 1);
    } else {
        merge(leaf, idx);
    }
}

void Index::borrowFromLeft(BPlusTreeNode* node, int idx) {
    BPlusTreeNode* leftSibling = reinterpret_cast<BPlusTreeNode*>(node->parent->pointers[idx - 1]);

    node->keys.prepend(leftSibling->keys.last());
    node->pointers.prepend(leftSibling->pointers.last());

    leftSibling->keys.removeLast();
    leftSibling->pointers.removeLast();

    node->parent->keys[idx - 1] = node->keys[0];
}

void Index::borrowFromRight(BPlusTreeNode* node, int idx) {
    BPlusTreeNode* rightSibling = reinterpret_cast<BPlusTreeNode*>(node->parent->pointers[idx + 1]);

    node->keys.append(rightSibling->keys.first());
    node->pointers.append(rightSibling->pointers.first());

    rightSibling->keys.removeFirst();
    rightSibling->pointers.removeFirst();

    node->parent->keys[idx] = rightSibling->keys[0];
}

void Index::merge(BPlusTreeNode* node, int idx) {
    BPlusTreeNode* parent = node->parent;
    BPlusTreeNode* leftNode = reinterpret_cast<BPlusTreeNode*>(parent->pointers[idx]);
    BPlusTreeNode* rightNode = reinterpret_cast<BPlusTreeNode*>(parent->pointers[idx + 1]);

    QString keyToRemove = parent->keys[idx];

    leftNode->keys += rightNode->keys;
    leftNode->pointers += rightNode->pointers;
    leftNode->next = rightNode->next;

    parent->keys.remove(idx);
    parent->pointers.remove(idx + 1);
    delete rightNode;

    if (parent == root && parent->keys.isEmpty()) {
        root = leftNode;
        leftNode->parent = nullptr;
        delete parent;
    } else if (parent->keys.size() < degree - 1) {
        removeInternal(parent, keyToRemove);
    }
}

void Index::removeInternal(BPlusTreeNode* node, const QString& key) {
    auto it = std::find(node->keys.begin(), node->keys.end(), key);
    if (it == node->keys.end()) return;

    int index = it - node->keys.begin();
    node->keys.remove(index);
    node->pointers.remove(index + 1);

    if (node == root) {
        if (node->keys.isEmpty()) {
            if (!node->isLeaf) {
                root = reinterpret_cast<BPlusTreeNode*>(node->pointers[0]);
                root->parent = nullptr;
            } else {
                root = nullptr;
            }
            delete node;
        }
        return;
    }

    if (node->keys.size() >= degree - 1) return;

    BPlusTreeNode* parent = node->parent;
    int idx = 0;
    while (idx < parent->pointers.size() &&
           reinterpret_cast<BPlusTreeNode*>(parent->pointers[idx]) != node) {
        idx++;
    }

    if (idx > 0) {
        BPlusTreeNode* leftSibling = reinterpret_cast<BPlusTreeNode*>(parent->pointers[idx - 1]);
        if (leftSibling->keys.size() >= degree) {
            borrowFromLeft(node, idx);
            return;
        }
    }

    if (idx < parent->pointers.size() - 1) {
        BPlusTreeNode* rightSibling = reinterpret_cast<BPlusTreeNode*>(parent->pointers[idx + 1]);
        if (rightSibling->keys.size() >= degree) {
            borrowFromRight(node, idx);
            return;
        }
    }

    if (idx > 0) {
        merge(node, idx - 1);
    } else {
        merge(node, idx);
    }
}

QVector<int> Index::searchCondition(const QString& condition) {
    QVector<int> results;

    if (!root) {
        return results; // 如果树为空，直接返回空结果
    }

    QString key;
    QString op;

    // 解析条件字符串
    if (condition.contains("<=")) {
        op = "<=";
        key = condition.mid(2);
    } else if (condition.contains(">=")) {
        op = ">=";
        key = condition.mid(2);
    } else if (condition.contains("<")) {
        op = "<";
        key = condition.mid(1);
    } else if (condition.contains(">")) {
        op = ">";
        key = condition.mid(1);
    } else if (condition.contains("=")) {
        op = "=";
        key = condition.mid(1);
    } else {
        qDebug() << "Invalid condition format";
        return results;
    }

    // 找到第一个叶子节点
    BPlusTreeNode* leaf = root;
    while (!leaf->isLeaf) {
        leaf = reinterpret_cast<BPlusTreeNode*>(leaf->pointers[0]);
    }

    // 根据条件进行范围查询
    if (op == "=") {
        // 精确匹配
        while (leaf) {
            for (int i = 0; i < leaf->keys.size(); ++i) {
                if (leaf->keys[i] == key) {
                    results.push_back(leaf->pointers[i]);
                }
            }
            leaf = leaf->next;
        }
    } else if (op == "<") {
        // 小于
        while (leaf) {
            for (int i = 0; i < leaf->keys.size(); ++i) {
                if (leaf->keys[i] < key) {
                    results.push_back(leaf->pointers[i]);
                } else {
                    break; // 由于叶子节点有序，一旦大于等于key，后续都不符合条件
                }
            }
            leaf = leaf->next;
        }
    } else if (op == ">") {
        // 大于
        while (leaf) {
            for (int i = 0; i < leaf->keys.size(); ++i) {
                if (leaf->keys[i] > key) {
                    results.push_back(leaf->pointers[i]);
                }
            }
            leaf = leaf->next;
        }
    } else if (op == "<=") {
        // 小于等于
        while (leaf) {
            for (int i = 0; i < leaf->keys.size(); ++i) {
                if (leaf->keys[i] <= key) {
                    results.push_back(leaf->pointers[i]);
                } else {
                    break;
                }
            }
            leaf = leaf->next;
        }
    } else if (op == ">=") {
        // 大于等于
        while (leaf) {
            for (int i = 0; i < leaf->keys.size(); ++i) {
                if (leaf->keys[i] >= key) {
                    results.push_back(leaf->pointers[i]);
                }
            }
            leaf = leaf->next;
        }
    }

    return results;
}

void Index::printTree() const {
    if (!root) {
        qDebug() << "Empty tree";
        return;
    }

    qDebug() << "Tree Structure:";
    printTreeHelper(root, 0);

    qDebug() << "\nSequential Access:";
    BPlusTreeNode* leaf = root;//找到第一个叶节点head
    while (!leaf->isLeaf) {
        leaf = reinterpret_cast<BPlusTreeNode*>(leaf->pointers[0]);
    }

    QString result;
    while (leaf) {
        for (const QString& key : leaf->keys) {
            result += key + " ";
        }
        result = result.trimmed() + ";";
        leaf = leaf->next;
    }
    if (!result.isEmpty()) {
        result.chop(1);
    }
    qDebug() << result;
}

void Index::printTreeHelper(BPlusTreeNode* node, int level) const {
    if (!node) return;

    QString levelStr = QString("Level %1: ").arg(level);
    QString nodeStr;

    for (const QString& key : node->keys) {
        nodeStr += key + " ";
    }
    nodeStr = nodeStr.trimmed();

    qDebug() << levelStr + nodeStr + (node->isLeaf ? " (leaf)" : " (internal)");

    if (!node->isLeaf) {
        for (const auto& ptr : node->pointers) {
            printTreeHelper(reinterpret_cast<BPlusTreeNode*>(ptr), level + 1);
        }
    }
}
