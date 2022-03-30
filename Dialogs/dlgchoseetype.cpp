#include "dlgchoseetype.h"
#include "ui_dlgchoseetype.h"

DlgChoseEType::DlgChoseEType(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgChoseEType)
{
    ui->setupUi(this);

    // 禁用右上角关闭按钮
//    setWindowFlag(Qt::WindowCloseButtonHint, false);
}

DlgChoseEType::~DlgChoseEType()
{
    delete ui;
}

void DlgChoseEType::CreateNewEvent()
{
    setWindowTitle("新建事件");

    ui->label->setText("事件类型：");
    ui->label->setVisible(true);

    ui->lineEdit->setVisible(true);
    ui->lineEdit->setText("");
    event_name = "";

    ui->label_name->setText("事件名称：");
    ui->label_name->setVisible(true);

    ui->comboBox->clear();
    ui->comboBox->addItems(EventType::GetInstance()->GetEventTypeList());
    ui->comboBox->setVisible(true);
    ui->comboBox->setEnabled(true);

    index = 0;
    text = "";
    ui->comboBox->setCurrentIndex(0);

    exec();
}

void DlgChoseEType::EditEventName(QString name)
{
    setWindowTitle("编辑事件名称");

    ui->label->setVisible(false);

    ui->lineEdit->setVisible(true);
    ui->lineEdit->setText(name);
    event_name = name;

    ui->label_name->setText("事件名称：");
    ui->label_name->setVisible(true);

    ui->comboBox->setVisible(false);

    index = 0;
    text = "";

    exec();
}

void DlgChoseEType::CreateNewCustomSeq()
{
    setWindowTitle("新建动作序列");

    ui->label->setVisible(false);

    ui->lineEdit->setVisible(true);
    ui->lineEdit->setText("");
    event_name = "";

    ui->label_name->setText("动作名称：");
    ui->label_name->setVisible(true);

    ui->comboBox->setVisible(false);

    index = 0;
    text = "";

    exec();
}

void DlgChoseEType::EditCustomSeqName(QString name)
{
    setWindowTitle("编辑动作名称");

    ui->label->setVisible(false);

    ui->lineEdit->setVisible(true);
    ui->lineEdit->setText(name);
    event_name = name;

    ui->label_name->setText("动作名称：");
    ui->label_name->setVisible(true);

    ui->comboBox->setVisible(false);

    index = 0;
    text = "";

    exec();
}

void DlgChoseEType::EditLevelName(const QString& name)
{
    setWindowTitle("编辑关卡名称");

    ui->label->setVisible(false);

    ui->lineEdit->setVisible(true);
    ui->lineEdit->setText(name);
    event_name = name;

    ui->label_name->setText("关卡名称：");
    ui->label_name->setVisible(true);

    ui->comboBox->setVisible(false);

    index = 0;
    text = "";

    exec();
}

void DlgChoseEType::EditLevelPrefix(const QString &name)
{
    setWindowTitle("关卡文件的前缀");

    ui->label->setVisible(false);

    ui->lineEdit->setVisible(true);
    ui->lineEdit->setText(name);
    event_name = name;

    ui->label_name->setText("关卡前缀：");
    ui->label_name->setVisible(true);

    ui->comboBox->setVisible(false);

    index = 0;
    text = "";

    exec();
}

int DlgChoseEType::ChoseCustActSeqNameIn(QStringList names)
{
    setWindowTitle("选择自定义动作");

    ui->label->setText("动作名称：");
    ui->label->setVisible(true);

    ui->lineEdit->setVisible(false);
    ui->label_name->setVisible(false);

    ui->comboBox->clear();
    ui->comboBox->addItems(names);
    ui->comboBox->setVisible(true);
    if(names.size() == 0)
        ui->comboBox->setEnabled(false);
    else
        ui->comboBox->setEnabled(true);

    index = 0;
    text = "";
    ui->comboBox->setCurrentIndex(0);

    exec();
    return index;
}

int DlgChoseEType::ChoseEventNameIn(QStringList enames)
{
    setWindowTitle("选择事件");

    ui->label->setText("事件名称：");
    ui->label->setVisible(true);

    ui->lineEdit->setVisible(false);
    ui->label_name->setVisible(false);

    ui->comboBox->clear();
    ui->comboBox->addItems(enames);
    ui->comboBox->setVisible(true);
    if(enames.size() == 0)
        ui->comboBox->setEnabled(false);
    else
        ui->comboBox->setEnabled(true);

    index = 0;
    text = "";
    ui->comboBox->setCurrentIndex(0);

    exec();
    return index;
}

void DlgChoseEType::EditEventType()
{
    setWindowTitle("编辑事件类型");

    ui->label->setText("事件类型：");
    ui->label->setVisible(true);

    ui->lineEdit->setVisible(false);
    ui->label_name->setVisible(false);

    ui->comboBox->clear();
    ui->comboBox->addItems(EventType::GetInstance()->GetEventTypeList());
    ui->comboBox->setCurrentIndex(0);
    ui->comboBox->setVisible(true);

    index = 0;
    text = "";

    exec();
}

void DlgChoseEType::on_lineEdit_textChanged(const QString &arg1)
{
    event_name = arg1;
}

void DlgChoseEType::on_DlgChoseEType_accepted()
{
}

void DlgChoseEType::on_DlgChoseEType_rejected()
{
    index = -1;
    if(ui->label_name->text() == "关卡名称：")
        event_name = "";
    if(ui->label_name->text() == "关卡前缀：")
        event_name = "-1";
}

void DlgChoseEType::on_comboBox_currentIndexChanged(int id)
{
    Q_UNUSED(id);
    text = ui->comboBox->currentText();
    if(text == "---" || text == "")
        index = -1;
    else
        index = ui->comboBox->currentIndex();
}
