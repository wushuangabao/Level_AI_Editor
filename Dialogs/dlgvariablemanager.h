#ifndef DLGVARIABLEMANAGER_H
#define DLGVARIABLEMANAGER_H

#include <QDialog>

namespace Ui {
class DlgVariableManager;
}

class TreeItemModel;
class NodeInfo;

class DlgVariableManager : public QDialog
{
    Q_OBJECT

public:
    explicit DlgVariableManager(QWidget *parent = 0);
    ~DlgVariableManager();

    void SetModel(TreeItemModel *m);
    void CreateVar(NodeInfo* node);

private slots:
    void on_pushButton_clicked();

private:
    Ui::DlgVariableManager *ui;
    TreeItemModel* model;
    NodeInfo* node;
};

#endif // DLGVARIABLEMANAGER_H
