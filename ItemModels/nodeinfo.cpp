#include "../nodesclipboard.h"
#include "../Values/structinfo.h"
#include "nodeinfo.h"

NodeInfo::NodeInfo(NodeInfo* p, NODE_TYPE nt, QString str)
    : parent(p),
      type(nt),
      text(str)
{
    new_child_pos = -1;
}

NodeInfo::NodeInfo(NodeInfo *&o)
{
    parent = nullptr;
    type = o->type;
    text = o->text;
    new_child_pos = -1;

    childs.clear();
    values.clear();

    int n = o->childs.size();
    for(int i = 0; i < n; i++)
    {
        NodeInfo* child = new NodeInfo(o->childs[i]);
        child->parent = this;
        childs.push_back(child);
    }

    n = o->values.size();
    for(int i = 0; i < n; i++)
    {
        values.push_back(o->values.at(i));
    }
}

NodeInfo::~NodeInfo()
{
    parent = nullptr;
    clear();
}

void NodeInfo::clear()
{
    type = NODE_TYPE::INVALID;
    text.clear();
    for(int i = 0; i < childs.size(); i++)
    {
        childs[i]->parent = nullptr;
        // 清除节点数据、子节点数据
        delete childs[i];
        childs[i] = nullptr;
    }
    childs.clear();
    values.clear();
}

void NodeInfo::operator=(NodeInfo &obj)
{
    type = obj.type;
    text = obj.text;

    values.clear();
    for(int i = 0; i < obj.getValuesCount(); i++)
    {
        values.push_back(obj.getValue(i));
    }

    for(int i = 0; i < childs.size(); i++)
    {
        delete childs[i];
    }

    childs.clear();
    for(int i = 0; i < obj.childs.size(); i++)
    {
        childs.push_back(obj.childs[i]);
    }
}

void NodeInfo::FindAndSetNewNodePos(NODE_TYPE parent_type, NodeInfo *&parent_node)
{
    if(type == parent_type)
    {
        parent_node = this;
        parent_node->new_child_pos = -1;
    }
    else
    {
        parent_node = parent;
        for(int i = 0; i < parent_node->childs.size(); i++)
        {
            if(parent_node->childs[i] == this)
            {
                parent_node->new_child_pos = i;
                return;
            }
        }
    }
}

NodeInfo *NodeInfo::addNewChildNode_SetVar(QString node_text, int id_var)
{
    if(type == SEQUENCE)
    {
        NodeInfo* new_node = new NodeInfo(this, SET_VAR, node_text);
        new_node->values.push_back(QString::number(id_var));
        if(new_child_pos != -1)
        {
            childs.insert(new_child_pos, new_node);
            new_child_pos = -1;
        }
        else
            childs.push_back(new_node);
        int pos = node_text.indexOf(" = ");
        if(pos != -1)
        {
            QStringList s_l = node_text.left(pos).split('.', QString::SkipEmptyParts);
            if(s_l.size() > 1)
            {
                QString var_type = ValueManager::GetValueManager()->GetVarTypeAt(id_var);
                MY_ASSERT(StructInfo::GetInstance()->CheckIsStruct(var_type));
                for(int i = 1; i < s_l.size(); i++)
                    new_node->addNewValue(s_l[i]);
            }
        }
        return new_node;
    }
     return nullptr;
}

NodeInfo *NodeInfo::GetRootNode_Event()
{
    static NodeInfo* root_node = nullptr;
    if(root_node == nullptr)
    {
        root_node = new NodeInfo(nullptr, NODE_TYPE::INVALID, "rootNode");
    }
    return root_node;
}

NodeInfo *NodeInfo::GetRootNode_Custom()
{
    static NodeInfo* custom_root_node = nullptr;
    if(custom_root_node == nullptr)
    {
        custom_root_node = new NodeInfo(nullptr, NODE_TYPE::INVALID, "rootNode");
    }
    return custom_root_node;
}

/////////////////////////////////////
/// 供外部调用的节点操作
/////////////////////////////////////

NodeInfo *NodeInfo::addNewChild(NODE_TYPE eType, QString str_data)
{
    NodeInfo* new_node = new NodeInfo(this, eType, str_data);
    bool is_valid = true;

    switch(eType)
    {
    case EVENT:
        new_node->initEventMembers();
        break;
    case ENODE:
        new_node->addNewChild(ETYPE, "");
        break;
    case ETYPE:
        new_node->UpdateEventType(0);
        break;
    case SET_VAR:
        new_node->text = "??? = nil";
        break;
    case CONDITION:
    case COMPARE:
        is_valid = tryAddCondition(new_node);
        break;
    case SEQUENCE:
    case END:
        break;
    case CHOICE:
        is_valid = tryAddChoice(new_node);
        break;
    case LOOP:
        new_node->addNewChild(SEQUENCE, "while 1 do:");
        break;
    case OPEN_EVENT:
    case CLOSE_EVENT:
        new_node->addNewValue("默认事件");
        break;
    default:
        break;
    }

    if(is_valid)
    {
        if(new_child_pos != -1)
        {
            this->childs.insert(new_child_pos, new_node);
            new_child_pos = -1;
        }
        else
        {
            this->childs.push_back(new_node);
        }
        return new_node;
    }
    else
    {
        delete new_node;
        return nullptr;
    }
}

NodeInfo *NodeInfo::addNewChild_Compare(QString compare_type, QString left_value, QString right_value)
{
    CONDITION_OP op = getConditionEnum(compare_type);
    MY_ASSERT(op >= EQUAL_TO && op <= EQUAL_LESS);

    NodeInfo* new_node = new NodeInfo(this, COMPARE, "");
    this->childs.append(new_node);

    new_node->addNewValue(compare_type);
    new_node->addNewValue(left_value);
    new_node->addNewValue(right_value);

    new_node->updateCompareText();

    return new_node;
}

NodeInfo *NodeInfo::addNewChild(NodeInfo *chid_node, int pos)
{
    NodeInfo* new_node = new NodeInfo(chid_node);
    new_node->parent = this;

    if(pos != -1)
    {
        this->childs.insert(pos, new_node);
    }
    else
    {
        this->childs.push_back(new_node);
    }
    return new_node;
}

bool NodeInfo::ContainNodeInChildren(NodeInfo *chid_node)
{
    int n = childs.size();
    for(int i = 0; i < n; i++)
    {
        if(childs.at(i) == chid_node)
            return true;
        else if(childs.at(i)->ContainNodeInChildren(chid_node))
            return true;
    }
    return false;
}

int NodeInfo::GetPosOfChildNode(NodeInfo *child_node)
{
    int n = childs.size();
    for(int i = 0; i < n; i++)
    {
        if(childs.at(i) == child_node)
            return i;
    }
    return -1;
}

NodeInfo *NodeInfo::GetRootNode(QList<int> &pos_list)
{
    NodeInfo* node = this;
    while(node->parent != nullptr)
    {
        pos_list.push_front(node->parent->GetPosOfChildNode(node));
        node = node->parent;
    }
    return node;
}

int NodeInfo::getValuesCount()
{
    return values.size();
}

QString NodeInfo::getValue(int id)
{
    if(id < 0 || id >= values.size())
    {
       info("node " + text + " values 取值越界");
       return "ERROR";
    }
    return values[id];
}

void NodeInfo::addNewValue(CONDITION_OP v)
{
    if(type == CONDITION || type == COMPARE)
    {
        if(values.size() != 0)
        {
            qDebug() << "NodeInfo::addNewValue：给条件节点赋值时覆盖了原有的值。";
            values.clear();
        }
        addNewValue(getConditionStr(v));
    }
    else
    {
        info(getConditionStr(v) + "只能赋值给条件节点。");
    }
}

bool NodeInfo::modifyValue(CONDITION_OP v)
{
    if(values.size() < 1)
        return false;

    if(type == CONDITION || type == COMPARE)
    {
        QString s = getConditionStr(v);
        if(s == "ERROR")
            return false;
        else
        {
            values[0] = s;
            if(type == CONDITION)
                text = s;
        }
    }

    return true;
}

void NodeInfo::addNewValue(QString v)
{
    values.append(v);
}

bool NodeInfo::modifyValue(int id, QString v)
{
    if(id >= values.size() || id < 0)
        return false;

    values[id] = v;
    return true;
}

void NodeInfo::clearValues()
{
    values.clear();
}

void NodeInfo::UpdateText()
{
    switch (type) {
    case CONDITION:
        updateCondionText();
        break;
    case COMPARE:
        updateCompareText();
        break;
    case OPEN_EVENT:
        if(values.size() == 1)
            text = "开始监听事件：" + values[0];
        else
            text = "开始监听事件：？？？";
        break;
    case CLOSE_EVENT:
        if(values.size() == 1)
            text = "停止监听事件：" + values[0];
        else
            text = "停止监听事件：？？？";
        break;
    default:
        break;
    }
}


/////////////////////////////////////
/// 更新节点的text（显示）
/////////////////////////////////////

void NodeInfo::UpdateEventType(int index)
{
    if(type != NODE_TYPE::ETYPE)
        return;

    if(index < 0 || index >= EventType::GetInstance()->GetCount())
        return;

    values.clear();
    values.append(QString::number(index));
    text = EventType::GetInstance()->GetEventTypeUIAt(index);
}

bool NodeInfo::IsBreakButNotReturn()
{
    MY_ASSERT(type == END);
    bool flag = false;

    NodeInfo* cur_node = parent;
    while(cur_node != nullptr && cur_node->type != EVENT)
    {
        if(cur_node->type == LOOP)
        {
            flag = true;
            break;
        }
        cur_node = cur_node->parent;
    }
    return flag;
}

QString NodeInfo::GetVarNameUI_SetVar()
{
    MY_ASSERT(type == SET_VAR);
    MY_ASSERT(!values.isEmpty());
    bool ok = true;
    int id = values[0].toInt(&ok);
    MY_ASSERT(ok);
    QString var_name = ValueManager::GetValueManager()->GetVarNameAt(id);
    int n = values.size();
    for(int i = 1; i < n; i++)
    {
        var_name += QString(".%1").arg(values[i]);
    }
    return var_name;
}

QString NodeInfo::GetVarNameLua_SetVar()
{
    MY_ASSERT(type == SET_VAR);
    MY_ASSERT(!values.isEmpty());
    bool ok = true;
    int id = values[0].toInt(&ok);
    MY_ASSERT(ok);
    QString var_name = ValueManager::GetValueManager()->GetVarNameAt(id);
    int n = values.size();
    if(n > 1)
    {
        QString var_type = ValueManager::GetValueManager()->GetVarTypeAt(id);
        StructInfo* struct_info = StructInfo::GetInstance();
        MY_ASSERT(struct_info->CheckIsStruct(var_type));
        for(int i = 1; i < n; i++)
        {
            QString key_name_lua = struct_info->GetKeyInLua(var_type, values[i]);
            var_name += QString(".%1").arg(key_name_lua);
        }
    }
    return var_name;
}

bool NodeInfo::CheckLeftText_SetVar(QString& var_type, bool should_update)
{
    MY_ASSERT(type == SET_VAR);
    MY_ASSERT(!values.isEmpty());
    bool ok = true;
    int var_id = values[0].toInt(&ok);
    MY_ASSERT(ok);

    var_type = ValueManager::GetValueManager()->GetVarTypeAt(var_id);
    int n = values.size();
    if(n == 1)
        return true;

    // 依次检查每个Key是否合法
    StructInfo* table_info = StructInfo::GetInstance();
    int i = 1;
    for(; i < n; i++)
    {
        if(table_info->CheckIsStruct(var_type))
        {
            // 检查values[i]是否合法
            QString type_at_i = table_info->GetValueTypeOf(var_type, values[i], true);
            if(type_at_i == "")
            {
                ok = false;
                break;
            }
            else
                var_type = type_at_i;
        }
        else
            break;
    }
    // 这时values[i]和后面的values都不合法
    if(!ok)
    {
        if(should_update)
        {
            for(int j = i; j < n; j++)
                values.removeLast();
        }
        return false;
    }
    // 这时values[i]合法，后面的values不合法
    if(ok && i < n - 1)
    {
        if(should_update)
        {
            for(int j = i + 1; j < n; j++)
                values.removeLast();
        }
        return false;
    }
    return true;
}

void NodeInfo::updateCompareText()
{
    if(type != COMPARE)
        return;

    if(values.size() < 3)
    {
        values.clear();
        values.append("==");
        values.append("未定义值");
        values.append("未定义值");
    }

    text = values[1] + " " + values[0] +  " " + values[2];
}

void NodeInfo::updateCondionText()
{
    if(type != CONDITION)
        return;

    if(values.size() == 0)
        values.append("AND");

    if(values[0] == "AND")
        text = parent->type == EVENT? "条件（and）" : "AND";
    else if(values[0] == "OR")
        text = parent->type == EVENT? "条件（or）" : "OR";
    else
        text = "Conditon node value error(" + values[0] + ")";
}

/////////////////////////////////////
/// 某些特殊节点创建时的处理
/////////////////////////////////////

bool NodeInfo::tryAddCondition(NodeInfo *new_node)
{
    if(this->type != CONDITION && new_node->type == COMPARE)
    {
        qDebug() << "Cannot add node(COMPARE) under node(" << this->type << ")." << endl;
        return false;
    }

    if(type != CHOICE && type != CONDITION && new_node->type == CONDITION)
    {
        qDebug() << "Cannot add node(CONDITION) under node(" << this->type << ")." << endl;
        return false;
    }

    if(new_node->type == COMPARE)
    {
        new_node->updateCompareText();
    }
    else if(new_node->type == CONDITION)
    {
        new_node->updateCondionText();
    }
    else
        return false;

    return true;
}

bool NodeInfo::tryAddChoice(NodeInfo *new_node)
{
    if(new_node->type != CHOICE)
        return false;

    NodeInfo* condition_node = new_node->addNewChild(CONDITION, "");
    if(condition_node != nullptr)
    {
        new_node->addNewChild(SEQUENCE, "then");
        new_node->addNewChild(SEQUENCE, "else");
        return true;
    }

    return false;
}

void NodeInfo::initEventMembers()
{
    this->addNewChild(ENODE, "触发节点");

    NodeInfo* new_node = new NodeInfo(this, CONDITION, "判断条件（and）");
    new_node->addNewValue(AND);
    this->childs.append(new_node);

    this->addNewChild(SEQUENCE, "执行行为");
}
