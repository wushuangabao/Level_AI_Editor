#include <QDebug>
#include "../ItemModels/enumdefine.h"
#include "../ItemModels/nodeinfo.h"
#include "../ItemModels/functioninfo.h"
#include "../Values/enuminfo.h"
#include "../Values/structinfo.h"
#include "../Values/valuemanager.h"
#include "valueclass.h"

BaseValueClass::BaseValueClass()
{
    params = QVector<CommonValueClass*>();
    ClearData();
//    qDebug() << "Create value 1 at" << (uintptr_t)this << endl;
}

BaseValueClass::BaseValueClass(BaseValueClass *obj)
{
    name = obj->name;
    lua_str = obj->lua_str;
    value_type = obj->value_type;
    func = obj->func;
    var_type = obj->var_type;
    g_var_id = obj->g_var_id;

    int n = obj->params.size();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            CommonValueClass* v;
            if(obj->params[i]->GetValueType() < VT_TABLE)
                v = new BaseValueClass(static_cast<BaseValueClass*>(obj->params[i]));
            else if(obj->params[i]->GetValueType() == VT_TABLE)
                v = new StructValueClass(static_cast<StructValueClass*>(obj->params[i]));
            params.append(v);
        }
    }

//    qDebug() << "Create value copy(" << (uintptr_t)obj << ") at" << (uintptr_t)this << endl;
}

BaseValueClass::BaseValueClass(QString str)
{
    params = QVector<CommonValueClass*>();
    ClearData();

    if(str != "")
    {
        this->lua_str = str;
        value_type = VT_STR;
    }

//    qDebug() << "Create value named \'" << name <<"\' at" << (uintptr_t)this << endl;
}

BaseValueClass::~BaseValueClass()
{
//    qDebug() << "Delete value at" << (uintptr_t)this << endl;
    if(this != nullptr)
        clearFuncParams();
}

void BaseValueClass::ClearData()
{
    value_type = VT_STR;
    func = nullptr;
    name = "未定义值";
    lua_str = "nil";
    var_type = "";
    g_var_id = -1;
    clearFuncParams();
}

void BaseValueClass::operator=(const BaseValueClass &obj)
{
    if(this == &obj)
        return;

    name = obj.name;
    lua_str = obj.lua_str;
    value_type = obj.value_type;
    func = obj.func;
    var_type = obj.var_type;
    g_var_id = obj.g_var_id;

    clearFuncParams();
    int n = obj.params.size();
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            CommonValueClass* v;
            if(obj.params[i]->GetValueType() < VT_TABLE)
                v = new BaseValueClass(static_cast<BaseValueClass*>(obj.params[i]));
            else if(obj.params[i]->GetValueType() == VT_TABLE)
                v = new StructValueClass(static_cast<StructValueClass*>(obj.params[i]));
            params.append(v);
        }
    }
}

QString BaseValueClass::GetText()
{
    if(value_type == VT_PARAM)
        return name;
    else if(value_type == VT_VAR)
        return lua_str == "" ? name : QString("%1.%2").arg(name).arg(lua_str);
    else if(value_type == VT_ENUM)
        return lua_str;
    else if(value_type == VT_FUNC)
        return getFunctionText();
    else if(value_type == VT_STR)
    {
        if(lua_str == "nil")
        {
            return lua_str;
        }
        else if(var_type == "number")
        {
            bool ok = true;
            lua_str.toDouble(&ok);
            if(!ok)
            {
                info(lua_str + " 不是number类型的值");
                return "0";
            }
        }
        else if(var_type == "string")
        {
            int pos = lua_str.indexOf('\"');
            if(pos == -1)
                lua_str = QString("\"%1\"").arg(lua_str);
            else
            {
                int last_pos = lua_str.lastIndexOf('\"');
                if(last_pos == pos && !lua_str.contains('\''))
                    lua_str = QString("\'%1\'").arg(lua_str);
                else if(pos > 0 || last_pos < lua_str.size() - 1)
                {
                    // 检查一下""之外的头尾是否有空格，有的话去掉。如果头尾有非空字符，最外层套上'
                    bool empty = true;
                    for(int i = 0; i < pos; i++)
                        if(lua_str[i] != ' ')
                        {
                            empty = false;
                            break;
                        }
                    for(int i = last_pos + 1; i < lua_str.size(); i++)
                        if(lua_str[i] != ' ')
                        {
                            empty = false;
                            break;
                        }
                    if(empty)
                        lua_str = lua_str.mid(pos, last_pos - pos + 1);
                    else if(!empty && !lua_str.contains('\''))
                        lua_str = QString("\'%1\'").arg(lua_str);
                }
            }
        }
        return lua_str;
    }
    else
        return "ERROR";

}

void BaseValueClass::SetVarType(const QString &t)
{
    var_type = t;
}

void BaseValueClass::SetVarName(const QString &text, QString type, int idx, const QString &key)
{
    value_type = VT_VAR;
    name = text;
    lua_str = key;
    var_type = type;
    if(idx != -1)
        g_var_id = idx;
    else
        info("未设置变量id！");
}

void BaseValueClass::ModifyVarName(const QString &name)
{
    this->name = name;
}

void BaseValueClass::SetLuaStr(const QString &text, const QString &type_text)
{
    value_type = VT_STR;
    lua_str = text;
    var_type = type_text;

    // string类型的加上引号
    if(var_type == "string")
    {
        lua_str.remove('"');
        lua_str.push_front('"');
        lua_str.push_back('"');
        return;
    }

    // 检测一下lua_str是否合法
    if(var_type != "")
    {
        checkLuaStrAndVarType(lua_str, type_text);
    }
}

void BaseValueClass::SetEnumValue(const QString &value_str)
{
    value_type = VT_ENUM;
    lua_str = value_str;

    // 检查一下变量类型是否和枚举值匹配
    bool ok = true;
    if(EnumInfo::GetInstance()->GetAllTypes().contains(var_type) == false)
    {
        info("变量类型错误！不存在" + var_type + "这个枚举类型");
        ok = false;
    }
    else if(EnumInfo::GetInstance()->GetEnumsOfType(var_type).indexOf(lua_str) == -1)
    {
        info("变量类型错误！" + var_type + "不存在" + lua_str + "这个值");
        ok = false;
    }
    // 强行设置 var_type (todo)
    if(!ok)
    {

    }
}

void BaseValueClass::SetEvtParam(const QString &lua_str, const QString &ui_str, QString var_type)
{
    value_type = VT_PARAM;
    name = ui_str;
    this->lua_str = lua_str;
    this->var_type = var_type;
}

void BaseValueClass::SetFunction(FunctionClass *function_class)
{
    value_type = VT_FUNC;
    func = function_class;
    var_type = func->GetReturnTypeAt(0);

    clearFuncParams();
    int n = func->GetParamNum();
    for(int i = 0; i < n; i++)
    {
        if(StructInfo::GetInstance()->CheckIsStruct(func->GetParamTypeAt(i)))
        {
            StructValueClass* struct_v = new StructValueClass();
            struct_v->SetVarType(func->GetParamTypeAt(i));
            params.append(struct_v);
        }
        else
            params.append(new BaseValueClass("nil"));
    }
}

void BaseValueClass::SetParamAt(int idx, CommonValueClass *v)
{
    MY_ASSERT(params.size() > idx);
    if(v == nullptr)
    {
        info("SetParamAt(" + QString::number(idx) +")参数v为空");
        return;
    }

    if(params[idx] != nullptr)
        delete params[idx];

    if(v->GetValueType() < VT_TABLE)
    {
        BaseValueClass* bv = static_cast<BaseValueClass*>(v);
        params[idx] = new BaseValueClass(bv);
    }
    else if(v->GetValueType() == VT_TABLE)
    {
        StructValueClass* sv = static_cast<StructValueClass*>(v);
        params[idx] = new StructValueClass(sv);
    }
}

QString BaseValueClass::GetFunctionName()
{
    MY_ASSERT(func != nullptr);

    return func->GetNameLua();
}

int BaseValueClass::GetFunctionParamsNum()
{
    MY_ASSERT(func != nullptr);

    return func->GetParamNum();
}

CommonValueClass *BaseValueClass::GetFunctionParamAt(int id)
{
    int n = params.size();
    MY_ASSERT(n == func->GetParamNum());
    MY_ASSERT(id >= 0);
    MY_ASSERT(n > id);

    return params[id];
}

FunctionClass *BaseValueClass::GetFunctionInfo()
{
    return func;
}

QString BaseValueClass::GetEventParamInLua()
{
    if(value_type != VT_PARAM)
        return "";
    return lua_str;
}

QString BaseValueClass::GetLuaValueString(QString var_prefix)
{
    switch (value_type) {
    case VT_STR:
    {
        QString lua_str = GetText();
        if(lua_str.contains("自定义动作："))
        {
            lua_str.replace("自定义动作：", "");
            int n = NodeInfo::GetRootNode_Custom()->childs.size();
            for(int i = 0; i < n; i++)
            {
                if(NodeInfo::GetRootNode_Custom()->childs[i]->text == lua_str)
                {
                    lua_str = "CustomAction_" + QString::number(i) + "(level)";
                    break;
                }
            }
        }
        return lua_str;
    }
        break;
    case VT_VAR:
    {
        int id = ValueManager::GetValueManager()->GetIdOfVariable((CommonValueClass*)this);
        if(id == -1)
        {
            info("Lua值错误：找不到" + GetText() + "所使用的变量");
            return "未知变量";
        }
        else
        {
            MY_ASSERT(ValueManager::GetValueManager()->GetVarNameAt(id) == name);
            return QString("%1%2").arg(var_prefix).arg(GetText());
        }
    }
        break;
    case VT_FUNC:
    {
        QString str = GetFunctionName() + QString("(%1flowController").arg(var_prefix);
        int n = GetFunctionParamsNum();
        for(int i = 0; i < n; i++)
        {
            str += ", ";
            CommonValueClass* p = GetFunctionParamAt(i);
            str = str + p->GetLuaValueString(var_prefix); // todo 容错处理
            if(p->GetVarType() != GetFunctionInfo()->GetParamTypeAt(i) && p->GetValueType() != VT_STR)
                info("Lua提示：函数" + GetFunctionName() + "的第" + QString::number(i) + "个参数" + p->GetText() + "的数据类型不正确");
        }
        str += ")";
        return str;
    }
        break;
    case VT_PARAM:
        if(GetEventParamInLua() != "")
            return QString("event.%1").arg(GetEventParamInLua());
        else
        {
            info("Lua值错误：找不到" + GetText() + "所使用的事件参数");
            return QString("\"" + GetText() + "\"");
        }
        break;
    case VT_ENUM:
        return EnumInfo::GetInstance()->GetLuaStr(GetVarType(), GetText());
        break;
    default:
        return "未知的值";
        break;
    }
}

bool BaseValueClass::UpdateVarNameAndType(int var_id, const QString &name, const QString &type)
{
    if(value_type == VT_VAR)
    {
        if(g_var_id == var_id)
        {
            this->name = name;
            if(var_type != "" && var_type != type)
                return false;
        }
    }
    else if(value_type == VT_FUNC && params.size() > 0)
    {
        int n = params.size();
        bool ok = true;
        for(int i = 0; i < n; i++)
        {
            if(!params[i]->UpdateVarNameAndType(var_id, name, type))
                ok = false;
        }
        if(!ok)
            info("函数" + GetText() + "的参数类型可能有错误");
    }
    else if(value_type == VT_STR)
    {
        if(lua_str.contains(name))
        {
            info("注意修改自定义值：" + lua_str);
        }
        if(var_type != "" && var_type != type)
            return false;
    }
    return true;
}

bool BaseValueClass::IsUsingVar(const QString &vname)
{
    switch (value_type) {
    case VT_VAR:
        if(name == vname)
        {
            return true;
        }
        break;
    case VT_STR:
        if(lua_str.contains(vname))
            info("注意检查：" + lua_str);
        break;
    case VT_FUNC:
        for(int i = 0; i < GetFunctionParamsNum(); i++)
        {
            if(GetFunctionParamAt(i)->IsUsingVar(vname))
            {
                return true;
            }
        }
        break;
    default:
        break;
    }

    return false;
}

void BaseValueClass::clearFuncParams()
{
    int n = params.size();
    if(n <= 0)
        return;

    for(int i = 0; i < n; i++)
    {
        delete params[i];
        params[i] = nullptr;
    }
    params.clear();
}

QString BaseValueClass::getFunctionText()
{
    MY_ASSERT(func != nullptr);

    int func_param_num = func->GetParamNum();
    int actually_p_num = params.size();
    MY_ASSERT(func_param_num <= actually_p_num);

    QString text = "";
    int pos = 0;
    int text_num = func->GetTextNum();

    if(func->param_is_before_text)
    {
        MY_ASSERT(func_param_num > 0);
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

// 检查lua_str是否合法（类型是否符合var_type）
// 这个方法目前还不完善
bool BaseValueClass::checkLuaStrAndVarType(QString &lua_str, const QString &var_type)
{
    QString text = lua_str;
    text.remove(' '); //去掉所有空格
    if(text == "nil")
        return true;

    // 检测lua_str是否调用了function（没有检查参数类型）
    int pos = text.indexOf('(');
    if(pos != -1)
    {
        QString str_func = lua_str.left(pos);
        pos = FunctionInfo::GetInstance()->CheckFunctionNameLuaExist(str_func);
        if(pos != -1)
        {
            FunctionClass* func = FunctionInfo::GetInstance()->GetFunctionInfoAt(pos);
            if(func->GetReturnTypeAt(0) != var_type)
            {
                info("自定义值中的函数" + str_func + "返回值不是" + var_type);
                lua_str = "nil";
                return false;
            }
            else
                return true;
        }
        else if(str_func.contains("math.") && var_type == "number")
        {
            return true;
        }
    }

    // 检查table类型
    if(StructInfo::GetInstance()->CheckIsStruct(var_type))
    {
        if(0 != text.indexOf('{') || text.size() - 1 != text.lastIndexOf('}'))
        {
            info("自定义值" + text + "的类型不正确");
            lua_str = "{}";
            return false;
        }
        // 检查每个key是否合法
        QString str_table = text.mid(1, str_table.size() - 2);
        int pos_dou = str_table.indexOf(',');
        while(pos_dou != -1)
        {
            // 拆出左半部分
            int pos_left_sb = str_table.indexOf('(');
            if(pos_left_sb != -1 && pos_left_sb < pos_dou)
            {
                int pos_right_sb = str_table.indexOf(')', pos_left_sb);
                if(pos_right_sb == -1)
                {
                    info("自定义值" + str_table + "的括号不匹配");
                    lua_str = "{}";
                    return false;
                }
                pos_dou = pos_right_sb;
                if(str_table.size() > pos_dou + 1 && str_table[pos_dou + 1] == ',')
                    pos_dou++;
            }
            QString str_temp = str_table.left(pos_dou);
            // 找出等号
            int pos_deng = str_temp.indexOf('=');
            if(pos_deng == -1)
            {
                info("自定义值" + str_temp + "的写法不正确");
                lua_str = "{}";
                return false;
            }
            // 找出属性名
            QString v_type = StructInfo::GetInstance()->GetValueTypeOfKeyLua(var_type, str_temp.left(pos_deng));
            if(v_type == "")
            {
                info("自定义值" + str_temp + "中的属性不正确");
                lua_str = "{}";
                return false;
            }
            // 检查属性值的类型
            QString str_temp_value = str_temp.mid(pos_deng + 1);
            if(str_temp_value != "nil")
            {
                if(StructInfo::GetInstance()->CheckIsStruct(v_type))
                {
                    int pos_left_backet = str_temp.indexOf('{', pos_deng);
                    if(pos_left_backet != -1)
                    {
                        int pos_right_backet = str_table.indexOf('}', pos_left_backet);
                        if(pos_right_backet == -1)
                        {
                            info("自定义值" + str_table + "的写法不正确");
                            lua_str = "{}";
                            return false;
                        }
                        QString str_temp_table = str_table.mid(pos_left_backet, pos_right_backet - pos_left_backet + 1);
                        if(checkLuaStrAndVarType(str_temp_table, v_type) == false)
                        {
                            lua_str = "{}";
                            return false;
                        }
                        // 将pos_dou的位置移到'}'的后面
                        pos_dou = pos_right_backet;
                        if(str_table.size() > pos_dou + 1 && str_table[pos_dou + 1] == ',')
                            pos_dou++;
                    }
                }
                else if(checkLuaStrAndVarType(str_temp_value, v_type) == false)
                {
                    lua_str = "{}";
                    return false;
                }
            }
            // 进入下一次迭代
            str_table = str_table.mid(pos_dou + 1);
            pos_dou = str_table.indexOf(',');
        }
    }
    // 检查enum类型
    else if(EnumInfo::GetInstance()->CheckVarTypeIsEnum(var_type))
    {
        if(!EnumInfo::GetInstance()->CheckValueIsEnumOfType(lua_str, var_type))
        {
            info("请检查自定义值" + lua_str + "是否为" + var_type + "类型");
            return false;
        }
    }
    // 检查number类型
    /*
    else if(var_type == "number")
    {
        QRegExp rx("(\+|-|\\*|/)");
        QStringList str_list = text.split(rx);
        int list_num = str_list.size();
        for(int i = 0; i < list_num; i++)
        {
            QString str_num = str_list[i];
            if(str_num.size() == 0)
            {
                info("自定义值" + text + "的写法有误");
                lua_str = "nil";
                return false;
            }
            if(str_num[0] == '(' || str_num[0] == ')')
                str_num.remove(0, 1);
            int pos_last = str_num.size() - 1;
            if(str_num[pos_last] == '(' || str_num[pos_last] == ')')
                str_num.remove(pos_last, 1);
            bool ok;
            str_num.toFloat(&ok);
//            if(!ok && str_num.contains('('))
//                ok = checkLuaStrAndVarType(str_num, var_type); //2+(1)这样的输入会无限循环
            if(!ok)
            {
                info("自定义值中的" + str_num + "不是数字");
                lua_str = "nil";
                return false;
            }
        }
    }
    */
    return true;
}


bool CommonValueClass::AreSameVarType(CommonValueClass *v1, CommonValueClass *v2)
{
    if((v1->GetValueType() == VT_STR || v2->GetValueType() == VT_STR))
        return true; // todo: 判断lua_str值的类型
    return (v1->GetVarType()) == (v2->GetVarType());
}
