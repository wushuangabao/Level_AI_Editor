#ifndef NODEINFO_H
#define NODEINFO_H

#include <QDebug>
#include <QString>
#include <QList>
#include "enumdefine.h"
#include "eventtype.h"

class BaseValueClass;

class NodeInfo
{
public:
    NodeInfo(NodeInfo* p, NODE_TYPE nt, QString str);
    ~NodeInfo();

    void clear();

    // new 一个新的 NodeInfo
    NodeInfo* addNewChild(NODE_TYPE eType, QString str_data);
    NodeInfo* addNewChild_Compare(QString compare_type, QString left_value, QString right_value);

    int getValuesCount();
    QString getValue(int id);
    void addNewValue(CONDITION_OP v);
    bool modifyValue(CONDITION_OP v);
    void addNewValue(QString v);
    bool modifyValue(int id, QString v);
    void clearValues();

    void UpdateText();
    void UpdateEventType(EVENT_TYPE_ID event_id);

    NodeInfo* parent;          // 父节点
    NODE_TYPE type;            // 节点类型
    QString text;              // UI文字
    QList<NodeInfo*> childs;   // 子节点

private:
    void updateCompareText();
    void updateCondionText();

    bool tryAddCondition(NodeInfo *new_node);
    bool tryAddChoice(NodeInfo *new_node);

    void initEventMembers();

    QStringList values;        // 存储一些变量
};

#endif // NODEINFO_H
