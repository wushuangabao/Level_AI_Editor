#ifndef NODEINFO_H
#define NODEINFO_H

#include <QDebug>
#include <QString>
#include <QList>
#include "enumdefine.h"
#include "eventtype.h"

class NodeInfo
{
public:
    NodeInfo(NodeInfo* p, NODE_TYPE nt, QString str);
    ~NodeInfo();

    void clear();

    NodeInfo* addNewChild(NODE_TYPE eType, QString str_data);

    int getValuesCount();
    QString getValue(int id);
    void addNewValue(CONDITION_OP v);
    bool modifyValue(CONDITION_OP v);
    void addNewValue(QString v);
    bool modifyValue(int id, QString v);
    void clearValues();

    void updateEventType(EVENT_TYPE_ID event_id);
    void updateCompareText();
    void updateCondionText();

    NodeInfo* parent;          // 父节点
    NODE_TYPE type;            // 节点类型
    QString text;              // UI文字
    QList<NodeInfo*> childs;   // 子节点

private:
    bool tryAddCondition(NodeInfo *new_node);
    bool tryAddChoice(NodeInfo *new_node);
    void initEventMembers();

    QStringList values;        // 存储一些变量
};

#endif // NODEINFO_H
