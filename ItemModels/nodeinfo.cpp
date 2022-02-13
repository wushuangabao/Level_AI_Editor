#include "nodeinfo.h"

NodeInfo::NodeInfo(NodeInfo* p, NODE_TYPE nt, QString str)
    : parent(p),
      type(nt),
      text(str)
{}

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
        delete childs[i];
    }
    childs.clear();
    values.clear();
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
        new_node->updateEventType(0);
        break;
    case SET_VAR:
        new_node->text = "a = 0";
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
        new_node->addNewChild(SEQUENCE, "do x times:");
        break;
    default:
        break;
    }

    if(is_valid)
    {
        this->childs.append(new_node);
        return new_node;
    }
    else
    {
        delete new_node;
        return nullptr;
    }
}

int NodeInfo::getValuesCount()
{
    return values.size();
}

QString NodeInfo::getValue(int id)
{
    if(id < 0 || id >= values.size())
        return "ERROR";
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
            values[0] = s;
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


/////////////////////////////////////
/// 更新节点的text（显示）
/////////////////////////////////////

void NodeInfo::updateEventType(EVENT_TYPE_ID event_id)
{
    if(type != NODE_TYPE::ETYPE)
        return;

    if(event_id < 0 || event_id >= EventType::GetInstance()->eventIdVector.size() || event_id >= EventType::GetInstance()->eventNameVector.size())
    {
        qDebug() << "ERROR param in updateEventType!" << endl;
        return;
    }

    values.clear();
    values.append("ETYPE: " + QString::number(event_id));
    text = EventType::GetInstance()->eventNameVector[event_id];
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
