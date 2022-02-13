#include "../ItemModels/nodeinfo.h"
#include "valueclass.h"
#include "valuemanager.h"

ValueManager::~ValueManager()
{
    ClearData();
}

ValueManager::ValueManager(NodeInfo* node)
{
    nameList = QStringList();
    dataList = QList<BaseValueClass*>();
    eventNode = node;

    int n = node->getValuesCount();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            AddNewVariable(node->getValue(i));
        }
    }
}

QStringList ValueManager::GetGlobalVarList() const
{
    return nameList;
}

bool ValueManager::AddNewVariable(QString name)
{
    if(nameList.contains(name))
    {
        info("已存在这个名字的变量了。");
        return false;
    }
    else
    {
        nameList << name;
        dataList << new BaseValueClass(name, VT_VAR);
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

QString ValueManager::GetVarTypeAt(int idx)
{
    return "整数";
}

QString ValueManager::GetInitValueOfVar(int idx)
{
    return "0";
}

// 在事件节点（EVENT类型节点）中更新“事件参数”
void ValueManager::UpdateEventParams(int eid)
{
    int n = eventNode->getValuesCount();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
            DeleteVariable(eventNode->getValue(i));
        eventNode->clearValues();
    }

    EVENT_TYPE_ID e_eid = static_cast<EVENT_TYPE_ID>(eid);
    int params_num = EventType::GetInstance()->paramNamesVector[e_eid].size();
    if(params_num > 0)
    {
        for(int i = 0; i < params_num; i++)
        {
            QString pname = EventType::GetInstance()->paramNamesVector[e_eid][i];
            eventNode->addNewValue(pname);
            AddNewVariable(pname);
        }
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
}
