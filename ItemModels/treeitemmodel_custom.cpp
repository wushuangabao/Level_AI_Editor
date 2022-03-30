#include "treeitemmodel_custom.h"

TreeItemModel_Custom::TreeItemModel_Custom(QObject *parent)
    :  TreeItemModel(parent)
{
}

//QVariant TreeItemModel_Custom::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    // FIXME: Implement me!
//}

QModelIndex TreeItemModel_Custom::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.internalPointer() == nullptr)
    {
        // 首层节点绑定关系
        if (m_pRootNode->childs.size() > row)
            return createIndex(row, column, m_pRootNode->childs[row]);
    }
    else
    {
        // 其它层节点绑定关系
        if (parent.internalPointer() != nullptr)
        {
            NodeInfo* pNode = reinterpret_cast<NodeInfo*>(parent.internalPointer());
            if (pNode->childs.size() > row)
            {
                return createIndex(row, column, pNode->childs[row]);
            }
        }
    }
    // 根节点索引
    return QModelIndex();
}

QModelIndex TreeItemModel_Custom::parent(const QModelIndex &index) const
{
    // FIXME: Implement me!
}

int TreeItemModel_Custom::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    // FIXME: Implement me!
}

int TreeItemModel_Custom::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    // FIXME: Implement me!
}

QVariant TreeItemModel_Custom::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    return QVariant();
}
