#include "treeitemmodel_custom.h"

TreeItemModel_Custom::TreeItemModel_Custom(QObject *parent)
    :  TreeItemModel(parent)
{
    m_pRootNode = NodeInfo::GetRootNode_Custom();
}

QStringList *TreeItemModel_Custom::GetEventParamsUIOf(NodeInfo *node)
{
    Q_UNUSED(node);
    return nullptr;
}

QStringList *TreeItemModel_Custom::GetEventParamsLuaOf(NodeInfo *node)
{
    Q_UNUSED(node);
    return nullptr;
}

NodeInfo* TreeItemModel_Custom::AddCustomSequence(const QString &name)
{
    beginResetModel();
    NodeInfo* node = m_pRootNode->addNewChild(SEQUENCE, name);
    endResetModel();
    return node;
}

void TreeItemModel_Custom::UpdateCustActSeqName(NodeInfo *node, const QString &name)
{
    GetValueManager()->UpdateCustomSeqName(node->text, name);
    node->text = name;
}

bool TreeItemModel_Custom::OnMoveNode(NodeInfo *begin_node, NodeInfo *end_node)
{
    Q_UNUSED(begin_node);
    Q_UNUSED(end_node);
    return false;
}
