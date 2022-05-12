#ifndef MULTILEVELCOMBOBOX_H
#define MULTILEVELCOMBOBOX_H

#include <QComboBox>
#include <QLineEdit>
#include <QMenu>
#include <QListWidget>
#include <QAction>

class ActionSelectVar: public QAction
{
    Q_OBJECT
public:
    explicit ActionSelectVar(const QString &text, QObject *parent = nullptr);
    ~ActionSelectVar();
public slots:
    void onClicked();
signals:
    void onSelected(QString name);

};

class MultiLevelComboBox : public QComboBox
{
    Q_OBJECT

public:
    MultiLevelComboBox(QWidget *parent = nullptr);
    ~MultiLevelComboBox();

    //设置变量类型过滤
    void SetVarType(const QString& var_type);
    //设置内容
    void SetLineText(const QString& str);
    //隐藏下拉框
    virtual void showPopup();
    virtual void hidePopup();
    //添加一条选项
    void addItem(const QString& _text, const QVariant& _variant = QVariant());
    //添加多条选项
    void addItems(const QStringList& _text_list);
    //返回当前内容
    QString currentText();
    //返回当前选项条数
    int count()const;
    //清空所有内容
    void clear();

protected:
    //事件过滤器
    virtual bool eventFilter(QObject *watched,QEvent *event);
    //滚轮事件
    virtual void wheelEvent(QWheelEvent *event);
    //按键事件
    virtual void keyPressEvent(QKeyEvent *event);
public slots:
    //隐藏菜单
    void onMenuHide();
private slots:
    //槽函数：点击下拉框选项
    void itemClicked(int _index);
    //选择菜单中的某项
    void onChoseVar(QString name);
signals:
    //信号：发送当前选中选项
    void selectionChange(const QString _data);

private:
    //设置菜单
    bool SetStructMenu(QMenu* menu, const QString &parent_name, const QStringList& members);

    QListWidget* m_listWidget; //下拉框
    QLineEdit* m_lineEdit; //文本框
    bool show_flag; //下拉框显示标志
    QList<QMenu*> m_menuList; //子菜单的集合
    QString m_filter_type;
};

#endif // MULTILEVELCOMBOBOX_H
