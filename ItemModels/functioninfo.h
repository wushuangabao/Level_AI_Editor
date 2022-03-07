#ifndef FUNCTIONINFO_H
#define FUNCTIONINFO_H

#include <QVector>

#define FUNCTION_ID int

class FunctionClass
{
private:
    friend class FunctionInfo;

    FUNCTION_ID id;

    QString name;       // 函数名（lua）
    QStringList texts;  // 拼接成描述的纯文本
    QStringList params; // 拼接成描述的参数类型
    QStringList values; // 返回值的类型
    QString note;       // 注释，在UI中显示

public:
    bool param_is_before_text;

    inline void Clear()
    {
        id = 0;
        name.clear();
        texts.clear();
        params.clear();
        values.clear();
        note.clear();
        param_is_before_text = false;
    }

    inline FUNCTION_ID GetID() { return id; }
    inline QString GetName() { return name; }
    inline QString GetNote() { return note; }

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

    FunctionClass* GetFunctionInfoByID(FUNCTION_ID id);
    FunctionClass* GetFunctionInfoAt(int idx);
    QVector<FunctionClass> infoList;

private:
    FunctionInfo();

    void InitFuncInfo();
    void ClearData();

    void createFakeData();
    bool createDateByConfig(QString path);

    void parseTextsAndParams(FunctionClass* func, QString str);

    static FunctionInfo* data;
};

#endif // FUNCTIONINFO_H
