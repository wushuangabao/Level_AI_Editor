#include <QFileDialog>
#include <QDateTime>
#include <QStyleFactory>

#include <QJsonParseError>
#include <QVariantMap>
#include <QJsonObject>
#include <QJsonArray>

#include "Dialogs/dlgchoseetype.h"
#include "Dialogs/dlgconditiontype.h"
#include "Dialogs/dlgeditvalue.h"
#include "Dialogs/dlgsetvariable.h"
#include "Dialogs/dlgvariablemanager.h"
#include "Dialogs/dlgchoseactiontype.h"
#include "Dialogs/dlgwaiting.h"
#include "Values/enuminfo.h"
#include "ItemModels/functioninfo.h"
#include "nodesclipboard.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "objbase.h"
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    m_curNode = nullptr;
    m_curModelIndex = QModelIndex();
    m_levelPrefix = "Level";
    m_LuaPath = "../../Assets/GameMain/LuaScripts/Module/BattleManager/AILogic/AIConfig/Level/";

    // 检查config等目录是否存在
    config_path = QCoreApplication::applicationFilePath().replace("LevelEditor.exe", "config/");
    QDir dir_config(config_path.left(config_path.size() - 1));
    if(!dir_config.exists())
    {
        info("找不到" + config_path + "文件夹");
    }
    QString backup_path = config_path + "backup";
    QDir dir_backup;
    if(!dir_backup.exists(backup_path))
    {
        /*bool ok = */dir_backup.mkdir(backup_path);
    }

    ui->setupUi(this);

    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->horizontalHeader()->setVisible(true);

    m_dlgChoseEvtType = new DlgChoseEType(this);
    m_dlgConditionType = new DlgConditionType(this);
    m_dlgSetVar = new DlgSetVariable(this);
    m_dlgEditFunction = new DlgEditValue(this);
    m_dlgEditFunction->SetUpforFunction();
    m_dlgManageVar = new DlgVariableManager(this);
    m_dlgChoseActionType = new DlgChoseActionType(this);

    // 设置 treeView 的连接线
    ui->eventTreeView->setStyle(QStyleFactory::create("windows"));
    ui->customTreeView->setStyle(QStyleFactory::create("windows"));

    // 自动调整第2、4列的宽度
    ui->tableWidget->resizeColumnToContents(1);
    ui->tableWidget->resizeColumnToContents(3);

    InitEventTree();
    InitCustomTree();
//    setTreeViewExpandSlots(true);
    NodesClipBoard::GetInstance()->SetTreeItemModel(m_eventTreeModel, m_customTreeModel);

    ui->tabWidget->setCurrentIndex(0);
    setModelForDlg(m_eventTreeModel);

    InitLevelTree();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(isNeedSave())
    {
        int ch = QMessageBox::warning(nullptr, "提示", "还有未保存的修改，确定直接退出？", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if(ch != QMessageBox::Yes)
        {
            event->ignore();
            return;
        }
    }

    deleteAllBackupFiles();
    QMainWindow::closeEvent(event);
}

void MainWindow::slotTreeMenu_Event(const QPoint &pos)
{
    QString qss = "QMenu::item{padding:3px 20px 3px 20px;}QMenu::indicator{width:13px;height:13px;}";
    // 参考 https://blog.csdn.net/dpsying/article/details/80149462

    QMenu menu;
    menu.setStyleSheet(qss);

    m_curModelIndex = ui->eventTreeView->indexAt(pos); //当前点击的元素的index
    QModelIndex index = m_curModelIndex.sibling(m_curModelIndex.row(),0); //该行的第1列元素的index

    if (index.isValid())
    {
        m_curNode = reinterpret_cast<NodeInfo*>(m_curModelIndex.internalPointer());

        menu.addAction(QStringLiteral("编辑节点"), this, SLOT(slotEditNode(bool)));
        if(m_curNode->childs.size() > 0)
        {
            menu.addSeparator();
            menu.addAction(QStringLiteral("全部展开"), this, SLOT(slotExpandAll(bool)));
            menu.addAction(QStringLiteral("全部折叠"), this, SLOT(slotCollapseAll(bool)));
        }
        menu.addSeparator();
//        QAction* actCutNode = menu.addAction(QStringLiteral("剪切"), this, SLOT(slotCutNode(bool)));
        menu.addAction(QStringLiteral("复制"), this, SLOT(slotCopyNode(bool)));
        menu.addAction(QStringLiteral("粘贴"), this, SLOT(slotPasteNode(bool)));
        if(m_curNode->parent->parent != m_eventTreeModel->m_pRootNode && m_curNode->type != ETYPE &&
            !(m_curNode->type == SEQUENCE && (m_curNode->parent->type == LOOP || m_curNode->parent->type == CHOICE)) &&
            !(m_curNode->type == CONDITION && m_curNode->parent->type == CHOICE) &&
            !(m_eventTreeModel->m_pRootNode->childs.size() <= 1 && m_curNode->type == EVENT)
          ) //以上节点不能剪切、删除
            menu.addAction(QStringLiteral("删除"), this, SLOT(slotDeleteNode(bool)));
        menu.addSeparator();
        if(m_curNode->type == CONDITION)
            menu.addAction(QStringLiteral("新增条件"), this, SLOT(slotNewCondition(bool)));
        else if(m_curNode->type == SEQUENCE)
            menu.addAction(QStringLiteral("新增动作"), this, SLOT(slotNewAction(bool)));
        else if(m_curNode->type >= SET_VAR)
            menu.addAction(QStringLiteral("插入动作"), this, SLOT(slotNewAction(bool)));
        else if(m_curNode->type == EVENT)
            menu.addAction(QStringLiteral("插入新的事件"), this, SLOT(slotNewEvent(bool)));
    }
    // 空白处点击右键
    else
    {
        menu.addAction(QStringLiteral("新增事件"), this, SLOT(slotNewEvent(bool)));
        if(NodesClipBoard::GetInstance()->GetTypeOfPasteNode() == INVALID)
            menu.addAction(QStringLiteral("粘贴事件"), this, SLOT(slotPasteEventOrCustAct(bool)));
    }

    menu.exec(QCursor::pos());  //显示菜单
}

void MainWindow::slotTreeMenu_Custom(const QPoint &pos)
{
    QString qss = "QMenu::item{padding:3px 20px 3px 20px;}QMenu::indicator{width:13px;height:13px;}";

    QMenu menu;
    menu.setStyleSheet(qss);

    m_curModelIndex = ui->customTreeView->indexAt(pos); //当前点击的元素的index
    QModelIndex index = m_curModelIndex.sibling(m_curModelIndex.row(),0); //该行的第1列元素的index

    if (index.isValid())
    {
        m_curNode = reinterpret_cast<NodeInfo*>(m_curModelIndex.internalPointer());

        menu.addAction(QStringLiteral("编辑节点"), this, SLOT(slotEditNode(bool)));
        if(m_curNode->childs.size() > 0)
        {
            menu.addSeparator();
            menu.addAction(QStringLiteral("全部展开"), this, SLOT(slotExpandAll(bool)));
            menu.addAction(QStringLiteral("全部折叠"), this, SLOT(slotCollapseAll(bool)));
        }
        menu.addSeparator();
//        menu.addAction(QStringLiteral("剪切"), this, SLOT(slotCutNode(bool)));
        menu.addAction(QStringLiteral("复制"), this, SLOT(slotCopyNode(bool)));
        menu.addAction(QStringLiteral("粘贴"), this, SLOT(slotPasteNode(bool)));
        menu.addAction(QStringLiteral("删除"), this, SLOT(slotDeleteNode(bool)));

        menu.addSeparator();
        if(m_curNode->type == CONDITION)
            menu.addAction(QStringLiteral("新增条件"), this, SLOT(slotNewCondition(bool)));
        else if(m_curNode->type == SEQUENCE)
            menu.addAction(QStringLiteral("新增动作"), this, SLOT(slotNewAction(bool)));
        else if(m_curNode->type >= SET_VAR)
            menu.addAction(QStringLiteral("插入动作"), this, SLOT(slotNewAction(bool)));

        if(m_curNode->parent->type == INVALID && m_curNode->type == SEQUENCE)
        {
            menu.addSeparator();
            menu.addAction(QStringLiteral("插入自定义动作"), this, SLOT(slotNewCustomSeq(bool)));
        }
    }
    // 空白处点击右键
    else
    {
        menu.addAction(QStringLiteral("新建自定义动作"), this, SLOT(slotNewCustomSeq(bool)));
        if(NodesClipBoard::GetInstance()->GetTypeOfPasteNode() == INVALID)
            menu.addAction(QStringLiteral("粘贴自定义动作"), this, SLOT(slotPasteEventOrCustAct(bool)));
    }

    menu.exec(QCursor::pos());  //显示菜单
}

void MainWindow::saveEventItemState_Expanded(const QModelIndex &index)
{
    if(index.isValid())
    {
        QString item_code = getItemCodeOf(0, index);
        if(m_itemState_Event.contains(item_code))
        {
            m_itemState_Event[item_code] = true;
        }
        else
            m_itemState_Event.insert(item_code, true);
    }
}

void MainWindow::saveCustomItemState_Expanded(const QModelIndex &index)
{
    if(index.isValid())
    {
        QString item_code = getItemCodeOf(1, index);
        if(m_itemState_Custom.contains(item_code))
        {
            m_itemState_Custom[item_code] = true;
        }
        else
            m_itemState_Custom.insert(item_code, true);
    }
}

void MainWindow::saveEventItemState_Collapsed(const QModelIndex &index)
{
    if(index.isValid())
    {
        QString item_code = getItemCodeOf(0, index);
        if(m_itemState_Event.contains(item_code))
        {
            m_itemState_Event[item_code] = false;
        }
        else
            m_itemState_Event.insert(item_code, false);
    }
}

void MainWindow::saveCustomItemState_Collapsed(const QModelIndex &index)
{
    if(index.isValid())
    {
        QString item_code = getItemCodeOf(1, index);
        if(m_itemState_Custom.contains(item_code))
        {
            m_itemState_Custom[item_code] = false;
        }
        else
            m_itemState_Custom.insert(item_code, false);
    }
}

void MainWindow::slotEditNode(bool b)
{
    Q_UNUSED(b);
//    ui->eventTreeView->viewport()->update();

    if(m_curNode != nullptr)
    {
        switch (m_curNode->type) {
        // 自定义动作序列
        case SEQUENCE:
            if(ui->tabWidget->currentIndex() == 1)
            {
                editCustActSeqName(m_curNode);
            }

        // 一个事件
        case EVENT:
            editEventName(m_curNode);
            break;
        // 事件类型
        case ETYPE:
            editEventType(m_curNode);
            updateVarTable();
            break;

        // 条件
        case CONDITION:
            m_dlgConditionType->ModifyCondition(m_curNode);
            break;
        case COMPARE:
            m_dlgConditionType->ModifyCompareNode(m_curNode);
            break;

        // 流程控制
        case CHOICE:
        case END:
            info("不能编辑该节点。");
            break;
        case LOOP:
            // 编辑循环次数
//            m_dlgManageVar todo
            break;

        // 设置变量
        case SET_VAR:
        // 执行函数
        case FUNCTION:
        // 开启事件监听
        case OPEN_EVENT:
        // 关闭事件监听
        case CLOSE_EVENT:
            editActionNode(m_curNode);
            break;
        default:
            return;
        }
        saveBackupJsonFile();
    }
}

void MainWindow::slotCutNode(bool b)
{
    Q_UNUSED(b);
}

void MainWindow::slotCopyNode(bool b)
{
    Q_UNUSED(b);
    NodesClipBoard* clip_board = NodesClipBoard::GetInstance();

    QModelIndexList selects;
    if(ui->tabWidget->currentIndex() == 0)
    {
        clip_board->ClearNodes(0);
        selects = ui->eventTreeView->selectionModel()->selectedRows(0);
    }
    else if(ui->tabWidget->currentIndex() == 1)
    {
        clip_board->ClearNodes(1);
        selects = ui->customTreeView->selectionModel()->selectedRows(0);
    }
    else
        return;

    // 拷贝所有节点（引用node，拷贝value)
    QList<QModelIndex>::const_iterator cit;
    for (cit = selects.begin(); cit != selects.end(); ++cit)
    {
        QModelIndex temp = *cit;
        NodeInfo* cur_node = reinterpret_cast<NodeInfo*>(temp.internalPointer());
        if(!clip_board->AddCopyNode(cur_node))
        {
            clip_board->ClearNodes(0);
            return;
        }
    }
}

void MainWindow::slotPasteNode(bool b)
{
    Q_UNUSED(b);

    int tree_type = ui->tabWidget->currentIndex();

    if(NodesClipBoard::GetInstance()->PasteToNode(m_curNode, tree_type))
    {
        updateTreeViewState();
        saveBackupJsonFile();
        updateVarTable();
    }
}

void MainWindow::slotPasteEventOrCustAct(bool b)
{
    Q_UNUSED(b);

    NodeInfo* root = nullptr;
    int tree_type = ui->tabWidget->currentIndex();

    if(tree_type == 0)
        root = m_eventTreeModel->m_pRootNode;
    else if(tree_type == 1)
        root = m_customTreeModel->m_pRootNode;
    else
        return;

    if(NodesClipBoard::GetInstance()->PasteToNode(root, tree_type))
    {
        updateTreeViewState();
        saveBackupJsonFile();
        updateVarTable();
    }
}

void MainWindow::slotDeleteNode(bool b)
{
    Q_UNUSED(b);
    if(ui->tabWidget->currentIndex() == 0)
    {
        if(m_eventTreeModel->deleteNode(m_curNode))
        {
            m_curNode = nullptr;
            updateTreeViewState();
            saveBackupJsonFile();
        }
    }
    else if(ui->tabWidget->currentIndex() == 1)
    {
        if(m_customTreeModel->deleteNode(m_curNode))
        {
            m_curNode = nullptr;
            updateTreeViewState();
            saveBackupJsonFile();
        }
    }
}

void MainWindow::slotNewEvent(bool b)
{
    Q_UNUSED(b);

    m_dlgChoseEvtType->CreateNewEvent();
    if(m_dlgChoseEvtType->index >= 0)
    {
        QString new_name = m_dlgChoseEvtType->event_name;
        if(new_name == "")
            info("事件名不能为空！");
        else
        {
            NodeInfo* event_node = m_eventTreeModel->FindUppestNodeByName(new_name);
            if(event_node != nullptr)
                info("已存在事件名：" + new_name);
            else
            {
                if(EventType::GetInstance()->GetCount() > m_dlgChoseEvtType->index)
                {
                    createNewEventOnTree(EventType::GetInstance()->GetEventLuaType(m_dlgChoseEvtType->index), new_name);
                    updateTreeViewState();
                    saveBackupJsonFile();
                }
                else
                    info("不存在这个事件类型！");
            }
        }
    }
}

void MainWindow::slotNewCondition(bool b)
{
    Q_UNUSED(b);
    if(m_curNode->type == CONDITION)
    {
        m_dlgConditionType->CreateCondition(m_curNode, "AND");

        updateTreeViewState();

        saveBackupJsonFile();
    }
}

void MainWindow::slotNewAction(bool b)
{
    Q_UNUSED(b);

    // 新节点加在哪个位置？
    NodeInfo* parent_node = nullptr;
    m_curNode->FindAndSetNewNodePos(parent_node);
    MY_ASSERT(parent_node != nullptr);
    MY_ASSERT(parent_node->type == SEQUENCE);

    // 选择并新建一个动作节点
    m_dlgChoseActionType->CreateActionType(parent_node);
    QString node_text;
    NODE_TYPE type = m_dlgChoseActionType->GetNodeTypeAndText(node_text);

    if(type == INVALID)
        return;

    m_dlgChoseActionType->BeginResetModel();
    bool success = true;
    switch (type)
    {
    case SET_VAR:
    {
        int pos = node_text.indexOf(" = ");
        MY_ASSERT(pos != -1);
        QString text_name = node_text.left(pos);
        pos = text_name.indexOf('.');
        int id_var;
        if(pos != -1)
            id_var = ValueManager::GetValueManager()->FindIdOfVarName(text_name.left(pos));
        else
            id_var = ValueManager::GetValueManager()->FindIdOfVarName(text_name);
        NodeInfo* new_node = parent_node->addNewChildNode_SetVar(node_text, id_var); //在动作序列中插入新建的setvar节点
        CommonValueClass* v_pointer = m_dlgChoseActionType->GetValue_SetVar();
        if(new_node != nullptr && v_pointer != nullptr)
            ValueManager::GetValueManager()->UpdateValueOnNode_SetValue(new_node, v_pointer);
        else
        {
            info("创建SET_VAR节点失败");
            success = false;
        }
        break;
    }
    case FUNCTION:
    {
        NodeInfo* new_node = parent_node->addNewChild(FUNCTION, node_text);
        if(new_node != nullptr)
        {
            ValueManager::GetValueManager()->UpdateValueOnNode_Function(new_node, m_dlgChoseActionType->GetValue_CallFunc());
        }
        else
        {
            info("创建FUNCTION节点失败");
            success = false;
        }
    }
        break;
    case OPEN_EVENT:
    case CLOSE_EVENT:
    {
        NodeInfo* new_node = parent_node->addNewChild(type, "");
        if(new_node == nullptr)
        {
            info("创建" + getNodeTypeStr(type) + "节点失败");
            success = false;
        }
        else
        {
            new_node->modifyValue(0, node_text);
            new_node->UpdateText();
        }
    }
        break;
    case SEQUENCE:
    {
        if("-- ERROR Custom Action" != node_text)
        {
            NodeInfo* new_node = parent_node->addNewChild(FUNCTION, node_text);
            BaseValueClass* value = new BaseValueClass(node_text);
            ValueManager::GetValueManager()->UpdateValueOnNode_Function(new_node, value);
        }
    }
        break;
    default:
        m_curNode = parent_node->addNewChild(type, node_text);
        break;
    }

    m_dlgChoseActionType->EndResetModel();

    // 重新展开到原来的形状
    updateTreeViewState();

    if(success)
    {
        // 备份关卡文件
        saveBackupJsonFile();
    }
}

void MainWindow::slotNewCustomSeq(bool b)
{
    Q_UNUSED(b);

    m_dlgChoseEvtType->CreateNewCustomSeq();
    if(m_dlgChoseEvtType->index >= 0)
    {
        QString new_name = m_dlgChoseEvtType->event_name;
        if(new_name == "")
            info("动作名称不能为空！");
        else
        {
            NodeInfo* event_node = m_customTreeModel->FindUppestNodeByName(new_name);
            if(event_node != nullptr)
                info("已存在动作名称：" + new_name);
            else
            {
                m_customTreeModel->AddCustomSequence(new_name);
                saveBackupJsonFile();
                updateTreeViewState();
            }
        }
    }
}

void MainWindow::slotExpandAll(bool b)
{
    Q_UNUSED(b);
    int type_id = ui->tabWidget->currentIndex();
    if(type_id == 0)
        expandAllNodes(ui->eventTreeView, m_eventTreeModel, m_curModelIndex);
    else if(type_id == 1)
        expandAllNodes(ui->customTreeView, m_customTreeModel, m_curModelIndex);
}

void MainWindow::slotCollapseAll(bool b)
{
    Q_UNUSED(b);
    int type_id = ui->tabWidget->currentIndex();
    if(type_id == 0)
        collapseAllNodes(ui->eventTreeView, m_eventTreeModel, m_curModelIndex);
    else if(type_id == 1)
        collapseAllNodes(ui->customTreeView, m_customTreeModel, m_curModelIndex);
}

// 初始化m_eventTreeModel
void MainWindow::InitEventTree()
{
    m_eventTreeModel = new TreeItemModel_Event(ui->eventTreeView);
    ui->eventTreeView->setModel(m_eventTreeModel);

//    m_eventTreeModel->createNode("默认事件", NODE_TYPE::EVENT);
//    ui->eventTreeView->expandAll();
//    ui->eventTreeView->setItemsExpandable(false); //暂时禁止折叠

    ui->eventTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->eventTreeView, &QTreeView::customContextMenuRequested, this, &MainWindow::slotTreeMenu_Event);
}

NodeInfo* MainWindow::createNewEventOnTree(QString event_type, const QString &event_name)
{
    if(!EventType::GetInstance()->IsEventIdValid(event_type))
    {
        info("ERROR param in createNewEventOnTree!");
        return nullptr;
    }

    NodeInfo* new_node = m_eventTreeModel->createNode(event_name, NODE_TYPE::EVENT, NodeInfo::GetRootNode_Event());
    if(new_node == nullptr)
        return nullptr;

    new_node->childs[0]->childs[0]->UpdateEventType(EventType::GetInstance()->GetIndexOf(event_type));
    return new_node;
}

void MainWindow::InitCustomTree()
{
    m_customTreeModel = new TreeItemModel_Custom(ui->customTreeView);
    ui->customTreeView->setModel(m_customTreeModel);

    ui->customTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->customTreeView, &QTreeView::customContextMenuRequested, this, &MainWindow::slotTreeMenu_Custom);
}

void MainWindow::expandAllNodes(QTreeView* tree, TreeItemModel* model, QModelIndex item)
{
    tree->expand(item);

    QModelIndex index = item.sibling(item.row(), 0);
    if (index.isValid())
    {
        NodeInfo* node = reinterpret_cast<NodeInfo*>(item.internalPointer());
        int children_n = node->childs.size();
        // 递归展开所有子孙结点
        if(children_n != 0)
        {
            for(int i = 0; i < children_n; i++)
                expandAllNodes(tree, model, model->index(i, 0, item));
        }
    }
}

void MainWindow::collapseAllNodes(QTreeView* tree, TreeItemModel* model, QModelIndex item)
{
    QModelIndex index = item.sibling(item.row(), 0);
    if (index.isValid())
    {
        NodeInfo* node = reinterpret_cast<NodeInfo*>(item.internalPointer());
        int children_n = node->childs.size();
        // 递归展开所有子孙结点
        if(children_n != 0)
        {
            for(int i = 0; i < children_n; i++)
                collapseAllNodes(tree, model, model->index(i, 0, item));
        }
    }
    tree->collapse(item);
}

void MainWindow::setTreeViewExpandSlots(bool ok)
{
    if(ok)
    {
        connect(ui->customTreeView, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(saveCustomItemState_Collapsed(const QModelIndex&)));
        connect(ui->customTreeView, SIGNAL(expanded(const QModelIndex&)), this, SLOT(saveCustomItemState_Expanded(const QModelIndex&)));
        connect(ui->eventTreeView, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(saveEventItemState_Collapsed(const QModelIndex&)));
        connect(ui->eventTreeView, SIGNAL(expanded(const QModelIndex&)), this, SLOT(saveEventItemState_Expanded(const QModelIndex&)));
    }
    else
    {
        disconnect(ui->customTreeView, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(saveCustomItemState_Collapsed(const QModelIndex&)));
        disconnect(ui->customTreeView, SIGNAL(expanded(const QModelIndex&)), this, SLOT(saveCustomItemState_Expanded(const QModelIndex&)));
        disconnect(ui->eventTreeView, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(saveEventItemState_Collapsed(const QModelIndex&)));
        disconnect(ui->eventTreeView, SIGNAL(expanded(const QModelIndex&)), this, SLOT(saveEventItemState_Expanded(const QModelIndex&)));
    }
}

void MainWindow::editEventName(NodeInfo *node)
{
    if(node->type != EVENT)
        return;

    m_dlgChoseEvtType->EditEventName(node->text);

    if(m_dlgChoseEvtType->index == -1)
        return;

    QString new_name = m_dlgChoseEvtType->event_name;
    NodeInfo* event_node = m_eventTreeModel->FindUppestNodeByName(new_name);
    if(new_name != "" && event_node == nullptr)
    {
        m_eventTreeModel->UpdateEventName(node, new_name);
    }
    else
    {
        if(event_node != nullptr && event_node != m_curNode)
            info("已存在事件名：" + new_name);
        if(new_name == "")
            info("不能使用空事件名！");
    }
}

void MainWindow::editEventType(NodeInfo *node)
{
    if(node->type != ETYPE)
        return;

    // 选择事件类型（EventType）
    m_dlgChoseEvtType->EditEventType();
    if(m_dlgChoseEvtType->index > -1  && m_dlgChoseEvtType->index < EventType::GetInstance()->GetCount())
    {
        node->UpdateEventType(m_dlgChoseEvtType->index);
    }
}

void MainWindow::editActionNode(NodeInfo *node)
{
    switch (node->type)
    {
    case SET_VAR:
        m_dlgSetVar->EditSetVarNode(node);
        break;
    case FUNCTION:
        if(node->text.contains("自定义动作："))
        {
            for(int i = 0; i < m_customTreeModel->m_pRootNode->childs.size(); i++)
            {
                NodeInfo* seq_node = m_customTreeModel->m_pRootNode->childs[i];
                if(node->text.contains(seq_node->text))
                {
                    // 直接去编辑自定义动作序列
                    ui->tabWidget->setCurrentIndex(1);
                    return;
                }
            }
        }
        m_dlgEditFunction->ModifyCallNode(node);
        break;
    case OPEN_EVENT:
    case CLOSE_EVENT:
    {
        QStringList enames = m_eventTreeModel->GetUppestNodeNames();
        int id_enames = m_dlgChoseEvtType->ChoseEventNameIn(enames);
        if(id_enames >= 0 && id_enames < enames.size())
            node->modifyValue(0, enames[id_enames]);
        node->UpdateText();
    }
        break;
    default:
        break;
    }

    //    updateEventTreeState();
}

void MainWindow::editCustActSeqName(NodeInfo *node)
{
    if(node->type != SEQUENCE || node->parent != NodeInfo::GetRootNode_Custom())
        return;

    m_dlgChoseEvtType->EditCustomSeqName(node->text);

    if(m_dlgChoseEvtType->index == -1)
        return;

    QString new_name = m_dlgChoseEvtType->event_name;
    NodeInfo* seq_node = m_eventTreeModel->FindUppestNodeByName(new_name);
    if(new_name != "" && seq_node == nullptr)
    {
        m_customTreeModel->UpdateCustActSeqName(node, new_name);
    }
    else
    {
        if(seq_node != nullptr && seq_node != m_curNode)
            info("已存在动作名：" + new_name);
        if(new_name == "")
            info("不能使用空动作名！");
    }
}

void MainWindow::addOneRowInTable(unsigned int row, const QString& s1, const QString& s2, const QString& s3, const QString& s4)
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

    // 关卡目标
    item = new QTableWidgetItem(s4, 3);
    ui->tableWidget->setItem(row, 3, item);
}

void MainWindow::updateVarTable()
{
    ui->tableWidget->clearContents();

//    NodeInfo* event_node = m_eventTreeModel->findUppestNodeOf(m_curETNode);

    unsigned int row = 0;
    ValueManager* vm = m_eventTreeModel->GetValueManager();
    int n = vm->GetGlobalVarList().size();
    for(int i = 0; i < n; i++)
    {
        QString var_name = vm->GetGlobalVarList().at(i);
        int var_id = vm->GetIdOfVariable(var_name);
//        if(!varNameIsEventParam(var_name, event_node))
//        {
            //                    变量名     变量类型
            addOneRowInTable(row, var_name, vm->GetVarTypeAt(var_id),
                             vm->GetInitValueOfVar(var_id)->GetText(), //初始值
                             vm->CheckVarIsLevelParam(var_id)? "√" : ""); //关卡目标
            row++;
//        }
    }

    // 自动调整列的宽度
//    ui->tableWidget->resizeColumnToContents(0);
//    ui->tableWidget->resizeColumnToContents(1);
//    ui->tableWidget->resizeColumnToContents(2);
}

void MainWindow::generateJsonDocument(QFile *file)
{
    int trigger_num = m_eventTreeModel->m_pRootNode->childs.size();
    int custom_num = m_customTreeModel->m_pRootNode->childs.size();
    if(trigger_num <= 0)
        return;

    QJsonObject object;
    addVariablesToJsonObj(&object);

    QJsonArray evtArray;
    for(int i = 0; i < trigger_num; i++)
    {
        QJsonObject event_type;
        createEventTypeJsonObj(m_eventTreeModel->m_pRootNode->childs[i]->childs[0]->childs[0], &event_type);

        QJsonObject trigger;

        trigger["event_name"] = getTriggerNameAt(i);
        trigger["event_type"] = event_type;
        addConditionToJsonObj(m_eventTreeModel->m_pRootNode->childs[i]->childs[1], &trigger);
        addActionSeqToJsonObj(m_eventTreeModel->m_pRootNode->childs[i]->childs[2], &trigger);

        evtArray.push_back(trigger);
    }
    object.insert("Event", evtArray);

    QJsonArray actArray;
    for(int i = 0; i < custom_num; i++)
    {
        QJsonObject seq;
        seq.insert("name", m_customTreeModel->m_pRootNode->childs[i]->text);
        addActionSeqToJsonObj(m_customTreeModel->m_pRootNode->childs[i], &seq);
        actArray.push_back(seq);
    }
    object.insert("CustomAction", actArray);

    QJsonDocument document(object);
    file->write(document.toJson());
}

void MainWindow::saveBackupJsonFile(QString& level_name)
{
    QString file_path = config_path + "backup/" + level_name + "_aoutosaved_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".json";
    QFile file(file_path);
    if(!file.open(QIODevice::ReadWrite))
    {
        info(file_path + "备份失败！");
        return;
    }
    else
    {
        // 生成备份文件
        file.resize(0);
        generateJsonDocument(&file);
        file.close();

        bool is_backup;
        QString old_file_path = getCurrentLevelFile(level_name, &is_backup);
        if(!is_backup || !isSameFile(old_file_path, file_path))
        {
            // 存储路径
            pushNewBackupFileName(level_name, file_path);

            // 设置未保存标记
            changeSavedFlag(level_name, isSameFile(file_path, config_path + level_name + ".json"));

            // 禁用重做
            if(backupFilePaths_Redo.contains(level_name))
                backupFilePaths_Redo[level_name].clear();
            resetUndoAndRedo(level_name);
        }
        else
            deleteFile(file_path);
    }
}

void MainWindow::saveBackupWhenInit(const QString &namelvl)
{
    if(!backupFilePaths.contains(namelvl) || backupFilePaths[namelvl].isEmpty())
    {
        QString file_path = config_path + "backup/" + namelvl + "_aoutosaved_0.json";
        QFile file(file_path);
        if(!file.open(QIODevice::WriteOnly))
        {
            info(file_path + "备份失败！");
            return;
        }
        else
        {
            file.resize(0);

            // 复制关卡文件
            QFile file_0(QString(config_path + namelvl + ".json"));
            if(!file_0.open(QIODevice::ReadOnly))
            {
                info(file_path + "备份失败！");
                return;
            }
            else
            {
                QByteArray b = file_0.readAll();
                file.write(b);
                file_0.close();
                pushNewBackupFileName(namelvl, file_path);
            }

            file.close();
        }
    }
}

void MainWindow::changeSavedFlag(const QString &level_name, bool already_saved)
{
    if(savedOrNot.contains(level_name))
    {
        if(savedOrNot[level_name] == already_saved)
            return;
        else
            savedOrNot[level_name] = already_saved;
    }
    else
    {
        savedOrNot.insert(level_name, already_saved);
    }

    int pos = m_levelList.indexOf(level_name);
    MY_ASSERT(pos != -1);
    QListWidgetItem* item = ui->levelList->item(pos);

    // 没保存，UI中显示星号
    if(!already_saved)
    {
        item->setText(level_name + " *");
    }
    // 已保存，去掉星号
    else
    {
        item->setText(level_name);
    }
}

bool MainWindow::isSameFile(const QString &path1, const QString &path2)
{
    bool same = false;
    QFile file1(path1);
    QFile file2(path2);
    if(file1.open(QIODevice::ReadOnly))
    {
        if(file2.open(QIODevice::ReadOnly))
        {
            QByteArray b1 = file1.readAll();
            QByteArray b2 = file2.readAll();
            int n = b1.size();
            if(n == b2.size())
            {
                same = true;
                for(int i = 0; i < n; i++)
                {
                    if(b1.at(i) != b2.at(i))
                    {
                        same = false;
                        break;
                    }
                }
            }
            file2.close();
        }
        else
        {
            info(path2 + "无法打开");
        }
        file1.close();
    }
    else
    {
        info(path1 + "无法打开");
    }
    return same;
}

void MainWindow::createEventTypeJsonObj(NodeInfo *node, QJsonObject *json)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(json != nullptr);
    MY_ASSERT(node->type == ETYPE);
    MY_ASSERT(node->getValuesCount() >= 1);

    int index = node->getValue(0).toInt();
    MY_ASSERT(index != -1);

    json->insert("name", EventType::GetInstance()->GetEventLuaType(index));

//    int params_num = EventType::GetInstance()->paramNamesVector[index].size();
//    if(params_num > 0)
//    {
//        QJsonArray event_params;
//        for(int i = 0; i < params_num; i++)
//        {
//            QString pname = EventType::GetInstance()->paramNamesVector[index][i];
//            event_params.push_back(pname);
//        }
//        json->insert("params", event_params);
//    }
}

void MainWindow::addVariablesToJsonObj(QJsonObject *json)
{
    MY_ASSERT(json != nullptr);

    QJsonArray var_array;
    ValueManager* vm = m_eventTreeModel->GetValueManager();
    QStringList var_list = vm->GetGlobalVarList();
    int n = var_list.size();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            QString var_name = var_list[i];
            int var_id = vm->GetIdOfVariable(var_name);
            MY_ASSERT(var_id != -1);

            QJsonObject var_obj;
            var_obj.insert("id", var_id);
            var_obj.insert("name", var_name);
            var_obj.insert("type", vm->GetVarTypeAt(var_id));
            if(vm->CheckVarIsLevelParam(var_id))
                var_obj.insert("level", true);

            QJsonObject value_obj;
            addValueToJsonObj(vm->GetInitValueOfVar(var_id), &value_obj);
            var_obj.insert("initValue", value_obj);

            var_array.push_back(var_obj);
        }
    }
    json->insert("Var", var_array);
}

// key_name : [
//   {
//      "ACTION" : "xxx",
//      ...
//   },
//   {
//      "ACTION" : "xxx",
//      ...
//   },
//   ...
// ]
void MainWindow::addActionSeqToJsonObj(NodeInfo *node, QJsonObject *json, QString key_name)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(json != nullptr);
    MY_ASSERT(node->type == SEQUENCE);

    QJsonArray node_list;
    int n = node->childs.size();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            NodeInfo* c_node = node->childs[i];
            QJsonObject node_obj;
            node_obj.insert("ACTION", getNodeTypeStr(c_node->type));

            switch(c_node->type)
            {
            case CHOICE:
                if(c_node->childs.size() >= 3)
                {
                    addConditionToJsonObj(c_node->childs[0], &node_obj);
                    addActionSeqToJsonObj(c_node->childs[1], &node_obj, "then");
                    addActionSeqToJsonObj(c_node->childs[2], &node_obj, "else");
                }
                break;
            case LOOP:
                if(c_node->childs.size() >= 1)
                {
//                    node_obj.insert("times", "5"); // todo 循环次数
                    addActionSeqToJsonObj(c_node->childs[0], &node_obj);
                }
                break;
            case END:
                break;
            case SET_VAR:
            {
                ValueManager* vm = m_eventTreeModel->GetValueManager();
                QJsonObject value_obj;
                addValueToJsonObj(vm->GetValueOnNode_SetVar(c_node), &value_obj);
                int var_id = c_node->getValue(0).toInt();
                QString var_name = c_node->GetVarName_SetVar();
                if(var_name != "")
                {
                    node_obj.insert("value", value_obj);
                    node_obj.insert("name", var_name);
                    node_obj.insert("id", var_id);
                }
            }
                break;
            case FUNCTION:
                addFunctionToJsonObj(m_eventTreeModel->GetValueManager()->GetValueOnNode_Function(c_node), &node_obj);
                break;
            case OPEN_EVENT:
            case CLOSE_EVENT:
                node_obj.insert("name", c_node->getValue(0));
                break;
            default:
                break;
            }

            node_list.push_back(node_obj);
        }
    }


    json->insert(key_name, node_list);
}

// "AND/OR" : [conditions]
void MainWindow::addConditionToJsonObj(NodeInfo *node, QJsonObject *json)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(json != nullptr);
    MY_ASSERT(node->type == CONDITION);

    int num = node->childs.size();

    QJsonArray conditions;
    if(num > 0)
    {
        for(int i = 0; i < num; i++)
        {
            if(node->childs[i]->type == CONDITION)
            {
                QJsonObject obj;
                addConditionToJsonObj(node->childs[i], &obj);
                conditions.push_back(obj);
            }
            else if(node->childs[i]->type == COMPARE)
            {
                addComparationToJsonArrary(node->childs[i], &conditions);
            }
        }
    }

    json->insert(node->getValue(0), conditions);
}

// {
//   type  :  "==",
//   value_left  : obj,
//   value_right : obj
// }
void MainWindow::addComparationToJsonArrary(NodeInfo *node, QJsonArray *conditions)
{
    MY_ASSERT(node != nullptr);
    MY_ASSERT(conditions != nullptr);
    MY_ASSERT(node->type == COMPARE);
    MY_ASSERT(node->getValuesCount() >= 3);

    QJsonObject comparation;
    comparation.insert("type", node->getValue(0));

    ValueManager* vm = m_eventTreeModel->GetValueManager();

    QJsonObject value_left;
    addValueToJsonObj(vm->GetValueOnNode_Compare_Left(node), &value_left);
    comparation.insert("value_left", value_left);

    QJsonObject value_right;
    addValueToJsonObj(vm->GetValueOnNode_Compare_Right(node), &value_right);
    comparation.insert("value_right", value_right);

    conditions->push_back(comparation);
}

void MainWindow::addValueToJsonObj(CommonValueClass* value, QJsonObject *json)
{
    MY_ASSERT(value != nullptr);
    MY_ASSERT(json != nullptr);

    switch (value->GetValueType()) {
    case VT_VAR:
    {
        int id = m_eventTreeModel->GetValueManager()->GetIdOfVariable(static_cast<BaseValueClass*>(value));
        if(id == -1)
            info("Json提示：找不到变量" + value->GetText());
        else
            json->insert("type", m_eventTreeModel->GetValueManager()->GetVarTypeOf_Key(value->GetText()));
        json->insert("name", value->GetText());
        json->insert("id", id);
    }
        break;
    case VT_FUNC:
        {
            QJsonObject function;
            addFunctionToJsonObj(static_cast<BaseValueClass*>(value), &function);
            json->insert("call", function);
            json->insert("type", static_cast<BaseValueClass*>(value)->GetFunctionInfo()->GetReturnTypeAt(0)); //可以是空值""，因为有些函数无返回值
        }
        break;
    case VT_STR:
        json->insert("code", value->GetText());
        json->insert("type", value->GetVarType());
        break;
    case VT_ENUM:
        json->insert("enum", value->GetText());
        json->insert("type", value->GetVarType());
        break;
    case VT_PARAM:
        json->insert("param", static_cast<BaseValueClass*>(value)->GetEventParamInLua());
        json->insert("name", value->GetText());
        json->insert("type", value->GetVarType());
        break;
    case VT_TABLE:
    {
        StructValueClass* s_v = static_cast<StructValueClass*>(value);
        QJsonObject table;
        QStringList keys = s_v->GetAllKeys();
        for(int i = 0; i < keys.size(); i++)
        {
            CommonValueClass* v = s_v->GetValueByKey(keys[i]);
            if(v != nullptr)
            {
                QJsonObject vo;
                addValueToJsonObj(v, &vo);
                table.insert(keys[i], vo);
            }
        }
        json->insert("table", table);
        json->insert("type", value->GetVarType());
    }
        break;
    }
}

void MainWindow::addFunctionToJsonObj(BaseValueClass *value, QJsonObject *json)
{
    MY_ASSERT(value != nullptr);
    MY_ASSERT(json != nullptr);
    VALUE_TYPE value_type = value->GetValueType();
    if(value_type == VT_FUNC)
    {
        json->insert("function", value->GetFunctionName());
        //json->insert("id", value->GetFunctionInfo()->GetID());

        QJsonArray params;
        int n = value->GetFunctionParamsNum();
        for(int i = 0; i < n; i++)
        {
            QJsonObject param;
            addValueToJsonObj(value->GetFunctionParamAt(i), &param);
            params.push_back(param);
        }
        json->insert("params", params);
    }
    else if(value_type == VT_STR)
    {
        json->insert("func_lua", value->GetText());
    }
    else
        info("MainWindow::addFunctionToJsonObj value_type error!");
}

bool MainWindow::openJsonFile(QString filePath)
{
    QFile file( filePath );
    if ( !file.open( QIODevice::ReadOnly ) ) {
        info(filePath + "打开失败！");
        return false;
    }

    QJsonParseError jsonParserError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson( file.readAll(), &jsonParserError );

    bool success = true;

    if ( !jsonDocument.isNull() && jsonParserError.error == QJsonParseError::NoError )
    {
//        qDebug() << "open json file: " << fileName << "success!" << endl;

        if ( jsonDocument.isObject() )
        {
            m_eventTreeModel->ClearAllData();
            m_customTreeModel->ClearAllData();
            QJsonObject jsonObject = jsonDocument.object();

            // 变量
            if(jsonObject.contains("Var") && jsonObject.value("Var").isArray())
            {
                QJsonArray varJsonArray = jsonObject.value("Var").toArray();
                parseJsonArray_Var( &varJsonArray );
            }

            // 事件
            if(jsonObject.contains("Event") && jsonObject.value("Event").isArray())
            {
                QJsonArray evtArray = jsonObject.value("Event").toArray();
                int n = evtArray.size();
                for(int i = 0; i < n; i++)
                {
                    if(evtArray.at(i).isObject())
                    {
                        QJsonObject eventJsonObj = evtArray.at(i).toObject();
                        if(eventJsonObj.contains("event_name") && eventJsonObj.value("event_name").isString())
                        {
                            QString event_name = eventJsonObj.value("event_name").toString();
                            if(!parseJsonObj_Event( &eventJsonObj, event_name ))
                                success = false;
                        }
                    }
                    else
                        success = false;
                }
            }

            // 自定义动作
            if(jsonObject.contains("CustomAction") && jsonObject.value("CustomAction").isArray())
            {
                QJsonArray actArray = jsonObject.value("CustomAction").toArray();
                int n = actArray.size();
                for(int i = 0; i < n; i++)
                {
                    if(actArray.at(i).isObject())
                    {
                        QJsonObject jsonObj = actArray.at(i).toObject();
                        if(jsonObj.contains("name") && jsonObj.contains("SEQUENCE") && jsonObj.value("name").isString() && jsonObj.value("SEQUENCE").isArray())
                        {
                            QString name = jsonObj.value("name").toString();
                            QJsonArray jsonArray = jsonObj.value("SEQUENCE").toArray();
                            NodeInfo* seq_node = m_customTreeModel->m_pRootNode->addNewChild(SEQUENCE, name);
                            if(!parseJsonArray_Sequence(&jsonArray, seq_node))
                            {
                                m_eventTreeModel->deleteNode(seq_node); //todo 可能内存泄漏
                                success = false;
                            }
                        }
                        else
                            success = false;
                    }
                    else
                        success = false;
                }
            }
        }
    }

    file.close();
    return success;
}

bool MainWindow::openJsonFile(QListWidgetItem *item, QString& level_name)
{
    QString file_path = this->config_path;

    level_name = getLevelNameOnItem(item);
    if(backupFilePaths.contains(level_name) && !backupFilePaths[level_name].isEmpty())
    {
        file_path = backupFilePaths[level_name].last();
    }
    else
    {
        file_path = file_path + level_name + ".json";
    }

    return openJsonFile(file_path);
}

bool MainWindow::parseJsonArray_Var(QJsonArray *varJsonArray)
{
    int num = varJsonArray->size();
    if(num > 0)
    {
        ValueManager* vm = m_eventTreeModel->GetValueManager();
        for(int i = 0; i < num; i++)
        {
            if(varJsonArray->at(i).isObject())
            {
                QJsonObject var = varJsonArray->at(i).toObject();
                if(var.contains("id") && var.value("id").isDouble() &&
                   var.contains("name") && var.value("name").isString() &&
                   var.contains("type") && var.value("type").isString() &&
                   var.contains("initValue") && var.value("initValue").isObject())
                {
                    int id = var.value("id").toInt();
                    QString name = var.value("name").toString();
                    QString type = var.value("type").toString();
                    QJsonObject init_v = var.value("initValue").toObject();
                    CommonValueClass* v = parseJsonObj_Value(&init_v);
                    bool is_level_param = false;
                    if(var.contains("level") && var.value("level").isBool())
                    {
                        is_level_param = var.value("level").toBool();
                    }
                    if(v != nullptr)
                    {
                        if(v->GetValueType() < VT_TABLE)
                            static_cast<BaseValueClass*>(v)->SetVarType(type);
                        else
                            static_cast<StructValueClass*>(v)->SetVarType(type);
                        if( !vm->AddNewVarAtPos(name, v, id, is_level_param) )
                            delete v;
                    }
                    else
                        info("变量" + name + "的初始值解析失败！");
                    continue;
                }
            }
        }
    }
    updateVarTable();
    return true;
}

bool MainWindow::parseJsonObj_Event(QJsonObject *eventJsonObj, QString event_name)
{
    // 解析事件类型，创建事件树的根节点
    NodeInfo* event_node = nullptr;
    if(eventJsonObj->contains("event_type") && eventJsonObj->value("event_type").isObject())
    {
        QJsonObject eventTypeObj = eventJsonObj->value("event_type").toObject();
        QString etype;
        if(eventTypeObj.contains("name") && eventTypeObj.value("name").isString())
            etype = eventTypeObj.value("name").toString();
        else
            return false;
        event_node = createNewEventOnTree(etype, event_name);
        if(event_node == nullptr)
            return false;
    }
    else
        return false;

    // 修改事件树的条件节点
    NodeInfo* condition_node = event_node->childs[1];
    if(eventJsonObj->contains("AND") && eventJsonObj->value("AND").isArray())
    {
        QJsonArray conditions = eventJsonObj->value("AND").toArray();
        parseJsonArray_Condition(&conditions, condition_node);
    }
    else if(eventJsonObj->contains("OR") && eventJsonObj->value("OR").isArray())
    {
        QJsonArray conditions = eventJsonObj->value("OR").toArray();
        parseJsonArray_Condition(&conditions, condition_node);
        condition_node->modifyValue(0, "OR");
        condition_node->UpdateText();
    }
    else
        return false;

    // 动作序列
    if ( eventJsonObj->contains("SEQUENCE") && eventJsonObj->value("SEQUENCE").isArray() )
    {
        QJsonArray jsonArray = eventJsonObj->value("SEQUENCE").toArray();
        if(!parseJsonArray_Sequence(&jsonArray, event_node->childs[2]))
        {
            m_eventTreeModel->deleteNode(event_node);
            return false;
        }
    }
    else
    {
        m_eventTreeModel->deleteNode(event_node);
        return false;
    }
    return true;
}

bool MainWindow::parseJsonArray_Condition(QJsonArray *conditions, NodeInfo *condition_node)
{
    MY_ASSERT(condition_node->type == CONDITION);
    MY_ASSERT(condition_node->getValuesCount() == 1);
    MY_ASSERT(condition_node->getValue(0) == "AND" || condition_node->getValue(0) == "OR"); //最顶层的条件节点必须是AND或OR类型
    bool ok = false;

    int num = conditions->size();
    if(num > 0)
    {
        for ( int i = 0; i < num; i++ )
        {
            if (conditions->at(i).isObject())
            {
                QJsonObject condition = conditions->at(i).toObject();
                // AND 节点
                if(condition.contains("AND") && condition.value("AND").isArray())
                {
                    NodeInfo* new_node = condition_node->addNewChild(CONDITION, "AND");
                    new_node->modifyValue(AND);
                    QJsonArray andArr = condition.value("AND").toArray();
                    ok = parseJsonArray_Condition(&andArr, new_node);
                }
                // OR 节点
                else if(condition.contains("OR") && condition.value("OR").isArray())
                {
                    NodeInfo* new_node = condition_node->addNewChild(CONDITION, "OR");
                    new_node->modifyValue(OR);
                    QJsonArray andArr = condition.value("OR").toArray();
                    ok = parseJsonArray_Condition(&andArr, new_node);
                }
                // compare 节点
                else if(condition.contains("type") && condition.value("type").isString() &&
                        condition.contains("value_left") && condition.value("value_left").isObject() &&
                        condition.contains("value_right") && condition.value("value_right").isObject())
                {
                    QJsonObject lv_obj = condition.value("value_left").toObject();
                    QJsonObject rv_obj = condition.value("value_right").toObject();
                    BaseValueClass* left_value = (BaseValueClass*)parseJsonObj_Value(&lv_obj); //暂定只能是BaseValue而非StructValue
                    BaseValueClass* right_value = (BaseValueClass*)parseJsonObj_Value(&rv_obj); //暂定只能是BaseValue而非StructValue
                    if(left_value != nullptr && right_value != nullptr)
                    {
                        QString compare_type = condition.value("type").toString();
                        NodeInfo* new_node = condition_node->addNewChild_Compare(compare_type, left_value->GetText(), right_value->GetText());
                        m_eventTreeModel->GetValueManager()->UpdateValueOnNode_Compare_Left(new_node, left_value);
                        m_eventTreeModel->GetValueManager()->UpdateValueOnNode_Compare_Right(new_node, right_value);
                        ok = true;
                    }
                    if(left_value != nullptr) delete left_value;
                    if(right_value != nullptr) delete right_value;
                }
            }
        }
    }
    else
        return true; // 条件为空
    return ok;
}

bool MainWindow::parseJsonObj_ActionNode(QJsonObject *actionJsonObj, NodeInfo *parent_node)
{
    if(actionJsonObj->contains( "ACTION" ) && actionJsonObj->value( "ACTION" ).isString())
    {
        NODE_TYPE node_type = GetActionNodeTypeEnum(actionJsonObj->value( "ACTION" ).toString());
        switch (node_type)
        {
        case CHOICE:
        {
            if(actionJsonObj->contains("AND") && actionJsonObj->value("AND").isArray() &&
               actionJsonObj->contains("then") && actionJsonObj->value("then").isArray() &&
               actionJsonObj->contains("else") && actionJsonObj->value("else").isArray())
            {
                QJsonArray if_cond = actionJsonObj->value("AND").toArray();
                QJsonArray then_seq = actionJsonObj->value("then").toArray();
                QJsonArray else_seq = actionJsonObj->value("else").toArray();

                NodeInfo* new_node = parent_node->addNewChild(node_type, "if");
                if(new_node != nullptr)
                {
                    if(parseJsonArray_Condition(&if_cond, new_node->childs[0]) &&
                       parseJsonArray_Sequence(&then_seq, new_node->childs[1]) &&
                       parseJsonArray_Sequence(&else_seq, new_node->childs[2]))
                    {
                        return true;
                    }
                }
            }
        }
            break;
        case LOOP:
        {
            if(actionJsonObj->contains("SEQUENCE") && actionJsonObj->value("SEQUENCE").isArray())
            {
                QJsonArray loop_seq = actionJsonObj->value("SEQUENCE").toArray();
                NodeInfo* new_node = parent_node->addNewChild(node_type, "循环");
                if(new_node != nullptr)
                {
                    if(parseJsonArray_Sequence(&loop_seq, new_node->childs[0]))
                    {
                        return true;
                    }
                }
            }
        }
            break;
        case END:
        {
            NodeInfo* new_node = parent_node->addNewChild(node_type, "跳出");
            if(new_node != nullptr)
                return true;
        }
        case FUNCTION:
        {
            BaseValueClass* v_func = parseJsonObj_Function(actionJsonObj);
            if(v_func != nullptr)
            {
                NodeInfo* new_node = parent_node->addNewChild(node_type, "");
                if(new_node != nullptr)
                {
                    m_eventTreeModel->GetValueManager()->UpdateValueOnNode_Function(new_node, v_func);
                    new_node->text = v_func->GetText();
                    return true;
                }
            }
        }
            break;
        case SET_VAR:
        {
            if(actionJsonObj->contains("name") && actionJsonObj->value("name").isString() &&
               actionJsonObj->contains("value") && actionJsonObj->value("value").isObject() &&
               actionJsonObj->contains("id") && actionJsonObj->value("id").isDouble())
            {
                QString var_text = actionJsonObj->value("name").toString();
                QString var_name = ValueManager::GetVarNameInKeyStr(var_text);
                int var_id = actionJsonObj->value("id").toInt();

                // 检查全局变量表中是否存在这个变量
                if(m_eventTreeModel->GetValueManager()->GetVarNameAt(var_id) == var_name && var_name != "")
                {
                    // 使用 new 创建 value
                    QJsonObject var_obj = actionJsonObj->value("value").toObject();
                    BaseValueClass* var_value = (BaseValueClass*)parseJsonObj_Value(&var_obj); //暂定只能是BaseValue而非StructValue
                    if(var_value != nullptr)
                    {
                        // 给父节点创建 set value 类型的子节点
                        QString node_text = var_text + " = " + var_value->GetText();
                        NodeInfo* new_node = parent_node->addNewChildNode_SetVar(node_text, var_id);
                        if(new_node != nullptr)
                        {
                            m_eventTreeModel->GetValueManager()->UpdateValueOnNode_SetValue(new_node, var_value);
                            return true;
                        }
                        else
                            delete var_value;
                    }
                }
            }
        }
            break;
        case OPEN_EVENT:
        case CLOSE_EVENT:
        {
            if(actionJsonObj->contains("name") && actionJsonObj->value("name").isString())
            {
                NodeInfo* new_node = parent_node->addNewChild(node_type, "");
                if(new_node != nullptr)
                {
                    new_node->modifyValue(0, actionJsonObj->value("name").toString());
                    new_node->UpdateText();
                    return true;
                }
            }
        }
        default:
            info(node_type + "是无法识别的动作类型！");
            return false;
        }
    }

    return false;
}

bool MainWindow::parseJsonArray_Sequence(QJsonArray *seqJsonArray, NodeInfo *seq_node)
{
    int action_num = seqJsonArray->size();
    for ( int i = 0; i < action_num; i++ )
    {
        if ( seqJsonArray->at(i).isObject() )
        {
            QJsonObject actionJsonObj = seqJsonArray->at(i).toObject();
            if(!parseJsonObj_ActionNode(&actionJsonObj, seq_node))
            {
               return false;
            }
        }
    }
    return true;
}

CommonValueClass *MainWindow::parseJsonObj_Value(QJsonObject *valueJsonObj)
{
    if(valueJsonObj->contains("table") && valueJsonObj->value("table").isObject() &&
       valueJsonObj->contains("type") && valueJsonObj->value("type").isString())
    {
        StructValueClass* value = new StructValueClass();
        QString type = valueJsonObj->value("type").toString();
        value->SetVarType(type);
        QJsonObject table = valueJsonObj->value("table").toObject();
        QJsonObject::iterator it = table.begin();
        for(; it != table.end(); ++it)
        {
            if(it.value().isObject())
            {
                QJsonObject tvobj = it.value().toObject();
                CommonValueClass* tv = parseJsonObj_Value(&tvobj);
                if(tv == nullptr)
                    info("table的值" + it.key() + "解析失败");
                else
                    value->UpdateMember_Move(it.key(), tv);
            }
        }
        return value;
    }

    BaseValueClass* value = nullptr;
    if(valueJsonObj->contains("name") && valueJsonObj->value("name").isString() &&
       valueJsonObj->contains("id") && valueJsonObj->value("id").isDouble() &&
       valueJsonObj->contains("type") && valueJsonObj->value("type").isString())
    {
        // 检查变量管理器中是否存在这个变量
        QString name = valueJsonObj->value("name").toString();
        int id = valueJsonObj->value("id").toInt();
        QString var_type = valueJsonObj->value("type").toString();
        int pos = -1;
        QString var_name = ValueManager::GetVarNameInKeyStr(name, &pos);
        if(var_name == m_eventTreeModel->GetValueManager()->GetVarNameAt(id))
        {
            QString key_name = pos == -1 ? "" : name.mid(pos + 1);
            value = new BaseValueClass();
            MY_ASSERT(var_type == m_eventTreeModel->GetValueManager()->GetVarTypeOf_Key(name));
            value->SetVarName(var_name, var_type, id, key_name);
        }
        else
            info("Json解析：找不到变量" + name + " id=" + id);
    }
    else if(valueJsonObj->contains("code") && valueJsonObj->value("code").isString() &&
            valueJsonObj->contains("type") && valueJsonObj->value("type").isString())
    {
        QString code = valueJsonObj->value("code").toString();
        value = new BaseValueClass(code);
        QString type = valueJsonObj->value("type").toString();
        value->SetVarType(type);
    }
    else if(valueJsonObj->contains("call") && valueJsonObj->value("call").isObject() &&
            valueJsonObj->contains("type") && valueJsonObj->value("type").isString())
    {
        QJsonObject func_obj = valueJsonObj->value("call").toObject();
        value = parseJsonObj_Function(&func_obj);
        if(value != nullptr)
        {
            QString type = valueJsonObj->value("type").toString();
            value->SetVarType(type);
        }
        else
            info("Json解析：Function" + func_obj.value("function").toString() + "创建失败");
    }
    else if(valueJsonObj->contains("enum") && valueJsonObj->value("enum").isString() &&
            valueJsonObj->contains("type") && valueJsonObj->value("type").isString())
    {
        QString code = valueJsonObj->value("enum").toString();
        value = new BaseValueClass();
        QString type = valueJsonObj->value("type").toString();
        value->SetVarType(type);
        value->SetEnumValue(code);
    }
    else if(valueJsonObj->contains("param") && valueJsonObj->value("param").isString() &&
            valueJsonObj->contains("name") && valueJsonObj->value("name").isString() &&
            valueJsonObj->contains("type") && valueJsonObj->value("type").isString())
    {
        QString code = valueJsonObj->value("param").toString();
        QString name = valueJsonObj->value("name").toString();
        QString type = valueJsonObj->value("type").toString();
        value = new BaseValueClass();
        value->SetEvtParam(code, name, type);
    }
    return value;
}

BaseValueClass *MainWindow::parseJsonObj_Function(QJsonObject *func_obj)
{
    BaseValueClass* value = nullptr;

    if(func_obj->contains("function") && func_obj->value("function").isString() &&
       //func_obj->contains("id") && func_obj->value("id").isDouble() &&
       func_obj->contains("params") && func_obj->value("params").isArray())
    {
        QString func_name = func_obj->value("function").toString();
        //FUNCTION_ID func_id = func_obj->value("id").toInt();
        FunctionClass* func = FunctionInfo::GetInstance()->GetFunctionInfoByLuaName(func_name);
        if(func != nullptr && func->GetNameLua() == func_name)
        {
            value = new BaseValueClass();
            value->SetFunction(func); //这里已经new指定数量的参数了
            bool success = true; //如果失败要把value释放掉

            QJsonArray params = func_obj->value("params").toArray();
            int param_num = params.size();
            if(param_num != func->GetParamNum())
                success = false;

            for(int i = 0; i < param_num; i++)
            {
                if(params[i].isObject())
                {
                    QJsonObject param = params[i].toObject();
                    CommonValueClass* param_v = parseJsonObj_Value(&param);
                    if(param_v != nullptr)
                    {
                        value->SetParamAt(i, param_v);
                        delete param_v;
                        param_v = nullptr;
                    }
                    else
                    {
                        info("解析Json时，设置函数参数值失败");
                        success = false;
                        break;
                    }
                }
                else {
                    success = false;
                    break;
                }
            }
            if(!success)
            {
                delete value;
                value = nullptr;
            }
        }
    }
    else if(func_obj->contains("func_lua") && func_obj->value("func_lua").isString())
    {
        QString func_lua = func_obj->value("func_lua").toString();
        value = new BaseValueClass(func_lua);
    }

    return value;
}

void MainWindow::generateLuaDocument(QFile *file)
{
    int trigger_num = m_eventTreeModel->m_pRootNode->childs.size();
    if(trigger_num <= 0)
        return;

    // 先把同一种事件类型的trigger放到一个vector里面
    event_pos_in_table.clear();
    QMap<QString, QVector<int>> event_map;
    for(int i = 0; i < trigger_num; i++)
    {
        NodeInfo* node = m_eventTreeModel->m_pRootNode->childs[i];
        QString event_id = node->childs[0]->childs[0]->getValue(0);
        if(!event_map.contains(event_id))
        {
            event_map.insert(event_id, QVector<int>());
        }
        event_pos_in_table.insert(i, event_map[event_id].size());
        event_map[event_id].append(i);
    }

    file->write("local levelTable = {}\n\n");

    // 定义所有变量
    //writeLuaVariables(file);
    //file->write("\n");

    // 生成自定义动作
    writeLuaCustomActions(file);

    // 生成条件检查函数、动作序列函数
    space_num = 0;
    for(int i = 0; i < trigger_num; i++)
    {
        NodeInfo* event_node = m_eventTreeModel->m_pRootNode->childs[i];
        writeLuaEventCheckFunc(file, event_node->childs[1]);
        writeLuaEventActionFunc(file, event_node->childs[2]);
    }

    // 创建一个 lua table 事件列表
    file->write("levelTable.EventFunc = {\n");
    QMap<QString, QVector<int>>::iterator itr;
    for(itr = event_map.begin(); itr != event_map.end(); ++itr)
    {
        QString etype = EventType::GetInstance()->GetEventLuaType(itr.key().toInt());
        QString line = QString("    [E_LEVEL_TRIGGER_TYPE.%1] = {\n").arg(etype); //[事件类型ID] = { {}, {}, ... }
        file->write(line.toStdString().c_str());
        space_num = 8;
        for(int j = 0; j < itr.value().size(); j++)
        {
            writeLuaEventInfo(file, m_eventTreeModel->m_pRootNode->childs[itr.value().at(j)]);
        }
        file->write("    },\n");
    }
    file->write("}\n\n");

    // 生成一个Init函数
    writeLuaVarInitFunc(file);

    // 生成一个GetLevelParams函数
    writeLuaGetParamFunc(file);

    // 生成一个Excute函数
    file->write("function levelTable:Execute(event)\n"
                "    local ret = false\n"
                "    if self.EventFunc[event.id] ~= nil then\n"
                "        for _, func in ipairs (self.EventFunc[event.id]) do\n"
                "            if func and func.check and func.call and func.enable and func.enable == 1 and func.check(event, self) == 1 then\n"
                "                func.call(event, self)\n"
                "                ret = true\n"
                "                -- break\n"
                "            end\n"
                "        end\n"
                "    end\n"
                "    return ret\n"
                "end\n\n");

    file->write("return levelTable\n");
}

void MainWindow::writeLuaVariables(QFile *file)
{
    ValueManager* vm = m_eventTreeModel->GetValueManager();
    QStringList list = vm->GetGlobalVarList();
    int n = list.size();
    for(int i = 0; i < n; i++)
    {
        if(list[i] != "")
        {
            QString line = QString("local %1\n").arg(list[i]);
            file->write(line.toStdString().c_str());
        }
    }
}

void MainWindow::writeLuaCustomActions(QFile *file)
{
    int n = NodeInfo::GetRootNode_Custom()->childs.size();
    for(int i = 0; i < n; i++)
    {
        NodeInfo* seq_node = NodeInfo::GetRootNode_Custom()->childs[i];
        MY_ASSERT(seq_node->type = SEQUENCE);
        QString line = "-- " + seq_node->text + "\nlocal function CustomAction_" + QString::number(i) + "(level)\n";
        file->write(line.toStdString().c_str());
        space_num = 4;
        if(!writeLuaSequence(file, seq_node))
            info("生成动作序列的Lua函数失败！");
        file->write("end\n\n");
        space_num = 0;
    }
}

void MainWindow::writeLuaVarInitFunc(QFile *file)
{
    file->write("function levelTable:init(flowController)\n    self.flowController = flowController\n");

    ValueManager* vm = m_eventTreeModel->GetValueManager();
    QStringList list = vm->GetGlobalVarList();
    int n = list.size();
    for(int i = 0; i < n; i++)
    {
        QString line = QString("    self.%1 = ").arg(list[i]);
        // 赋值为 initValue
        line += vm->GetInitValueOfVarByName(list[i])->GetLuaValueString("");
        // 注释变量类型
        for(int si = line.length(); si < 24; si++)
        {
            line += " ";
        }
        line = line + "\t-- " + vm->GetVarTypeOf_Table(list[i]) + "\n";

        file->write(line.toStdString().c_str());
    }

    for(int i = 0; i < m_eventTreeModel->m_pRootNode->childs.size(); i++)
    {
        QString line = "    self.";
        writeLuaOpenOrCloseEvent(file, i, line, true);
    }

    file->write("    self.bParamsDirty = false\nend\n\n");
}

void MainWindow::writeLuaGetParamFunc(QFile *file)
{
    file->write("function levelTable:GetLevelParams()\n    return {\n");

    QString line;
    ValueManager* vm = m_eventTreeModel->GetValueManager();
    QStringList list = vm->GetGlobalVarList();
    int n = list.size();
    for(int i = 0; i < n; i++)
    {
        if(vm->CheckVarIsLevelParam(vm->GetIdOfVariable(list[i])))
        {
            if(i + 1 == n)
                line = QString("        %1 = self.%2\n").arg(list[i]).arg(list[i]);
            else
                line = QString("        %1 = self.%2,\n").arg(list[i]).arg(list[i]);
            file->write(line.toStdString().c_str());
        }
    }

    file->write("    }\nend\n\n");
}

// {
//     "事件名称",
//     function(args) return Check(args) end,
//     function(sceneId, args) Execute(sceneId, args) end
// }
bool MainWindow::writeLuaEventInfo(QFile *file, NodeInfo *event_node)
{
    MY_ASSERT(event_node != nullptr);
    MY_ASSERT(event_node->type == EVENT);

    int id = findLuaIndexOfEvent(event_node);
    MY_ASSERT(id != -1);
    QString index = QString::number(id);

    QString str_space = "";
    for(int i = 0; i < space_num; i++)
        str_space += " ";

    QString line = str_space + "{   --";
    file->write(line.toStdString().c_str());

    // 事件名称
    line = event_node->text + "\n";
    file->write(line.toStdString().c_str());
    // 条件检查函数
    line = str_space + "    check = Check_" + index + ",\n";
    file->write(line.toStdString().c_str());
    // 执行动作函数
    line = str_space + "    call = Execute_" + index + ",\n";
    file->write(line.toStdString().c_str());
    // enable
    line = str_space + "    enable = 1\n";
    file->write(line.toStdString().c_str());

    line = str_space + "},\n";
    file->write(line.toStdString().c_str());

    return true;
}

// function Check(args)
// end
bool MainWindow::writeLuaEventCheckFunc(QFile *file, NodeInfo *condition_node)
{
    MY_ASSERT(condition_node->parent->type == EVENT);
    MY_ASSERT(condition_node->type = CONDITION);

    int id = findLuaIndexOfEvent(condition_node);
    MY_ASSERT(id != -1);
    QString index = QString::number(id);

    QString line = "local function Check_" + index + "(event, level)\n";
    file->write(line.toStdString().c_str());

    if(condition_node->childs.size() == 0)
    {
        file->write("    return 1\nend\n\n");
        return true;
    }
    else
    {
        file->write("    if ");
        bool success = writeLuaCondition(file, condition_node);
        file->write(" then\n        return 1\n    else\n        return 0\n    end\nend\n\n");
        return success;
    }
}

bool MainWindow::writeLuaCondition(QFile *file, NodeInfo *condition_node)
{
    MY_ASSERT(condition_node != nullptr);
    MY_ASSERT(condition_node->type == CONDITION || condition_node->type == COMPARE);

    CONDITION_OP opt_enum = getConditionEnum(condition_node->getValue(0));
    switch (opt_enum) {
    case AND:
    {
        file->write("(");
        int children_num = condition_node->childs.size();
        for(int i = 0; i < children_num; i++)
        {
            if(i != 0)
                file->write(" and ");
            writeLuaCondition(file, condition_node->childs[i]);
        }
        if(children_num < 1)
        {
            if(condition_node->parent->type == CONDITION && condition_node->parent->getValue(0) == "OR")
                file->write("0");
            else
                file->write("1");
        }
        file->write(")");
    }
        break;
    case OR:
    {
        file->write("(");
        int children_num = condition_node->childs.size();
        for(int i = 0; i < children_num; i++)
        {
            if(i != 0)
                file->write(" or ");
            writeLuaCondition(file, condition_node->childs[i]);
        }
        if(children_num < 1)
        {
            if(condition_node->parent->type == CONDITION && condition_node->parent->getValue(0) == "OR")
                file->write("0");
            else
                file->write("1");
        }
        file->write(")");
    }
        break;
    case EQUAL_TO:
    case GREATER_THAN:
    case LESS_THAN:
    case EQUAL_GREATER:
    case EQUAL_LESS:
    {
        MY_ASSERT(condition_node->getValuesCount() == 3);
        ValueManager* vm = m_eventTreeModel->GetValueManager();
        CommonValueClass* v_left = vm->GetValueOnNode_Compare_Left(condition_node);
        CommonValueClass* v_right = vm->GetValueOnNode_Compare_Right(condition_node);
        if(v_left != nullptr && v_right != nullptr)
        {
            QString line = v_left->GetLuaValueString("level.") + " " + condition_node->getValue(0) + " " + v_right->GetLuaValueString("level.");
            file->write(line.toStdString().c_str());
            if(!CommonValueClass::AreSameVarType(v_left, v_right))
                info("Lua提示：比较大小时，左值" + v_left->GetText() + "和右值" + v_right->GetText() + "的数据类型不一致");
        }
        else
            return false;
    }
        break;
    default:
        return false;
        break;
    }
    return true;
}

bool MainWindow::writeLuaEventActionFunc(QFile *file, NodeInfo *sequence_node)
{
    MY_ASSERT(sequence_node->parent->type == EVENT);
    MY_ASSERT(sequence_node->type = SEQUENCE);

    int id = findLuaIndexOfEvent(sequence_node);
    MY_ASSERT(id != -1);

    QString line = "local function Execute_" + QString::number(id) + "(event, level)\n";
    file->write(line.toStdString().c_str());

    if(sequence_node->childs.size() <= 0)
    {
        file->write("end\n\n");
        return true;
    }
    else
    {
        space_num = 4;
        if(writeLuaSequence(file, sequence_node))
        {
            file->write("end\n\n");
            return true;
        }
        else
        {
            space_num = 0;
            info("生成动作序列的Lua函数失败！");
            return false;
        }
    }
}

bool MainWindow::writeLuaSequence(QFile *file, NodeInfo *sequence_node)
{
    QString str_space = "";
    for(int i =0; i < space_num; i++)
        str_space += " ";

    bool success = true;
    int children_num = sequence_node->childs.size();

    for(int i = 0; i < children_num; i++)
    {
        NodeInfo* node = sequence_node->childs[i];
        switch (node->type) {
        case SET_VAR:
            success = writeLuaSetVar(file, node);
            break;
        case FUNCTION:
        {
            ValueManager* vm = m_eventTreeModel->GetValueManager();
            QString str_call = vm->GetValueOnNode_Function(node)->GetLuaValueString("level.");
            QString line = str_space + str_call + "\n";
            file->write(line.toStdString().c_str());
        }
            break;
        case CHOICE:
            if(node->childs.size() != 3 || node->childs[0]->type != CONDITION || node->childs[1]->type != SEQUENCE || node->childs[2]->type != SEQUENCE)
            {
                info("CHOICE节点的子节点数量和类型错误！");
                success = false;
            }
            else
            {
                // if ... then
                QString line = str_space + "if ";
                file->write(line.toStdString().c_str());
                if(!writeLuaCondition(file, node->childs[0]))
                    success = false;
                file->write(" then\n");

                // 一系列动作
                space_num += 4;
                if(!writeLuaSequence(file, node->childs[1]))
                    success = false;
                space_num -= 4;

                // else
                line = str_space + "else\n";
                file->write(line.toStdString().c_str());

                // 一系列动作
                space_num += 4;
                if(!writeLuaSequence(file, node->childs[2]))
                    success = false;
                space_num -= 4;

                // end
                line = str_space + "end\n";
                file->write(line.toStdString().c_str());
            }
            break;
        case LOOP:
            if(node->childs.size() != 1 || node->childs[0]->type != SEQUENCE)
            {
                info("LOOP节点的子节点数量和类型错误！");
                success = false;
            }
            else
            {
                // while 1 do
                QString line = str_space + "while 1 do\n";
                file->write(line.toStdString().c_str());

                // 一系列动作
                space_num += 4;
                if(!writeLuaSequence(file, node->childs[0]))
                    success = false;
                space_num -= 4;

                // end
                line = str_space + "end\n";
                file->write(line.toStdString().c_str());
            }
            break;
        case END:
            if(node->parent->type == SEQUENCE)
            {
                QString line = str_space;
                bool flag = node->IsBreakButNotReturn();

                if(flag)
                {
                    line = line + "break\n";
                    file->write(line.toStdString().c_str());
                }
                else
                {
                    line = line + "return\n";
                    file->write(line.toStdString().c_str());
                }
            }
            else
            {
                info("END跳出节点的位置有错误！");
            }
            break;
        case OPEN_EVENT:
        case CLOSE_EVENT:
        {
            int id = m_eventTreeModel->FindUppestNodePosByName(node->getValue(0));
            if(id == -1)
                success = false;
            else if(event_pos_in_table.contains(id))
            {
                QString line = str_space + "level.";
                success = writeLuaOpenOrCloseEvent(file, id, line, node->type == OPEN_EVENT ? true : false);
            }
            else
                success = false;
        }
            break;
        default:
            break;
        }
    }

    return success;
}

bool MainWindow::writeLuaSetVar(QFile *file, NodeInfo *setvar_node)
{
    MY_ASSERT(setvar_node != nullptr);
    MY_ASSERT(setvar_node->type == SET_VAR);
    MY_ASSERT(setvar_node->getValuesCount() >= 1);

    ValueManager* vm = m_eventTreeModel->GetValueManager();
    CommonValueClass* value = vm->GetValueOnNode_SetVar(setvar_node);

    bool ok = false;
    int id = setvar_node->getValue(0).toInt(&ok);
    if(!ok)
    {
        info("set_var节点的value[0]不是var_id");
        return false;
    }
    QString var_name = setvar_node->GetVarName_SetVar();
//    if(vm->GetVarTypeOf(va) != value->GetVarType() && value->GetValueType() != VT_STR)
//        info("Lua提示：设置变量" + var_name + "的类型与值" + value->GetText() + "的类型不一致");

    QString str_value = value->GetLuaValueString("level.");
    if(str_value == "")
        return false;

    QString line = "";
    for(int i = 0; i < space_num; i++)
    {
        line += " ";
    }

    QString line_1 = line + QString("level.%1 = %2\n").arg(var_name).arg(str_value);
    file->write(line_1.toStdString().c_str());

    // level param 变更标记
    if(vm->CheckVarIsLevelParam(id))
    {
        QString line_2 = line + "level.bParamsDirty = true\n";
        file->write(line_2.toStdString().c_str());
    }

    return true;
}

bool MainWindow::writeLuaOpenOrCloseEvent(QFile *file, int event_pos, const QString &pre_str, bool is_open)
{
    // todo 判断 false
    NodeInfo* evt_node = m_eventTreeModel->m_pRootNode->childs[event_pos];
    int pos = event_pos_in_table[event_pos] + 1;
    QString etype = EventType::GetInstance()->GetEventLuaType(evt_node->childs[0]->childs[0]->getValue(0).toInt());
    QString line = pre_str;
    line = line + "EventFunc[E_LEVEL_TRIGGER_TYPE." + etype + "][" + QString::number(pos) + "].enable = ";
    if(is_open)
        line = line + "1\n";
    else
        line = line + "0\n";
    file->write(line.toStdString().c_str());
    return true;
}

int MainWindow::findLuaIndexOfEvent(NodeInfo *node)
{
    for(int i = 0; i < m_eventTreeModel->m_pRootNode->childs.size(); i++)
    {
        if(m_eventTreeModel->findUppestNodeOf(node) == m_eventTreeModel->m_pRootNode->childs[i])
            return i;
    }
    return -1;
}

QString MainWindow::getTriggerNameAt(int id)
{
    int event_num = m_eventTreeModel->m_pRootNode->childs.size();
    MY_ASSERT(event_num > id && id >= 0);

    return m_eventTreeModel->m_pRootNode->childs[id]->text;
}

void MainWindow::on_eventTreeView_clicked(const QModelIndex &index)
{
    if (index.isValid())
    {
        m_curNode = reinterpret_cast<NodeInfo*>(index.internalPointer());
    }
    else
    {
        m_curNode = nullptr;
    }
}

void MainWindow::on_eventTreeView_doubleClicked(const QModelIndex &index)
{
    on_eventTreeView_clicked(index);
    slotEditNode();
}

void MainWindow::on_btnAddVar_clicked()
{
    m_dlgManageVar->CreateVar();
    updateVarTable();
    saveBackupJsonFile();
}

void MainWindow::on_actionSave_triggered()
{
    QString file_path = this->config_path;

    QString level_str = getLevelNameOnItem(ui->levelList->currentItem());
    if(level_str != "" && level_str.contains(m_levelPrefix))
    {
        QString level_file_path = file_path + level_str + ".json";

        QFile file_level(level_file_path);
        if(!file_level.open(QIODevice::ReadWrite))
        {
            qDebug() << "File open error: " << level_file_path << endl;
        }
        else
        {
            file_level.resize(0);
            generateJsonDocument(&file_level);
            file_level.close();
            changeSavedFlag(level_str, true);
        }
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString file_path = this->config_path;
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", file_path, "Json (*.json)");
    if(!fileName.isNull())
    {
        openJsonFile(fileName);
    }
}

void MainWindow::on_actionLua_triggered()
{
    QString file_path = QCoreApplication::applicationFilePath();
    file_path.replace("LevelEditor.exe", m_LuaPath);
    QString fileName = QFileDialog::getSaveFileName(this, "Save Lua", file_path, "Lua (*.lua)");
    if(!fileName.isEmpty())
    {
        QFile file(fileName);
        if(!file.open(QIODevice::ReadWrite))
        {
            info("无效路径！");
        }
        else
        {
            file.resize(0);
            generateLuaDocument(&file);
            file.close();
        }
    }
}

void MainWindow::InitLevelTree()
{
    DlgWaiting* wait = new DlgWaiting(this);
    wait->show();

    m_levelList.clear();
    savedOrNot.clear();
    ui->levelList->clear();
    backupFilePaths.clear();
    backupFilePaths_Redo.clear();
    deleteAllBackupFiles();
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);

    QDir dir(config_path.left(config_path.length() - 1));
    if(!dir.exists())
        return;
    dir.setFilter(QDir::Files);
    QStringList file_list = dir.entryList();

    int file_num = file_list.size();
    for(int i = 0; i < file_num; i++)
    {
        bool ok = false;
        int lvl_id = -1;
        QString name = file_list[i];
        int pos = name.indexOf(".json");
        int prefix_num = m_levelPrefix.length();
        if(pos != -1 && name.left(prefix_num) == m_levelPrefix)
        {
            lvl_id = name.mid(prefix_num, pos - prefix_num).toInt(&ok);
        }
        if(ok && lvl_id != -1)
            m_levelList.insert(lvl_id - 1, name.left(pos));
    }

    ui->levelList->addItems(m_levelList);
    lastLevelIndex = -1;
    saveBackupJsonFile();

    if(m_levelList.size() > 0)
    {
        ui->levelList->setCurrentRow(0);
        on_levelList_itemClicked(ui->levelList->item(0));
    }

    wait->close();
}

bool MainWindow::checkNewLevelName()
{
    if(m_levelList.contains(m_dlgChoseEvtType->event_name))
    {
        info("已经存在这个名字的关卡！");
        return false;
    }

    int prefix_num = m_levelPrefix.length();
    if(m_dlgChoseEvtType->event_name.left(prefix_num) != m_levelPrefix)
    {
        info(QString("请命名为\"") + m_levelPrefix + QString("数字\"的形式。"));
        return false;
    }

    QString s_num = m_dlgChoseEvtType->event_name.mid(prefix_num);
    bool ok = false;
    s_num.toInt(&ok);
    if(!ok)
    {
        info(QString("请命名为\"") + m_levelPrefix + QString("数字\"的形式。"));
        return false;
    }

    return true;
}

bool MainWindow::checkLevelPrefix(const QString &str)
{
    QByteArray ba = str.toLatin1();
    const char *s = ba.data();
    while(*s)
    {
        if( !((*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z') || *s == '_') )
        {
            info("前缀只能由英文字母和下划线组成");
            return false;
        }
        s++;
    }
    return true;
}

QString MainWindow::getCurrentLevelFile(const QString &level_name, bool *is_backup)
{
    if(backupFilePaths.contains(level_name) && !backupFilePaths[level_name].isEmpty())
    {
        if(is_backup != nullptr)
            *is_backup = true;
        return backupFilePaths[level_name].last();
    }
    else
    {
        if(is_backup != nullptr)
            *is_backup = false;
        return QString(config_path + level_name + ".json");
    }
}

void MainWindow::resetUndoAndRedo(const QString &level_name)
{
    // 撤销
    if(backupFilePaths.contains(level_name) && backupFilePaths[level_name].size() > 1)
        ui->actionUndo->setEnabled(true);
    else
        ui->actionUndo->setEnabled(false);

    // 重做
    if(backupFilePaths_Redo.contains(level_name) && !backupFilePaths_Redo[level_name].isEmpty())
        ui->actionRedo->setEnabled(true);
    else
        ui->actionRedo->setEnabled(false);
}

void MainWindow::saveBackupJsonFile()
{
    if(lastLevelIndex == -1)
    {
        // 生成初始备份文件
        foreach(QString namelvl, m_levelList)
        {
            saveBackupWhenInit(namelvl);
            savedOrNot.insert(namelvl, true);
        }
    }
    else
    {
        // 备份当前关卡
        QString level_name = m_levelList[lastLevelIndex];
        saveBackupJsonFile(level_name);
    }
}

void MainWindow::on_levelList_itemClicked(QListWidgetItem *item)
{
    // 存一个备份的文件
    if(lastLevelIndex != -1)
        saveBackupJsonFile();
    lastLevelIndex = ui->levelList->currentRow();

    setTreeViewExpandSlots(false);

    QString level_name;
    if(openJsonFile(item, level_name))
    {
        QString title = windowTitle();
        QString new_title = QString("当前关卡：%1").arg(level_name);
        if(title != new_title)
        {
            setWindowTitle(new_title);
            QString c_lvl_id = level_name.mid(m_levelPrefix.length());
            resetTreeViewSate(c_lvl_id);
        }
        else
            updateTreeViewState(false);
        resetUndoAndRedo(level_name);
    }

    setTreeViewExpandSlots(true);
}

void MainWindow::on_btnDeleteVar_clicked()
{
    int idx = ui->tableWidget->currentRow();
    if(idx < 0)
        return;

    ValueManager* vm = m_eventTreeModel->GetValueManager();
    QString var_name = vm->GetGlobalVarList().at(idx);

    if(vm->CheckVarIsUsedOrNot(var_name))
    {
        info("这个变量正在被使用，无法删除");
        return;
    }

    if( !vm->DeleteVariable(var_name) )
        info("删除失败！");
    else
    {
        saveBackupJsonFile();
        updateVarTable();
    }
}

void MainWindow::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    int idx = item->row();
    m_dlgManageVar->ModifyVar(idx);
    updateVarTable();
    saveBackupJsonFile();
}

void MainWindow::deleteFile(const QString &path)
{
    QFile fileTemp(path);
    fileTemp.remove();
}

void MainWindow::deleteAllBackupFiles()
{
    QString backup_path = this->config_path;
    backup_path += "backup";
    QDir dir(backup_path);
    if(!dir.exists())
        return;
    dir.setFilter(QDir::Files);
    QStringList file_list = dir.entryList();
    int file_num = file_list.size();
    backup_path += "/";
    for(int i = 0; i < file_num; i++)
    {
        QString file_path = backup_path + file_list[i];
        deleteFile(file_path);
    }
}

QString MainWindow::getLevelNameOnItem(QListWidgetItem *item)
{
    QString level_name = item->text();
    int pos = level_name.indexOf(" *");
    if(pos != -1)
    {
        level_name = level_name.left(level_name.size() - 2);
    }
    return level_name;
}

bool MainWindow::isNeedSave()
{
    foreach (bool already_saved, savedOrNot)
    {
        if(!already_saved)
        {
            return true;
        }
    }
    return false;
}

bool MainWindow::pushNewBackupFileName(const QString &lvl_name, const QString &bk_file_path)
{
    bool need_push = true;

    if(!backupFilePaths.contains(lvl_name))
        backupFilePaths.insert(lvl_name, QStringList());
    else if(!backupFilePaths[lvl_name].isEmpty())
    {
        // 路径相同，则内容已经被覆盖了，无须再存储路径
        if(backupFilePaths[lvl_name].last() == bk_file_path)
        {
            need_push = false;
        }
        // 备份文件数量有上限
        if(backupFilePaths[lvl_name].size() >= MAX_BACKUP_NUM)
        {
            deleteFile(backupFilePaths[lvl_name].first());
            backupFilePaths[lvl_name].removeFirst();
        }
    }

    // 保存备份文件路径
    if(need_push)
        backupFilePaths[lvl_name].push_back(bk_file_path);

    return need_push;
}

void MainWindow::on_levelList_customContextMenuRequested(const QPoint &pos)
{
    // 创建菜单
    QMenu *popMenu = new QMenu( this );

    // 创建动作
    QAction *copy_act = nullptr;
    QAction *del_act = nullptr;
    static QAction *create_act = nullptr;
    static QAction *openConfig_act = nullptr;
    static QAction *openLua_act = nullptr;
    static QAction *reload_act = nullptr;
    static QAction *save_act = nullptr;
    static QAction *lua_act = nullptr;
    if(create_act == nullptr)
    {
        create_act = new QAction("新建关卡", this);
        connect( create_act, SIGNAL(triggered()), this, SLOT(CreateNewLevel_EmptyLvl()) );
    }
    if(openConfig_act == nullptr)
    {
        openConfig_act = new QAction("打开编辑文件位置", this);
        connect( openConfig_act, SIGNAL(triggered()), this, SLOT(OpenConfigFolder()) );
    }
    if(openLua_act == nullptr)
    {
        openLua_act = new QAction("打开lua文件位置", this);
        connect( openLua_act, SIGNAL(triggered()), this, SLOT(OpenLuaFolder()) );
    }
    if(reload_act == nullptr)
    {
        reload_act = new QAction("重载所有关卡", this);
        connect( reload_act, SIGNAL(triggered()), this, SLOT(ReloadAllLevels()) );
    }
    if(save_act == nullptr)
    {
        save_act = new QAction("保存所有关卡", this);
        connect( save_act, SIGNAL(triggered()), this, SLOT(SaveAllLevels_Json()) );
    }
    if(lua_act == nullptr)
    {
        lua_act = new QAction("全部json转lua", this);
        connect( lua_act, SIGNAL(triggered()), this, SLOT(SaveAllLevels_Lua()) );
    }

    // 添加选项 新建关卡
    popMenu->addAction( create_act );

    // 添加选项 拷贝、删除
    QListWidgetItem* cur_item = ui->levelList->itemAt(pos);
    if( cur_item != nullptr )
    {
        QString level_name = getLevelNameOnItem(cur_item);
        copy_act = new QAction("拷贝" + level_name + "为新关卡", this);
        del_act = new QAction("删除关卡" + level_name, this);
        popMenu->addAction( copy_act );
        popMenu->addAction( del_act );
        connect( copy_act, SIGNAL(triggered()), this, SLOT(CreateNewLevel_CopyCurLvl()) );
        connect( del_act, SIGNAL(triggered()), this, SLOT(DeleteCurrentLevel()) );
    }

    // 添加选项 重载、保存、一键生成Lua
    popMenu->addSeparator();

    popMenu->addAction( openConfig_act );
    popMenu->addAction( openLua_act );
    popMenu->addAction( reload_act );
    popMenu->addAction( save_act );
    popMenu->addAction( lua_act );

    // 弹出菜单
    popMenu->exec( QCursor::pos() );

    // 清理内存
    delete popMenu;
    if(copy_act != nullptr)
    {
        disconnect( copy_act, SIGNAL(triggered()), this, SLOT(CreateNewLevel_CopyCurLvl()) );
        delete copy_act;
    }
    if(del_act != nullptr)
    {
        disconnect( del_act, SIGNAL(triggered()), this, SLOT(DeleteCurrentLevel()) );
        delete del_act;
    }
}

void MainWindow::OpenConfigFolder()
{
    QDesktopServices::openUrl(QUrl(config_path));
}

void MainWindow::OpenLuaFolder()
{
    QString file_path = QCoreApplication::applicationFilePath();
    file_path.replace("LevelEditor.exe", m_LuaPath);

    QDesktopServices::openUrl(QUrl(file_path));
}

void MainWindow::CreateNewLevel_CopyCurLvl()
{
    QListWidgetItem* item = ui->levelList->currentItem();
    if( item == nullptr )
        return;

    QString level_name = getLevelNameOnItem(item);
    m_dlgChoseEvtType->EditLevelName(level_name);

    if(m_dlgChoseEvtType->event_name != "")
    {
        if(!checkNewLevelName())
            return;

        QString path = this->config_path;
        QString path1 = path + level_name + ".json";
        QString path2 = path + m_dlgChoseEvtType->event_name + ".json";
        QFile file1(path1);
        QFile file2(path2);

        bool success = false;

        if(file1.open(QIODevice::ReadOnly))
        {
            if(file2.open(QIODevice::WriteOnly))
            {
                file2.resize(0);
                file2.write(file1.readAll());
                file2.close();
                saveBackupWhenInit(level_name);
            }
            else
            {
                info(m_dlgChoseEvtType->event_name + "是不合法的文件名");
            }
            file1.close();
            success = true;
        }
        else
        {
            info(level_name + ".json文件无法打开");
        }

        if(success)
        {
            int pos = m_levelList.size();
            m_levelList.push_back(m_dlgChoseEvtType->event_name);

            savedOrNot.insert(m_dlgChoseEvtType->event_name, true);

            ui->levelList->addItem(m_dlgChoseEvtType->event_name);
            ui->levelList->setCurrentRow(pos);
            on_levelList_itemClicked(ui->levelList->item(pos));
        }
    }
}

void MainWindow::CreateNewLevel_EmptyLvl()
{
    m_dlgChoseEvtType->EditLevelName(m_levelPrefix);

    if(m_dlgChoseEvtType->event_name != "")
    {
        if(!checkNewLevelName())
            return;

        QString path = this->config_path;
        path = path + m_dlgChoseEvtType->event_name + ".json";
        QFile file(path);
        if(file.open(QIODevice::WriteOnly))
        {
            file.resize(0);
            QString str = "{\n"
                          "    \"CustomAction\": [\n"
                          "    ],\n"
                          "    \"Event\": [\n"
                          "        {\n"
                          "            \"AND\": [\n"
                          "            ],\n"
                          "            \"SEQUENCE\": [\n"
                          "            ],\n"
                          "            \"event_name\": \"未命名事件\",\n"
                          "            \"event_type\": {\n"
                          "                \"name\": \"" + EventType::GetInstance()->GetEventLuaType(0) + "\"\n"
                          "            }\n"
                          "        }\n"
                          "    ],\n"
                          "    \"Var\": [\n"
                          "    ]\n"
                          "}\n";
            file.write(str.toStdString().c_str());
            file.close();
            saveBackupWhenInit(m_dlgChoseEvtType->event_name);

            savedOrNot.insert(m_dlgChoseEvtType->event_name, true);

            m_levelList.push_back(m_dlgChoseEvtType->event_name);
            ui->levelList->addItem(m_dlgChoseEvtType->event_name);
        }
        else
        {
            info("新关卡的json文件创建失败");
        }
    }
}

void MainWindow::DeleteCurrentLevel()
{
    QListWidgetItem* item = ui->levelList->currentItem();
    if( item == nullptr )
        return;
    QString level_name = getLevelNameOnItem(item);

    int ch = QMessageBox::warning(nullptr, "提示", "确定删除这个关卡？", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ch != QMessageBox::Yes)
        return;

    QString path = this->config_path;
    path = path + level_name + ".json";

    // 删除文件
    deleteFile(path);

    // 更新数据结构
    int pos = m_levelList.indexOf(level_name);
    m_levelList.removeAt(pos);
    savedOrNot.remove(level_name);

    // UI中删除
    ui->levelList->removeItemWidget(item); //这个删除不彻底，还要delete
    delete item; item = nullptr;

    // 清理相关的节点展开状态
    clearTreeViewState(level_name.mid(m_levelPrefix.length()));

    // 刷新UI选择的项
    if(lastLevelIndex == pos)
    {
        int new_pos = (pos == m_levelList.size()) ? pos - 1 : pos;
        lastLevelIndex = -1;
        ui->levelList->setCurrentRow(new_pos);
        on_levelList_itemClicked(ui->levelList->item(new_pos));
        return;
    }
    else if(lastLevelIndex > pos)
        lastLevelIndex --;
    else if(lastLevelIndex != -1)
        ui->levelList->setCurrentRow(lastLevelIndex);

    // todo 删除备份文件
}

void MainWindow::ReloadAllLevels()
{
    if(isNeedSave())
    {
        int ch = QMessageBox::warning(nullptr, "提示", "尚未保存，确定重载？", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ch != QMessageBox::Yes)
            return;
    }

    m_dlgChoseEvtType->EditLevelPrefix(m_levelPrefix);
    if(m_dlgChoseEvtType->event_name != "-1")
    {
        if(!checkLevelPrefix(m_dlgChoseEvtType->event_name))
            return;
        m_levelPrefix = m_dlgChoseEvtType->event_name;
        InitLevelTree();
    }
}

void MainWindow::SaveAllLevels_Json()
{
    m_dlgChoseEvtType->EditLevelPrefix(m_levelPrefix);
    if(m_dlgChoseEvtType->event_name != "-1")
    {
        if(!checkLevelPrefix(m_dlgChoseEvtType->event_name))
            return;
        QString old_prefix = m_levelPrefix;
        m_levelPrefix = m_dlgChoseEvtType->event_name;

        DlgWaiting* wait = new DlgWaiting(this);
        wait->show();

        // 遍历所有关卡文件，对每个关卡都加载Json，然后删除原Json，输出新Json
        int num = m_levelList.size();
        for(int i = 0; i < num; i++)
        {
            QString old_level_name;
            if(!openJsonFile(ui->levelList->item(i), old_level_name))
                continue;

            QString file_path = this->config_path;
            file_path = file_path + old_level_name + ".json";
            deleteFile(file_path);

            QString new_level_name = old_level_name;
            new_level_name.replace(old_prefix, m_levelPrefix);
            file_path.replace(old_level_name, new_level_name);

            QFile file_level(file_path);
            if(!file_level.open(QIODevice::ReadWrite))
            {
                continue;
            }
            else
            {
                file_level.resize(0);
                generateJsonDocument(&file_level);
                file_level.close();
            }

            // 防止假死
            QCoreApplication::processEvents();
        }

        // 重载levelList
        InitLevelTree();

        wait->close();
    }
}

void MainWindow::SaveAllLevels_Lua()
{
    QString lua_path = QCoreApplication::applicationFilePath();
    lua_path.replace("LevelEditor.exe", m_LuaPath);
    QDir dir(lua_path);
    if(!dir.exists())
    {
        lua_path = QCoreApplication::applicationFilePath();
        lua_path.replace("LevelEditor.exe", "");
        lua_path = QFileDialog::getExistingDirectory(this, "Lua Path", lua_path, QFileDialog::ShowDirsOnly);
        if(lua_path.isEmpty())
        {
            return;
        }
        if(lua_path.right(1) != "/")
            lua_path += "/";
    }

    DlgWaiting* wait = new DlgWaiting(this);
    wait->show();

    // 先存储当前关卡的备份
    lastLevelIndex = ui->levelList->currentRow();
    if(lastLevelIndex != -1)
        saveBackupJsonFile();
    int cur_level = lastLevelIndex;
    lastLevelIndex = -1;

    // 遍历所有关卡文件，对每个关卡都加载Json，然后输出Lua
    int num = m_levelList.size();
    for(int i = 0; i < num; i++)
    {
        QString level_name = getLevelNameOnItem(ui->levelList->item(i));
        if(!openJsonFile(config_path + level_name + ".json"))
            continue;

        QString file_path = lua_path + level_name + ".lua";
        QFile file(file_path);
        if(file.open(QIODevice::WriteOnly))
        {
            file.resize(0);
            generateLuaDocument(&file);
            file.close();
        }

        // 防止假死
        QCoreApplication::processEvents();
    }

    // 恢复原来选中的关卡
    if(cur_level != -1)
        on_levelList_itemClicked(ui->levelList->item(cur_level));

    wait->close();
}

void MainWindow::setModelForDlg(TreeItemModel* model)
{
    m_dlgConditionType->SetModelPointer(model);
    m_dlgSetVar->SetModel(model);
    m_dlgEditFunction->SetModel(model);
    m_dlgManageVar->SetModel(model);
    m_dlgChoseActionType->SetModel(model);
}

void MainWindow::updateTreeViewState(bool default_state)
{
    int prefix_num = m_levelPrefix.length();
    QString c_lvl_id = getLevelNameOnItem(ui->levelList->currentItem()).mid(prefix_num);

    QMap<QString, bool>::iterator itr;
    for(itr = m_itemState_Event.begin(); itr != m_itemState_Event.end(); ++itr)
    {
        int pos = itr.key().indexOf(':');
        if(pos == -1) continue;
        if(itr.key().left(pos) != c_lvl_id)
            continue;

        QModelIndex index = getModelIndexBy(itr.key().mid(pos + 1));
        if(index.isValid())
            ui->eventTreeView->setExpanded(index, itr.value());
        else
            itr.value() = default_state; //如果没有这个code对应的QModelIndex，说明这个节点被删了，那就重置成默认的状态
    }
    for(itr = m_itemState_Custom.begin(); itr != m_itemState_Custom.end(); ++itr)
    {
        int pos = itr.key().indexOf(':');
        if(pos == -1) continue;
        if(itr.key().left(pos) != c_lvl_id)
            continue;

        QModelIndex index = getModelIndexBy(itr.key());
        if(index.isValid())
            ui->customTreeView->setExpanded(index, itr.value());
        else
            itr.value() = default_state; //如果没有这个code对应的QModelIndex，说明这个节点被删了，那就重置成默认的状态
    }
}

void MainWindow::resetTreeViewSate(const QString &level_flag)
{
    // 检查标记：是否打开过
    if(m_itemState_Event.contains(level_flag) && m_itemState_Event[level_flag])
    {
        updateTreeViewState();
        return;
    }

    // 默认新打开时展开全部（updateTreeViewState的default_state默认也是true）
    setTreeViewExpandSlots(true);
    ui->eventTreeView->expandAll();
    ui->customTreeView->expandAll();
    setTreeViewExpandSlots(false); //不关的话会connect两次，触发两次槽函数

    // 设置标记
    if(m_itemState_Event.contains(level_flag))
        m_itemState_Event[level_flag] = true;
    else
        m_itemState_Event.insert(level_flag, true);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if(index == 0)
        setModelForDlg(m_eventTreeModel);
    else if(index == 1)
        setModelForDlg(m_customTreeModel);
}

void MainWindow::on_customTreeView_clicked(const QModelIndex &index)
{
    if (index.isValid())
        m_curNode = reinterpret_cast<NodeInfo*>(index.internalPointer());
    else
        m_curNode = nullptr;
}

void MainWindow::on_customTreeView_doubleClicked(const QModelIndex &index)
{
    on_eventTreeView_clicked(index);
    slotEditNode();
}

void MainWindow::on_actionUndo_triggered()
{
    QString level_name = getLevelNameOnItem(ui->levelList->currentItem());
    if(!backupFilePaths.contains(level_name) || backupFilePaths[level_name].size() <= 1)
    {
        ui->actionUndo->setEnabled(false);
        info("无备份，不能撤销");
        return;
    }

    // 把当前的备份文件路径移到重做里去
    if(!backupFilePaths_Redo.contains(level_name))
        backupFilePaths_Redo.insert(level_name, QStringList());
    backupFilePaths_Redo[level_name].push_back(backupFilePaths[level_name].last());
    backupFilePaths[level_name].removeLast();

    resetUndoAndRedo(level_name);

    // 重新打开关卡（节点展开状态只能刷新了）
    QString file_path = this->config_path;
    file_path = file_path + level_name + ".json";
    if(!backupFilePaths[level_name].isEmpty())
    {
        openJsonFile(backupFilePaths[level_name].last());
        changeSavedFlag(level_name, isSameFile(backupFilePaths[level_name].last(), file_path));
    }
//    resetTreeSate();
    updateTreeViewState();
}

void MainWindow::on_actionRedo_triggered()
{
    QString level_name = getLevelNameOnItem(ui->levelList->currentItem());
    if(!backupFilePaths_Redo.contains(level_name) || backupFilePaths_Redo[level_name].isEmpty())
    {
        info("不能执行重做");
        return;
    }

    // 重新打开关卡
    openJsonFile(backupFilePaths_Redo[level_name].last());
    QString file_path = this->config_path;
    file_path = file_path + level_name + ".json";
    changeSavedFlag(level_name, isSameFile(backupFilePaths_Redo[level_name].last(), file_path));
    // 刷新节点展开状态
//    resetTreeSate();
    updateTreeViewState();

    // 把备份文件路径移到撤销里去
    if(backupFilePaths.contains(level_name))
    {
        backupFilePaths[level_name].push_back(backupFilePaths_Redo[level_name].last());
        backupFilePaths_Redo[level_name].removeLast();

        resetUndoAndRedo(level_name);
    }
    else
        info("backupFilePaths不含" + level_name);
}

void MainWindow::on_actionShowLevels_triggered(bool checked)
{
    if(checked)
        ui->listWidget->show();
    else
        ui->listWidget->hide();
}

void MainWindow::on_actionShowVars_triggered(bool checked)
{
    if(checked)
        ui->propertiesWidget->show();
    else
        ui->propertiesWidget->hide();
}

void MainWindow::on_listWidget_visibilityChanged(bool visible)
{
    ui->actionShowLevels->setChecked(visible);
}

void MainWindow::on_propertiesWidget_visibilityChanged(bool visible)
{
    ui->actionShowVars->setChecked(visible);
}

void MainWindow::on_action_ExpandAllNodes_triggered()
{
    int i_type = ui->tabWidget->currentIndex();
    if(i_type == 0)
    {
        QModelIndex root_item = ui->eventTreeView->rootIndex();
        int n = m_eventTreeModel->m_pRootNode->childs.size();
        for(int i = 0; i < n; i++)
            expandAllNodes(ui->eventTreeView, m_eventTreeModel, m_eventTreeModel->index(i, 0, root_item));
    }
    else if(i_type == 1)
    {
        QModelIndex root_item = ui->customTreeView->rootIndex();
        int n = m_customTreeModel->m_pRootNode->childs.size();
        for(int i = 0; i < n; i++)
            expandAllNodes(ui->customTreeView, m_customTreeModel, m_customTreeModel->index(i, 0, root_item));
    }
}

void MainWindow::on_action_CollapseAllNodes_triggered()
{
    int i_type = ui->tabWidget->currentIndex();
    if(i_type == 0)
    {
        QModelIndex root_item = ui->eventTreeView->rootIndex();
        int n = m_eventTreeModel->m_pRootNode->childs.size();
        for(int i = 0; i < n; i++)
            collapseAllNodes(ui->eventTreeView, m_eventTreeModel, m_eventTreeModel->index(i, 0, root_item));
    }
    else if(i_type == 1)
    {
        QModelIndex root_item = ui->customTreeView->rootIndex();
        int n = m_customTreeModel->m_pRootNode->childs.size();
        for(int i = 0; i < n; i++)
            collapseAllNodes(ui->customTreeView, m_customTreeModel, m_customTreeModel->index(i, 0, root_item));
    }
}

void MainWindow::on_action_CollapseAllEvents_triggered()
{
    int i_type = ui->tabWidget->currentIndex();
    if(i_type == 0)
    {
        QModelIndex root_item = ui->eventTreeView->rootIndex();
        int n = m_eventTreeModel->m_pRootNode->childs.size();
        for(int i = 0; i < n; i++)
            ui->eventTreeView->collapse(m_eventTreeModel->index(i, 0, root_item));
    }
    else if(i_type == 1)
    {
        QModelIndex root_item = ui->customTreeView->rootIndex();
        int n = m_customTreeModel->m_pRootNode->childs.size();
        for(int i = 0; i < n; i++)
            ui->customTreeView->collapse(m_customTreeModel->index(i, 0, root_item));
    }
}

QString MainWindow::getItemCodeOf(int tree_type, const QModelIndex &index)
{
    int prefix_num = m_levelPrefix.length();
    QString s_num = getLevelNameOnItem(ui->levelList->currentItem()).mid(prefix_num);

    return QString("%1:%2%3").arg(s_num).arg(QString::number(tree_type)).arg(index.data(Qt::UserRole).toString());
}

QModelIndex MainWindow::getModelIndexBy(const QString &code)
{
    QStringList sl = code.split('.', QString::SkipEmptyParts);
    int n = sl.size();
    QModelIndex index;
    TreeItemModel* m = nullptr;
    if(sl[0] == "0")
        m = m_eventTreeModel;
    else if(sl[0] == "1")
        m = m_customTreeModel;
    else
    {
        info("非法的code: " + code);
        return index;
    }

    index = (m == m_eventTreeModel ? ui->eventTreeView->rootIndex() : ui->customTreeView->rootIndex());
    for(int i = 1; i < n; i++)
    {
        int row = sl[i].toInt();
        index = m->index(row, 0, index);
    }
    return index;
}

void MainWindow::clearTreeViewState(const QString &lvl_id)
{
    if(!m_itemState_Event.contains(lvl_id))
        return;

    QStringList to_remove_list;
    QMap<QString, bool>::iterator itr;
    for(itr = m_itemState_Event.begin(); itr != m_itemState_Event.end(); ++itr)
    {
        int pos = itr.key().indexOf(':');
        if(pos == -1) continue;
        if(itr.key().left(pos) == lvl_id)
            to_remove_list.append(itr.key());
    }
    int n = to_remove_list.size();
    for(int i = 0; i < n; i++)
        m_itemState_Event.remove(to_remove_list[i]);

    to_remove_list.clear();
    for(itr = m_itemState_Custom.begin(); itr != m_itemState_Custom.end(); ++itr)
    {
        int pos = itr.key().indexOf(':');
        if(pos == -1) continue;
        if(itr.key().left(pos) != lvl_id)
            to_remove_list.append(itr.key());
    }
    n = to_remove_list.size();
    for(int i = 0; i < n; i++)
        m_itemState_Custom.remove(to_remove_list[i]);

    m_itemState_Event.remove(lvl_id);
}

void MainWindow::on_action_ExpandAllEvents_triggered()
{
    int i_type = ui->tabWidget->currentIndex();
    if(i_type == 0)
    {
        QModelIndex root_item = ui->eventTreeView->rootIndex();
        int n = m_eventTreeModel->m_pRootNode->childs.size();
        for(int i = 0; i < n; i++)
        {
            QModelIndex event_item = m_eventTreeModel->index(i, 0, root_item);
            ui->eventTreeView->expand(event_item);
            ui->eventTreeView->expand(m_eventTreeModel->index(0, 0, event_item));
            expandAllNodes(ui->eventTreeView, m_eventTreeModel, m_eventTreeModel->index(1, 0, event_item));
            ui->eventTreeView->collapse(m_eventTreeModel->index(2, 0, event_item));
        }
    }
    else if(i_type == 1)
    {
        QModelIndex root_item = ui->customTreeView->rootIndex();
        int n = m_customTreeModel->m_pRootNode->childs.size();
        for(int i = 0; i < n; i++)
            ui->customTreeView->expand(m_customTreeModel->index(i, 0, root_item));
    }
}
