#include "../ItemModels/nodeinfo.h"
#include "valueclass.h"
#include "valuemanager.h"

ValueManager::~ValueManager()
{
    ClearData();
}

ValueManager *ValueManager::GetValueManager()
{
    static ValueManager* instance = nullptr;
    if(instance == nullptr)
        instance = new ValueManager();
    return instance;
}

ValueManager *ValueManager::GetClipBoardValueManager()
{
    static ValueManager* instance = nullptr;
    if(instance == nullptr)
        instance = new ValueManager();
    return instance;
}

ValueManager::ValueManager()
{
    nameList = QStringList();
    dataList = QList<CommonValueClass*>();
}

void ValueManager::insertVarToMap(QMap<NodeInfo *, CommonValueClass *> &node_map, NodeInfo* node, CommonValueClass *value)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(value != nullptr);

    if(node_map.contains(node))
        delete node_map[node];

    if(value->GetValueType() < VT_TABLE)
        node_map.insert(node, new BaseValueClass((BaseValueClass*)value));
    else if(value->GetValueType() == VT_TABLE)
        node_map.insert(node, new StructValueClass((StructValueClass*)value));
}


QStringList ValueManager::GetGlobalVarList() const
{
    QStringList list;
    for(int i = 0; i < nameList.size(); i++)
    {
        if(dataList.at(i) != nullptr)
            list.push_back(nameList.at(i));
    }
    return list;
}

QString ValueManager::GetVarNameAt(int id)
{
    if(id < 0 || id >= nameList.size())
    {
        info("GetVarName id 越界");
        return "";
    }
    return nameList[id];
}

int ValueManager::GetIdOfVariable(CommonValueClass *v)
{
    if(v->GetValueType() != VT_VAR)
        return -1;
    return nameList.indexOf(v->GetText());
}

int ValueManager::GetIdOfVariable(const QString &var_name)
{
    return nameList.indexOf(var_name);
}

bool ValueManager::AddNewVariable(QString name, CommonValueClass* v, bool is_level_param)
{
    if(nameList.contains(name))
    {
        info("已存在这个名字的变量了。");
        return false;
    }
    if("" == name)
    {
        info("变量名不能为空");
        return false;
    }
    else
    {
        CommonValueClass* new_value;
        if(v == nullptr)
            new_value = new BaseValueClass("nil");
        else if(v->GetValueType() == VT_TABLE)
            new_value = new StructValueClass((StructValueClass*)v);
        else
            new_value = new BaseValueClass((BaseValueClass*)v);

        for(int i = 0; i < nameList.size(); i++)
        {
            if(nameList[i] == "")
            {
                nameList[i] = name;
                dataList[i] = new_value;
                if(is_level_param)
                    updateLevelParam(i, is_level_param);
                return true;
            }
        }

        if(is_level_param)
            updateLevelParam(nameList.size(), is_level_param);
        nameList << name;
        dataList << new_value;

        return true;
    }
}

bool ValueManager::AddNewVarAtPos(QString name, CommonValueClass *v, int pos, bool is_level_param)
{
    if(nameList.contains(name))
    {
        info("已存在这个名字的变量了。");
        return false;
    }
    if("" == name)
    {
        info("变量名不能为空");
        return false;
    }
    else
    {
        int n = nameList.size();
        if(pos < n)
        {
            info("AddNewVarAtPos(" + QString::number(pos) + ") fail.");
            return false;
        }
        else if(pos > n)
        {
            for(int i = n; i < pos; i++)
            {
                nameList << "";
                dataList << nullptr;
            }
        }

        if(is_level_param)
            updateLevelParam(nameList.size(), is_level_param);
        nameList << name;
        dataList << v;

        return true;
    }
}

bool ValueManager::CheckVarIsUsedOrNot(const QString &var_name)
{
    QMap<NodeInfo*, CommonValueClass*>::iterator itr;

    // function
    for(itr = nodeFunctionMap.begin(); itr != nodeFunctionMap.end(); ++itr)
    {
        if(itr.value()->IsUsingVar(var_name))
            return true;
    }

    // set var
    for(itr = nodeSetVarMap.begin(); itr != nodeSetVarMap.end(); ++itr)
    {
        int var_id = itr.key()->getValue(0).toInt();
        if(nameList[var_id] == var_name)
        {
//            info("set_var " + itr.key()->text + "在使用");
            return true;
        }
        if(itr.value()->IsUsingVar(var_name))
            return true;
    }

    // compare
    for(itr = nodeCompareValueLeftMap.begin(); itr != nodeCompareValueLeftMap.end(); ++itr)
    {
        if(itr.value()->IsUsingVar(var_name))
            return true;
    }
    for(itr = nodeCompareValueRightMap.begin(); itr != nodeCompareValueRightMap.end(); ++itr)
    {
        if(itr.value()->IsUsingVar(var_name))
            return true;
    }

    return false;
}

bool ValueManager::DeleteVariable(QString name)
{
    int id = -1;
    int n = nameList.size();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            if(nameList[i] == name)
            {
                id = i;
                break;
            }
        }
    }
    if(id != -1)
    {
        DeleteVarAt(id);
        return true;
    }
    else
        return false;
}

void ValueManager::DeleteVarAt(int id)
{
    if(id < 0 || id >= dataList.size())
    {
        info("id越界，删除变量失败！");
        return;
    }

    nameList[id] = "";
    delete dataList[id];
    dataList[id] = nullptr;
}

void ValueManager::ModifyVarValueAt(int idx, QString name, CommonValueClass *value, bool is_level_param)
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

    delete dataList[idx];
    if(value == nullptr)
        dataList[idx] = nullptr;
    else if(value->GetValueType() < VT_TABLE)
        dataList[idx] = new BaseValueClass((BaseValueClass*)value);
    else
        dataList[idx] = new StructValueClass((StructValueClass*)value);

    if(nameList[idx] != name)
    {
        // 检测并更新 VT_TABLE 变量中引用到的 old_name
        int n = dataList.size();
        for(int i = 0; i < n; i++)
        {
            if(dataList[i]->GetValueType() == VT_TABLE)
                updateVarNameOfInitVar(static_cast<StructValueClass*>(dataList[i]), nameList[idx], name);
        }

        nameList[idx] = name;
    }

    updateLevelParam(idx, is_level_param);
    updateVarOnNodes(idx);
}

void ValueManager::ModifyInitValueAt(int idx, CommonValueClass *value)
{
    if(idx < 0 || idx >= nameList.size())
    {
        info("不存在这个id的变量");
        return;
    }

    if(value != nullptr)
    {
        delete dataList[idx];
        if(value->GetValueType() < VT_TABLE)
            dataList[idx] = new BaseValueClass((BaseValueClass*)value);
        else
            dataList[idx] = new StructValueClass((StructValueClass*)value);
    }
}

bool ValueManager::CheckVarIsLevelParam(int idx)
{
    if(levelParamMap.contains(idx))
        return levelParamMap[idx];
    else
        return false;
}

void ValueManager::UpdateValueOnNode_SetValue(NodeInfo *node, CommonValueClass *value)
{
    MY_ASSERT(node != nullptr);
    if(node->type != SET_VAR)
    {
        info("UpdateValueOnNode_SetValue node->type=" + getNodeTypeStr(node->type));
        return;
    }

    insertVarToMap(nodeSetVarMap, node, value);
}

CommonValueClass *ValueManager::GetValueOnNode_SetVar(NodeInfo* node)
{
    MY_ASSERT(node != nullptr);
    if(node->type != SET_VAR)
    {
        info("GetValueOnNode_SetVar node->type=" + getNodeTypeStr(node->type));
        return nullptr;
    }

    if(nodeSetVarMap.contains(node))
        return nodeSetVarMap[node];
    else
        return nullptr;
}

void ValueManager::UpdateValueOnNode_Function(NodeInfo *node, CommonValueClass *value)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(node->type == FUNCTION);

    MY_ASSERT(value != nullptr);
    VALUE_TYPE vt = value->GetValueType();
    MY_ASSERT(vt == VT_FUNC || vt == VT_STR);

    insertVarToMap(nodeFunctionMap, node, value);
}

BaseValueClass *ValueManager::GetValueOnNode_Function(NodeInfo *node)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(node->type == FUNCTION);

    if(nodeFunctionMap.contains(node))
        return (BaseValueClass*)nodeFunctionMap[node];
    else
        return nullptr;
}

void ValueManager::UpdateValueOnNode_Compare_Left(NodeInfo *node, CommonValueClass *value)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(value != nullptr);
    MY_ASSERT(node->type == COMPARE);
    insertVarToMap(nodeCompareValueLeftMap, node, value);
}

void ValueManager::UpdateValueOnNode_Compare_Right(NodeInfo *node, CommonValueClass *value)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(value != nullptr);
    MY_ASSERT(node->type == COMPARE);
    insertVarToMap(nodeCompareValueRightMap, node, value);
}

CommonValueClass *ValueManager::GetValueOnNode_Compare_Left(NodeInfo *node)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(node->type == COMPARE);

    if(nodeCompareValueLeftMap.contains(node))
        return nodeCompareValueLeftMap[node];
    else
        return nullptr;
}

CommonValueClass *ValueManager::GetValueOnNode_Compare_Right(NodeInfo *node)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(node->type == COMPARE);

    if(nodeCompareValueRightMap.contains(node))
        return nodeCompareValueRightMap[node];
    else
        return nullptr;
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
    {
        info("GetVarTypeOf " + name + "fail.");
        return "";
    }
}

CommonValueClass* ValueManager::GetInitValueOfVar(int idx)
{
    return dataList.at(idx);
}

CommonValueClass *ValueManager::GetInitValueOfVarByName(const QString &name)
{
    int idx = nameList.indexOf(name);
    if(idx == -1)
        return nullptr;
    else
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

    NodeInfo* etype_node = node->childs[0]->childs[0];
    MY_ASSERT(etype_node->type == ETYPE);
    MY_ASSERT(etype_node->getValuesCount() > 0);

    int eid = etype_node->getValue(0).toInt();
    return EventType::GetInstance()->GetEventParamsUIAt(eid);
}

QStringList *ValueManager::GetEventParamsLua(NodeInfo *node)
{
    MY_ASSERT(node->type == EVENT);
    MY_ASSERT(node->childs.size() > 1);

    NodeInfo* etype_node = node->childs[0]->childs[0];
    MY_ASSERT(etype_node->type == ETYPE);
    MY_ASSERT(etype_node->getValuesCount() > 0);

    int eid = etype_node->getValue(0).toInt();
    return EventType::GetInstance()->GetEventParamsLuaAt(eid);
}

bool ValueManager::CustomSeqNameIsUsed(const QString &name)
{
    QMap<NodeInfo*, CommonValueClass*>::iterator itr;
    for(itr = nodeFunctionMap.begin(); itr != nodeFunctionMap.end(); ++itr)
    {
        if(itr.value()->GetValueType() == VT_STR)
        {
            QString lua_str = itr.value()->GetText();
            if(lua_str.contains("自定义动作：") && lua_str.contains(name))
                return true;
        }
    }
    return false;
}

void ValueManager::UpdateCustomSeqName(const QString &old_name, const QString &new_name)
{
    // 更新Function节点上的自定义动作名称
    QMap<NodeInfo*, CommonValueClass*>::iterator itr;
    for(itr = nodeFunctionMap.begin(); itr != nodeFunctionMap.end(); ++itr)
    {
        if(itr.value()->GetValueType() == VT_STR)
        {
            QString lua_str = itr.value()->GetText();
            if(lua_str.contains("自定义动作：") && lua_str.contains(old_name))
            {
                lua_str.replace(old_name, new_name);
                static_cast<BaseValueClass*>(itr.value())->SetLuaStr(lua_str);
                itr.key()->text = lua_str;
            }
        }
    }

//    if(customSeqNodeMap.contains(old_name))
//    {
//        if(customSeqNodeMap.contains(new_name))
//        {
//            info(new_name + "已存在");
//        }
//        else
//        {
//            customSeqNodeMap.insert(new_name, customSeqNodeMap[old_name]);
//            customSeqNodeMap.remove(old_name);
//        }
//    }
//    else
//        info(old_name + "不存在");
}

//void ValueManager::AddNewCustomSequence(const QString &name, NodeInfo *seq_node)
//{
//    if(customSeqNodeMap.contains(name))
//    {
//        info(name + "已存在");
//    }
//    else
//    {
//        customSeqNodeMap.insert(name, seq_node);
//    }
//}

//void ValueManager::DeleteCustomSequence(const QString &name)
//{
//    if(customSeqNodeMap.contains(name))
//    {
//        customSeqNodeMap.remove(name);
//    }
//    else
//        info(name + "不存在");
//}

void ValueManager::updateVarOnNodes(int var_id)
{
    QString init_var_type = dataList.at(var_id)->GetVarType();
    QMap<NodeInfo*, CommonValueClass*>::iterator itr;

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
//        QString value_var_type = itr.value()->GetVarType();
//        if(init_var_type != value_var_type && value_var_type != "")
//            info(itr.key()->text + "，变量和值的类型不一致");
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

void ValueManager::updateVarNameOfInitVar(StructValueClass *v, const QString &old_name, const QString &new_name)
{
    QStringList keys = v->GetAllKeys();
    int n = keys.size();
    for(int i = 0; i < n; i++)
    {
        CommonValueClass* value = v->GetValueByKey(keys[i]);

        if(value->GetValueType() == VT_VAR)
        {
            if(value->GetText() == old_name)
                static_cast<BaseValueClass*>(value)->ModifyVarName(new_name);
        }
        else if(value->GetValueType() == VT_TABLE)
        {
            updateVarNameOfInitVar(static_cast<StructValueClass*>(value), old_name, new_name);
        }
    }
}

void ValueManager::updateLevelParam(int var_id, bool is_param)
{

    if(levelParamMap.contains(var_id))
    {
        levelParamMap[var_id] = is_param;
    }
    else if(is_param)
    {
        levelParamMap.insert(var_id, true);
    }
}

void ValueManager::clearNodeMap(QMap<NodeInfo *, CommonValueClass *> &node_map)
{
    QMap<NodeInfo*, CommonValueClass*>::iterator itr;

    for(itr = node_map.begin(); itr != node_map.end(); ++itr)
    {
        delete itr.value();
    }

    node_map.clear();
}

void ValueManager::deleteNodeInMap(QMap<NodeInfo *, CommonValueClass *> &node_map, NodeInfo *node)
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
    levelParamMap.clear();

    clearNodeMap(nodeFunctionMap);
    clearNodeMap(nodeSetVarMap);
    clearNodeMap(nodeCompareValueLeftMap);
    clearNodeMap(nodeCompareValueRightMap);
}
