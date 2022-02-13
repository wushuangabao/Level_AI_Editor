#ifndef TREEITEMMODEL_H
#define TREEITEMMODEL_H

#include <QAbstractItemModel>
#include "nodeinfo.h"

class ValueManager;

class TreeItemModel : public QAbstractItemModel
{
public:
    explicit TreeItemModel(QObject *parent = 0);

    // 构造父节点下子节点的索引
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    // 通过子节点索引获取父节点索引
    virtual QModelIndex parent(const QModelIndex &child) const override;
    // 获取父节点下子节点的行数
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    // 获取父节点下子节点列数
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    // 获取节点数据：包括DisplayRole|TextAlignmentRole等
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // 删除节点
    bool deleteNode(NodeInfo* node);
    // 创建新节点
    NodeInfo* createNode(QString str_data, NODE_TYPE eType, NodeInfo* parent = nullptr);

    // 当前节点属于哪个EVENT节点
    NodeInfo* findUppestNodeOf(NodeInfo* cur_node);
    // 当前节点对应哪个ETYPE节点
    NodeInfo* findEvtTypeNodeOf(NodeInfo* cur_node);
    // 当前节点对应哪个顶层条件节点
    NodeInfo* findEvtCondNodeOf(NodeInfo* cur_node);
    // 当前节点对应哪个顶层动作节点（sequence)
    NodeInfo* findEvtActNodeOf(NodeInfo* cur_node);
    // 当前节点是不是顶层条件节点
    bool isEventCondition(NodeInfo* cur_node);
    // 当前节点是不是顶层动作节点（sequence)
    bool isEventActionSeq(NodeInfo* cur_node);

    ValueManager* GetValueManagerOf(NodeInfo* node);

    NodeInfo* m_pRootNode;

private:
    QMap<NodeInfo*, ValueManager*> eventsValueManager;
};

#endif // TREEITEMMODEL_H
