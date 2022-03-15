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

    *(dataList[idx]) = *value;
    nameList[idx] = name;

    UpdateVarOnNodes(idx);
}

void ValueManager::UpdateValueOnNode_SetValue(NodeInfo *node, BaseValueClass *value)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(node->type == SET_VAR);

    MY_ASSERT(value != nullptr);

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
    MY_ASSERT(node != nullptr);
    MY_ASSERT(node->type == SET_VAR);

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
    MY_ASSERT(node != nullptr);
    MY_ASSERT(node->type == FUNCTION);

    MY_ASSERT(value != nullptr);
    VALUE_TYPE vt = value->GetValueType();
    MY_ASSERT(vt == VT_FUNC || vt == VT_STR);

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
    MY_ASSERT(node != nullptr);
    MY_ASSERT(node->type == FUNCTION);

    if(nodeFunctionMap.contains(node))
    {
        return nodeFunctionMap[node];
    }
    else
    {
        return nullptr;
    }
}

void ValueManager::UpdateValueOnNode_Compare_Left(NodeInfo *node, BaseValueClass *value)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(value != nullptr);
    MY_ASSERT(node->type == COMPARE);

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
    MY_ASSERT(node != nullptr);
    MY_ASSERT(value != nullptr);
    MY_ASSERT(node->type == COMPARE);

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
    MY_ASSERT(node != nullptr);
    MY_ASSERT(node->type == COMPARE);

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
    MY_ASSERT(node != nullptr);
    MY_ASSERT(node->type == COMPARE);

    if(nodeCompareValueRightMap.contains(node))
    {
        return nodeCompareValueRightMap[node];
    }
    else
    {
        return nullptr;
    }
}

void ValueManager::OnDeleteNode(NodeInfo *node)
{
    for(int i = 0; i < node->childs.size(); i++)
       OnDeleteNode(node->childs[i]);

    deleteNodeInMap(nodeFunctionMap, node);
    deleteNodeInMap(nodeSetVarMap, node);
    deleteNodeInMap(nodeCompareValueLeftMap, node);
    deleteNodeInMap(nodeCompareValueRightMap, node);
}

QString ValueManager::GetVarTypeAt(int idx)
{
    if(idx >= dataList.size() || idx < 0)
    {
        info("GetVarTypeAt(" + QString::number(idx) + ")参数越界！");
        return "";
    }
    else
    {
        QString init_value_type = dataList.at(idx)->GetVarType();
        if(init_value_type == "")
            return "未知类型";
        else
            return init_value_type;
    }
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

QStringList *ValueManager::GetEventParamsUI(NodeInfo *node)
{
    MY_ASSERT(node->type == EVENT);
    MY_ASSERT(node->childs.size() > 1);

    NodeInfo* etype_node = node->childs[0];
    MY_ASSERT(etype_node->type == ETYPE);
    MY_ASSERT(etype_node->getValuesCount() > 0);

    int eid = etype_node->getValue(0).toInt();
    return EventType::GetInstance()->GetEventParamsUIAt(eid);
}

QStringList *ValueManager::GetEventParamsLua(NodeInfo *node)
{
    MY_ASSERT(node->type == EVENT);
    MY_ASSERT(node->childs.size() > 1);

    NodeInfo* etype_node = node->childs[0];
    MY_ASSERT(etype_node->type == ETYPE);
    MY_ASSERT(etype_node->getValuesCount() > 0);

    int eid = etype_node->getValue(0).toInt();
    return EventType::GetInstance()->GetEventParamsLuaAt(eid);
}

void ValueManager::UpdateVarOnNodes(int var_id)
{
    QString init_var_type = dataList.at(var_id)->GetVarType();
    QMap<NodeInfo*, BaseValueClass*>::iterator itr;

    // function
    for(itr = nodeFunctionMap.begin(); itr != nodeFunctionMap.end(); ++itr)
    {
        itr.value()->UpdateVarNameAndType(var_id, nameList[var_id], init_var_type);
        itr.key()->text = itr.value()->GetText();
    }

    // set var
    for(itr = nodeSetVarMap.begin(); itr != nodeSetVarMap.end(); ++itr)
    {
        itr.value()->UpdateVarNameAndType(var_id, nameList[var_id], init_var_type);
        if(itr.key()->getValue(0).toInt() == var_id)
        {
            itr.key()->modifyValue(0, QString::number(var_id));
            itr.key()->text = nameList[var_id] + " = " + itr.value()->GetText();
        }
        QString value_var_type = itr.value()->GetVarType();
        if(init_var_type != value_var_type && value_var_type != "")
            info(itr.key()->text + "，变量和值的类型不一致");
    }

    // compare
    for(itr = nodeCompareValueLeftMap.begin(); itr != nodeCompareValueLeftMap.end(); ++itr)
    {
        itr.value()->UpdateVarNameAndType(var_id, nameList[var_id], init_var_type);
        itr.key()->modifyValue(1, itr.value()->GetText());
    }
    for(itr = nodeCompareValueRightMap.begin(); itr != nodeCompareValueRightMap.end(); ++itr)
    {
        itr.value()->UpdateVarNameAndType(var_id, nameList[var_id], init_var_type);
        itr.key()->modifyValue(2, itr.value()->GetText());
        itr.key()->UpdateText();
        QString var_type_left = nodeCompareValueLeftMap[itr.key()]->GetVarType();
        QString var_type_right = itr.value()->GetVarType();
        if(var_type_left != "" && var_type_right != "" && var_type_left != var_type_right)
            info("比较" + itr.key()->text + "，左右值类型不一致");
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

void ValueManager::deleteNodeInMap(QMap<NodeInfo *, BaseValueClass *> &node_map, NodeInfo *node)
{
    if(node_map.contains(node))
    {
        delete node_map[node];
        node_map[node] = nullptr;
        node_map.remove(node);
    }
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

    clearNodeMap(nodeFunctionMap);
    clearNodeMap(nodeSetVarMap);
    clearNodeMap(nodeCompareValueLeftMap);
    clearNodeMap(nodeCompareValueRightMap);
}
