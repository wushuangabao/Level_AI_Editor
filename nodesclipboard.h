#ifndef NODESCLIPBOARD_H
#define NODESCLIPBOARD_H

#include "Values/valuemanager.h"
#include "ItemModels/nodeinfo.h"

class TreeItemModel;
class MainWindow;

class NodesClipBoard
{
public:
    static NodesClipBoard* GetInstance();
    ~NodesClipBoard();

    NODE_TYPE GetTypeOfPasteNode();
    
    void ClearNodes(int tree_type);
    void SetTreeItemModel(TreeItemModel* et, TreeItemModel* ct);

    bool AddCopyNode(NodeInfo* node);
    bool HasCopyNode(NodeInfo* node);
    bool PasteToNode(NodeInfo* node, int tree_type);

private:
    NodesClipBoard();
    void ClearNodes();
    MainWindow* getMainWindow();

    QStringList pasteChildrenNodesTo(NodeInfo* root_node, int pos = -1);
    NodeInfo *pasteChildNodeTo(NodeInfo* root_node, NodeInfo *node, int pos = -1);
    void pasteValuesOnNode(NodeInfo* old_node, NodeInfo *new_node); //将m_valueManager中和node及其子节点有关的value全部粘贴到当前vm

    bool copyValueOnNode(NodeInfo* node, NodeInfo *&new_node, bool copy_node = false); //copy_node为真的话，既拷贝value又拷贝node
    void copyNode(NodeInfo *&cur_node, NodeInfo* node_be_copy);
    CommonValueClass *tryCopyValue(CommonValueClass *v); //new一个Value
    bool checkValueBeforeCopy(CommonValueClass *v);
    int tryAddNewVar(const QString& var_name);

    NODE_TYPE getTypeOfPasteNode(NodeInfo* node);
    bool findNodeNameInTree(const QString &name);
    
    int m_treeType;
    TreeItemModel* m_eventTreeModel;
    TreeItemModel* m_customTreeModel;
    QList<NodeInfo*> m_nodes;
    ValueManager* m_valueManager;
    QMap<NodeInfo*, int> m_nodeSetVarId;
};

#endif // NODESCLIPBOARD_H
