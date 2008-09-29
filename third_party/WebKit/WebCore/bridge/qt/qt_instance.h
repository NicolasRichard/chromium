/*
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef BINDINGS_QT_INSTANCE_H_
#define BINDINGS_QT_INSTANCE_H_

#include "runtime.h"
#include "runtime_root.h"
#include <qpointer.h>
#include <qhash.h>
#include <qset.h>

namespace JSC {

namespace Bindings {

class QtClass;
class QtField;
class QtRuntimeMetaMethod;

class QtInstance : public Instance
{
public:
    ~QtInstance ();

    virtual Class* getClass() const;

    virtual void begin();
    virtual void end();

    virtual JSValue* valueOf(ExecState* exec) const;
    virtual JSValue* defaultValue(ExecState*, PreferredPrimitiveType) const;

    virtual void mark(); // This isn't inherited

    virtual JSValue* invokeMethod (ExecState *exec, const MethodList &method, const ArgList &args);

    virtual void getPropertyNames(ExecState*, PropertyNameArray&);

    virtual BindingLanguage getBindingLanguage() const { return QtLanguage; }

    JSValue* stringValue(ExecState* exec) const;
    JSValue* numberValue(ExecState* exec) const;
    JSValue* booleanValue() const;

    QObject* getObject() const { return m_object; }

    static PassRefPtr<QtInstance> getQtInstance(QObject*, PassRefPtr<RootObject>);
    static RuntimeObjectImp* getRuntimeObject(ExecState* exec, PassRefPtr<QtInstance>);

private:
    static PassRefPtr<QtInstance> create(QObject *instance, PassRefPtr<RootObject> rootObject)
    {
        return adoptRef(new QtInstance(instance, rootObject));
    }

    friend class QtClass;
    friend class QtField;
    QtInstance(QObject*, PassRefPtr<RootObject>); // Factory produced only..
    mutable QtClass* m_class;
    QPointer<QObject> m_object;
    QObject* m_hashkey;
    mutable QHash<QByteArray,JSValue*> m_methods;
    mutable QHash<QString,QtField*> m_fields;
    mutable QSet<JSValue*> m_children;
    mutable QtRuntimeMetaMethod* m_defaultMethod;
};

} // namespace Bindings

} // namespace JSC

#endif
