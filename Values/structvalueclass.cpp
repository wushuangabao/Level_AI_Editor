#include "structinfo.h"
#include "valueclass.h"

StructValueClass::StructValueClass()
{
    members.clear();
    var_type = "";
}

StructValueClass::StructValueClass(StructValueClass *obj)
{
    members.clear();
    if(obj != nullptr)
    {
        QMap<QString, CommonValueClass*>::iterator i;
        for(i = obj->members.begin(); i != obj->members.end(); ++i)
        {
            UpdateMember_Copy(i.key(), i.value());
        }
        var_type = obj->var_type;
    }
}

StructValueClass::~StructValueClass()
{
    clearMembers();
}

void StructValueClass::operator=(const StructValueClass &obj)
{
    clearMembers();
    QMap<QString, CommonValueClass*>::const_iterator i;
    for(i = obj.members.constBegin(); i != obj.members.constEnd(); ++i)
    {
        UpdateMember_Copy(i.key(), i.value());
    }
    var_type = obj.var_type;
}

QString StructValueClass::GetLuaValueString()
{
    if(members.size() == 0)
        return "nil";

    StructInfo* struct_info = StructInfo::GetInstance();
    QString text = "{";
    QMap<QString, CommonValueClass*>::iterator i;
    for(i = members.begin(); i != members.end(); ++i)
    {
        QString value_text = i.value()->GetLuaValueString();
        if(value_text != "nil" && value_text != "" && value_text != "{}")
        {
            if(text != "{")
                text += ", ";
            text = text + struct_info->GetKeyInLua(var_type, i.key()) + " = " + value_text;
        }
    }
    text += "}";

    if(text == "{}") text = "nil";
    return text;
}

QString StructValueClass::GetText()
{
    QString text = "{";
    QMap<QString, CommonValueClass*>::iterator i;
    for(i = members.begin(); i != members.end(); ++i)
    {
        QString value_text = i.value()->GetText();
        if(value_text != "nil" && value_text != "")
        {
            if(text != "{")
                text += ", ";
            text = text + i.key() + " = " + value_text;
        }
    }
    text += "}";
    return text;
}

void StructValueClass::SetVarType(QString name)
{
    if(var_type != name)
    {
        var_type = name;
        clearMembers();
        QStringList keys = StructInfo::GetInstance()->GetKeysOf(name);
        int n = keys.size();
        for(int j = 0; j < n; j++)
        {
            QString var_type = StructInfo::GetInstance()->GetValueTypeOf(name, keys[j]);
            if(StructInfo::GetInstance()->CheckIsStruct(var_type))
            {
                StructValueClass* v = new StructValueClass();
                v->InitWithType(var_type);
                members.insert(keys[j], v);
            }
            else
            {
                BaseValueClass* v = new BaseValueClass("nil");
                v->SetVarType(var_type);
                members.insert(keys[j], v);
            }
        }
    }
}

bool StructValueClass::UpdateVarNameAndType(int var_id, const QString &name, const QString &type)
{
    bool ok = true;
    QMap<QString, CommonValueClass*>::iterator i;
    for(i = members.begin(); i != members.end(); ++i)
    {
        if(i.value()->UpdateVarNameAndType(var_id, name, type) == false)
            ok = false;
    }
    return ok;
}

bool StructValueClass::IsUsingVar(const QString &vname)
{
    QMap<QString, CommonValueClass*>::iterator i;
    for(i = members.begin(); i != members.end(); ++i)
    {
        if(i.value()->IsUsingVar(vname))
            return true;
    }
    return false;
}

CommonValueClass *StructValueClass::GetValueByKey(QString name)
{
    if(members.contains(name))
    {
        return members[name];
    }
    else
    {
        return nullptr;
    }
}

void StructValueClass::UpdateMember_Copy(QString key_name, CommonValueClass *value)
{
    if(members.contains(key_name))
    {
        delete members[key_name];
        members.remove(key_name);
    }

    if(value->GetValueType() < VT_TABLE)
    {
        members.insert(key_name, new BaseValueClass(static_cast<BaseValueClass*>(value)));
    }
    else if(value->GetValueType() == VT_TABLE)
    {
        members.insert(key_name, new StructValueClass(static_cast<StructValueClass*>(value)));
    }
}

void StructValueClass::UpdateMember_Move(QString key_name, CommonValueClass *value)
{
    if(members.contains(key_name))
    {
        delete members[key_name];
    }
    members[key_name] = value;
}

QStringList StructValueClass::GetAllKeys()
{
    return StructInfo::GetInstance()->GetKeysOf(var_type);
}

bool StructValueClass::InitWithType(QString var_type)
{
    clearMembers();
    this->var_type = var_type;
    StructInfo* struct_info = StructInfo::GetInstance();
    if(struct_info->CheckIsStruct(var_type))
    {
        QStringList keys = struct_info->GetKeysOf(var_type);
        QStringList vts = struct_info->GetValueTypesOf(var_type);
        int n = keys.size();
        if(n != vts.size())
            return false;
        for(int i = 0; i < n; i++)
        {
            if(struct_info->CheckIsStruct(vts[i]))
            {
                StructValueClass* v = new StructValueClass();
                v->SetVarType(vts[i]);
                UpdateMember_Move(keys[i], v);
            }
            else
            {
                BaseValueClass* v = new BaseValueClass();
                v->SetVarType(vts[i]);
                UpdateMember_Move(keys[i], v);
            }
        }
    }
    return false;
}

void StructValueClass::clearMembers()
{
    QMap<QString, CommonValueClass*>::iterator i;
    for(i = members.begin(); i != members.end(); ++i)
    {
        if(i.value() != nullptr)
        {
            delete i.value();
            i.value() = nullptr;
        }
    }
    members.clear();
}

