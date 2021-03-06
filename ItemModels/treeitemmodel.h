#ifndef TREEITEMMODEL_H
#define TREEITEMMODEL_H

#include <QMimeData>
#include <QAbstractItemModel>
#include "nodeinfo.h"
#include "Values/valuemanager.h"

class MainWindow;

class TreeItemModel : public QAbstractItemModel
{
public:
    explicit TreeItemModel(QObject *parent = 0);

    MainWindow* getMainWindow();

    void beginResetModel();
    void endResetModel();

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

    bool ClearAllData();

    // 删除节点
    bool deleteNode(NodeInfo* node);
    // 创建新节点
    NodeInfo* createNode(QString str_data, NODE_TYPE eType, NodeInfo* parent = nullptr);

    // 当前节点属于哪个顶层节点
    NodeInfo* findUppestNodeOf(NodeInfo* cur_node);

    virtual QStringList* GetEventParamsUIOf(NodeInfo* node) = 0;
    virtual QStringList* GetEventParamsLuaOf(NodeInfo* node) = 0;

    ValueManager* GetValueManager();

    NodeInfo* FindUppestNodeByName(QString ename);
    int FindUppestNodePosByName(QString ename);
    QStringList GetUppestNodeNames();
    QStringList getEventNames();
    QStringList getCustActSeqNames();

    // 节点拖拽
    virtual bool OnMoveNode(NodeInfo* begin_node, NodeInfo* end_node) = 0;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &obj_node) override;
    virtual Qt::DropActions supportedDropActions() const override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

    NodeInfo* m_pRootNode;

private:
    ValueManager* globalValueManager;
};

#endif // TREEITEMMODEL_H
