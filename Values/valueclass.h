#ifndef VALUECLASS_H
#define VALUECLASS_H

#include <QString>
#include <QVector>

enum VALUE_TYPE
{
    VT_VAR, //用一个变量名来表示值，变量具有类型（VAR_TYPE），可以被赋为另一个相同VAR_TYPE的值
    VT_FUNC,//用一个函数（FunctionClass指针）来表示值
    VT_STR  //用自定义的lua代码来表示值
};

class FunctionClass;

class BaseValueClass
{
public:
    BaseValueClass();
    BaseValueClass(BaseValueClass* obj); //拷贝构造
    BaseValueClass(QString name, VALUE_TYPE vtype);
    ~BaseValueClass();

    void ClearData();
    void operator=(const BaseValueClass& obj);

    QString GetText();
    VALUE_TYPE GetValueType();

    void SetVarName(const QString& name);
    void SetLuaStr(const QString& lua_str);

    void SetFunction(FunctionClass* function_class);
    void SetParamAt(int idx, BaseValueClass* v);

private:
    QString name;        //变量名
    FunctionClass* func; //函数：函数名、参数信息
    QString lua_str;     //自定义lua代码

    QVector<BaseValueClass*> params; //函数参数
    void clearFuncParams();
    QString getFunctionText();

    VALUE_TYPE value_type;
};

#endif // VALUECLASS_H
