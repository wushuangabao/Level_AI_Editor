#ifndef TREEITEMMODEL_CUSTOM_H
#define TREEITEMMODEL_CUSTOM_H

#include "treeitemmodel.h"

class TreeItemModel_Custom : public TreeItemModel
{
    Q_OBJECT

public:
    explicit TreeItemModel_Custom(QObject *parent = nullptr);

    virtual QStringList* GetEventParamsUIOf(NodeInfo* node) override;
    virtual QStringList* GetEventParamsLuaOf(NodeInfo* node) override;

    void AddCustomSequence(const QString& name);
    void UpdateCustActSeqName(NodeInfo* node, const QString& name);
};

#endif // TREEITEMMODEL_CUSTOM_H
