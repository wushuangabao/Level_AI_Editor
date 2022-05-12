#include <QLabel>
#include <QEvent>
#include "../Values/structinfo.h"
#include "../Values/valuemanager.h"
#include "multilevelcombobox.h"

MultiLevelComboBox::MultiLevelComboBox(QWidget *parent)
    : QComboBox(parent)
{
    show_flag = false;
    m_listWidget = new QListWidget();
    m_lineEdit = new QLineEdit();
    m_filter_type = "";

    //设为只读，因为该输入框只用来显示选中的选项，称为文本框更合适些
    m_lineEdit->setReadOnly(true);
    //把当前对象注册为事件过滤器，当前也称为过滤器对象。事件过滤器通常在构造函数中进行注册。
    m_lineEdit->installEventFilter(this);
    //设置禁用样式，因为不受样式表控制，临时这样解决
    m_lineEdit->setStyleSheet("QLineEdit:disabled{background:rgb(233,233,233);}");

    this->setModel(m_listWidget->model());
    this->setView(m_listWidget);
    this->setLineEdit(m_lineEdit);
    connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &MultiLevelComboBox::itemClicked);
}

MultiLevelComboBox::~MultiLevelComboBox()
{

}

void MultiLevelComboBox::SetVarType(const QString &var_type)
{
    m_filter_type = var_type;
}

void MultiLevelComboBox::SetLineText(const QString &str)
{
    m_lineEdit->setText(str);
}

void MultiLevelComboBox::showPopup()
{
    QComboBox::showPopup();
}

void MultiLevelComboBox::hidePopup()
{
    QPoint p0 = QCursor::pos();
    QPoint p1 = mapToGlobal(m_listWidget->geometry().topLeft());
    QPoint p2 = mapToGlobal(m_listWidget->geometry().bottomRight());
    p1.rx() += geometry().height();
    p2.ry() += geometry().height();
    bool in_list_widget = p0.x() >= p1.x() && p0.x() <= p2.x() && p0.y() >= p1.y() && p0.y() <= p2.y();

    if(in_list_widget)
        return;

    if(!show_flag)
        QComboBox::hidePopup();
}

void MultiLevelComboBox::addItem(const QString &_text, const QVariant &_variant)
{
    Q_UNUSED(_variant);
    StructInfo* info = StructInfo::GetInstance();
    QString var_type = ValueManager::GetValueManager()->GetVarTypeOf_Table(_text);

    // 初始化m_menuList
    if(info->CheckIsStruct(var_type))
    {
        QMenu* var_menu = new QMenu(this);
        bool add_action = false;
        if(m_filter_type == "" || var_type == m_filter_type)
        {
            ActionSelectVar* act = new ActionSelectVar(_text);
            connect( act, SIGNAL(triggered()), act, SLOT(onClicked()) );
            connect( act, SIGNAL(onSelected(QString)), this, SLOT(onChoseVar(QString)) );
            var_menu->addAction(act);
            add_action = true;
        }
        QMenu* menu = new QMenu(_text);
        QStringList members = info->GetKeysOf(var_type);
        if(SetStructMenu(menu, _text, members))
        {
            var_menu->addMenu(menu);
            add_action = true;
        }
        if(!add_action)
        {
            delete menu;
            delete var_menu;
            return;
        }
        connect(var_menu, SIGNAL(aboutToHide()), this, SLOT(onMenuHide()));
        m_menuList.append(var_menu);
    }
    else
    {
        if(m_filter_type != "" && var_type != m_filter_type)
            return;
        else
            m_menuList.append(nullptr);
    }

    QLabel* label = new QLabel(this);
    label->setText(_text);
    QListWidgetItem* item = new QListWidgetItem(m_listWidget);
    m_listWidget->addItem(item);
    m_listWidget->setItemWidget(item, label);
}

void MultiLevelComboBox::addItems(const QStringList &_text_list)
{
    for(const auto& text_one : _text_list)
        addItem(text_one);
}

QString MultiLevelComboBox::currentText()
{
    return m_lineEdit->text();
}

int MultiLevelComboBox::count() const
{
    return 1;
}

void MultiLevelComboBox::clear()
{
    m_lineEdit->clear();
    m_listWidget->clear();

    int n = m_menuList.size();
    for(int i = 0; i < n; i++)
    {
        if(m_menuList[i] != nullptr)
        {
            delete m_menuList[i];
            m_menuList[i] = nullptr;
        }
    }
    m_menuList.clear();
}

bool MultiLevelComboBox::eventFilter(QObject *watched, QEvent *event)
{
    //设置点击输入框也可以弹出下拉框
    if (watched == m_lineEdit && event->type() == QEvent::MouseButtonRelease && this->isEnabled())
    {
        showPopup();
        return true;
    }
    else if(watched == m_listWidget && event->type() == QEvent::MouseButtonRelease && this->isEnabled())
    {
        return false;
    }
    return false;
}

void MultiLevelComboBox::wheelEvent(QWheelEvent *event)
{
    // 禁用QComboBox默认的滚轮事件
    Q_UNUSED(event);
}

void MultiLevelComboBox::keyPressEvent(QKeyEvent *event)
{
    QComboBox::keyPressEvent(event);
}

void MultiLevelComboBox::onMenuHide()
{
    show_flag = false;
}

void MultiLevelComboBox::onChoseVar(QString var_name)
{
    emit selectionChange(var_name);
    m_lineEdit->setText(var_name);
}

void MultiLevelComboBox::itemClicked(int _index)
{
    QLabel *label = static_cast<QLabel*>(m_listWidget->itemWidget(m_listWidget->item(_index)));
    QMenu* var_menu = m_menuList[_index];
    if(var_menu != nullptr)
    {
        QPoint label_pos = label->mapToGlobal(QPoint(label->width(), 0));
        var_menu->move(label_pos);
        show_flag = true; //让下拉框不消失
        var_menu->show();
    }
    else
    {
        onChoseVar(label->text());
    }
}

bool MultiLevelComboBox::SetStructMenu(QMenu *menu, const QString &parent_name, const QStringList &members)
{
    bool return_bool = false;
    StructInfo* info = StructInfo::GetInstance();
    int n = members.size();
    for(int i = 0; i < n; i++)
    {
        QString my_name = QString("%1.%2").arg(parent_name).arg(members[i]);

        if(m_filter_type == "" ||
           m_filter_type == info->GetValueTypeOf(ValueManager::GetValueManager()->GetVarTypeOf_Table(parent_name), members[i]))
        {
            ActionSelectVar* act = new ActionSelectVar(my_name);
            connect( act, SIGNAL(triggered()), act, SLOT(onClicked()) );
            connect( act, SIGNAL(onSelected(QString)), this, SLOT(onChoseVar(QString)) );
            menu->addAction(act);
            return_bool = true;
        }

//        int pos = parent_name.lastIndexOf('.');
//        QString var_type = info->GetValueTypeOf(parent_name.mid(pos + 1), members[i]);
//        if(info->CheckIsStruct(var_type))
//        {
//            QMenu* son_menu = new QMenu(my_name);
//            SetStructMenu(son_menu, my_name, info->GetKeysOf(var_type));
//            menu->addMenu(son_menu);
//        }
    }
    return return_bool;
}

ActionSelectVar::ActionSelectVar(const QString &text, QObject *parent)
    : QAction(text, parent)
{

}

ActionSelectVar::~ActionSelectVar()
{

}

void ActionSelectVar::onClicked()
{
    emit onSelected(this->text());
}
