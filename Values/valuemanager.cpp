#include "../ItemModels/nodeinfo.h"
#include "valueclass.h"
#include "valuemanager.h"

ValueManager::~ValueManager()
{
    ClearData();
}

ValueManager::ValueManager()
{
    nameList = QStringList();
    dataList = QList<BaseValueClass*>();
}


QStringList ValueManager::GetGlobalVarList() const
{
    return nameList;
}

int ValueManager::GetIdOfVariable(BaseValueClass *v)
{
    return nameList.indexOf(v->GetText());
}

bool ValueManager::AddNewVariable(QString name, BaseValueClass* v)
{
    if(nameList.contains(name))
    {
        info("已存在这个名字的变量了。");
        return false;
    }
    else
    {
        nameList << name;
        BaseValueClass* new_value = new BaseValueClass(v);
        dataList << new_value;
        return true;
    }
}

void ValueManager::DeleteVariable(QString name)
{
    int id = -1;
    int n = nameList.size();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            if(nameList[i] == name)
                id = i;
        }
    }
    if(id != -1)
    {
        nameList.removeAt(id);
        delete dataList[id];
        dataList.removeAt(id);
    }
}

void ValueManager::ModifyVarValueAt(int idx, QString name, BaseValueClass *value)
{
    if(name.isEmpty() || name == "")
    {
        info("变量名为空，变量值设置失败！");
        return;
    }
    if(idx < 0 || idx >= nameList.size())
    {
        info("不存在这个id的变量");
        return;
    }

    nameList[idx] = name;
    *(dataList[idx]) = *value;

    UpdateVarName(idx);
}

void ValueManager::UpdateValueOnNode_SetValue(NodeInfo *node, BaseValueClass *value)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(node->type == SET_VAR);

    Q_ASSERT(value != nullptr);

    if(nodeSetVarMap.contains(node))
    {
        *(nodeSetVarMap[node]) = *value;
    }
    else
    {
        nodeSetVarMap.insert(node, new BaseValueClass(value)); // TODO delete
    }
}

BaseValueClass *ValueManager::GetValueOnNode_SetVar(NodeInfo* node)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(node->type == SET_VAR);

    if(nodeSetVarMap.contains(node))
    {
        return nodeSetVarMap[node];
    }
    else
    {
        return nullptr;
    }
}

void ValueManager::UpdateValueOnNode_Function(NodeInfo *node, BaseValueClass *value)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(node->type == FUNCTION);

    Q_ASSERT(value != nullptr);
    VALUE_TYPE vt = value->GetValueType();
    Q_ASSERT(vt == VT_FUNC || vt == VT_STR);

    if(nodeFunctionMap.contains(node))
    {
        *(nodeFunctionMap[node]) = *value;
    }
    else
    {
        nodeFunctionMap.insert(node, new BaseValueClass(value)); // TODO delete when node remove
    }
}

BaseValueClass *ValueManager::GetValueOnNode_Function(NodeInfo *node)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(node->type == FUNCTION);

    if(nodeFunctionMap.contains(node))
    {
        return nodeFunctionMap[node];
    }
    else
    {
        return nullptr;
    }
}

void ValueManager::AddValueOnNode_Compare_Left(NodeInfo *node, BaseValueClass *value)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(value != nullptr);
    Q_ASSERT(node->type == COMPARE);

    if(nodeCompareValueLeftMap.contains(node))
    {
        delete nodeCompareValueLeftMap[node];
        nodeCompareValueLeftMap[node] = value;
    }
    else
    {
        nodeCompareValueLeftMap.insert(node, value);
    }
}

void ValueManager::AddValueOnNode_Compare_Right(NodeInfo *node, BaseValueClass *value)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(value != nullptr);
    Q_ASSERT(node->type == COMPARE);

    if(nodeCompareValueRightMap.contains(node))
    {
        delete nodeCompareValueRightMap[node];
        nodeCompareValueRightMap[node] = value;
    }
    else
    {
        nodeCompareValueRightMap.insert(node, value);
    }
}

void ValueManager::UpdateValueOnNode_Compare_Left(NodeInfo *node, BaseValueClass *value)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(value != nullptr);
    Q_ASSERT(node->type == COMPARE);

    if(nodeCompareValueLeftMap.contains(node))
    {
        *(nodeCompareValueLeftMap[node]) = *value;
    }
    else
    {
        nodeCompareValueLeftMap.insert(node, new BaseValueClass(value)); // TODO delete
    }
}

void ValueManager::UpdateValueOnNode_Compare_Right(NodeInfo *node, BaseValueClass *value)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(value != nullptr);
    Q_ASSERT(node->type == COMPARE);

    if(nodeCompareValueRightMap.contains(node))
    {
        *(nodeCompareValueRightMap[node]) = *value;
    }
    else
    {
        nodeCompareValueRightMap.insert(node, new BaseValueClass(value)); // TODO delete
    }
}

BaseValueClass *ValueManager::GetValueOnNode_Compare_Left(NodeInfo *node)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(node->type == COMPARE);

    if(nodeCompareValueLeftMap.contains(node))
    {
        return nodeCompareValueLeftMap[node];
    }
    else
    {
        return nullptr;
    }
}

BaseValueClass *ValueManager::GetValueOnNode_Compare_Right(NodeInfo *node)
{
    Q_ASSERT(node != nullptr);
    Q_ASSERT(node->type == COMPARE);

    if(nodeCompareValueRightMap.contains(node))
    {
        return nodeCompareValueRightMap[node];
    }
    else
    {
        return nullptr;
    }
}

QString ValueManager::GetVarTypeAt(int idx)
{
    if(idx >= dataList.size() || idx < 0)
    {
        info("GetVarTypeAt(" + QString::number(idx) + ")参数越界！");
        return "";
    }
    else
        return dataList.at(idx)->GetVarType();
}

QString ValueManager::GetVarTypeOf(const QString &name)
{
    int i = FindIdOfVarName(name);
    if(i != -1)
        return GetVarTypeAt(i);
    else
        return "";
}

BaseValueClass* ValueManager::GetInitValueOfVar(int idx)
{
    return dataList.at(idx);
}

int ValueManager::FindIdOfVarName(const QString &name)
{
    int n = nameList.size();
    for(int i = 0; i < n; i++)
    {
        if(nameList[i] == name)
            return i;
    }
    info("没有找到变量" + name + "的ID！");
    return -1;
}

// 在事件节点（EVENT类型节点）中更新“事件参数”
void ValueManager::UpdateEventParams(NodeInfo* eventNode, int eid)
{
    Q_ASSERT(eventNode != nullptr);
    Q_ASSERT(eventNode->type == EVENT);
    if(eventParamsMap.contains(eventNode))
    {
        eventParamsMap[eventNode]->clear();
    }
    else
    {
        QStringList* str_list = new QStringList();
        eventParamsMap.insert(eventNode, str_list);
    }

    int n = eventNode->getValuesCount();
    if(n > 0)
    {
        eventNode->clearValues();
    }

    EVENT_TYPE_ID e_eid = static_cast<EVENT_TYPE_ID>(eid);
    int index = EventType::GetInstance()->eventIdVector.indexOf(e_eid);
    if(index == -1)
    {
        qDebug() << "not find e_id in UpdateEventParams!" << endl;
        return;
    }
    int params_num = EventType::GetInstance()->paramNamesVector[index].size();
    if(params_num > 0)
    {
        for(int i = 0; i < params_num; i++)
        {
            QString pname = EventType::GetInstance()->paramNamesVector[index][i];
            eventNode->addNewValue(pname);
            eventParamsMap[eventNode]->push_back(pname);
        }
    }
}

QStringList *ValueManager::GetEventParams(NodeInfo *node)
{
    if(eventParamsMap.contains(node))
    {
        return eventParamsMap[node];
    }
    return nullptr;
}

void ValueManager::UpdateVarName(int var_id)
{
    QMap<NodeInfo*, BaseValueClass*>::iterator itr;

    for(itr = nodeFunctionMap.begin(); itr != nodeFunctionMap.end(); ++itr)
    {
        itr.value()->UpdateVarName(var_id, nameList[var_id]);
        itr.key()->text = itr.value()->GetText();
    }
    for(itr = nodeSetVarMap.begin(); itr != nodeSetVarMap.end(); ++itr)
    {
        itr.value()->UpdateVarName(var_id, nameList[var_id]);
        itr.key()->modifyValue(0, nameList[var_id]);
        itr.key()->text = itr.key()->getValue(0) + " = " + itr.value()->GetText();
    }
    for(itr = nodeCompareValueLeftMap.begin(); itr != nodeCompareValueLeftMap.end(); ++itr)
    {
        itr.value()->UpdateVarName(var_id, nameList[var_id]);
        itr.key()->modifyValue(1, itr.value()->GetText());
    }
    for(itr = nodeCompareValueRightMap.begin(); itr != nodeCompareValueRightMap.end(); ++itr)
    {
        itr.value()->UpdateVarName(var_id, nameList[var_id]);
        itr.key()->modifyValue(2, itr.value()->GetText());
        itr.key()->UpdateText();
    }
}

void ValueManager::clearNodeMap(QMap<NodeInfo *, BaseValueClass *> &node_map)
{
    QMap<NodeInfo*, BaseValueClass*>::iterator itr;

    for(itr = node_map.begin(); itr != node_map.end(); ++itr)
    {
        delete itr.value();
    }

    node_map.clear();
}

void ValueManager::ClearData()
{
    if(dataList.size() > 0)
    {
        for(int i = 0; i < dataList.size(); i++)
        {
            delete dataList[i];
        }
    }
    dataList.clear();
    nameList.clear();

    QMap<NodeInfo*, QStringList*>::iterator itr;
    for(itr = eventParamsMap.begin(); itr != eventParamsMap.end(); ++itr)
    {
        delete itr.value();
    }
    eventParamsMap.clear();

    clearNodeMap(nodeFunctionMap);
    clearNodeMap(nodeSetVarMap);
    clearNodeMap(nodeCompareValueLeftMap);
    clearNodeMap(nodeCompareValueRightMap);
}
