#ifndef TREEITEMMODEL_H
#define TREEITEMMODEL_H

#include <QAbstractItemModel>
#include "nodeinfo.h"
#include "Values/valuemanager.h"

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

    bool ClearAllEvents();

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
    // 当前节点属于事件条件节点，还是属于事件动作序列节点
    int findBelongToWhichNode(NodeInfo* cur_node);
    // 当前节点是不是顶层条件节点
    bool isEventCondition(NodeInfo* cur_node);
    // 当前节点是不是顶层动作节点（sequence)
    bool isEventActionSeq(NodeInfo* cur_node);

    QStringList* GetEventParamsUIOf(NodeInfo* node);
    QStringList* GetEventParamsLuaOf(NodeInfo* node);

    ValueManager* GetValueManager();

    NodeInfo* FindEventByName(QString ename);
    int FindEventPosByName(QString ename);
    QStringList GetEventNames();
    void UpdateEventName(NodeInfo* evt_node, QString new_name);

    NodeInfo* m_pRootNode;

private:
//    QMap<Level*, ValueManager*> eventsValueManager;

    ValueManager* globalValueManager;

    // 找出parent_node的子节点中，所有与监听事件event_name有关的节点
    void findNodesOpenOrCloseEventIn(NodeInfo* parent_node, QString event_name, QList<NodeInfo*> &node_list);
};

#endif // TREEITEMMODEL_H
