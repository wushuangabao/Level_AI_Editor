#include "dlgwaiting.h"
#include "ui_dlgwaiting.h"

DlgWaiting::DlgWaiting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgWaiting)
{
    ui->setupUi(this);

    // 设置透明度
    this->setWindowOpacity(0.8);
    // 取消对话框标题和边框
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    // 设置为模态对话框
    setWindowModality(Qt::WindowModal);

//    m_Move = new QMovie(":/fileMove/wait.gif");
//    ui.label->setMovie(m_Move);
//    ui.label->setScaledContents(true);
//    m_Move->start();

    //原文链接：https://blog.csdn.net/qq_24282081/article/details/95782291
}

DlgWaiting::~DlgWaiting()
{
    delete ui;
}
