#include "treeitemmodel.h"

TreeItemModel::TreeItemModel(QObject *parent /*= 0*/)
    : QAbstractItemModel(parent)
{
    globalValueManager = ValueManager::GetValueManager();
}

void TreeItemModel::beginResetModel()
{
    QAbstractItemModel::beginResetModel();
}

void TreeItemModel::endResetModel()
{
    QAbstractItemModel::endResetModel();
}

// 用于查找树中子项对应的QModelIndex
QModelIndex TreeItemModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const
{
//    // 创建普通索引
//    return createIndex(row, column);

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

// 用于查找树中父项对应的QModelIndex
QModelIndex TreeItemModel::parent(const QModelIndex &child) const
{
    if (child.internalPointer() != nullptr)
       {
           NodeInfo* pNode = reinterpret_cast<NodeInfo*>(child.internalPointer());
           NodeInfo* pParent = pNode->parent;
           if (pParent != nullptr && pParent->parent != nullptr)
           {
               int row = pParent->parent->childs.indexOf(pParent);
               // 根据父节点信息：row/col/node*获取Index
               return createIndex(row, 0/*nCol只有0*/, pParent);
           }
       }
    // 根节点索引
    return QModelIndex();
}

int TreeItemModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    if (parent.internalPointer() == nullptr)
        {
            // 根节点下的数据行数
            return m_pRootNode->childs.count();
        }
        else
        {
            // 节点下的数据行数
            NodeInfo* pNode = reinterpret_cast<NodeInfo*>(parent.internalPointer());
            return pNode->childs.size();
        }
}

int TreeItemModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    Q_UNUSED(parent);

    // 只有1列
    return 1;
}

QVariant TreeItemModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    if(!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::TextAlignmentRole:
        return int(Qt::AlignLeft | Qt::AlignVCenter);
        break;
    case Qt::DisplayRole:
        if (index.internalPointer() == nullptr)
        {
            return QString("NO DATA");
        }
        else
        {
            NodeInfo* pNode = reinterpret_cast<NodeInfo*>(index.internalPointer());
            return pNode->text;
        }
        break;
    case Qt::ToolTipRole:
    {
        NodeInfo* pNode = reinterpret_cast<NodeInfo*>(index.internalPointer());
        if(pNode != nullptr)
        {
            if(pNode->type == END)
            {
                if(pNode->IsBreakButNotReturn())
                    return "跳出 LOOP";
                else
                    return "退出 事件";
            }
            else
                return getNodeTypeStr(pNode->type);
        }
        else
            return QString("row=%1,col=%2").arg(index.row()).arg(index.column());
    }
        break;
    case Qt::ForegroundRole:
    {
        QColor color(Qt::black);
        NodeInfo* pNode = reinterpret_cast<NodeInfo*>(index.internalPointer());
        if(pNode != nullptr)
        {
            // 顶层节点 及事件下的条件、动作序列、return
            if((pNode->type == CONDITION && pNode->parent->type == EVENT)
               || (pNode->type == SEQUENCE && pNode->parent->type == EVENT)
               || (pNode->type == ENODE) || (pNode->parent == m_pRootNode)
               || (pNode->type == END && pNode->IsBreakButNotReturn() == false))
            {
                color.setRgb(255, 0, 0);
            }
            // 循环节点下的流程控制节点
            else if(pNode->type == LOOP || (pNode->type == SEQUENCE && pNode->parent->type == LOOP)
                    || (pNode->type == END && pNode->IsBreakButNotReturn()))
            {
                color.setRgb(75, 0, 255);
            }
            // if then else
            else if(pNode->text == "if" || pNode->text == "then" || pNode->text == "else"
                    || (pNode->type == CONDITION && pNode->parent->type == CHOICE))
            {
                color.setRgb(0, 127, 0);
            }
        }
        return QBrush(color);
    }
        break;
    case Qt::BackgroundRole:
    {
        QColor color(Qt::white);
        NodeInfo* pNode = reinterpret_cast<NodeInfo*>(index.internalPointer());
        if(pNode != nullptr)
        {
            // 事件及其条件、动作序列、return
//            if(pNode->type == EVENT || (pNode->type == CONDITION && pNode->parent->type == EVENT)
//               || (pNode->type == SEQUENCE && pNode->parent->type == EVENT)
//               || (pNode->type == END && pNode->IsBreakButNotReturn() == false))
//            {
//                color.setRgb(255, 144, 160);
//            }
            // 循环节点下的流程控制节点
//            else if(pNode->type == LOOP || (pNode->type == SEQUENCE && pNode->parent->type == LOOP)
//                    || (pNode->type == END && pNode->IsBreakButNotReturn()))
//            {
//                color.setRgb(152, 209, 244);
//            }
            // if then else
//            else if(pNode->text == "if" || pNode->text == "then" || pNode->text == "else"
//                    || (pNode->type == CONDITION && pNode->parent->type == CHOICE))
//            {
//                color.setRgb(71, 233, 155);
//            }
        }
        return QBrush(color);
    }
        break;
    default:
        return QVariant();
        break;
    }
}

bool TreeItemModel::ClearAllData()
{
    bool ok = true;
    globalValueManager->ClearData();
    for(int i = 0; i < m_pRootNode->childs.size(); i++)
    {
        if(!deleteNode(m_pRootNode->childs[i]))
            ok = false;
        m_pRootNode->childs.clear();
    }
    return ok;
}

bool TreeItemModel::deleteNode(NodeInfo *node)
{
    if(node == nullptr || node->parent == nullptr)
        return false;

    if(node->parent->type == COMPARE && node->parent->childs.size() <= 2)
    {
        info("删除失败！比较表达式的子节点必须有2个。");
        return false;
    }
    if(node->type == SEQUENCE && node->parent == NodeInfo::GetRootNode_Custom() && ValueManager::GetValueManager()->CustomSeqNameIsUsed(node->text))
    {
        info("删除失败！这个动作还在被使用。");
        return false;
    }

    // 如果 value manager 中有node和对应的value，也要删掉
    globalValueManager->OnDeleteNode(node);

    // todo: 如果删除的是Event，那么其他事件中的OPEN_EVENT和CLOSE_EVENT节点会失效

    beginResetModel();

    // 清除节点数据、子节点数据
    node->clear();

    // 然后从父节点中移除
    for(int i = 0; i < node->parent->childs.size(); i++)
    {
        if(node->parent->childs[i] == node)
        {
            node->parent->childs.removeAt(i);
            node->parent = nullptr;
            delete node;
            endResetModel();
            return true;
        }
    }

    qDebug() << "deleteNode fails." << endl;
    endResetModel();
    return false;
}

NodeInfo* TreeItemModel::createNode(QString str_data, NODE_TYPE eType, NodeInfo *parent)
{
    if(parent == nullptr)
        return nullptr;
    if(parent->type == EVENT)
        return nullptr;
    if(parent->type == ETYPE)
        return nullptr;
//    if(isEventCondition(parent) && (eType != CONDITION && eType != COMPARE))
//        parent = parent->parent->childs[2];

    beginResetModel();
    NodeInfo* node = parent->addNewChild(eType, str_data);
    endResetModel();

    return node;
}

NodeInfo *TreeItemModel::findUppestNodeOf(NodeInfo *cur_node)
{
    if(cur_node == nullptr || cur_node->parent == nullptr)
        return nullptr;

    NodeInfo* node = cur_node;
    while(node->parent != m_pRootNode)
    {
        node = node->parent;
    }
    return node;
}

ValueManager *TreeItemModel::GetValueManager()
{
    return globalValueManager;
}

NodeInfo *TreeItemModel::FindUppestNodeByName(QString ename)
{
    int n = m_pRootNode->childs.size();
    for(int i = 0; i < n; i++)
    {
        if(m_pRootNode->childs[i]->text == ename)
        {
            return m_pRootNode->childs[i];
        }
    }
    return nullptr;
}

int TreeItemModel::FindUppestNodePosByName(QString ename)
{
    int n = m_pRootNode->childs.size();
    for(int i = 0; i < n; i++)
    {
        if(m_pRootNode->childs[i]->text == ename)
        {
            return i;
        }
    }
    return -1;
}

QStringList TreeItemModel::GetUppestNodeNames()
{
    QStringList enames;
    int n = m_pRootNode->childs.size();
    for(int i = 0; i < n; i++)
    {
        enames.push_back(m_pRootNode->childs[i]->text);
    }
    return enames;
}

QStringList TreeItemModel::getEventNames()
{
    QStringList enames;
    int n = NodeInfo::GetRootNode_Event()->childs.size();
    for(int i = 0; i < n; i++)
    {
        enames.push_back(NodeInfo::GetRootNode_Event()->childs[i]->text);
    }
    return enames;
}

QStringList TreeItemModel::getCustActSeqNames()
{
    QStringList names;
    int n = NodeInfo::GetRootNode_Custom()->childs.size();
    for(int i = 0; i < n; i++)
    {
        names.push_back(NodeInfo::GetRootNode_Custom()->childs[i]->text);
    }
    return names;
}
