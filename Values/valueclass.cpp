#include <QDebug>
#include "../ItemModels/functioninfo.h"
#include "valueclass.h"

BaseValueClass::BaseValueClass()
{
    ClearData();
    qDebug() << "Create value 1 at" << (uintptr_t)this << endl;
}

BaseValueClass::BaseValueClass(BaseValueClass *obj)
{
    name = obj->name;
    lua_str = obj->lua_str;
    value_type = obj->value_type;
    func = obj->func;

    int n = obj->params.size();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            params.append(new BaseValueClass(obj->params[i]));
        }
    }

    qDebug() << "Create value copy(" << (uintptr_t)obj << ") at" << (uintptr_t)this << endl;
}

BaseValueClass::BaseValueClass(QString name, VALUE_TYPE vtype)
{
    ClearData();
    this->name = name;
    value_type = vtype;

    qDebug() << "Create value named \'" << name <<"\' at" << (uintptr_t)this << endl;
}

BaseValueClass::~BaseValueClass()
{
    qDebug() << "Delete value at" << (uintptr_t)this << endl;
    clearFuncParams();
}

void BaseValueClass::ClearData()
{
    value_type = VT_VAR;
    func = nullptr;
    name = "未定义值";
    lua_str = "";
    clearFuncParams();
}

void BaseValueClass::operator=(const BaseValueClass &obj)
{
    name = obj.name;
    lua_str = obj.lua_str;
    value_type = obj.value_type;
    func = obj.func;

    clearFuncParams();
    int n = obj.params.size();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            params.append(new BaseValueClass(obj.params[i]));
        }
    }
}

QString BaseValueClass::GetText()
{
    if(value_type == VT_VAR)
        return name;
    else if(value_type == VT_STR)
        return lua_str;
    else if(value_type == VT_FUNC)
        return getFunctionText();
    else
        return "ERROR";
}

VALUE_TYPE BaseValueClass::GetValueType()
{
    return value_type;
}

void BaseValueClass::SetVarName(const QString &text)
{
    value_type = VT_VAR;
    name = text;
}

void BaseValueClass::SetLuaStr(const QString &text)
{
    value_type = VT_STR;
    lua_str = text;
}

void BaseValueClass::SetFunction(FunctionClass *function_class)
{
    value_type = VT_FUNC;
    func = function_class;

    clearFuncParams();
    int n = func->GetParamNum();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            params.append(new BaseValueClass("未定义值", VT_VAR));
        }
    }
}

void BaseValueClass::SetParamAt(int idx, BaseValueClass *v)
{
    Q_ASSERT(params.size() > idx);

    qDebug() << "set value(" << (uintptr_t)this << ")'s param(" << (uintptr_t)(params[idx]) <<") = " << (uintptr_t)v << endl;

    *(params[idx]) = *v;
}

void BaseValueClass::clearFuncParams()
{
    int n = params.size();
    if(n <= 0)
        return;

    qDeleteAll(params);
    params.clear();
}

QString BaseValueClass::getFunctionText()
{
    Q_ASSERT(func != nullptr);

    int func_param_num = func->GetParamNum();
    int actually_p_num = params.size();
    Q_ASSERT(func_param_num <= actually_p_num);

    QString text = "";
    int pos = 0;
    int text_num = func->GetTextNum();

    if(func->param_is_before_text)
    {
        Q_ASSERT(func_param_num > 0);
        text = "(" + params[0]->GetText() + ")";
        pos++;
    }

    for(int i = 0; i < text_num; i++)
    {
        text = text + func->GetTextAt(i);
        if(pos < func_param_num)
        {
            text = text + "(" + params[pos]->GetText() + ")";
            pos++;
        }
    }

    return text;
}
