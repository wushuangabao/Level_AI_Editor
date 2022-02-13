#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ItemModels/treeitemmodel.h"

class DlgChoseEType;
class DlgConditionType;
class DlgEditValue;
class DlgSetVariable;
class DlgVariableManager;

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

private:
    Ui::MainWindow *ui;

    DlgChoseEType* m_dlgChoseEvtType;
    DlgConditionType* m_dlgConditionType;
    DlgSetVariable* m_dlgSetVar;
    DlgEditValue* m_dlgEditFunction;
    DlgVariableManager* m_dlgManageVar;

    NodeInfo* m_curETNode;
    TreeItemModel* m_eventTreeModel;

    void InitEventTree();

    void editEventNode(NodeInfo* node);
    void editActionNode(NodeInfo* node);

    void addOneRowInTable(unsigned int row, const QString& s1, const QString& s2, const QString& s3);
    void addEventVarInTable(NodeInfo * event_node);
    void updateVarTable();
};

#endif // MAINWINDOW_H
