#ifndef TREEITEMMODEL_EVENT_H
#define TREEITEMMODEL_EVENT_H

#include <QMimeData>
#include "treeitemmodel.h"

class TreeItemModel_Event : public TreeItemModel
{
    Q_OBJECT

public:
    explicit TreeItemModel_Event(QObject *parent = nullptr);

    // 当前节点对应哪个ETYPE节点
    NodeInfo* findEvtTypeNodeOf(NodeInfo* cur_node);
    // 当前节点对应哪个顶层条件节点
    NodeInfo* findEvtCondNodeOf(NodeInfo* cur_node);
    // 当前节点对应哪个顶层动作节点（sequence)
    NodeInfo* findEvtActNodeOf(NodeInfo* cur_node);
    // 当前节点属于事件条件节点，还是属于事件动作序列节点
    int findBelongToWhichNode(NodeInfo* cur_node);
    // 当前节点是不是顶层条件节点
    bool isEventCondition(NodeInfo* cur_node);
    // 当前节点是不是顶层动作节点（sequence)
    bool isEventActionSeq(NodeInfo* cur_node);

    virtual QStringList* GetEventParamsUIOf(NodeInfo* node) override;
    virtual QStringList* GetEventParamsLuaOf(NodeInfo* node) override;

    void UpdateEventName(NodeInfo* evt_node, QString new_name);

    // 节点拖拽
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    Qt::DropActions supportedDropActions() const override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    // 找出parent_node的子节点中，所有与监听事件event_name有关的节点
    void findNodesOpenOrCloseEventIn(NodeInfo* parent_node, QString event_name, QList<NodeInfo*> &node_list);
};

#endif // TREEITEMMODEL_EVENT_H
