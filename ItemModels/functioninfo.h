#ifndef FUNCTIONINFO_H
#define FUNCTIONINFO_H

#include <QVector>

#define FUNCTION_POS int

class FunctionClass
{
private:
    friend class FunctionInfo;

    FUNCTION_POS id;

    QString name_lua;   // 函数名（lua）
    QString name_ui;    // 函数名（UI显示）
    QStringList texts;  // 拼接成描述的纯文本
    QStringList params; // 拼接成描述的参数类型
    QStringList values; // 返回值的类型
    bool can_be_call;   // 是否可被用于Call Function
    QString note;       // 注释，在UI中显示
public:
    bool param_is_before_text;

    inline void Clear()
    {
        id = -1;
        name_lua.clear();
        name_ui.clear();
        texts.clear();
        params.clear();
        values.clear();
        note.clear();
        param_is_before_text = false;
        can_be_call = false;
    }

    inline FUNCTION_POS GetID() { return id; }
    inline QString GetNameLua() { return name_lua; }
    inline QString GetNameUI() { return name_ui; }
    inline QString GetNote() { return note; }
    inline bool CanBeCall() { return can_be_call; }
    inline int GetTextNum() { return texts.size(); }
    inline int GetParamNum() {  return params.size(); }
    inline int GetReturnNum() { return values.size(); }

    inline QString GetTextAt(int i)
    {
        if(i >= 0 && i < texts.size())
            return texts[i];
        else
            return "";
    }

    inline QString GetParamTypeAt(int i)
    {
        if(i >= 0 && i < params.size())
            return params[i];
        else
            return "";
    }

    inline QString GetReturnTypeAt(int i) //目前还没有返回2个值的函数，所以使用的i都是0
    {
        if(i >= 0 && i < values.size())
            return values[i];
        else
            return "";
    }
};

class FunctionInfo
{
public:
    ~FunctionInfo();

    static FunctionInfo* GetInstance();

    int GetFunctionInfoCount();
    FunctionClass* GetFunctionInfoByLuaName(const QString& name);
    FunctionClass* GetFunctionInfoAt(int idx);
    int CheckFunctionNameLuaExist(const QString& name);

    QStringList GetTagList();
    bool CheckFunctionInTag(FunctionClass* func, const QString &tag);

private:
    FunctionInfo();

    void InitFuncInfo();
    void ClearData();

    void createFakeData();
    bool createDateByConfig(QString path);
    bool createDataByLuaFile(const QString& lua_path);
    bool parseLuaLine(FunctionClass* func, QString line);
    void parseTextsAndParams(FunctionClass* func, QString str);
    void parseTags(QString str, int func_id);

    QString config_path;
    QString parse_flag;

    static FunctionInfo* data;
    QVector<FunctionClass> infoList;
    QMap<QString, QVector<FUNCTION_POS>> tagMap;
};

#endif // FUNCTIONINFO_H
