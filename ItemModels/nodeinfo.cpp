#include "nodeinfo.h"

NodeInfo::NodeInfo(NodeInfo* p, NODE_TYPE nt, QString str)
    : parent(p),
      type(nt),
      text(str)
{
    new_child_pos = -1;
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
        childs[i]->clear();
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
        childs[i]->clear();
        delete childs[i];
    }

    childs.clear();
    for(int i = 0; i < obj.childs.size(); i++)
    {
        childs.push_back(obj.childs[i]);
    }
}

void NodeInfo::FindAndSetNewNodePos(NodeInfo *&parent_node)
{
    if(type == SEQUENCE)
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

NodeInfo *NodeInfo::addNewChildNode_SetVar(QString var_name, QString value_str, int id_var)
{
    QString node_text = var_name + " = " + value_str;
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
        return new_node;
    }
     return nullptr;
}

NodeInfo *NodeInfo::GetRootNode()
{
    static NodeInfo* root_node = nullptr;
    if(root_node == nullptr)
    {
        root_node = new NodeInfo(nullptr, NODE_TYPE::INVALID, "rootNode");
        qDebug() << "new root node.";
    }
    return root_node;
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
    text = EventType::GetInstance()->GetEventNameAt(index);
}

bool NodeInfo::IsBreakButNotReturn()
{
    MY_ASSERT(type == END);
    bool flag = false;

    NodeInfo* cur_node = parent;
    while(cur_node->type != EVENT)
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
        text = "条件（and）";
    else if(values[0] == "OR")
        text = "条件（or）";
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
    this->addNewChild(ETYPE, "事件类型");

    NodeInfo* new_node = new NodeInfo(this, CONDITION, "条件（and）");
    new_node->addNewValue(AND);
    this->childs.append(new_node);

    this->addNewChild(SEQUENCE, "动作序列");
}
