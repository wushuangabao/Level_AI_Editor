#ifndef TREEITEMMODEL_CUSTOM_H
#define TREEITEMMODEL_CUSTOM_H

#include "treeitemmodel.h"

class TreeItemModel_Custom : public TreeItemModel
{
    Q_OBJECT

public:
    explicit TreeItemModel_Custom(QObject *parent = nullptr);

    virtual QStringList* GetEventParamsUIOf(NodeInfo* node) override;
    virtual QStringList* GetEventParamsLuaOf(NodeInfo* node) override;

    NodeInfo *AddCustomSequence(const QString& name);
    void UpdateCustActSeqName(NodeInfo* node, const QString& name);

    // 处理节点拖拽
    virtual bool OnMoveNode(NodeInfo* begin_node, NodeInfo* end_node) override;
};

#endif // TREEITEMMODEL_CUSTOM_H
