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
class QModelIndex;
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

    void closeEvent(QCloseEvent *e);

private slots:
    // 弹出菜单
    void slotTreeMenu(const QPoint &pos);
    // 展开Tree节点
    void slotTreeMenuExpand(bool b = false);
    void saveEventItemState_Expanded(const QModelIndex &index);
    // 折叠Tree节点
    void slotTreeMenuCollapse(bool b = false);
    void saveEventItemState_Collapsed(const QModelIndex &index);

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
    // 关卡列表右键菜单
    void on_levelList_customContextMenuRequested(const QPoint &pos);
    // 创建新的关卡（拷贝当前选中的关卡）
    void CreateNewLevel_CopyCurLvl();
    // 删除当前选中的关卡
    void DeleteCurrentLevel();

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
    QMap<QModelIndex, bool> m_itemState;
    void updateEventTreeState();

    void InitEventTree();
    NodeInfo* createNewEventOnTree(QString event_type, const QString& event_name);

    void editEventName(NodeInfo* node); //编辑事件名称
    void editEventType(NodeInfo* node); //编辑事件类型
    void editActionNode(NodeInfo* node); //编辑动作节点

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
    QMap<int, int> event_pos_in_table;
    void writeLuaVariables(QFile* file);
    void writeLuaVarInitFunc(QFile* file);
    QString getLuaValueString(BaseValueClass* value);
    QString getLuaCallString(BaseValueClass* value_func);
    bool writeLuaEventInfo(QFile* file, NodeInfo* event_node);
    bool writeLuaEventCheckFunc(QFile* file, NodeInfo* condition_node);
    bool writeLuaCondition(QFile* file, NodeInfo* condition_node);
    bool writeLuaEventActionFunc(QFile* file, NodeInfo* sequence_node);
    bool writeLuaSequence(QFile* file, NodeInfo* sequence_node);
    bool writeLuaSetVar(QFile* file, NodeInfo* setvar_node);
    bool writeLuaOpenOrCloseEvent(QFile* file, int event_pos, const QString& pre_str, bool is_open);
    int findLuaIndexOfEvent(NodeInfo* node);

    // 获取exe所在目录中的config目录
    void getConfigPath(QString& s);

    // 左侧关卡列表
    QStringList m_levelList;
    void InitLevelTree();

    // 备份Json关卡文件
    int lastLevelIndex;
    QMap<QString, QStringList> backupFilePaths;
    QMap<QString, bool> savedOrNot;
    bool saveBackupJsonFile();
    void changeSavedFlag(const QString& level_name, bool already_saved);
    bool isSameFile(const QString& path1, const QString& path2);
    void deleteFile(const QString& path);
    QString getLevelNameOnItem(QListWidgetItem* item);
};

#endif // MAINWINDOW_H
