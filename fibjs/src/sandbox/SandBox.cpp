/*
 * SandBox.cpp
 *
 *  Created on: Oct 22, 2012
 *      Author: lion
 */

#include "object.h"
#include "SandBox.h"
#include "ifs/vm.h"
#include "ifs/util.h"
#include "ifs/test.h"

namespace fibjs
{

DECLARE_MODULE(vm);

result_t SandBox_base::_new(v8::Local<v8::Object> mods, exlib::string name,
                            obj_ptr<SandBox_base> &retVal, v8::Local<v8::Object> This)
{
    obj_ptr<SandBox> sbox = new SandBox(name);
    sbox->wrap(This);

    result_t hr = sbox->add(mods);
    if (hr < 0)
        return hr;

    retVal = sbox;

    return 0;
}

result_t SandBox_base::_new(v8::Local<v8::Object> mods,
                            v8::Local<v8::Function> require,
                            exlib::string name,
                            obj_ptr<SandBox_base> &retVal,
                            v8::Local<v8::Object> This)
{
    obj_ptr<SandBox> sbox = new SandBox(name);
    sbox->wrap(This);

    sbox->initRequire(require);
    result_t hr = sbox->add(mods);
    if (hr < 0)
        return hr;

    retVal = sbox;

    return 0;
}

void SandBox::InstallModule(exlib::string fname, v8::Local<v8::Value> o)
{
    mods()->Set(holder()->NewFromUtf8(fname), o);
}

RootModule* RootModule::g_root = NULL;

void SandBox::initRoot()
{
    Isolate* isolate = holder();

    RootModule* pModule = RootModule::g_root;

    while (pModule)
    {
        ClassInfo& ci = pModule->class_info();
        InstallModule(ci.name(), ci.getFunction(isolate));

        pModule = pModule->m_next;
    }

    InstallModule("expect", createV8Function("expect", isolate->m_isolate, test_base::s_expect));
}

result_t SandBox::add(exlib::string id, v8::Local<v8::Value> mod)
{
    util_base::clone(mod, mod);
    InstallModule(id, mod);

    return 0;
}

result_t SandBox::add(v8::Local<v8::Object> mods)
{
    v8::Local<v8::Array> ks = mods->GetPropertyNames();
    int32_t len = ks->Length();
    int32_t i;

    for (i = 0; i < len; i++)
    {
        v8::Local<v8::Value> k = ks->Get(i);
        add(*v8::String::Utf8Value(k), mods->Get(k));
    }

    return 0;
}

result_t SandBox::remove(exlib::string id)
{
    mods()->Delete(holder()->NewFromUtf8(id));
    return 0;
}

result_t SandBox::clone(obj_ptr<SandBox_base> &retVal)
{
    obj_ptr<SandBox> sbox = new SandBox();
    sbox->m_name = m_name;
    holder()->SetPrivate(sbox->wrap(), "_mods", mods()->Clone());

    retVal = sbox;

    return 0;
}

} /* namespace fibjs */
