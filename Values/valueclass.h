#ifndef VALUECLASS_H
#define VALUECLASS_H

#include <QString>
#include <QVector>

enum VALUE_TYPE
{
    VT_VAR,  //用一个变量名来表示值，变量具有类型（VAR_TYPE），可以被赋为另一个相同VAR_TYPE的值
    VT_FUNC, //用一个函数（FunctionClass指针）来表示值
    VT_STR,  //用自定义的lua代码来表示值
    VT_ENUM, //在一系列预设的值中选择其中一个
    VT_PARAM //这个值是事件触发时传入的参数
};

class FunctionClass;

class BaseValueClass
{
public:
    BaseValueClass();
    BaseValueClass(BaseValueClass* obj); //拷贝构造
    BaseValueClass(QString str);
    ~BaseValueClass();

    void ClearData();
    void operator=(const BaseValueClass& obj);

    QString GetText();
    VALUE_TYPE GetValueType(); //值类型，指值的存储方式，是变量名、函数、lua代码还是其他
    QString GetVarType(); //变量类型，指值在逻辑层面表示哪一类东西，比如数值、点、棋子……

    static bool AreSameVarType(BaseValueClass* v1, BaseValueClass* v2);
    void SetVarType(const QString& t);

    void SetVarName(const QString& name, QString var_type, int idx);
    void SetLuaStr(const QString& lua_str);
    void SetEnumValue(const QString& value_str);
    void SetEvtParam(const QString& lua_str, const QString& ui_str, QString var_type);

    void SetFunction(FunctionClass* function_class);
    void SetParamAt(int idx, BaseValueClass* v);

    QString GetFunctionName();
    int GetFunctionParamsNum();
    BaseValueClass* GetFunctionParamAt(int id);
    FunctionClass* GetFunctionInfo();

    QString GetEventParamInLua();

    bool UpdateVarNameAndType(int var_id, const QString& name, const QString &type);
    bool IsUsingVar(const QString& vname);

private:
    QString name;        //变量名
    FunctionClass* func; //函数：函数名、参数信息
    QString lua_str;     //自定义lua代码
    QString var_type;    //变量类型

    QVector<BaseValueClass*> params; //函数参数
    void clearFuncParams();
    QString getFunctionText();

    VALUE_TYPE value_type;
    int g_var_id;
};

#endif // VALUECLASS_H
