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

void TreeItemModel_Custom::AddCustomSequence(const QString &name)
{
    beginResetModel();
    NodeInfo* node = m_pRootNode->addNewChild(SEQUENCE, name);
    endResetModel();

//    if(node != nullptr)
//        GetValueManager()->AddNewCustomSequence(name, node);
}

void TreeItemModel_Custom::UpdateCustActSeqName(NodeInfo *node, const QString &name)
{
    GetValueManager()->UpdateCustomSeqName(node->text, name);
    node->text = name;
}

