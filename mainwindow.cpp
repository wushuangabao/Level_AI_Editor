#include "Dialogs/dlgchoseetype.h"
#include "Dialogs/dlgconditiontype.h"
#include "Dialogs/dlgeditvalue.h"
#include "Dialogs/dlgsetvariable.h"
#include "Dialogs/dlgvariablemanager.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    m_curETNode = nullptr;

    ui->setupUi(this);

    m_dlgChoseEvtType = new DlgChoseEType(this);
    m_dlgConditionType = new DlgConditionType(this);
    m_dlgSetVar = new DlgSetVariable(this);
    m_dlgEditFunction = new DlgEditValue(this);
    m_dlgManageVar = new DlgVariableManager(this);

    InitEventTree();

    // 自动调整第2列的宽度
    ui->tableWidget->resizeColumnToContents(1);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotTreeMenu(const QPoint &pos)
{
    QString qss = "QMenu::item{padding:3px 20px 3px 20px;}QMenu::indicator{width:13px;height:13px;}";
    // 参考 https://blog.csdn.net/dpsying/article/details/80149462

    QMenu menu;
    menu.setStyleSheet(qss);

    QModelIndex curIndex = ui->eventTreeView->indexAt(pos); //当前点击的元素的index
    QModelIndex index = curIndex.sibling(curIndex.row(),0); //该行的第1列元素的index

    if (index.isValid())
    {
        m_curETNode = reinterpret_cast<NodeInfo*>(curIndex.internalPointer());

        menu.addAction(QStringLiteral("编辑节点"), this, SLOT(slotEditNode(bool)));
        menu.addSeparator();
        QAction* actCutNode = menu.addAction(QStringLiteral("剪切"), this, SLOT(slotCutNode(bool)));
        menu.addAction(QStringLiteral("复制"), this, SLOT(slotCopyNode(bool)));
        menu.addAction(QStringLiteral("粘贴"), this, SLOT(slotPasteNode(bool)));
        QAction* actDeleteNode = menu.addAction(QStringLiteral("删除"), this, SLOT(slotDeleteNode(bool)));
        menu.addSeparator();
        menu.addAction(QStringLiteral("新增事件"), this, SLOT(slotNewEvent(bool)));
        QAction* actCondition = menu.addAction(QStringLiteral("新增条件"), this, SLOT(slotNewCondition(bool)));
        QAction* actAddAction = menu.addAction(QStringLiteral("新增动作"), this, SLOT(slotNewAction(bool)));

        // 第一层节点不能剪切、删除
        if(m_curETNode->parent == m_eventTreeModel->m_pRootNode)
        {
            actCutNode->setEnabled(false);
            actDeleteNode->setEnabled(false);
        }
        // 第二层节点不能剪切、删除
        else if(m_curETNode->parent->parent == m_eventTreeModel->m_pRootNode)
        {
            actCutNode->setEnabled(false);
            actDeleteNode->setEnabled(false);
        }

        // 只有 condition 节点可以新增条件
        if(m_curETNode->type != CONDITION)
        {
            actCondition->setEnabled(false);
        }

        // 只有“序列”节点可以新增动作（暂定）
        if(m_curETNode->type != SEQUENCE)
        {
            actAddAction->setEnabled(false);
        }
    }
    // 空白处点击右键，能新增事件
    else
    {
        menu.addAction(QStringLiteral("新增事件"), this, SLOT(slotNewEvent(bool)));
    }
    menu.exec(QCursor::pos());  //显示菜单
}

void MainWindow::slotTreeMenuExpand(bool b)
{
    Q_UNUSED(b);
    QModelIndex curIndex = ui->treeView->currentIndex();
    QModelIndex index = curIndex.sibling(curIndex.row(),0); //同一行第一列元素的index
    if(index.isValid())
    {
        ui->treeView->expand(index);
    }
}

void MainWindow::slotTreeMenuCollapse(bool b)
{
    Q_UNUSED(b);
    QModelIndex curIndex = ui->treeView->currentIndex();
    QModelIndex index = curIndex.sibling(curIndex.row(),0); //同一行第一列元素的index
    if(index.isValid())
    {
        ui->treeView->collapse(index);
    }
}

void MainWindow::slotEditNode(bool b)
{
    Q_UNUSED(b);
//    ui->eventTreeView->viewport()->update();

    if(m_curETNode != nullptr)
    {
        switch (m_curETNode->type) {

        // 一个事件
        case EVENT:
        // 事件类型
        case ETYPE:
            editEventNode(m_curETNode);
            ui->eventTreeView->expandAll();
            updateVarTable();
            break;

        // 条件
        case CONDITION:
            m_dlgConditionType->ModifyCondition(m_curETNode);
            break;
        case COMPARE:
            m_dlgConditionType->ModifyCompareNode(m_curETNode);
            break;

        // 流程控制
        case SEQUENCE:
        case CHOICE:
        case END:
            info("不能编辑该节点。");
            break;
        case LOOP:
            // todo 编辑循环次数
            break;

        // 设置变量
        case SET_VAR:
        // 执行函数
        case FUNCTION:
            editActionNode(m_curETNode);

        default:
            break;
        }
    }

}

void MainWindow::slotCutNode(bool b)
{
    Q_UNUSED(b);
}

void MainWindow::slotCopyNode(bool b)
{
    Q_UNUSED(b);
}

void MainWindow::slotPasteNode(bool b)
{
    Q_UNUSED(b);
}

void MainWindow::slotDeleteNode(bool b)
{
    Q_UNUSED(b);
    if(m_eventTreeModel->deleteNode(m_curETNode))
    {
        m_curETNode = nullptr;
        ui->eventTreeView->expandAll();
    }
}

void MainWindow::slotNewEvent(bool b)
{
    Q_UNUSED(b);

    m_dlgChoseEvtType->ShowWithEventType();
    if(m_dlgChoseEvtType->index != -1)
    {
        EVENT_TYPE_ID event_id = static_cast<EVENT_TYPE_ID>(m_dlgChoseEvtType->index);
        if(event_id < 0 || event_id >= EventType::GetInstance()->eventIdVector.size() || event_id >= EventType::GetInstance()->eventNameVector.size())
        {
            qDebug() << "ERROR param in slotNewEvent!" << endl;
            return;
        }

        NodeInfo* new_node = m_eventTreeModel->createNode(m_dlgChoseEvtType->event_name, NODE_TYPE::EVENT, m_eventTreeModel->m_pRootNode);
        new_node->childs[0]->updateEventType(event_id);
        m_eventTreeModel->GetValueManagerOf(new_node)->UpdateEventParams(m_dlgChoseEvtType->index);

        ui->eventTreeView->expandAll();
    }
}

void MainWindow::slotNewCondition(bool b)
{
    Q_UNUSED(b);
    if(m_curETNode->type == CONDITION)
    {
        m_dlgConditionType->CreateCondition(m_curETNode, "AND");
        ui->eventTreeView->expandAll();
    }
}

void MainWindow::slotNewAction(bool b)
{
    Q_UNUSED(b);
    m_dlgChoseEvtType->ShowWithActionType();
    NODE_TYPE type = static_cast<NODE_TYPE>(m_dlgChoseEvtType->index);
    if(type != INVALID)
    {
        NodeInfo* new_node = m_eventTreeModel->createNode(m_dlgChoseEvtType->text, type, m_curETNode);
        m_curETNode = new_node;
        ui->eventTreeView->expandAll();
        editActionNode(new_node);
    }
}

// 初始化m_eventTreeModel
void MainWindow::InitEventTree()
{
    m_eventTreeModel = new TreeItemModel(ui->eventTreeView);
    ui->eventTreeView->setModel(m_eventTreeModel);

    m_eventTreeModel->createNode("默认事件", NODE_TYPE::EVENT);
    ui->eventTreeView->expandAll();
    ui->eventTreeView->setItemsExpandable(false); //暂时禁止折叠

    ui->eventTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->eventTreeView, &QTreeView::customContextMenuRequested, this, &MainWindow::slotTreeMenu);

    m_dlgConditionType->SetModelPointer(m_eventTreeModel);
    m_dlgSetVar->SetModelPointer(m_eventTreeModel);
    m_dlgEditFunction->SetModel(m_eventTreeModel);
    m_dlgEditFunction->SetUpforFunction();
    m_dlgManageVar->SetModel(m_eventTreeModel);
}

void MainWindow::editEventNode(NodeInfo *node)
{
    if(node->type != EVENT && node->type != ETYPE)
        return;

    NodeInfo* event_node = nullptr;
    NodeInfo* etype_node = nullptr;
    if(node->type == EVENT)
    {
        event_node = node;
        etype_node = node->childs[0];
    }
    else if(node->type == ETYPE)
    {
        event_node = node->parent;
        etype_node = node;
    }

    // 编辑事件名称（只是显示用）、选择事件类型（EventType）
    m_dlgChoseEvtType->ShowWithEventType(node->text);
    if(m_dlgChoseEvtType->index != -1)
    {
        EVENT_TYPE_ID event_id = static_cast<EVENT_TYPE_ID>(m_dlgChoseEvtType->index);
        event_node->text = m_dlgChoseEvtType->event_name;
        etype_node->updateEventType(event_id);
        m_eventTreeModel->GetValueManagerOf(event_node)->UpdateEventParams(m_dlgChoseEvtType->index);
    }
}

void MainWindow::editActionNode(NodeInfo *node)
{
    switch (node->type)
    {
    case SET_VAR:
        m_dlgSetVar->EditSetVarNode(node);
        node->text = m_dlgSetVar->GetNodeText();
        break;
    case FUNCTION:
        m_dlgEditFunction->ModifyCallNode(node);
        node->text = m_dlgEditFunction->GetValueText();
        break;
    default:
        break;
    }
    ui->eventTreeView->expandAll();
}

void MainWindow::addOneRowInTable(unsigned int row, const QString& s1, const QString& s2, const QString& s3)
{
    QTableWidgetItem* item;

    unsigned int row_count = ui->tableWidget->rowCount();
    if(row_count <= row)
        for(unsigned int i = row_count; i <= row; i++)
            ui->tableWidget->insertRow(i);

    // 变量名
    item = new QTableWidgetItem(s1, 0);
    ui->tableWidget->setItem(row, 0, item);

    // 变量类型
    item = new QTableWidgetItem(s2, 1);
    ui->tableWidget->setItem(row, 1, item);

    // 初始值
    item = new QTableWidgetItem(s3, 2);
    ui->tableWidget->setItem(row, 2, item);
}

void MainWindow::addEventVarInTable(NodeInfo *event_node)
{
    unsigned int row = 0;
//    int n = event_node->getValuesCount();
//    if(n > 0)
//    {
//        for(int i = 0; i < n; i++)
//        {
//            addOneRowInTable(row, event_node->getValue(i), event_node->getValue(i), "");
//            row++;
//        }
//    }

    ValueManager* vm = m_eventTreeModel->GetValueManagerOf(event_node);
    int n = vm->GetGlobalVarList().size();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            addOneRowInTable(row, vm->GetGlobalVarList().at(i), vm->GetVarTypeAt(i), vm->GetInitValueOfVar(i));
            row++;
        }
    }
}

void MainWindow::updateVarTable()
{
    ui->tableWidget->clearContents();

    NodeInfo* event_node = m_eventTreeModel->findUppestNodeOf(m_curETNode);
    addEventVarInTable(event_node);

    // 自动调整列的宽度
//    ui->tableWidget->resizeColumnToContents(0);
//    ui->tableWidget->resizeColumnToContents(1);
//    ui->tableWidget->resizeColumnToContents(2);
}

void MainWindow::on_eventTreeView_clicked(const QModelIndex &index)
{
    if (index.isValid())
    {
        m_curETNode = reinterpret_cast<NodeInfo*>(index.internalPointer());

        updateVarTable();
    }
}

void MainWindow::on_eventTreeView_doubleClicked(const QModelIndex &index)
{
    on_eventTreeView_clicked(index);
    slotEditNode();
}

void MainWindow::on_btnAddVar_clicked()
{
    if(m_curETNode == nullptr)
    {
        m_curETNode = m_eventTreeModel->m_pRootNode->childs[0];
    }

    m_dlgManageVar->CreateVar(m_curETNode);

    updateVarTable();
}
