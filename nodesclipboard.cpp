#include "qapplication.h"
#include "mainwindow.h"
#include "ItemModels/treeitemmodel.h"
#include "nodesclipboard.h"

NodesClipBoard *NodesClipBoard::GetInstance()
{
    static NodesClipBoard* instance = nullptr;
    if(instance == nullptr)
        instance = new NodesClipBoard();
    return instance;
}

NodesClipBoard::~NodesClipBoard()
{
    ClearNodes();
}

void NodesClipBoard::ClearNodes(int tree_type)
{
    m_treeType = tree_type;
    ClearNodes();
}

void NodesClipBoard::SetTreeItemModel(TreeItemModel *et, TreeItemModel *ct)
{
    m_eventTreeModel = et;
    m_customTreeModel = ct;
}

bool NodesClipBoard::AddCopyNode(NodeInfo *node)
{
    MY_ASSERT(node != nullptr);
    NODE_TYPE paste_node_type = getTypeOfPasteNode(node);
    if(paste_node_type == MAX)
    {
        info("无法复制" + getNodeTypeStr(node->type) + "类型的节点。");
        return false;
    }

    NODE_TYPE node_type = GetTypeOfPasteNode();
    if(node_type != NONE && paste_node_type != node_type)
    {
        info("这些节点不能粘贴到一起，复制失败。");
        return false;
    }

    // 复制一份node的拷贝，包括相关的value数据
    NodeInfo* new_node = nullptr;
    if(copyValueOnNode(node, new_node, true))
    {
        m_nodes.push_back(new_node);
        return true;
    }
    else
        return false;
}

bool NodesClipBoard::HasCopyNode(NodeInfo *node)
{
    if(m_nodes.contains(node))
        return true;
    else
    {
        int n = m_nodes.size();
        for(int i = 0; i < n; i++)
        {
            if(m_nodes.at(i)->ContainNodeInChildren(node))
                return true;
        }
    }
    return false;
}

bool NodesClipBoard::PasteToNode(NodeInfo *node, int tree_type)
{
    m_treeType = tree_type;
    NODE_TYPE node_type = GetTypeOfPasteNode();
    if(node_type == MAX || node_type == NONE)
        return false;

    NodeInfo* parent = node;
    int pos = -1;

    // 如果不能直接粘贴到node中
    if(node_type != node->type)
    {
        if(node->parent != nullptr && node->parent->type == node_type)
        {
            // 粘贴到node的父节点中，并提供位置
            int n = node->parent->childs.size();
            for(int i = 0; i < n; i++)
            {
                if(node->parent->childs[i] == node)
                {
                    pos = i;
                    parent = node->parent;
                    break;
                }
            }
        }
        else
        {
            info("请粘贴到" + getNodeTypeStr(node_type) + "节点中");
            return false;
        }
    }

    ValueManager* vm = ValueManager::GetValueManager();
    QStringList var_names = m_valueManager->GetGlobalVarList();
    int var_n = var_names.size();
    for(int i = 0; i < var_n; i++)
    {
        // vm中这个变量的类型和剪切板中的不一致
        int idx = vm->GetIdOfVariable(var_names[i]);
        if(idx != -1 && !CommonValueClass::AreSameVarType(vm->GetInitValueOfVar(idx), m_valueManager->GetInitValueOfVar(i)))
        {
            info("存在变量名冲突");
            return false;
        }
    }

    // 添加缺少的变量
    for(int i = 0; i < var_n; i++)
    {
        if(vm->GetIdOfVariable(var_names[i]) == -1)
        {
            vm->AddNewVariable(var_names[i], m_valueManager->GetInitValueOfVar(i), m_valueManager->CheckVarIsLevelParam(i));
        }
    }

    MainWindow* win = getMainWindow();
    MY_ASSERT(win != nullptr);

    if(tree_type == 0)
        m_eventTreeModel->beginResetModel();
    else if(tree_type == 1)
        m_customTreeModel->beginResetModel();

    // 记录原来在pos的节点的code
    QString code;
    if(pos != -1)
        code = win->GetItemCodeOfNode(parent->childs.at(pos));
    // 实施粘贴
    QStringList codes = pasteChildrenNodesTo(parent, pos);
    // 移动原pos位置的节点及其后的兄弟节点的位置
    if(pos != -1)
        win->MoveBackItemStateOf(code, parent->childs.size() - codes.size(), codes.size());
    // 进行备份
    win->SaveBackup(pos != -1);

    if(tree_type == 0)
        m_eventTreeModel->endResetModel();
    else if(tree_type == 1)
        m_customTreeModel->endResetModel();

    // 通过进行展开、折叠操作，记录新增的这些节点的状态，并选中
    win->OnPasteNodes(codes);

    return true;
}

NodesClipBoard::NodesClipBoard()
{
    m_nodes.clear();
    m_valueManager = ValueManager::GetClipBoardValueManager();
}

void NodesClipBoard::ClearNodes()
{
    // 释放没有parent的node
    int n = m_nodes.size();
    for(int i = 0; i < n; i++)
    {
        if(m_nodes[i] != nullptr && m_nodes[i]->parent == nullptr)
        {
            // 如果 value manager 中有node和对应的value，也要删掉
            ValueManager::GetValueManager()->OnDeleteNode(m_nodes[i]);
            // 清除节点数据、子节点数据
            delete m_nodes[i];
            m_nodes[i] = nullptr;
        }
    }
    m_nodes.clear();
    m_valueManager->ClearData();
    m_nodeSetVarId.clear();
}

MainWindow *NodesClipBoard::getMainWindow()
{
    foreach (QWidget *w, qApp->topLevelWidgets())
        if (MainWindow* mainWin = qobject_cast<MainWindow*>(w))
            return mainWin;
    return nullptr;
}

QStringList NodesClipBoard::pasteChildrenNodesTo(NodeInfo *root_node, int pos)
{
    QStringList codes;
    MainWindow* win = getMainWindow();
    if(pos != -1)
    {
        int n = m_nodes.size();
        for(int i = 0; i < n; i++)
        {
            QString code = win->GetItemCodeOfNode(pasteChildNodeTo(root_node, m_nodes[i], pos + i));
            codes.push_back(code);
        }
    }
    else
    {
        int n = m_nodes.size();
        for(int i = 0; i < n; i++)
        {
            QString code = win->GetItemCodeOfNode(pasteChildNodeTo(root_node, m_nodes[i]));
            codes.push_back(code);
        }
    }
    return codes;
}

NodeInfo* NodesClipBoard::pasteChildNodeTo(NodeInfo *root_node, NodeInfo* node, int pos)
{
    // 防止事件重名
    QString name = node->text;
    if(root_node->type == INVALID)
    {
        bool ok = findNodeNameInTree(name);
        while(ok)
        {
            name += "_Copy";
            ok = findNodeNameInTree(name);
        }
    }

    // 复制节点，加到树中
    NodeInfo* new_node = nullptr;
    if(pos != -1)
        new_node = root_node->addNewChild(node, pos);
    else
        new_node = root_node->addNewChild(node);

    if(new_node == nullptr)
        info("节点" + node->text + "粘贴失败");
    else
    {
        new_node->text = name;
        pasteValuesOnNode(node, new_node);
    }

    return new_node;
}

void NodesClipBoard::pasteValuesOnNode(NodeInfo *node, NodeInfo* new_node)
{
    ValueManager* vm = ValueManager::GetValueManager();
    if(node->type == COMPARE)
    {
        CommonValueClass* v_left = m_valueManager->GetValueOnNode_Compare_Left(node);
        CommonValueClass* v_right = m_valueManager->GetValueOnNode_Compare_Right(node);
        MY_ASSERT(v_left != nullptr && v_right != nullptr);
        vm->UpdateValueOnNode_Compare_Left(new_node, v_left);
        vm->UpdateValueOnNode_Compare_Right(new_node, v_right);
    }
    else if(node->type == FUNCTION)
    {
        CommonValueClass* v = m_valueManager->GetValueOnNode_Function(node);
        MY_ASSERT(v != nullptr);
        vm->UpdateValueOnNode_Function(new_node, v);
    }
    else if(node->type == SET_VAR)
    {
        CommonValueClass* v = m_valueManager->GetValueOnNode_SetVar(node);
        MY_ASSERT(v != nullptr);
        vm->UpdateValueOnNode_SetValue(new_node, v);

        // 更新 SET_VAR 节点上存储的变量 id
        MY_ASSERT(m_nodeSetVarId.contains(node));
        QString var_name = m_valueManager->GetVarNameAt(m_nodeSetVarId[node]);
        new_node->modifyValue(0, QString::number(vm->FindIdOfVarName(var_name)));
    }
    // 其他类型 EVENT SEQUENCE CONDITION LOOP CHOICE 都（可能）有子节点
    else
    {
        int children_n = node->childs.size();
        for(int i = 0; i < children_n; i++)
        {
            pasteValuesOnNode(node->childs[i], new_node->childs[i]);
        }
    }
}

bool NodesClipBoard::copyValueOnNode(NodeInfo *node, NodeInfo *&new_node, bool copy_node)
{
    ValueManager* vm = ValueManager::GetValueManager();
    CommonValueClass* v = nullptr;

    switch(node->type)
    {
    case COMPARE:
    {
        v = vm->GetValueOnNode_Compare_Left(node);
        if(v != nullptr)
        {
            CommonValueClass* v_2 = vm->GetValueOnNode_Compare_Right(node);
            if(v_2 != nullptr)
            {
                CommonValueClass* new_value_1 = tryCopyValue(v);
                CommonValueClass* new_value_2 = tryCopyValue(v_2);
                if(new_value_1 != nullptr && new_value_2 != nullptr)
                {
                    if(copy_node) copyNode(new_node, node);
                    m_valueManager->UpdateValueOnNode_Compare_Left(new_node, new_value_1);
                    m_valueManager->UpdateValueOnNode_Compare_Right(new_node, new_value_2);
                    return true;
                }
            }
        }
    }
        break;
    case SET_VAR:
    {
        MY_ASSERT(node->getValuesCount() >= 1);
        bool ok = true;
        int var_id = node->getValue(0).toInt(&ok);
        if(!ok)
        {
            info("SET_VAR节点上没有记录var_id");
            return false;
        }
        else
        {
            // 添加 set_var 节点等号左边的变量
            int pos = tryAddNewVar(vm->GetVarNameAt(var_id));
            if(-1 != pos)
            {
                if(copy_node) copyNode(new_node, node);
                m_nodeSetVarId.insert(new_node, pos);
            }
            else
                return false;
        }

        v = vm->GetValueOnNode_SetVar(node);
        if(v != nullptr)
        {
            CommonValueClass* new_value = tryCopyValue(v);
            if(new_value != nullptr)
            {
                m_valueManager->UpdateValueOnNode_SetValue(new_node, new_value);
                return true;
            }
        }
        delete new_node;
        new_node = nullptr;
        return false;
    }
        break;
    case FUNCTION:
    {
        v = vm->GetValueOnNode_Function(node);
        if(v != nullptr)
        {
            CommonValueClass* new_value = tryCopyValue(v);
            if(new_value != nullptr)
            {
                if(copy_node) copyNode(new_node, node);
                m_valueManager->UpdateValueOnNode_Function(new_node, new_value);
                return true;
            }
        }
        return false;
    }
        break;
    case OPEN_EVENT:
    case CLOSE_EVENT:
        info("无法复制" + node->text);
        return false;
    default:
        break;
    }

    // EVENT SEQUENCE CONDITION LOOP CHOICE 都（可能）有子节点
    if(new_node == nullptr)
        copyNode(new_node, node);
    int children_n = new_node->childs.size();
    MY_ASSERT(children_n == node->childs.size());
    for(int i = 0; i < children_n; i++)
    {
        NodeInfo* child_node = new_node->childs.at(i);
        if(!copyValueOnNode(node->childs.at(i), child_node))
        {
            delete new_node;
            new_node = nullptr;
            return false;
        }
    }
    return true;
}

void NodesClipBoard::copyNode(NodeInfo *&cur_node, NodeInfo *node_be_copy)
{
    if(cur_node != nullptr)
        delete cur_node;
    cur_node = new NodeInfo(node_be_copy);
}

CommonValueClass* NodesClipBoard::tryCopyValue(CommonValueClass *v)
{
    CommonValueClass* new_value = nullptr;
    if(checkValueBeforeCopy(v))
    {
        if(v->GetValueType() < VT_TABLE)
            new_value = new BaseValueClass((BaseValueClass*)v);
        else
            new_value = new StructValueClass((StructValueClass*)v);
    }
    return new_value;
}

bool NodesClipBoard::checkValueBeforeCopy(CommonValueClass *v)
{
    switch (v->GetValueType())
    {
    case VT_PARAM:
    {
        if(GetTypeOfPasteNode() != INVALID)
        {
            info("拷贝值使用了事件参数：" + v->GetText());
            //todo: 现在暂时不处理，只提示。
        }
    }
        break;
    case VT_VAR:
        return ( -1 != tryAddNewVar(v->GetText()) );
    case VT_FUNC:
    {
        BaseValueClass* bv = static_cast<BaseValueClass*>(v);
        int n = bv->GetFunctionParamsNum();
        for(int i = 0; i < n; i++)
        {
            if(!checkValueBeforeCopy(bv->GetFunctionParamAt(i)))
                return false;
        }
    }
    case VT_STR:
        if(v->GetText().contains(BaseValueClass::custom_name_prefix))
        {
            info("拷贝值使用了自定义动作");
            //todo: 可以在clipboard中存储使用的自定义动作，一起粘贴。现在暂时不处理。
        }
        break;
    case VT_TABLE:
    {
        StructValueClass* sv = static_cast<StructValueClass*>(v);
        QStringList keys = sv->GetAllKeys();
        int n = keys.size();
        for(int i = 0; i < n; i++)
        {
            CommonValueClass* c_v = sv->GetValueByKey(keys[i]);
            if(c_v == nullptr)
                continue;
            if(!checkValueBeforeCopy(c_v))
                return false;
        }
    }
    default:
        break;
    }
    return true;
}

int NodesClipBoard::tryAddNewVar(const QString &var_name_)
{
    QString var_name = ValueManager::GetVarNameInKeyStr(var_name_);
    int pos = m_valueManager->GetIdOfVariable(var_name);
    if(pos == -1)
    {
        int var_id_in_event = ValueManager::GetValueManager()->GetIdOfVariable(var_name);
        if(var_id_in_event == -1)
        {
            info(var_name + "不在变量列表中");
        }
        else
        {
            CommonValueClass* init_value = ValueManager::GetValueManager()->GetInitValueOfVar(var_id_in_event);
            if(init_value->GetValueType() == VT_TABLE)
            {
                StructValueClass* s_v = static_cast<StructValueClass*>(init_value);
                QStringList keys = s_v->GetAllKeys();
                for(int i = 0; i < keys.size(); i++)
                {
                    if(s_v->GetValueByKey(keys[i])->GetValueType() == VT_VAR)
                        tryAddNewVar(s_v->GetValueByKey(keys[i])->GetText());
                }
            }
            m_valueManager->AddNewVariable(var_name, init_value, ValueManager::GetValueManager()->CheckVarIsLevelParam(var_id_in_event));
            pos = m_valueManager->GetIdOfVariable(var_name);
        }
    }
    return pos;
}

NODE_TYPE NodesClipBoard::GetTypeOfPasteNode()
{
    if(m_nodes.size() > 0)
    {
        return getTypeOfPasteNode(m_nodes[0]);
    }
    else
        return NONE; //表示可以粘贴到任意节点
}

NODE_TYPE NodesClipBoard::getTypeOfPasteNode(NodeInfo *node)
{
    if(m_treeType == 0)
    {
        switch (node->type)
        {
        case EVENT:
            return INVALID;
        case CONDITION:
        case COMPARE:
            return CONDITION;
        case SET_VAR:
        case FUNCTION:
        case LOOP:
        case CHOICE:
//        case OPEN_EVENT:
//        case CLOSE_EVENT:
            return SEQUENCE;
        default:
            break;
        }
    }
    else if(m_treeType == 1)
    {
        switch (node->type)
        {
        case CONDITION:
        case COMPARE:
            return CONDITION;
        case SET_VAR:
        case FUNCTION:
        case LOOP:
        case CHOICE:
//        case OPEN_EVENT:
//        case CLOSE_EVENT:
            return SEQUENCE;
        case SEQUENCE:
            if(node->parent == NodeInfo::GetRootNode_Custom())
                return INVALID;
        default:
            break;
        }
    }

    return MAX; //不能粘贴
}

bool NodesClipBoard::findNodeNameInTree(const QString& name)
{
    NodeInfo* root = nullptr;
    if(m_treeType == 0)
        root = m_eventTreeModel->m_pRootNode;
    else if(m_treeType == 1)
        root = m_customTreeModel->m_pRootNode;
    else
        info("ERROR m_treeType = " + QString::number(m_treeType));

    int rn = root->childs.size();
    for(int j = 0; j < rn; j++)
    {
        if(root->childs[j]->text == name)
        {
            return true;
        }
    }
    return false;
}
