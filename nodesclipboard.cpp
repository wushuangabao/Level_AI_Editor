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

    if(copyValueOnNode(node))
    {
        m_nodes.push_back(node);
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
        if(vm->GetIdOfVariable(var_names[i]) != -1 && !BaseValueClass::AreSameVarType(vm->GetInitValueOfVarByName(var_names[i]), m_valueManager->GetInitValueOfVar(i)))
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
            vm->AddNewVariable(var_names[i], m_valueManager->GetInitValueOfVar(i));
        }
    }

    // 实施粘贴
    if(tree_type == 0)
        m_eventTreeModel->beginResetModel();
    else if(tree_type == 1)
        m_customTreeModel->beginResetModel();
    pasteChildrenNodesTo(parent, pos);
    if(tree_type == 0)
        m_eventTreeModel->endResetModel();
    else if(tree_type == 1)
        m_customTreeModel->endResetModel();
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

void NodesClipBoard::pasteChildrenNodesTo(NodeInfo *root_node, int pos)
{
    if(pos != -1)
    {
        int n = m_nodes.size();
        for(int i = 0; i < n; i++)
            pasteChildNodeTo(root_node, m_nodes[i], pos + i);
    }
    else
    {
        int n = m_nodes.size();
        for(int i = 0; i < n; i++)
            pasteChildNodeTo(root_node, m_nodes[i]);
    }
}

void NodesClipBoard::pasteChildNodeTo(NodeInfo *root_node, NodeInfo* node, int pos)
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
    {
        info("节点" + node->text + "粘贴失败");
        return;
    }

    new_node->text = name;

    pasteValuesOnNode(node, new_node);
}

void NodesClipBoard::pasteValuesOnNode(NodeInfo *node, NodeInfo* new_node)
{
    ValueManager* vm = ValueManager::GetValueManager();
    if(node->type == COMPARE)
    {
        BaseValueClass* v_left = m_valueManager->GetValueOnNode_Compare_Left(node);
        BaseValueClass* v_right = m_valueManager->GetValueOnNode_Compare_Right(node);
        MY_ASSERT(v_left != nullptr && v_right != nullptr);
        vm->UpdateValueOnNode_Compare_Left(new_node, v_left);
        vm->UpdateValueOnNode_Compare_Right(new_node, v_right);
    }
    else if(node->type == FUNCTION)
    {
        BaseValueClass* v = m_valueManager->GetValueOnNode_Function(node);
        MY_ASSERT(v != nullptr);
        vm->UpdateValueOnNode_Function(new_node, v);
    }
    else if(node->type == SET_VAR)
    {
        BaseValueClass* v = m_valueManager->GetValueOnNode_SetVar(node);
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

bool NodesClipBoard::copyValueOnNode(NodeInfo *node)
{
    ValueManager* vm = ValueManager::GetValueManager();
    BaseValueClass* v = nullptr;

    switch(node->type)
    {
    case COMPARE:
    {
        v = vm->GetValueOnNode_Compare_Left(node);
        if(v != nullptr)
        {
            BaseValueClass* v_2 = vm->GetValueOnNode_Compare_Right(node);
            if(v_2 != nullptr)
            {
                BaseValueClass* new_value_1 = tryCopyValue(v);
                BaseValueClass* new_value_2 = tryCopyValue(v_2);
                if(new_value_1 != nullptr && new_value_2 != nullptr)
                {
                    m_valueManager->UpdateValueOnNode_Compare_Left(node, new_value_1);
                    m_valueManager->UpdateValueOnNode_Compare_Right(node, new_value_2);
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
                m_nodeSetVarId.insert(node, pos);
            else
                return false;
        }

        v = vm->GetValueOnNode_SetVar(node);
        if(v != nullptr)
        {
            BaseValueClass* new_value = tryCopyValue(v);
            if(new_value != nullptr)
            {
                m_valueManager->UpdateValueOnNode_SetValue(node, new_value);
                return true;
            }
        }
        return false;
    }
        break;
    case FUNCTION:
    {
        v = vm->GetValueOnNode_Function(node);
        if(v != nullptr)
        {
            BaseValueClass* new_value = tryCopyValue(v);
            if(new_value != nullptr)
            {
                m_valueManager->UpdateValueOnNode_Function(node, new_value);
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
    int children_n = node->childs.size();
    for(int i = 0; i < children_n; i++)
    {
        if(!copyValueOnNode(node->childs.at(i)))
            return false;
    }
    return true;
}

BaseValueClass* NodesClipBoard::tryCopyValue(BaseValueClass *v)
{
    BaseValueClass* new_value = nullptr;
    if(checkValueBeforeCopy(v))
        new_value = new BaseValueClass(v);
    return new_value;
}

bool NodesClipBoard::checkValueBeforeCopy(BaseValueClass *v)
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
        int n = v->GetFunctionParamsNum();
        for(int i = 0; i < n; i++)
        {
            if(!checkValueBeforeCopy(v->GetFunctionParamAt(i)))
                return false;
        }
    }
    case VT_STR:
        if(v->GetText().contains("自定义动作："))
        {
            info("拷贝值使用了自定义动作");
            //todo: 可以在clipboard中存储使用的自定义动作，一起粘贴。现在暂时不处理。
        }
        break;
    default:
        break;
    }
    return true;
}

int NodesClipBoard::tryAddNewVar(const QString &var_name)
{
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
            BaseValueClass* init_value = ValueManager::GetValueManager()->GetInitValueOfVar(var_id_in_event);
            m_valueManager->AddNewVariable(var_name, init_value);
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