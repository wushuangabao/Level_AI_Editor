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
    static NodeInfo* GetRootNode_Event();
    static NodeInfo* GetRootNode_Custom();
    ~NodeInfo();

    void clear();
    void operator=(NodeInfo& obj); //深拷贝除子节点之外的其他数据。父节点维持不变，子节点用obj的（原来的子节点全删了）。

    // new 一个新的 NodeInfo
    void FindAndSetNewNodePos(NodeInfo* &parent_node); //找到选择当前节点添加新的动作节点时，实际应该往哪个parent_node的childs中添加节点
    NodeInfo* addNewChildNode_SetVar(QString node_text, int id_var);
    NodeInfo* addNewChild(NODE_TYPE eType, QString str_data);
    NodeInfo* addNewChild_Compare(QString compare_type, QString left_value, QString right_value);
    NodeInfo* addNewChild(NodeInfo* chid_node, int pos = -1); // new一个新的节点，新节点的数据复制chid_node

    bool ContainNodeInChildren(NodeInfo* chid_node);

    int getValuesCount();
    QString getValue(int id);
    void addNewValue(CONDITION_OP v);
    bool modifyValue(CONDITION_OP v);
    void addNewValue(QString v);
    bool modifyValue(int id, QString v);
    void clearValues();

    void UpdateText();
    void UpdateEventType(int event_idx);

    bool IsBreakButNotReturn();
    QString GetVarName_SetVar();

    NodeInfo* parent;          // 父节点
    NODE_TYPE type;            // 节点类型
    QString text;              // UI文字
    QList<NodeInfo*> childs;   // 子节点

private:
    NodeInfo(NodeInfo* p, NODE_TYPE nt, QString str);
    NodeInfo(NodeInfo* &o); // 完全拷贝节点o的数据

    void updateCompareText();
    void updateCondionText();

    bool tryAddCondition(NodeInfo *new_node);
    bool tryAddChoice(NodeInfo *new_node);

    void initEventMembers();

    QStringList values;        // 存储一些变量
    int new_child_pos;         // 新建子节点的位置
};

#endif // NODEINFO_H
