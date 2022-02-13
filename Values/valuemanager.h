#ifndef VALUEMANAGER_H
#define VALUEMANAGER_H

#include <QList>
#include "valueclass.h"

class NodeInfo;

class ValueManager
{
public:
    ~ValueManager();
    ValueManager(NodeInfo* node);

    QStringList GetGlobalVarList() const;

    bool AddNewVariable(QString name); //新增全局变量（作用域为整个事件）
    void DeleteVariable(QString name); //删除全局变量

    QString GetVarTypeAt(int idx);
    QString GetInitValueOfVar(int idx);

    void UpdateEventParams(int eid); //根据事件类型（EventType）添加变量（也就是事件参数）

private:
    void ClearData();

    QList<BaseValueClass*> dataList; //变量表
    QStringList nameList;  //变量名表，与dataList变量表一一对应

    NodeInfo* eventNode; //关联的节点（m_pRootNode的子节点）
};

#endif // VALUEMANAGER_H
