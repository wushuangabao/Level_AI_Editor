#include <QCheckBox>
#include <QEvent>
#include "multiselectcombobox.h"

MultiSelectComboBox::MultiSelectComboBox(QWidget *parent)
    : QComboBox(parent)
{
    hidden_flag = true;
    show_flag = false;
    m_listWidget = new QListWidget();
    m_lineEdit = new QLineEdit();
    m_searchBar = new QLineEdit();

    //设置搜索框
    QListWidgetItem* currentItem = new QListWidgetItem(m_listWidget);
    //设置搜索框提示信息
    m_searchBar->setPlaceholderText("Search...");
    //显示清除按钮
    m_searchBar->setClearButtonEnabled(true);
    m_listWidget->addItem(currentItem);
    m_listWidget->setItemWidget(currentItem, m_searchBar);

    //设为只读，因为该输入框只用来显示选中的选项，称为文本框更合适些
    m_lineEdit->setReadOnly(true);
    //把当前对象安装(或注册)为事件过滤器，当前也称为过滤器对象。事件过滤器通常在构造函数中进行注册。
    m_lineEdit->installEventFilter(this);
    //设置禁用样式，因为不受样式表控制，临时这样解决
    m_lineEdit->setStyleSheet("QLineEdit:disabled{background:rgb(233,233,233);}");

    this->setModel(m_listWidget->model());
    this->setView(m_listWidget);
    this->setLineEdit(m_lineEdit);
    connect(m_searchBar, SIGNAL(textChanged(const QString&)), this, SLOT(onSearch(const QString&)));
    connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &MultiSelectComboBox::itemClicked);
}

MultiSelectComboBox::~MultiSelectComboBox()
{
}

void MultiSelectComboBox::hidePopup()
{
    show_flag = false;
    int width = this->width();
    int height = this->height();
    int x = QCursor::pos().x() - mapToGlobal(geometry().topLeft()).x() + geometry().x();
    int y = QCursor::pos().y() - mapToGlobal(geometry().topLeft()).y() + geometry().y();
    if (x >= 0 && x <= width && y >= this->height() && y <= height + this->height())
    {
    }
    else
    {
        QComboBox::hidePopup();
    }
}

void MultiSelectComboBox::addItem(const QString& _text, const QVariant& _variant /*= QVariant()*/)
{
    Q_UNUSED(_variant);
    QListWidgetItem* item = new QListWidgetItem(m_listWidget);
    QCheckBox* checkbox = new QCheckBox(this);
    checkbox->setText(_text);
    m_listWidget->addItem(item);
    m_listWidget->setItemWidget(item, checkbox);
    connect(checkbox, &QCheckBox::stateChanged, this, &MultiSelectComboBox::stateChange);
}

void MultiSelectComboBox::addItems(const QStringList& _text_list)
{
    for (const auto& text_one : _text_list)
    {
        addItem(text_one);
    }
}

QStringList MultiSelectComboBox::currentText()
{
    QStringList text_list;
    if (!m_lineEdit->text().isEmpty())
    {
        text_list = m_lineEdit->text().split(',');
    }
    return text_list;
}

int MultiSelectComboBox::count() const
{
    int count = m_listWidget->count() - 1;
    if (count < 0)
    {
        count = 0;
    }
    return count;
}

void MultiSelectComboBox::SetSearchBarPlaceHolderText(const QString _text)
{
    m_searchBar->setPlaceholderText(_text);
}

void MultiSelectComboBox::SetPlaceHolderText(const QString& _text)
{
    m_lineEdit->setPlaceholderText(_text);
}

void MultiSelectComboBox::ResetSelection()
{
    int count = m_listWidget->count();
    for (int i = 1; i < count; i++)
    {
        //获取对应位置的QWidget对象
        QWidget *widget = m_listWidget->itemWidget(m_listWidget->item(i));
        //将QWidget对象转换成对应的类型
        QCheckBox *check_box = static_cast<QCheckBox*>(widget);
        check_box->setChecked(false);
    }
}

void MultiSelectComboBox::clear()
{
    m_lineEdit->clear();
    m_listWidget->clear();
    QListWidgetItem* currentItem = new QListWidgetItem(m_listWidget);
    m_searchBar->setPlaceholderText("Search...");
    m_searchBar->setClearButtonEnabled(true);
    m_listWidget->addItem(currentItem);
    m_listWidget->setItemWidget(currentItem, m_searchBar);
    SetSearchBarHidden(hidden_flag);
    connect(m_searchBar, SIGNAL(textChanged(const QString&)), this, SLOT(onSearch(const QString&)));
}

void MultiSelectComboBox::TextClear()
{
    m_lineEdit->clear();
    ResetSelection();
}

void MultiSelectComboBox::setCurrentText(const QString& _text)
{
    int count = m_listWidget->count();
    for (int i = 1; i < count; i++)
    {
        //获取对应位置的QWidget对象
        QWidget *widget = m_listWidget->itemWidget(m_listWidget->item(i));
        //将QWidget对象转换成对应的类型
        QCheckBox *check_box = static_cast<QCheckBox*>(widget);
        if (_text.compare(check_box->text()))
            check_box->setChecked(true);
    }
}

void MultiSelectComboBox::setCurrentText(const QStringList& _text_list)
{
    int count = m_listWidget->count();
    for (int i = 1; i < count; i++)
    {
        //获取对应位置的QWidget对象
        QWidget *widget = m_listWidget->itemWidget(m_listWidget->item(i));
        //将QWidget对象转换成对应的类型
        QCheckBox *check_box = static_cast<QCheckBox*>(widget);
        if (_text_list.contains(check_box->text()))
            check_box->setChecked(true);
    }
}

void MultiSelectComboBox::SetSearchBarHidden(bool _flag)
{
    hidden_flag = _flag;
    m_listWidget->item(0)->setHidden(hidden_flag);
}

bool MultiSelectComboBox::eventFilter(QObject *watched, QEvent *event)
{
    //设置点击输入框也可以弹出下拉框
    if (watched == m_lineEdit && event->type() == QEvent::MouseButtonRelease && this->isEnabled())
    {
        showPopup();
        return true;
    }
    return false;
}

void MultiSelectComboBox::wheelEvent(QWheelEvent *event)
{
    //禁用QComboBox默认的滚轮事件
    Q_UNUSED(event);
}

void MultiSelectComboBox::keyPressEvent(QKeyEvent *event)
{
    QComboBox::keyPressEvent(event);
}

void MultiSelectComboBox::stateChange(int _row)
{
    Q_UNUSED(_row);
    QString selected_data("");
    int count = m_listWidget->count();
    for (int i = 1; i < count; i++)
    {
        QWidget *widget = m_listWidget->itemWidget(m_listWidget->item(i));
        QCheckBox *check_box = static_cast<QCheckBox*>(widget);
        if (check_box->isChecked())
        {
            selected_data.append(check_box->text()).append(",");
        }
    }
    selected_data.chop(1);
    if (!selected_data.isEmpty())
    {
        m_lineEdit->setText(selected_data);
    }
    else
    {
        m_lineEdit->setText("All");
    }
    m_lineEdit->setToolTip(selected_data);
    emit selectionChange(selected_data);
}

void MultiSelectComboBox::onSearch(const QString& _text)
{
    for (int i = 1; i < m_listWidget->count(); i++)
    {
        QCheckBox *check_box = static_cast<QCheckBox*>(m_listWidget->itemWidget(m_listWidget->item(i)));
        //文本匹配则显示，反之隐藏
        //Qt::CaseInsensitive模糊查询
        if (check_box->text().contains(_text, Qt::CaseInsensitive))
            m_listWidget->item(i)->setHidden(false);
        else
            m_listWidget->item(i)->setHidden(true);
    }
}

void MultiSelectComboBox::itemClicked(int _index)
{
    if (_index != 0)
    {
        QCheckBox *check_box = static_cast<QCheckBox*>(m_listWidget->itemWidget(m_listWidget->item(_index)));
        check_box->setChecked(!check_box->isChecked());
    }
}
