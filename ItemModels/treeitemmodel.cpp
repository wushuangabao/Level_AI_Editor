#include "Values/valuemanager.h"
#include "treeitemmodel.h"

TreeItemModel::TreeItemModel(QObject *parent /*= 0*/)
    : QAbstractItemModel(parent)
{
    m_pRootNode = new NodeInfo(nullptr, NODE_TYPE::INVALID, "rootNode");
    eventsValueManager.clear();
}

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

QModelIndex TreeItemModel::parent(const QModelIndex &child) const
{
    if (child.internalPointer() != nullptr)
       {
           NodeInfo* pNode = reinterpret_cast<NodeInfo*>(child.internalPointer());
           NodeInfo* pParent = pNode->parent;
           if (pParent != nullptr)
           {
               // 根据父节点信息：row/col/node*获取Index
               return createIndex(1, 0/*nCol只有0*/, pParent);
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
//    // 节点内容：左对齐，显示行列号
//    if (role == Qt::TextAlignmentRole)
//        return int(Qt::AlignLeft | Qt::AlignVCenter);
//    else if (role == Qt::DisplayRole)
//        return QString("row=%1,col=%2").arg(index.row()).arg(index.column());
//    else
//        return QVariant();

    if (role == Qt::TextAlignmentRole)
    {
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    }
    else if (role == Qt::DisplayRole)
    {
        if (index.internalPointer() == nullptr)
        {
            return QString("row=%1,col=%2").arg(index.row()).arg(index.column());
        }
        else
        {
            NodeInfo* pNode = reinterpret_cast<NodeInfo*>(index.internalPointer());
            return pNode->text;
        }
    }
    else
    {
        return QVariant();
    }
}

bool TreeItemModel::deleteNode(NodeInfo *node)
{
    if(node == nullptr || node->parent == m_pRootNode || node->parent->parent == m_pRootNode)
        return false;

    if(node->parent->type == COMPARE && node->parent->childs.size() <= 2)
    {
        info("删除失败！比较表达式的子节点必须有2个。");
        return false;
    }

    node->clear();
    for(int i = 0; i < node->parent->childs.size(); i++)
    {
        if(node->parent->childs[i] == node)
        {
            beginResetModel();
            node->parent->childs.removeAt(i);
            node->parent = nullptr;
            delete node;
            endResetModel();
            return true;
        }
    }

    qDebug() << "deleteNode fails." << endl;
    return false;
}

NodeInfo* TreeItemModel::createNode(QString str_data, NODE_TYPE eType, NodeInfo *parent)
{
    if(parent == nullptr)
        parent = m_pRootNode;

    if(parent->type == EVENT)
        parent = parent->childs[2];

    if(parent->type == ETYPE)
        parent = parent->parent->childs[2];

    if(isEventCondition(parent) && (eType != CONDITION && eType != COMPARE))
        parent = parent->parent->childs[2];

    beginResetModel();

    NodeInfo* node = parent->addNewChild(eType, str_data);

    endResetModel();

    // 增加事件节点
    if(parent == m_pRootNode && node != nullptr)
        eventsValueManager.insert(node, new ValueManager(node));

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

NodeInfo *TreeItemModel::findEvtTypeNodeOf(NodeInfo *cur_node)
{
    NodeInfo* ai_node = findUppestNodeOf(cur_node);
    if(ai_node == nullptr)
        return nullptr;
    else
        return ai_node->childs[0];
}

NodeInfo *TreeItemModel::findEvtCondNodeOf(NodeInfo *cur_node)
{
    NodeInfo* ai_node = findUppestNodeOf(cur_node);
    if(ai_node == nullptr)
        return nullptr;
    else
        return ai_node->childs[1];
}

NodeInfo *TreeItemModel::findEvtActNodeOf(NodeInfo *cur_node)
{
    NodeInfo* ai_node = findUppestNodeOf(cur_node);
    if(ai_node == nullptr)
        return nullptr;
    else
        return ai_node->childs[2];
}

bool TreeItemModel::isEventCondition(NodeInfo *cur_node)
{
    if(cur_node == nullptr || cur_node == m_pRootNode || cur_node->parent == m_pRootNode)
        return false;

    if(cur_node->parent->type == EVENT && cur_node->type == CONDITION)
        return true;
    else
        return false;
}

bool TreeItemModel::isEventActionSeq(NodeInfo *cur_node)
{
    if(cur_node == nullptr || cur_node == m_pRootNode || cur_node->parent == m_pRootNode)
        return false;

    if(cur_node->parent->type == EVENT && cur_node->type == SEQUENCE)
        return true;
    else
        return false;
}

ValueManager *TreeItemModel::GetValueManagerOf(NodeInfo* node)
{
    NodeInfo* event_node = findUppestNodeOf(node);

    QMap<NodeInfo*, ValueManager*>::iterator itr = eventsValueManager.find(event_node);
    if(itr != eventsValueManager.end())
    {
        return itr.value();
    }
    else
        return nullptr;
}
