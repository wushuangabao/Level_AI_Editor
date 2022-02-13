#include "dlgchoseetype.h"
#include "ui_dlgchoseetype.h"

DlgChoseEType::DlgChoseEType(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgChoseEType)
{
    ui->setupUi(this);

    action_map.insert("设置变量", SET_VAR);
    action_map.insert("call..", FUNCTION);
    action_map.insert("序列（顺序执行）", SEQUENCE);
    action_map.insert("选择（if）", CHOICE);
    action_map.insert("循环（重复执行）", LOOP);
    action_map.insert("跳出（序列或循环）", END);

    QMapIterator<QString, NODE_TYPE> i(action_map);
    while (i.hasNext()) {
        i.next();
        action_list.append(i.key());
    }

    // 禁用右上角关闭按钮
    setWindowFlag(Qt::WindowCloseButtonHint, false);
}

DlgChoseEType::~DlgChoseEType()
{
    delete ui;
}

void DlgChoseEType::ShowWithEventType(QString name)
{
    setWindowTitle("编辑事件类型");

    ui->label->setText("事件类型：");

    ui->lineEdit->setVisible(true);
    ui->lineEdit->setText(name);
    event_name = name;
    ui->label_name->setVisible(true);

    ui->comboBox->clear();
    ui->comboBox->addItems(EventType::GetInstance()->eventNameVector);
    ui->comboBox->setCurrentIndex(0);

    index = 0;
    text = "";

    exec();
}

void DlgChoseEType::ShowWithActionType()
{
    setWindowTitle("新建动作");

    ui->label->setText("节点类型：");

    ui->lineEdit->setVisible(false);
    ui->label_name->setVisible(false);

    ui->comboBox->clear();
    ui->comboBox->addItems(action_list);
    ui->comboBox->setCurrentIndex(0);

    index = -1;
    text = "";

    exec();
}

void DlgChoseEType::on_buttonBox_accepted()
{
    text = ui->comboBox->currentText();

    if(action_map.contains(text))
        index = action_map[text];
    else if(text == "---")
        index = -1;
    else
        index = ui->comboBox->currentIndex();
}

void DlgChoseEType::on_buttonBox_rejected()
{
    if(action_map.contains(ui->comboBox->currentText()))
    {
        index = INVALID;
    }
    index = -1;
}

void DlgChoseEType::on_lineEdit_textChanged(const QString &arg1)
{
    event_name = arg1;
}
