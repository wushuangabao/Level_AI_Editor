#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ItemModels/treeitemmodel_event.h"
#include "ItemModels/treeitemmodel_custom.h"

#define MAX_BACKUP_NUM 16 //最多连续撤销15次

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
    void slotTreeMenu_Event(const QPoint &pos);
    void slotTreeMenu_Custom(const QPoint &pos);
    // 展开Tree节点
    void saveEventItemState_Expanded(const QModelIndex &index);
    void saveCustomItemState_Expanded(const QModelIndex &index);
    // 折叠Tree节点
    void saveEventItemState_Collapsed(const QModelIndex &index);
    void saveCustomItemState_Collapsed(const QModelIndex &index);

    // eventTree右键菜单功能：
    void slotEditNode(bool b = false);
    void slotCutNode(bool b = false);
    void slotCopyNode(bool b = false);
    void slotPasteNode(bool b = false);
    void slotPasteEventOrCustAct(bool b = false);
    void slotDeleteNode(bool b = false);
    void slotNewEvent(bool b = false);
    void slotNewCondition(bool b = false);
    void slotNewAction(bool b = false);
    // customTree右键菜单：
    void slotNewCustomSeq(bool b = false);

    // 点击eventTree节点：
    void on_eventTreeView_clicked(const QModelIndex &index);
    void on_eventTreeView_doubleClicked(const QModelIndex &index);
    // 点击customTree节点：
    void on_customTreeView_clicked(const QModelIndex &index);
    void on_customTreeView_doubleClicked(const QModelIndex &index);

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
    // 创建新的关卡
    void CreateNewLevel_CopyCurLvl();
    void CreateNewLevel_EmptyLvl();
    // 删除当前选中的关卡
    void DeleteCurrentLevel();
    // 针对所有关卡的操作
    void ReloadAllLevels();
    void SaveAllLevels_Json();
    void SaveAllLevels_Lua();

    void OpenConfigFolder();
    void OpenLuaFolder();

    // 切换tab时 切换树模型
    void on_tabWidget_currentChanged(int index);

    // 编辑 - 撤销、重做
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();

    // View - 显示、隐藏
    void on_actionShowLevels_triggered(bool checked);
    void on_actionShowVars_triggered(bool checked);
    void on_listWidget_visibilityChanged(bool visible);
    void on_propertiesWidget_visibilityChanged(bool visible);

private:
    Ui::MainWindow *ui;

    DlgChoseEType* m_dlgChoseEvtType;
    DlgConditionType* m_dlgConditionType;
    DlgSetVariable* m_dlgSetVar;
    DlgEditValue* m_dlgEditFunction;
    DlgVariableManager* m_dlgManageVar;
    DlgChoseActionType* m_dlgChoseActionType;

    // 树形结构
    NodeInfo* m_curNode;
    QMap<QModelIndex, bool> m_itemState_Event;
    QMap<QModelIndex, bool> m_itemState_Custom;
    void resetTreeSate();
    void setModelForDlg(TreeItemModel *model);
    // 事件树
    TreeItemModel_Event* m_eventTreeModel;
    void updateEventTreeState();
    void InitEventTree();
    NodeInfo* createNewEventOnTree(QString event_type, const QString& event_name);
    // 自定义动作树
    TreeItemModel_Custom* m_customTreeModel;
    void InitCustomTree();

    void editEventName(NodeInfo* node); //编辑事件名称
    void editEventType(NodeInfo* node); //编辑事件类型
    void editActionNode(NodeInfo* node); //编辑动作节点
    void editCustActSeqName(NodeInfo* node); //自定义动作序列的名称

    // 右侧变量列表
    void addOneRowInTable(unsigned int row, const QString& s1, const QString& s2, const QString& s3, const QString &s4);
    void updateVarTable();

    // 生成Json文件
    QString getTriggerNameAt(int id);
    void generateJsonDocument(QFile* file);
    void createEventTypeJsonObj(NodeInfo* node, QJsonObject* json);
    void addVariablesToJsonObj(QJsonObject* json);
    void addActionSeqToJsonObj(NodeInfo* node, QJsonObject* json, QString key_name = "SEQUENCE");
    void addConditionToJsonObj(NodeInfo* node, QJsonObject* json);
    void addComparationToJsonArrary(NodeInfo* node, QJsonArray *conditions);
    void addValueToJsonObj(CommonValueClass *value, QJsonObject* json);
    void addFunctionToJsonObj(BaseValueClass* value, QJsonObject* json);

    // 解析Json文件
    bool openJsonFile(QString filePath);
    bool openJsonFile(QListWidgetItem* item, QString &level_name); // 打开新的关卡文件或者对应的备份文件
    bool parseJsonArray_Var(QJsonArray* varJsonArray);
    bool parseJsonObj_Event(QJsonObject* eventJsonObj, QString event_name);
    bool parseJsonArray_Condition(QJsonArray* conditions, NodeInfo* condition_node);
    bool parseJsonObj_ActionNode(QJsonObject* actionJsonObj, NodeInfo* parent_node);
    bool parseJsonArray_Sequence(QJsonArray* seqJsonArray, NodeInfo* seq_node);
    CommonValueClass *parseJsonObj_Value(QJsonObject* valueJsonObj); //这个函数会new一个BaseValueClass
    BaseValueClass* parseJsonObj_Function(QJsonObject* funcJsonObj); //这个函数会new一个BaseValueClass

    // 生成Lua文件
    void generateLuaDocument(QFile* file);
    int space_num = 0;
    QMap<int, int> event_pos_in_table;
    void writeLuaVariables(QFile* file);
    void writeLuaCustomActions(QFile* file);
    void writeLuaVarInitFunc(QFile* file);
    void writeLuaGetParamFunc(QFile* file);
    bool writeLuaEventInfo(QFile* file, NodeInfo* event_node);
    bool writeLuaEventCheckFunc(QFile* file, NodeInfo* condition_node);
    bool writeLuaCondition(QFile* file, NodeInfo* condition_node);
    bool writeLuaEventActionFunc(QFile* file, NodeInfo* sequence_node);
    bool writeLuaSequence(QFile* file, NodeInfo* sequence_node);
    bool writeLuaSetVar(QFile* file, NodeInfo* setvar_node);
    bool writeLuaOpenOrCloseEvent(QFile* file, int event_pos, const QString& pre_str, bool is_open);
    int findLuaIndexOfEvent(NodeInfo* node);

    // config目录
    QString config_path;

    // 左侧关卡列表
    QString m_levelPrefix;
    QStringList m_levelList;
    QString m_LuaPath;
    void InitLevelTree();
    bool checkNewLevelName(); //检查新建的关卡名称
    bool checkLevelPrefix(const QString& str);
    QString getCurrentLevelFile(const QString& level_name, bool* is_backup = nullptr);

    void resetUndoAndRedo(const QString& level_name);

    // 备份Json关卡文件
    int lastLevelIndex;
    QMap<QString, bool> savedOrNot;
    QMap<QString, QStringList> backupFilePaths;
    QMap<QString, QStringList> backupFilePaths_Redo;
    void saveBackupJsonFile();
    void saveBackupJsonFile(QString &level_name);
    void saveBackupWhenInit(const QString &namelvl);
    void changeSavedFlag(const QString& level_name, bool already_saved);
    bool isSameFile(const QString& path1, const QString& path2);
    void deleteFile(const QString& path);
    void deleteAllBackupFiles();
    QString getLevelNameOnItem(QListWidgetItem* item);
    bool isNeedSave();
    bool pushNewBackupFileName(const QString& lvl_name, const QString& bk_file_path); // 在backupFilePaths最后插入新的备份文件路径
};

#endif // MAINWINDOW_H
