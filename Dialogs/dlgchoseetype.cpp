#include "../ItemModels/eventtype.h"
#include "multiselectcombobox.h"
#include "dlgchoseetype.h"
#include "ui_dlgchoseetype.h"

DlgChoseEType::DlgChoseEType(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgChoseEType)
{
    ui->setupUi(this);

    ui->multiComboBox->addItems(EventType::GetInstance()->GetTagList());
    ui->multiComboBox->stateChange(0);

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
    ui->multiComboBox->setVisible(true);
    ui->multiComboBox->ResetSelection();

    ui->lineEdit->setVisible(true);
    ui->lineEdit->setText("");
    event_name = "";

    ui->label_name->setText("事件名称：");
    ui->label_name->setVisible(true);

    ui->comboBox->clear();
    resetETypeComboBox();
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
    ui->multiComboBox->setVisible(false);

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
    ui->multiComboBox->setVisible(false);

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
    ui->multiComboBox->setVisible(false);

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
    ui->multiComboBox->setVisible(false);

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
    ui->multiComboBox->setVisible(false);

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
    ui->multiComboBox->setVisible(false);

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
    ui->multiComboBox->setVisible(false);

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
    ui->multiComboBox->setVisible(true);
    ui->multiComboBox->ResetSelection();

    ui->lineEdit->setVisible(false);
    ui->label_name->setVisible(false);

    ui->comboBox->clear();
    resetETypeComboBox();
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
    if(ui->multiComboBox->isVisible() == false)
    {
        if(text == "---" || text == "")
            index = -1;
        else
            index = ui->comboBox->currentIndex();
    }
    else
    {
        index = EventType::GetInstance()->GetIndexOfName(text);
    }
}

void DlgChoseEType::on_multiComboBox_editTextChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    resetETypeComboBox();
}

void DlgChoseEType::resetETypeComboBox()
{
    EventType* events = EventType::GetInstance();
    QStringList etypes = events->GetEventTypeList();
    QStringList items;
    QStringList tags = ui->multiComboBox->currentText();

    int n = etypes.size();
    for(int i = 0; i < n; i++)
    {
        bool ok = false;

        // 过滤标签
        int tags_n = tags.size();
        if(!(tags_n == 1 && tags[0] == "All"))
        {
            for(int j = 0; j < tags_n; j++)
            {
                if(events->CheckEventInTag(events->GetEventLuaType(i), tags[j]))
                {
                    ok = true;
                    break;
                }
            }
        }
        else
            ok = true;

        if(ok)
        {
            items.push_back(events->GetEventNameAt(i));
        }
    }

    disconnect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_currentIndexChanged(int)));
    ui->comboBox->clear();
    ui->comboBox->addItems(items);
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_currentIndexChanged(int)));

    ui->comboBox->setCurrentIndex(0);
    on_comboBox_currentIndexChanged(0);
}
