#include "../ItemModels/treeitemmodel.h"
#include "../Values/valuemanager.h"
#include "dlgvariablemanager.h"
#include "ui_dlgvariablemanager.h"

DlgVariableManager::DlgVariableManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgVariableManager)
{
    model = nullptr;
    node = nullptr;
    ui->setupUi(this);
}

DlgVariableManager::~DlgVariableManager()
{
    delete ui;
}

void DlgVariableManager::SetModel(TreeItemModel *m)
{
    if(model == m)
        return;
    if(model != nullptr)
        delete model;
    model = m;
}

void DlgVariableManager::CreateVar(NodeInfo *node)
{
    Q_ASSERT(model != nullptr);
    Q_ASSERT(node != nullptr);

    this->node = node;

    exec();
}

void DlgVariableManager::on_pushButton_clicked()
{
    QString var_name = ui->lineEdit->text();
    if(var_name == "")
        info("变量名不能为空！");

    Q_ASSERT(model != nullptr);
    Q_ASSERT(node != nullptr);
    ValueManager* vm = model->GetValueManagerOf(node);

    Q_ASSERT(vm != nullptr);
    vm->AddNewVariable(var_name);

    this->hide();
}
