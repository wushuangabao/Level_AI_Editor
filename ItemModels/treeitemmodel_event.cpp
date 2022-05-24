#include "../mainwindow.h"
#include "treeitemmodel_event.h"

TreeItemModel_Event::TreeItemModel_Event(QObject *parent)
    : TreeItemModel(parent)
{
    m_pRootNode = NodeInfo::GetRootNode_Event();
}

NodeInfo *TreeItemModel_Event::findEvtTypeNodeOf(NodeInfo *cur_node)
{
    NodeInfo* ai_node = findUppestNodeOf(cur_node);
    if(ai_node == nullptr)
        return nullptr;
    else
        return ai_node->childs[0]->childs[0];
}

NodeInfo *TreeItemModel_Event::findEvtCondNodeOf(NodeInfo *cur_node)
{
    NodeInfo* ai_node = findUppestNodeOf(cur_node);
    if(ai_node == nullptr)
        return nullptr;
    else
        return ai_node->childs[1];
}

NodeInfo *TreeItemModel_Event::findEvtActNodeOf(NodeInfo *cur_node)
{
    NodeInfo* ai_node = findUppestNodeOf(cur_node);
    if(ai_node == nullptr)
        return nullptr;
    else
        return ai_node->childs[2];
}

int TreeItemModel_Event::findBelongToWhichNode(NodeInfo *cur_node)
{
    if(cur_node == nullptr || cur_node == m_pRootNode || cur_node->parent == m_pRootNode)
        return -1;

    NodeInfo* node = cur_node;
    while(node->parent != m_pRootNode)
    {
        if(isEventCondition(node))
        {
            return 1; //node是事件条件的子节点
        }
        else if(isEventActionSeq(node))
        {
            return 2; //node是动作队列的子节点
        }
        node = node->parent;
    }

    return 0;
}

bool TreeItemModel_Event::isEventCondition(NodeInfo *cur_node)
{
    if(cur_node == nullptr || cur_node == m_pRootNode || cur_node->parent == m_pRootNode)
        return false;

    if(cur_node->parent->type == EVENT && cur_node->type == CONDITION)
        return true;
    else
        return false;
}

bool TreeItemModel_Event::isEventActionSeq(NodeInfo *cur_node)
{
    if(cur_node == nullptr || cur_node == m_pRootNode || cur_node->parent == m_pRootNode)
        return false;

    if(cur_node->parent->type == EVENT && cur_node->type == SEQUENCE)
        return true;
    else
        return false;
}

QStringList *TreeItemModel_Event::GetEventParamsUIOf(NodeInfo *node)
{
    NodeInfo* event_node = findUppestNodeOf(node);
    MY_ASSERT(event_node != nullptr);

    return GetValueManager()->GetEventParamsUI(event_node);
}

QStringList *TreeItemModel_Event::GetEventParamsLuaOf(NodeInfo *node)
{
    NodeInfo* event_node = findUppestNodeOf(node);
    MY_ASSERT(event_node != nullptr);

    return GetValueManager()->GetEventParamsLua(event_node);
}

void TreeItemModel_Event::UpdateEventName(NodeInfo *evt_node, QString new_name)
{
    QString old_name = evt_node->text;
    evt_node->text = new_name;

    // 更新相关的 OPEN_EVENT 和 CLOSE_EVENT 节点
    QList<NodeInfo*> node_list;
    findNodesOpenOrCloseEventIn(m_pRootNode, old_name, node_list);
    findNodesOpenOrCloseEventIn(NodeInfo::GetRootNode_Custom(), old_name, node_list);
    int n = node_list.size();
    for(int i = 0; i < n; i++)
    {
        node_list[i]->modifyValue(0, new_name);
        node_list[i]->UpdateText();
    }
}

// 参考 https://blog.csdn.net/weixin_43435307/article/details/109469207
bool TreeItemModel_Event::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);

    QByteArray array = data->data(QString("hehe"));
    QDataStream stream(&array, QIODevice::ReadOnly);
    qint64 p;
    stream >> p;
    QModelIndex* index = (QModelIndex*)p;

    NodeInfo* begin_node = static_cast<NodeInfo*>(index->internalPointer());
    NodeInfo* end_node = static_cast<NodeInfo*>(parent.internalPointer());
    int begin_pos = m_pRootNode->GetPosOfChildNode(begin_node);
    int end_pos = m_pRootNode->GetPosOfChildNode(end_node);

    // 插入到end_pos位置
    beginResetModel();
    m_pRootNode->childs.insert(end_pos, begin_node);
    if(begin_pos < end_pos)
        m_pRootNode->childs.removeAt(begin_pos);
    else if(begin_pos > end_pos)
        m_pRootNode->childs.removeAt(begin_pos + 1);
    endResetModel();

    // 刷新节点的展开状态
    MainWindow* main_win = getMainWindow();
    MY_ASSERT(main_win != nullptr);
    main_win->OnMoveEventNode(begin_pos, end_pos);

    delete index;
    return true;
}

Qt::DropActions TreeItemModel_Event::supportedDropActions() const
{
    return Qt::MoveAction;
}

QMimeData* TreeItemModel_Event::mimeData(const QModelIndexList & indexes) const
{
    QMimeData* mimeData = QAbstractItemModel::mimeData(indexes);
    for (int i = 0; i < indexes.count(); i++)
    {
        QModelIndex index = indexes[i];
        QModelIndex* p = new QModelIndex(index);
        QByteArray array;
        QDataStream stream(&array, QIODevice::WriteOnly);
        stream << (qint64)p;
        mimeData->setData(QString("hehe"), array);

        return mimeData; //只取第一个节点的数据
    }
    return mimeData;
}

Qt::ItemFlags TreeItemModel_Event::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    NodeInfo* item = (NodeInfo*)index.internalPointer();
    Qt::ItemFlags flag = QAbstractItemModel::flags(index);

    if(item->type == EVENT)
        return flag | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    else
        return flag;
}

void TreeItemModel_Event::findNodesOpenOrCloseEventIn(NodeInfo *parent_node, QString event_name, QList<NodeInfo *> &node_list)
{
    int n = parent_node->childs.size();
    for(int i = 0; i < n; i++)
    {
        NODE_TYPE t = parent_node->childs[i]->type;
        if((t == OPEN_EVENT || t == CLOSE_EVENT) && parent_node->childs[i]->getValue(0) == event_name)
        {
            node_list.append(parent_node->childs[i]);
        }
        else if(t == SEQUENCE || t == LOOP || t == CHOICE || t == EVENT)
        {
            findNodesOpenOrCloseEventIn(parent_node->childs[i], event_name, node_list);
        }
    }
}
