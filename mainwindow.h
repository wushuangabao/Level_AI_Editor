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
class QTreeView;
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
    void paintEvent(QPaintEvent *e);

    // 树形结构
    QString getItemCodeOf(int tree_type, const QModelIndex& index);
    QModelIndex getModelIndexBy(const QString& code, int *tree_type = nullptr);
    void updateTreeViewStateByData(bool default_state = true); //根据存储的展开状态进行刷新
    void replaceItemStateInMap(const QString& code_before, const QString& code_after, QStringList& to_do_list, QMap<QString, bool>* info_map, bool do_replace = true); //moveBackItemStateOf的辅助函数
    void selectTreeViewItem(const QModelIndex& index); //选择TreeView上的index节点

    void SaveBackup(bool save_states = false);
    QString GetItemCodeOfNode(NodeInfo *node); //获取node在itemState map中的Key

    // code对应的节点往后移动（在其后的兄弟节点也都要后移）
    void MoveBackItemStateOf(const QString& code, int brothers_size, int move_len = 1);
    // code对应的节点往前移动（在其之前的数个兄弟节点也都要前移）
    void MoveForwardItemStateOf(const QString& code, int front_id, int move_len = 1);
    // 将一个事件节点从begin_pos到end_pos（被m_eventTreeModel调用）
    void OnMoveEventNode(int begin_pos, int end_pos);
    // 粘贴成功后，新增节点状态记录到itemState map中
    void OnPasteNodes(QStringList item_codes);

    // 根据文件中的record_move_item来修改itemStates（默认true表示反操作）
    void ModifyItemStatesByFile(const QString& file_path, bool is_undo = true);
    bool ModifyItemStatesByRecord(const QString &record, bool is_undo);
    // itemStates Move（Back\Forward\From..to..）的记录
    QString record_move_item;
    bool record_enabled;

    QMap<QString, bool> m_itemState_Event; //存储事件TreeView的展开状态
    QMap<QString, bool> m_itemState_Custom; //存储自定义动作TreeView的展开状态

    QModelIndex m_curModelIndex; //当前选中的Tree View节点
    NodeInfo* m_curNode; //当前选中的节点数据

private slots:
    // 弹出菜单
    void slotTreeMenu_Event(const QPoint &pos);
    void slotTreeMenu_Custom(const QPoint &pos);
    // 展开Tree节点时触发
    void saveEventItemState_Expanded(const QModelIndex &index);
    void saveCustomItemState_Expanded(const QModelIndex &index);
    // 折叠Tree节点时触发
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
    // 展开、折叠事件节点或者自定义动作节点
    void slotExpandAll(bool b = false);
    void slotCollapseAll(bool b = false);

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
    void on_action_Reload_triggered();
    void on_action_SaveAllLevel_triggered();
    void on_action_jsonTolua_triggered();

    void OpenConfigFolder();
    void OpenLuaFolder();
    void EditCustomLevelName();

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
    // View - 展开、折叠节点
    void on_action_ExpandAllNodes_triggered();
    void on_action_CollapseAllNodes_triggered();
    void on_action_ExpandAllEvents_triggered();
    void on_action_CollapseAllEvents_triggered();

    // 保存关卡备注m_customLevelName
    void on_action_SaveCustomLvlName_triggered();

private:
    Ui::MainWindow *ui;

    DlgChoseEType* m_dlgChoseEvtType;
    DlgConditionType* m_dlgConditionType;
    DlgSetVariable* m_dlgSetVar;
    DlgEditValue* m_dlgEditFunction;
    DlgVariableManager* m_dlgManageVar;
    DlgChoseActionType* m_dlgChoseActionType;

    // 树形结构
    void removeItemCode(const QString& code, int tree_type = -1);
    void clearTreeViewState(const QString& lvl_id); //删除关卡时清数据
    void resetTreeViewSate(const QString& lvl_id); //每次打开新的关卡时，都会初始化TreeView上的节点状态
    void setTreeViewExpandSlots(bool ok); //是否存储节点的展开、折叠状态
    void setNewCurModelIndex(QModelIndex parent_index, int child_pos); //更新m_curModelIndex，变成parent_index的第child_pos个子节点
    void setModelForDlg(TreeItemModel *model);
    // 事件树
    TreeItemModel_Event* m_eventTreeModel;
    void InitEventTree();
    NodeInfo* createNewEventOnTree(QString event_type, const QString& event_name);
    // 自定义动作树
    TreeItemModel_Custom* m_customTreeModel;
    void InitCustomTree();
    int findCustomSeqIndex(const QString &name, NodeInfo* &node);
    // 展开、折叠所有子孙节点
    void expandAllNodes(QTreeView *tree, TreeItemModel* model, QModelIndex item);
    void collapseAllNodes(QTreeView* tree, TreeItemModel* model, QModelIndex item);
    void editEventName(NodeInfo* node); //编辑事件名称
    void editEventType(NodeInfo* node); //编辑事件类型
    void editActionNode(NodeInfo* node); //编辑动作节点
    void editCustActSeqName(NodeInfo* node); //自定义动作序列的名称

    // 右侧变量列表
    void addOneRowInTable(unsigned int row, const QString& s1, const QString& s2, const QString& s3, const QString &s4);
    void updateVarTable();

    // 生成Json文件
    QString m_temp_etype;
    QString getTriggerNameAt(int id);
    void generateJsonDocument(QFile* file);
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
    bool parseJsonArray_Condition(QJsonArray* conditions, NodeInfo* condition_node, const QString &etype);
    bool parseJsonObj_ActionNode(QJsonObject* actionJsonObj, NodeInfo* parent_node, const QString &etype);
    bool parseJsonArray_Sequence(QJsonArray* seqJsonArray, NodeInfo* seq_node, QString etype = "");
    CommonValueClass *parseJsonObj_Value(QJsonObject* valueJsonObj, const QString &etype = ""); //这个函数会new一个BaseValueClass
    BaseValueClass* parseJsonObj_Function(QJsonObject* funcJsonObj, const QString &etype = ""); //这个函数会new一个BaseValueClass

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
    QString backup_path;
    QString backup_states_path;

    // 左侧关卡列表
    QString m_levelPrefix;
    QStringList m_levelList;
    QString m_LuaPath;
    QMap<QString, QDateTime> m_filesLastModTime; //存储config文件的最后修改时间
    QList<QString> m_filesToReload; //存储发生修改的config文件名（需要被重载的）
    QMap<QString, QString> m_customLevelName; //关卡名称备注
    void InitLevelTree(); //初始化关卡列表，顺带初始化m_filesModifyTime
    bool checkNewLevelName(); //检查新建的关卡名称
    bool checkLevelPrefix(const QString& str);
    QString getCurrentLevelFile(const QString& level_name, bool* is_backup = nullptr);

    void resetUndoAndRedo(const QString& level_name);
    bool reloadCurrentLevel(const QString& file_path = "");

    // 备份Json关卡文件
    int lastLevelIndex;
    QMap<QString, bool> savedOrNot;
    QMap<QString, QStringList> backupFilePaths;
    QMap<QString, QStringList> backupFilePaths_Redo;
    QMap<QString, QString> backupItemStates; //QMap<备份的关卡文件地址, 对应的“节点位置移动状态记录”的文件名>
    QString saveBackupItemStates(const QString &level_name);
    QString saveBackupJsonFileOf(QString &level_name);
    bool saveBackupWhenInit(const QString &namelvl);
    void changeSavedFlag(const QString& level_name, bool already_saved);
    bool isSameFile(const QString& path1, const QString& path2);
    void deleteFile(const QString& path);
    void deleteAllBackupFiles();
    QString getLevelNameOnItem(QListWidgetItem* item);
    bool isNeedSave();
    bool pushNewBackupFileName(const QString& lvl_name, const QString& bk_file_path); // 在backupFilePaths最后插入新的备份文件路径
    bool isBackUpFileInUsed(const QString& path, QString &lvl_name);
    void onCreateNewLevel(const QString& lvl_name, const QDateTime &date_time);
};

#endif // MAINWINDOW_H
