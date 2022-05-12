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
    static QString GetVarNameInKeyStr(const QString &str, int *pos = nullptr); //从类似v.x.y这样的字符串中提取出v
    static QString GetKeyNameInKeyStr(const QString &str, int *pos = nullptr); //从类似v.x.y这样的字符串中提取出y
    void ClearData();

    QStringList GetGlobalVarList() const;
    QString GetVarNameAt(int id);
    int GetIdOfVariable(CommonValueClass* v);
    int GetIdOfVariable(const QString& var_name);
    bool AddNewVariable(QString name, CommonValueClass *v, bool is_level_param = false); //新增全局变量（作用域为整个事件）
    bool AddNewVarAtPos(QString name, CommonValueClass* v, int pos, bool is_level_param = false);
    bool CheckVarIsUsedOrNot(const QString& var_name);
    bool DeleteVariable(QString name); //删除全局变量
    void DeleteVarAt(int id);
    void ModifyVarValueAt(int idx, QString name, CommonValueClass* value, bool is_level_param = false); //修改全局变量的名字和初始值
    void ModifyInitValueAt(int idx, CommonValueClass* value); //只设置变量的初始值
    bool CheckVarIsLevelParam(int idx);

    QString GetVarTypeAt(int idx);
    QString GetVarTypeOf_Table(const QString& name);
    QString GetVarTypeOf_Key(const QString& name);
    CommonValueClass* GetInitValueOfVar(int idx);
    CommonValueClass* GetInitValueOfVarByName(const QString& name);
    int FindIdOfVarName(const QString& name);

    void UpdateValueOnNode_SetValue(NodeInfo* node, CommonValueClass *value); //setValue节点上的变量
    CommonValueClass* GetValueOnNode_SetVar(NodeInfo* node);

    void UpdateValueOnNode_Function(NodeInfo* node, CommonValueClass *value); //function节点上的变量
    BaseValueClass* GetValueOnNode_Function(NodeInfo* node);

    void UpdateValueOnNode_Compare_Left(NodeInfo* node, CommonValueClass *value); //compare节点上的变量
    void UpdateValueOnNode_Compare_Right(NodeInfo* node, CommonValueClass* value);
    CommonValueClass *GetValueOnNode_Compare_Left(NodeInfo* node);
    CommonValueClass* GetValueOnNode_Compare_Right(NodeInfo* node);

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

    void insertVarToMap(QMap<NodeInfo*, CommonValueClass*>& node_map, NodeInfo *node, CommonValueClass* value);
    void clearNodeMap(QMap<NodeInfo*, CommonValueClass*>& node_map);
    void deleteNodeInMap(QMap<NodeInfo*, CommonValueClass*>& node_map, NodeInfo* node);

    void updateVarOnNodes(int var_id); //更新所有变量ID为var_id的value的name（包括函数参数中的value）
    void updateVarNameOfInitVar(StructValueClass* v, const QString& old_name, const QString& new_name);
    void updateLevelParam(int var_id, bool is_param);

    QList<CommonValueClass*> dataList; //全局变量表（存放初始值）
    QStringList nameList;  //全局变量名表，与dataList变量表一一对应
    QMap<int, bool> levelParamMap; //指定id的变量是否为关卡参数

    // 节点上关联的值
    QMap<NodeInfo*, CommonValueClass*> nodeFunctionMap;
    QMap<NodeInfo*, CommonValueClass*> nodeSetVarMap;
    QMap<NodeInfo*, CommonValueClass*> nodeCompareValueLeftMap;
    QMap<NodeInfo*, CommonValueClass*> nodeCompareValueRightMap;

    // 自定义动作
//    QMap<QString, NodeInfo*> customSeqNodeMap;
};

#endif // VALUEMANAGER_H
