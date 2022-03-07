#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ItemModels/treeitemmodel.h"

class QFile;
class QJsonObject;
class QJsonArray;
class QJsonParseError;
class QListWidgetItem;
class QTableWidgetItem;
class BaseValueClass;

class DlgChoseEType;
class DlgConditionType;
class DlgEditValue;
class DlgSetVariable;
class DlgVariableManager;
class DlgChoseActionType;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    // 弹出菜单
    void slotTreeMenu(const QPoint &pos);
    // 展开Tree节点
    void slotTreeMenuExpand(bool b = false);
    // 折叠Tree节点
    void slotTreeMenuCollapse(bool b = false);

    // eventTree右键菜单功能：
    void slotEditNode(bool b = false);
    void slotCutNode(bool b = false);
    void slotCopyNode(bool b = false);
    void slotPasteNode(bool b = false);
    void slotDeleteNode(bool b = false);
    void slotNewEvent(bool b = false);
    void slotNewCondition(bool b = false);
    void slotNewAction(bool b = false);

    // 点击eventTree节点：
    void on_eventTreeView_clicked(const QModelIndex &index);
    void on_eventTreeView_doubleClicked(const QModelIndex &index);

    // 创建变量
    void on_btnAddVar_clicked();
    // 删除变量
    void on_btnDeleteVar_clicked();
    // 编辑变量
    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);

    // 输出、输入json文件
    void on_actionSave_triggered();
    void on_actionOpen_triggered();

    // 生成lua文件
    void on_actionLua_triggered();

    // 点击左侧关卡列表
    void on_levelList_itemClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;

    DlgChoseEType* m_dlgChoseEvtType;
    DlgConditionType* m_dlgConditionType;
    DlgSetVariable* m_dlgSetVar;
    DlgEditValue* m_dlgEditFunction;
    DlgVariableManager* m_dlgManageVar;
    DlgChoseActionType* m_dlgChoseActionType;

    // 中间树形结构
    NodeInfo* m_curETNode;
    TreeItemModel* m_eventTreeModel;

    void InitEventTree();
    NodeInfo* createNewEventOnTree(int event_type_id, const QString& event_name);

    void editEventName(NodeInfo* node); //编辑事件名称
    void editEventType(NodeInfo* node); //编辑事件类型
    void editActionNode(NodeInfo* node); //编辑动作节点

    // 左侧关卡列表
    QStringList m_levelList;
    void InitLevelTree();

    // 右侧变量列表
    void addOneRowInTable(unsigned int row, const QString& s1, const QString& s2, const QString& s3);
    void updateVarTable();

    // 生成Json文件
    QString getTriggerNameAt(int id);
    void generateJsonDocument(QFile* file);
    void createEventTypeJsonObj(NodeInfo* node, QJsonObject* json);
    void addVariablesToJsonObj(QJsonObject* json);
    void addActionSeqToJsonObj(NodeInfo* node, QJsonObject* json, QString key_name = "SEQUENCE");
    void addConditionToJsonObj(NodeInfo* node, QJsonObject* json);
    void addComparationToJsonArrary(NodeInfo* node, QJsonArray *conditions);
    void addValueToJsonObj(BaseValueClass* value, QJsonObject* json);
    void addFunctionToJsonObj(BaseValueClass* value, QJsonObject* json);

    // 解析Json文件
    bool openJsonFile(QString fileName);
    bool parseJsonArray_Var(QJsonArray* varJsonArray);
    bool parseJsonObj_Event(QJsonObject* eventJsonObj, QString event_name);
    bool parseJsonArray_Condition(QJsonArray* conditions, NodeInfo* condition_node);
    bool parseJsonObj_ActionNode(QJsonObject* actionJsonObj, NodeInfo* parent_node);
    bool parseJsonArray_Sequence(QJsonArray* seqJsonArray, NodeInfo* seq_node);
    BaseValueClass* parseJsonObj_Value(QJsonObject* valueJsonObj); //这个函数会new一个BaseValueClass
    BaseValueClass* parseJsonObj_Function(QJsonObject* funcJsonObj); //这个函数会new一个BaseValueClass

    // 生成Lua文件
    void generateLuaDocument(QFile* file);
    int space_num = 0;
    QStringList* event_args;
    QMap<int, int> event_pos_in_table;
    void writeLuaVariables(QFile* file);
    QString getLuaValueString(BaseValueClass* value);
    QString getLuaCallString(BaseValueClass* value_func);
    bool writeLuaEventInfo(QFile* file, NodeInfo* event_node);
    bool writeLuaEventCheckFunc(QFile* file, NodeInfo* condition_node);
    bool writeLuaCondition(QFile* file, NodeInfo* condition_node);
    bool writeLuaEventActionFunc(QFile* file, NodeInfo* sequence_node);
    bool writeLuaSequence(QFile* file, NodeInfo* sequence_node);
    bool writeLuaSetVar(QFile* file, NodeInfo* setvar_node);
    int findLuaIndexOfEvent(NodeInfo* node);
};

#endif // MAINWINDOW_H
