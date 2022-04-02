#ifndef VALUEMANAGER_H
#define VALUEMANAGER_H

#include <QList>
#include "valueclass.h"

class NodeInfo;

class ValueManager
{
public:
    ~ValueManager();
    static ValueManager* GetValueManager();
    static ValueManager* GetClipBoardValueManager();
    void ClearData();

    QStringList GetGlobalVarList() const;
    QString GetVarNameAt(int id);
    int GetIdOfVariable(BaseValueClass* v);
    int GetIdOfVariable(const QString& var_name);
    bool AddNewVariable(QString name, BaseValueClass* v); //新增全局变量（作用域为整个事件）
    bool AddNewVarAtPos(QString name, BaseValueClass* v, int pos);
    bool CheckVarIsUsedOrNot(const QString& var_name);
    bool DeleteVariable(QString name); //删除全局变量
    void DeleteVarAt(int id);
    void ModifyVarValueAt(int idx, QString name, BaseValueClass* value); //修改全局变量的名字和初始值
    void ModifyInitValueAt(int idx, BaseValueClass* value); //只设置变量的初始值

    QString GetVarTypeAt(int idx);
    QString GetVarTypeOf(const QString& name);
    BaseValueClass* GetInitValueOfVar(int idx);
    BaseValueClass* GetInitValueOfVarByName(const QString& name);
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

    // 自定义动作序列
    bool CustomSeqNameIsUsed(const QString& name);
    void UpdateCustomSeqName(const QString& old_name, const QString& new_name);
//    void AddNewCustomSequence(const QString& name, NodeInfo* seq_node);
//    void DeleteCustomSequence(const QString& name);

private:
    ValueManager();

    void clearNodeMap(QMap<NodeInfo*, BaseValueClass*>& node_map);
    void deleteNodeInMap(QMap<NodeInfo*, BaseValueClass*>& node_map, NodeInfo* node);

    void updateVarOnNodes(int var_id); //更新所有变量ID为var_id的value的name（包括函数参数中的value）

    QList<BaseValueClass*> dataList; //全局变量表（存放初始值）
    QStringList nameList;  //全局变量名表，与dataList变量表一一对应

    // 节点上关联的值
    QMap<NodeInfo*, BaseValueClass*> nodeFunctionMap;
    QMap<NodeInfo*, BaseValueClass*> nodeSetVarMap;
    QMap<NodeInfo*, BaseValueClass*> nodeCompareValueLeftMap;
    QMap<NodeInfo*, BaseValueClass*> nodeCompareValueRightMap;

    // 自定义动作
//    QMap<QString, NodeInfo*> customSeqNodeMap;
};

#endif // VALUEMANAGER_H
