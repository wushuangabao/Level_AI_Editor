#include "qapplication.h"
#include "../mainwindow.h"
#include "../nodesclipboard.h"
#include "treeitemmodel.h"

TreeItemModel::TreeItemModel(QObject *parent /*= 0*/)
    : QAbstractItemModel(parent)
{
    globalValueManager = ValueManager::GetValueManager();
}

MainWindow *TreeItemModel::getMainWindow()
{
    foreach (QWidget *w, qApp->topLevelWidgets())
        if (MainWindow* mainWin = qobject_cast<MainWindow*>(w))
            return mainWin;
    return nullptr;
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
    {
        if(role == Qt::UserRole)
            return QStringLiteral("");
        return QVariant();
    }

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
    case Qt::UserRole:
        return QString("%1.%2").arg(index.parent().data(Qt::UserRole).toString()).arg(QString::number(index.row()));
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

    // 从父节点中移除
    bool is_del = false;
    int n = node->parent->childs.size();
    for(int i = 0; i < n; i++)
    {
        if(node->parent->childs[i] == node)
        {
            node->parent->childs.removeAt(i);
            node->parent = nullptr;
            is_del = true;
            break;
        }
    }

    // 如果 value manager 中有node和对应的value，也要删掉
    globalValueManager->OnDeleteNode(node);
    // 清除节点数据、子节点数据
    delete node;
    node = nullptr;

    // todo: 如果删除的是Event，那么其他事件中的OPEN_EVENT和CLOSE_EVENT节点会失效

    return is_del;
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
    while(node->parent != NodeInfo::GetRootNode_Event() && node->parent != NodeInfo::GetRootNode_Custom())
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

bool TreeItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &obj_node)
{
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);

    QByteArray ba = data->data(QString("count"));
    QDataStream ds(&ba, QIODevice::ReadOnly);
    int n;
    ds >> n;

    bool res = false;
    for(int i = 0; i < n; i++)
    {
        QByteArray array = data->data(QString("id_%1").arg(i));
        QDataStream stream(&array, QIODevice::ReadOnly);
        qint64 p;
        stream >> p;
        QModelIndex* index = (QModelIndex*)p;

        NodeInfo* begin_node = static_cast<NodeInfo*>(index->internalPointer());
        NodeInfo* end_node = static_cast<NodeInfo*>(obj_node.internalPointer());
        do
        {
            if(begin_node == nullptr || end_node == nullptr)
                break;

            if(begin_node->type == SEQUENCE)
                break;

            // Event Tree & Custom Tree 通用部分：移动动作节点
            if(begin_node->type >= SET_VAR && begin_node->type <= CLOSE_EVENT)
            {
                int begin_pos, end_pos;
                NodeInfo* parent_node;
                begin_pos = begin_node->parent->GetPosOfChildNode(begin_node);
                if(end_node->type == SEQUENCE)
                {
                    end_pos = end_node->childs.size();
                    parent_node = end_node;
                }
                else if(end_node->type >= SET_VAR && end_node->type <= CLOSE_EVENT)
                {
                    parent_node = end_node->parent;
                    end_pos = parent_node->GetPosOfChildNode(end_node);
                }
                else
                    break;
                QList<int> pos_list_1, pos_list_2;
                NodeInfo* begin_root = begin_node->GetRootNode(pos_list_1);
                NodeInfo* end_root = end_node->GetRootNode(pos_list_2);
                if(begin_root != end_root)
                    break;
                NodeInfo* event_root_node = NodeInfo::GetRootNode_Event();
                if(begin_pos == -1 || end_pos == -1 || (event_root_node == begin_root && (pos_list_1.size() < 2 || pos_list_2.size() < 2 || pos_list_1[0] != pos_list_2[0])))
                    break;
                if((begin_node->parent == end_node->parent || begin_node->parent == end_node) && (begin_pos == end_pos || begin_pos + 1 == end_pos))
                    break;

                MainWindow* main_win = getMainWindow();
                MY_ASSERT(main_win != nullptr);
                beginResetModel();

                // 找到被移动的节点及其子节点temp_codes
                QStringList temp_codes;
                QString begin_code = main_win->GetItemCodeOfNode(begin_node);
                QMap<QString, bool> *state_info = nullptr;
                if(m_pRootNode == event_root_node)
                    state_info = &(main_win->m_itemState_Event);
                else if(m_pRootNode == NodeInfo::GetRootNode_Custom())
                    state_info = &(main_win->m_itemState_Custom);
                if(state_info == nullptr)
                    break;
                main_win->replaceItemStateInMap(begin_code, "", temp_codes, state_info, false);

                // 存储一个临时副本temp_states（删除原数据）
                QMap<QString, bool> temp_states;
                int n = temp_codes.size();
                for(int i = 0; i < n; i++)
                {
                    if(state_info->contains(temp_codes[i]))
                    {
                        temp_states.insert(temp_codes[i], state_info->value(temp_codes[i]));
                        state_info->remove(temp_codes[i]);
                    }
                }

                // 先删除
                if(begin_pos != begin_node->parent->childs.size() - 1)
                    main_win->MoveForwardItemStateOf(main_win->GetItemCodeOfNode(begin_node->parent->childs.last()), begin_pos + 1);
                begin_node->parent->childs.removeAt(begin_pos);

                if(begin_node->parent == parent_node && begin_pos < end_pos)
                    end_pos--;

                // 后插入
                QString end_code;
                if(end_pos != parent_node->childs.size())
                {
                    end_code = main_win->GetItemCodeOfNode(parent_node->childs.at(end_pos));
                    main_win->MoveBackItemStateOf(end_code, parent_node->childs.size());
                }
                parent_node->childs.insert(end_pos, begin_node);
                begin_node->parent = parent_node;

                // 根据temp_states中的展开状态，更新被移动节点的展开状态
                if(end_code != "")
                    end_code = main_win->GetItemCodeOfNode(parent_node->childs.at(end_pos));
                QMap<QString, bool>::iterator itr;
                for(itr = temp_states.begin(); itr != temp_states.end(); ++itr)
                {
                    QString new_key = itr.key();
                    new_key.replace(begin_code, end_code);
                    state_info->insert(new_key, itr.value());
                }

                temp_states.clear();
                temp_codes.clear();

                endResetModel();
                main_win->updateTreeViewStateByData();
                main_win->SaveBackup(true);
                res = true;

                // 选中被移动之后的节点
                main_win->m_curModelIndex = main_win->getModelIndexBy(end_code);
                main_win->m_curNode = begin_node;
                main_win->selectTreeViewItem(main_win->m_curModelIndex);
            }

            // Event Tree & Custom Tree 相异部分：各自实现 OnMoveNode 接口
            res = OnMoveNode(begin_node, end_node) || res;
        }
        while(false);

        delete index;
    }

    return res;
}

Qt::DropActions TreeItemModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QMimeData *TreeItemModel::mimeData(const QModelIndexList &indexes) const
{
    // 记录每个节点的pos序列（pos是指位于父节点childs的第几个位置）
    QList<QList<int>> pos_list;
    int n = indexes.count();
    for(int i = 0; i < n; i++)
    {
        NodeInfo* node = static_cast<NodeInfo*>(indexes[i].internalPointer());
        QList<int> pos_l_i;
        node->GetRootNode(pos_l_i);
        pos_list.push_back(pos_l_i);
    }

    // 对这些节点重新排序，按照TreeView中从上到下的顺序
    QList<int> index_list; //记录indexes中的下标，按顺序排列
    for(int i = 0; i < n; i++)
    {
        int num = index_list.size();
        int j = 0;
        for(; j < num; j++)
        {
            QList<int> *pos_list_j = &(pos_list[index_list[j]]);
            QList<int> *pos_list_i = &(pos_list[i]);
            int deep = pos_list_i->size();
            if(pos_list_j->size() < deep)
                deep = pos_list_j->size();
            int deep_compare = 0;
            for(; deep_compare < deep; deep_compare++)
            {
                if(pos_list_i->at(deep_compare) < pos_list_j->at(deep_compare))
                    break;
            }
            if(deep_compare < deep)
                break;
        }
        index_list.insert(j, i);
    }

    QMimeData* mimeData = QAbstractItemModel::mimeData(indexes);
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << n;
    mimeData->setData(QString("count"), ba);

    for (int i = 0; i < n; i++)
    {
        QModelIndex index = indexes[index_list[i]];
        QModelIndex* p = new QModelIndex(index);
        QByteArray array;
        QDataStream stream(&array, QIODevice::WriteOnly);
        stream << (qint64)p;
        mimeData->setData(QString("id_%1").arg(i), array);
        //return mimeData; //只取第一个节点的数据
    }

    return mimeData;
}

Qt::ItemFlags TreeItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    NodeInfo* item = (NodeInfo*)index.internalPointer();
    Qt::ItemFlags flag = QAbstractItemModel::flags(index);

    if(item->type == EVENT || (item->type >= SET_VAR && item->type <= CLOSE_EVENT)) // 动作节点和事件节点可拖拽，其他节点都有可能是固定位置的节点所以不能拖
        return flag | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    else
        return flag;
}
