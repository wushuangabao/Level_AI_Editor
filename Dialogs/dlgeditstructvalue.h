#ifndef DIALOGEDITSTRUCTVALUE_H
#define DIALOGEDITSTRUCTVALUE_H

#include <QDialog>
#include "../Values/valueclass.h"

namespace Ui {
class DlgEditStructValue;
}

class QTableWidgetItem;
class TreeItemModel;
class NodeInfo;

class DlgEditStructValue : public QDialog
{
    Q_OBJECT

public:
    explicit DlgEditStructValue(QWidget *parent = 0);
    ~DlgEditStructValue();

    void EditStructValue(StructValueClass* v);
    StructValueClass* GetValuePointer();
    inline bool CheckIsAccepted() {return is_accepted;}
    inline void SetModelAndNode(TreeItemModel* m, NodeInfo* n) {model = m; node = n;}
    inline void SetUpForInitValue(bool ok) {is_init_value = ok;}

private slots:
    void on_DlgEditStructValue_accepted();
    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);

private:
    void updateTable();

    Ui::DlgEditStructValue *ui;
    TreeItemModel* model;
    NodeInfo* node;
    StructValueClass* m_value;
    bool is_accepted;
    bool is_init_value;
};

#endif // DIALOGEDITSTRUCTVALUE_H
