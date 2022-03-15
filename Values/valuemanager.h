#ifndef VALUEMANAGER_H
#define VALUEMANAGER_H

#include <QList>
#include "valueclass.h"

class NodeInfo;

class ValueManager
{
public:
    ~ValueManager();
    ValueManager();

    void ClearData();

    QStringList GetGlobalVarList() const;
    int GetIdOfVariable(BaseValueClass* v);
    bool AddNewVariable(QString name, BaseValueClass* v); //新增全局变量（作用域为整个事件）
    void DeleteVariable(QString name); //删除全局变量
    void ModifyVarValueAt(int idx, QString name, BaseValueClass* value); //设置全局变量的初始值

    QString GetVarTypeAt(int idx);
    QString GetVarTypeOf(const QString& name);
    BaseValueClass* GetInitValueOfVar(int idx);
    int FindIdOfVarName(const QString& name);

    void UpdateValueOnNode_SetValue(NodeInfo* node, BaseValueClass* value); //setValue节点上的变量
    BaseValueClass* GetValueOnNode_SetVar(NodeInfo* node);

    void UpdateValueOnNode_Function(NodeInfo* node, BaseValueClass* value); //function节点上的变量
    BaseValueClass* GetValueOnNode_Function(NodeInfo* node);

    void UpdateValueOnNode_Compare_Left(NodeInfo* node, BaseValueClass* value); //compare节点上的变量
    void UpdateValueOnNode_Compare_Right(NodeInfo* node, BaseValueClass* value);
    BaseValueClass* GetValueOnNode_Compare_Left(NodeInfo* node);
    BaseValueClass* GetValueOnNode_Compare_Right(NodeInfo* node);

    void OnDeleteNode(NodeInfo* node); //删除节点时更新map

    QStringList* GetEventParamsUI(NodeInfo* node);
    QStringList* GetEventParamsLua(NodeInfo* node);

    void UpdateVarOnNodes(int var_id); //更新所有变量ID为var_id的value的name（包括函数参数中的value）

private:
    void clearNodeMap(QMap<NodeInfo*, BaseValueClass*>& node_map);
    void deleteNodeInMap(QMap<NodeInfo*, BaseValueClass*>& node_map, NodeInfo* node);

    QList<BaseValueClass*> dataList; //全局变量表（存放初始值）
    QStringList nameList;  //全局变量名表，与dataList变量表一一对应

    // 节点上关联的值
    QMap<NodeInfo*, BaseValueClass*> nodeFunctionMap;
    QMap<NodeInfo*, BaseValueClass*> nodeSetVarMap;
    QMap<NodeInfo*, BaseValueClass*> nodeCompareValueLeftMap;
    QMap<NodeInfo*, BaseValueClass*> nodeCompareValueRightMap;

    // LevelClass* lvl; // 对应的关卡
};

#endif // VALUEMANAGER_H
