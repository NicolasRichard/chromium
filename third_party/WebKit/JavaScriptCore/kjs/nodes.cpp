// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2002 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003 Apple Computer, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "nodes.h"

#include <math.h>
#include <assert.h>
#ifdef KJS_DEBUG_MEM
#include <stdio.h>
#include <typeinfo>
#endif

#include "collector.h"
#include "context.h"
#include "debugger.h"
#include "function_object.h"
#include "internal.h"
#include "value.h"
#include "object.h"
#include "types.h"
#include "interpreter.h"
#include "lexer.h"
#include "operations.h"
#include "ustring.h"
#include "reference_list.h"

using namespace KJS;

#define KJS_BREAKPOINT \
  if (Debugger::debuggersPresent > 0 && !hitStatement(exec)) \
    return Completion(Normal);

#define KJS_ABORTPOINT \
  if (Debugger::debuggersPresent > 0 && \
      exec->dynamicInterpreter()->imp()->debugger() && \
      exec->dynamicInterpreter()->imp()->debugger()->imp()->aborted()) \
    return Completion(Normal);

#define KJS_CHECKEXCEPTION \
  if (exec->hadException()) { \
    setExceptionDetailsIfNeeded(exec); \
    return Completion(Throw, exec->exception()); \
  } \
  if (Collector::outOfMemory()) \
    return Completion(Throw, Error::create(exec,GeneralError,"Out of memory"));

#define KJS_CHECKEXCEPTIONVALUE \
  if (exec->hadException()) { \
    setExceptionDetailsIfNeeded(exec); \
    return exec->exception(); \
  } \
  if (Collector::outOfMemory()) \
    return Undefined(); // will be picked up by KJS_CHECKEXCEPTION

#define KJS_CHECKEXCEPTIONREFERENCE \
  if (exec->hadException()) { \
    setExceptionDetailsIfNeeded(exec); \
    return Reference::makeValueReference(Undefined()); \
  } \
  if (Collector::outOfMemory()) \
    return Reference::makeValueReference(Undefined()); // will be picked up by KJS_CHECKEXCEPTION

#define KJS_CHECKEXCEPTIONLIST \
  if (exec->hadException()) { \
    setExceptionDetailsIfNeeded(exec); \
    return List(); \
  } \
  if (Collector::outOfMemory()) \
    return List(); // will be picked up by KJS_CHECKEXCEPTION

#ifdef KJS_DEBUG_MEM
std::list<Node *> * Node::s_nodes = 0L;
#endif

// ------------------------------ Node -----------------------------------------

Node::Node()
{
  line = Lexer::curr()->lineNo();
  sourceURL = Lexer::curr()->sourceURL();
  refcount = 0;
#ifdef KJS_DEBUG_MEM
  if (!s_nodes)
    s_nodes = new std::list<Node *>;
  s_nodes->push_back(this);
#endif
}

Node::~Node()
{
#ifdef KJS_DEBUG_MEM
  s_nodes->remove( this );
#endif
}

Reference Node::evaluateReference(ExecState *exec)
{
  ValueImp *v = evaluate(exec);
  KJS_CHECKEXCEPTIONREFERENCE
  return Reference::makeValueReference(v);
}

#ifdef KJS_DEBUG_MEM
void Node::finalCheck()
{
  fprintf( stderr, "Node::finalCheck(): list count       : %d\n", (int)s_nodes.size() );
  std::list<Node *>::iterator it = s_nodes->begin();
  for ( uint i = 0; it != s_nodes->end() ; ++it, ++i )
    fprintf( stderr, "[%d] Still having node %p (%s) (refcount %d)\n", i, (void*)*it, typeid( **it ).name(), (*it)->refcount );
  delete s_nodes;
  s_nodes = 0L;
}
#endif

ValueImp *Node::throwError(ExecState *exec, ErrorType e, const char *msg)
{
  ObjectImp *err = Error::create(exec, e, msg, lineNo(), sourceId(), &sourceURL);
  exec->setException(err);
  return err;
}

ValueImp *Node::throwError(ExecState *exec, ErrorType e, const char *msg, ValueImp *v, Node *expr)
{
  char *vStr = strdup(v->toString(exec).ascii());
  char *exprStr = strdup(expr->toString().ascii());
  
  int length =  strlen(msg) - 4 /* two %s */ + strlen(vStr) + strlen(exprStr) + 1 /* null terminator */;
  char *str = new char[length];
  sprintf(str, msg, vStr, exprStr);
  free(vStr);
  free(exprStr);

  ValueImp *result = throwError(exec, e, str);
  delete [] str;
  
  return result;
}


ValueImp *Node::throwError(ExecState *exec, ErrorType e, const char *msg, Identifier label)
{
  const char *l = label.ascii();
  int length = strlen(msg) - 2 /* %s */ + strlen(l) + 1 /* null terminator */;
  char *message = new char[length];
  sprintf(message, msg, l);

  ValueImp *result = throwError(exec, e, message);
  delete [] message;

  return result;
}

void Node::setExceptionDetailsIfNeeded(ExecState *exec)
{
    if (exec->hadException()) {
        ObjectImp *exception = exec->exception()->toObject(exec);
        if (!exception->hasProperty(exec, "line") &&
            !exception->hasProperty(exec, "sourceURL")) {
            exception->put(exec, "line", Number(line));
            exception->put(exec, "sourceURL", String(sourceURL));
        }
    }
}


// ------------------------------ StatementNode --------------------------------
StatementNode::StatementNode() : l0(-1), l1(-1), sid(-1), breakPoint(false)
{
}

void StatementNode::setLoc(int line0, int line1, int sourceId)
{
    l0 = line0;
    l1 = line1;
    sid = sourceId;
}

// return true if the debugger wants us to stop at this point
bool StatementNode::hitStatement(ExecState *exec)
{
  Debugger *dbg = exec->dynamicInterpreter()->imp()->debugger();
  if (dbg)
    return dbg->atStatement(exec,sid,l0,l1);
  else
    return true; // continue
}

// return true if the debugger wants us to stop at this point
bool StatementNode::abortStatement(ExecState *exec)
{
  Debugger *dbg = exec->dynamicInterpreter()->imp()->debugger();
  if (dbg)
    return dbg->imp()->aborted();
  else
    return false;
}

void StatementNode::processFuncDecl(ExecState *exec)
{
}

// ------------------------------ NullNode -------------------------------------

ValueImp *NullNode::evaluate(ExecState */*exec*/)
{
  return Null();
}

// ------------------------------ BooleanNode ----------------------------------

ValueImp *BooleanNode::evaluate(ExecState */*exec*/)
{
  return jsBoolean(value);
}

// ------------------------------ NumberNode -----------------------------------

ValueImp *NumberNode::evaluate(ExecState */*exec*/)
{
  return jsNumber(value);
}

// ------------------------------ StringNode -----------------------------------

ValueImp *StringNode::evaluate(ExecState */*exec*/)
{
  return jsString(value);
}

// ------------------------------ RegExpNode -----------------------------------

ValueImp *RegExpNode::evaluate(ExecState *exec)
{
  List list;
  list.append(jsString(pattern));
  list.append(jsString(flags));

  ObjectImp *reg = exec->lexicalInterpreter()->imp()->builtinRegExp();
  return reg->construct(exec,list);
}

// ------------------------------ ThisNode -------------------------------------

// ECMA 11.1.1
ValueImp *ThisNode::evaluate(ExecState *exec)
{
  return exec->context().imp()->thisValue();
}

// ------------------------------ ResolveNode ----------------------------------

// ECMA 11.1.2 & 10.1.4
ValueImp *ResolveNode::evaluate(ExecState *exec)
{
  ScopeChain chain = exec->context().imp()->scopeChain();

  assert(!chain.isEmpty());

  PropertySlot slot;
  do { 
    ObjectImp *o = chain.top();

    if (o->getPropertySlot(exec, ident, slot))
      return slot.getValue(exec, ident);
    
    chain.pop();
  } while (!chain.isEmpty());

  return Reference(ident).getValue(exec);
}

Reference ResolveNode::evaluateReference(ExecState *exec)
{
  ScopeChain chain = exec->context().imp()->scopeChain();

  assert(!chain.isEmpty());

  PropertySlot slot;
  do { 
    ObjectImp *o = chain.top();
    if (o->getPropertySlot(exec, ident, slot))
      return Reference(o, ident);
    
    chain.pop();
  } while (!chain.isEmpty());

  return Reference(ident);
}

// ------------------------------ GroupNode ------------------------------------

void GroupNode::ref()
{
  Node::ref();
  if ( group )
    group->ref();
}

bool GroupNode::deref()
{
  if ( group && group->deref() )
    delete group;
  return Node::deref();
}

// ECMA 11.1.6
ValueImp *GroupNode::evaluate(ExecState *exec)
{
  return group->evaluate(exec);
}

Reference GroupNode::evaluateReference(ExecState *exec)
{
  return group->evaluateReference(exec);
}

// ------------------------------ ElementNode ----------------------------------

void ElementNode::ref()
{
  for (ElementNode *n = this; n; n = n->list) {
    n->Node::ref();
    if (n->node)
      n->node->ref();
  }
}

bool ElementNode::deref()
{
  ElementNode *next;
  for (ElementNode *n = this; n; n = next) {
    next = n->list;
    if (n->node && n->node->deref())
      delete n->node;
    if (n != this && n->Node::deref())
      delete n;
  }
  return Node::deref();
}

// ECMA 11.1.4
ValueImp *ElementNode::evaluate(ExecState *exec)
{
  ObjectImp *array = exec->lexicalInterpreter()->builtinArray()->construct(exec, List::empty());
  int length = 0;
  for (ElementNode *n = this; n; n = n->list) {
    ValueImp *val = n->node->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
    length += n->elision;
    array->put(exec, length++, val);
  }
  return array;
}

// ------------------------------ ArrayNode ------------------------------------

void ArrayNode::ref()
{
  Node::ref();
  if ( element )
    element->ref();
}

bool ArrayNode::deref()
{
  if ( element && element->deref() )
    delete element;
  return Node::deref();
}

// ECMA 11.1.4
ValueImp *ArrayNode::evaluate(ExecState *exec)
{
  ObjectImp *array;
  int length;

  if (element) {
    array = static_cast<ObjectImp*>(element->evaluate(exec));
    KJS_CHECKEXCEPTIONVALUE
    length = opt ? array->get(exec,lengthPropertyName)->toInt32(exec) : 0;
  } else {
    ValueImp *newArr = exec->lexicalInterpreter()->builtinArray()->construct(exec,List::empty());
    array = static_cast<ObjectImp*>(newArr);
    length = 0;
  }

  if (opt)
    array->put(exec,lengthPropertyName, jsNumber(elision + length), DontEnum | DontDelete);

  return array;
}

// ------------------------------ ObjectLiteralNode ----------------------------

void ObjectLiteralNode::ref()
{
  Node::ref();
  if ( list )
    list->ref();
}

bool ObjectLiteralNode::deref()
{
  if ( list && list->deref() )
    delete list;
  return Node::deref();
}

// ECMA 11.1.5
ValueImp *ObjectLiteralNode::evaluate(ExecState *exec)
{
  if (list)
    return list->evaluate(exec);

  return exec->lexicalInterpreter()->builtinObject()->construct(exec,List::empty());
}

// ------------------------------ PropertyValueNode ----------------------------

void PropertyValueNode::ref()
{
  for (PropertyValueNode *n = this; n; n = n->list) {
    n->Node::ref();
    if (n->name)
      n->name->ref();
    if (n->assign)
      n->assign->ref();
  }
}

bool PropertyValueNode::deref()
{
  PropertyValueNode *next;
  for (PropertyValueNode *n = this; n; n = next) {
    next = n->list;
    if ( n->name && n->name->deref() )
      delete n->name;
    if ( n->assign && n->assign->deref() )
      delete n->assign;
    if (n != this && n->Node::deref() )
      delete n;
  }
  return Node::deref();
}

// ECMA 11.1.5
ValueImp *PropertyValueNode::evaluate(ExecState *exec)
{
  ObjectImp *obj = exec->lexicalInterpreter()->builtinObject()->construct(exec, List::empty());
  
  for (PropertyValueNode *p = this; p; p = p->list) {
    ValueImp *n = p->name->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
    ValueImp *v = p->assign->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE

    obj->put(exec, Identifier(n->toString(exec)), v);
  }

  return obj;
}

// ------------------------------ PropertyNode ---------------------------------

// ECMA 11.1.5
ValueImp *PropertyNode::evaluate(ExecState */*exec*/)
{
  ValueImp *s;

  if (str.isNull()) {
    s = String(UString::from(numeric));
  } else {
    s = String(str.ustring());
  }

  return s;
}

// ------------------------------ AccessorNode1 --------------------------------

void AccessorNode1::ref()
{
  Node::ref();
  if ( expr1 )
    expr1->ref();
  if ( expr2 )
    expr2->ref();
}

bool AccessorNode1::deref()
{
  if ( expr1 && expr1->deref() )
    delete expr1;
  if ( expr2 && expr2->deref() )
    delete expr2;
  return Node::deref();
}

// ECMA 11.2.1a
ValueImp *AccessorNode1::evaluate(ExecState *exec)
{
  ValueImp *v1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  ValueImp *v2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  ObjectImp *o = v1->toObject(exec);
  uint32_t i;
  if (v2->getUInt32(i))
    return o->get(exec, i);
  return o->get(exec, Identifier(v2->toString(exec)));
}

Reference AccessorNode1::evaluateReference(ExecState *exec)
{
  ValueImp *v1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONREFERENCE
  ValueImp *v2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONREFERENCE
  ObjectImp *o = v1->toObject(exec);
  uint32_t i;
  if (v2->getUInt32(i))
    return Reference(o, i);
  return Reference(o, Identifier(v2->toString(exec)));
}

// ------------------------------ AccessorNode2 --------------------------------

void AccessorNode2::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool AccessorNode2::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.2.1b
ValueImp *AccessorNode2::evaluate(ExecState *exec)
{
  ValueImp *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  return v->toObject(exec)->get(exec, ident);

}

Reference AccessorNode2::evaluateReference(ExecState *exec)
{
  ValueImp *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONREFERENCE
  ObjectImp *o = v->toObject(exec);
  return Reference(o, ident);
}

// ------------------------------ ArgumentListNode -----------------------------

void ArgumentListNode::ref()
{
  for (ArgumentListNode *n = this; n; n = n->list) {
    n->Node::ref();
    if (n->expr)
      n->expr->ref();
  }
}

bool ArgumentListNode::deref()
{
  ArgumentListNode *next;
  for (ArgumentListNode *n = this; n; n = next) {
    next = n->list;
    if (n->expr && n->expr->deref())
      delete n->expr;
    if (n != this && n->Node::deref())
      delete n;
  }
  return Node::deref();
}

ValueImp *ArgumentListNode::evaluate(ExecState */*exec*/)
{
  assert(0);
  return NULL; // dummy, see evaluateList()
}

// ECMA 11.2.4
List ArgumentListNode::evaluateList(ExecState *exec)
{
  List l;

  for (ArgumentListNode *n = this; n; n = n->list) {
    ValueImp *v = n->expr->evaluate(exec);
    KJS_CHECKEXCEPTIONLIST
    l.append(v);
  }

  return l;
}

// ------------------------------ ArgumentsNode --------------------------------

void ArgumentsNode::ref()
{
  Node::ref();
  if ( list )
    list->ref();
}

bool ArgumentsNode::deref()
{
  if ( list && list->deref() )
    delete list;
  return Node::deref();
}

ValueImp *ArgumentsNode::evaluate(ExecState */*exec*/)
{
  assert(0);
  return NULL; // dummy, see evaluateList()
}

// ECMA 11.2.4
List ArgumentsNode::evaluateList(ExecState *exec)
{
  if (!list)
    return List();

  return list->evaluateList(exec);
}

// ------------------------------ NewExprNode ----------------------------------

// ECMA 11.2.2

void NewExprNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
  if ( args )
    args->ref();
}

bool NewExprNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  if ( args && args->deref() )
    delete args;
  return Node::deref();
}

ValueImp *NewExprNode::evaluate(ExecState *exec)
{
  ValueImp *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  List argList;
  if (args) {
    argList = args->evaluateList(exec);
    KJS_CHECKEXCEPTIONVALUE
  }

  if (!v->isObject()) {
    return throwError(exec, TypeError, "Value %s (result of expression %s) is not an object. Cannot be used with new.", v, expr);
  }

  ObjectImp *constr = static_cast<ObjectImp*>(v);
  if (!constr->implementsConstruct()) {
    return throwError(exec, TypeError, "Value %s (result of expression %s) is not a constructor. Cannot be used with new.", v, expr);
  }

  return constr->construct(exec, argList);
}

// ------------------------------ FunctionCallNode -----------------------------

void FunctionCallNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
  if ( args )
    args->ref();
}

bool FunctionCallNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  if ( args && args->deref() )
    delete args;
  return Node::deref();
}

// ECMA 11.2.3
ValueImp *FunctionCallNode::evaluate(ExecState *exec)
{
  Reference ref = expr->evaluateReference(exec);
  KJS_CHECKEXCEPTIONVALUE

  List argList = args->evaluateList(exec);
  KJS_CHECKEXCEPTIONVALUE

  ValueImp *v = ref.getValue(exec);
  KJS_CHECKEXCEPTIONVALUE
  
  if (!v->isObject()) {
    return throwError(exec, TypeError, "Value %s (result of expression %s) is not object.", v, expr);
  }
  
  ObjectImp *func = static_cast<ObjectImp*>(v);

  if (!func->implementsCall()) {
    return throwError(exec, TypeError, "Object %s (result of expression %s) does not allow calls.", v, expr);
  }

  ObjectImp *thisObjImp = 0;
  ValueImp *thisValImp = ref.baseIfMutable();
  if (thisValImp && thisValImp->isObject() && !static_cast<ObjectImp *>(thisValImp)->inherits(&ActivationImp::info))
    thisObjImp = static_cast<ObjectImp *>(thisValImp);

  if (!thisObjImp) {
    // ECMA 11.2.3 says that in this situation the this value should be null.
    // However, section 10.2.3 says that in the case where the value provided
    // by the caller is null, the global object should be used. It also says
    // that the section does not apply to interal functions, but for simplicity
    // of implementation we use the global object anyway here. This guarantees
    // that in host objects you always get a valid object for this.
    thisObjImp = exec->dynamicInterpreter()->globalObject();
  }

  ObjectImp *thisObj(thisObjImp);
  return func->call(exec, thisObj, argList);
}

// ------------------------------ PostfixNode ----------------------------------

void PostfixNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool PostfixNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.3
ValueImp *PostfixNode::evaluate(ExecState *exec)
{
  Reference ref = expr->evaluateReference(exec);
  KJS_CHECKEXCEPTIONVALUE
  ValueImp *v = ref.getValue(exec);

  bool knownToBeInteger;
  double n = v->toNumber(exec, knownToBeInteger);

  double newValue = (oper == OpPlusPlus) ? n + 1 : n - 1;
  ref.putValue(exec, jsNumber(newValue, knownToBeInteger));

  return jsNumber(n, knownToBeInteger);
}

// ------------------------------ DeleteNode -----------------------------------

void DeleteNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool DeleteNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.4.1
ValueImp *DeleteNode::evaluate(ExecState *exec)
{
  Reference ref = expr->evaluateReference(exec);
  KJS_CHECKEXCEPTIONVALUE
  return jsBoolean(ref.deleteValue(exec));
}

// ------------------------------ VoidNode -------------------------------------

void VoidNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool VoidNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.4.2
ValueImp *VoidNode::evaluate(ExecState *exec)
{
  expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return Undefined();
}

// ------------------------------ TypeOfNode -----------------------------------

void TypeOfNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool TypeOfNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.4.3
ValueImp *TypeOfNode::evaluate(ExecState *exec)
{
  const char *s = 0L;
  Reference ref = expr->evaluateReference(exec);
  KJS_CHECKEXCEPTIONVALUE
  ValueImp *b = ref.baseIfMutable();
  if (b && b->isNull())
    return jsString("undefined");
  ValueImp *v = ref.getValue(exec);
  switch (v->type())
    {
    case UndefinedType:
      s = "undefined";
      break;
    case NullType:
      s = "object";
      break;
    case BooleanType:
      s = "boolean";
      break;
    case NumberType:
      s = "number";
      break;
    case StringType:
      s = "string";
      break;
    default:
      if (v->isObject() && static_cast<ObjectImp*>(v)->implementsCall())
	s = "function";
      else
	s = "object";
      break;
    }

  return jsString(s);
}

// ------------------------------ PrefixNode -----------------------------------

void PrefixNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool PrefixNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.4.4 and 11.4.5
ValueImp *PrefixNode::evaluate(ExecState *exec)
{
  Reference ref = expr->evaluateReference(exec);
  KJS_CHECKEXCEPTIONVALUE
  ValueImp *v = ref.getValue(exec);

  bool knownToBeInteger;
  double n = v->toNumber(exec, knownToBeInteger);

  double newValue = (oper == OpPlusPlus) ? n + 1 : n - 1;
  ValueImp *n2 = jsNumber(newValue, knownToBeInteger);

  ref.putValue(exec, n2);

  return n2;
}

// ------------------------------ UnaryPlusNode --------------------------------

void UnaryPlusNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool UnaryPlusNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.4.6
ValueImp *UnaryPlusNode::evaluate(ExecState *exec)
{
  ValueImp *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return jsNumber(v->toNumber(exec)); /* TODO: optimize */
}

// ------------------------------ NegateNode -----------------------------------

void NegateNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool NegateNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.4.7
ValueImp *NegateNode::evaluate(ExecState *exec)
{
  ValueImp *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  bool knownToBeInteger;
  double n = v->toNumber(exec, knownToBeInteger);
  return jsNumber(-n, knownToBeInteger && n != 0);
}

// ------------------------------ BitwiseNotNode -------------------------------

void BitwiseNotNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool BitwiseNotNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.4.8
ValueImp *BitwiseNotNode::evaluate(ExecState *exec)
{
  ValueImp *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  return jsNumber(~v->toInt32(exec));
}

// ------------------------------ LogicalNotNode -------------------------------

void LogicalNotNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool LogicalNotNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.4.9
ValueImp *LogicalNotNode::evaluate(ExecState *exec)
{
  ValueImp *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  return jsBoolean(!v->toBoolean(exec));
}

// ------------------------------ MultNode -------------------------------------

void MultNode::ref()
{
  Node::ref();
  if ( term1 )
    term1->ref();
  if ( term2 )
    term2->ref();
}

bool MultNode::deref()
{
  if ( term1 && term1->deref() )
    delete term1;
  if ( term2 && term2->deref() )
    delete term2;
  return Node::deref();
}

// ECMA 11.5
ValueImp *MultNode::evaluate(ExecState *exec)
{
  ValueImp *v1 = term1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  ValueImp *v2 = term2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return mult(exec, v1, v2, oper);
}

// ------------------------------ AddNode --------------------------------------

void AddNode::ref()
{
  Node::ref();
  if ( term1 )
    term1->ref();
  if ( term2 )
    term2->ref();
}

bool AddNode::deref()
{
  if ( term1 && term1->deref() )
    delete term1;
  if ( term2 && term2->deref() )
    delete term2;
  return Node::deref();
}

// ECMA 11.6
ValueImp *AddNode::evaluate(ExecState *exec)
{
  ValueImp *v1 = term1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  ValueImp *v2 = term2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return add(exec, v1, v2, oper);
}

// ------------------------------ ShiftNode ------------------------------------

void ShiftNode::ref()
{
  Node::ref();
  if ( term1 )
    term1->ref();
  if ( term2 )
    term2->ref();
}

bool ShiftNode::deref()
{
  if ( term1 && term1->deref() )
    delete term1;
  if ( term2 && term2->deref() )
    delete term2;
  return Node::deref();
}

// ECMA 11.7
ValueImp *ShiftNode::evaluate(ExecState *exec)
{
  ValueImp *v1 = term1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  ValueImp *v2 = term2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  unsigned int i2 = v2->toUInt32(exec);
  i2 &= 0x1f;

  switch (oper) {
  case OpLShift:
    return jsNumber(v1->toInt32(exec) << i2);
  case OpRShift:
    return jsNumber(v1->toInt32(exec) >> i2);
  case OpURShift:
    return jsNumber(v1->toUInt32(exec) >> i2);
  default:
    assert(!"ShiftNode: unhandled switch case");
    return Undefined();
  }
}

// ------------------------------ RelationalNode -------------------------------

void RelationalNode::ref()
{
  Node::ref();
  if ( expr1 )
    expr1->ref();
  if ( expr2 )
    expr2->ref();
}

bool RelationalNode::deref()
{
  if ( expr1 && expr1->deref() )
    delete expr1;
  if ( expr2 && expr2->deref() )
    delete expr2;
  return Node::deref();
}

// ECMA 11.8
ValueImp *RelationalNode::evaluate(ExecState *exec)
{
  ValueImp *v1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  ValueImp *v2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  bool b;
  if (oper == OpLess || oper == OpGreaterEq) {
    int r = relation(exec, v1, v2);
    if (r < 0)
      b = false;
    else
      b = (oper == OpLess) ? (r == 1) : (r == 0);
  } else if (oper == OpGreater || oper == OpLessEq) {
    int r = relation(exec, v2, v1);
    if (r < 0)
      b = false;
    else
      b = (oper == OpGreater) ? (r == 1) : (r == 0);
  } else if (oper == OpIn) {
      // Is all of this OK for host objects?
      if (!v2->isObject())
          return throwError(exec,  TypeError,
                             "Value %s (result of expression %s) is not an object. Cannot be used with IN expression.", v2, expr2);
      ObjectImp *o2(static_cast<ObjectImp*>(v2));
      b = o2->hasProperty(exec, Identifier(v1->toString(exec)));
  } else {
    if (!v2->isObject())
        return throwError(exec,  TypeError,
                           "Value %s (result of expression %s) is not an object. Cannot be used with instanceof operator.", v2, expr2);

    ObjectImp *o2(static_cast<ObjectImp*>(v2));
    if (!o2->implementsHasInstance()) {
      // According to the spec, only some types of objects "implement" the [[HasInstance]] property.
      // But we are supposed to throw an exception where the object does not "have" the [[HasInstance]]
      // property. It seems that all object have the property, but not all implement it, so in this
      // case we return false (consistent with mozilla)
      return jsBoolean(false);
      //      return throwError(exec, TypeError,
      //			"Object does not implement the [[HasInstance]] method." );
    }
    return jsBoolean(o2->hasInstance(exec, v1));
  }

  return jsBoolean(b);
}

// ------------------------------ EqualNode ------------------------------------

void EqualNode::ref()
{
  Node::ref();
  if ( expr1 )
    expr1->ref();
  if ( expr2 )
    expr2->ref();
}

bool EqualNode::deref()
{
  if ( expr1 && expr1->deref() )
    delete expr1;
  if ( expr2 && expr2->deref() )
    delete expr2;
  return Node::deref();
}

// ECMA 11.9
ValueImp *EqualNode::evaluate(ExecState *exec)
{
  ValueImp *v1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  ValueImp *v2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  bool result;
  if (oper == OpEqEq || oper == OpNotEq) {
    // == and !=
    bool eq = equal(exec,v1, v2);
    result = oper == OpEqEq ? eq : !eq;
  } else {
    // === and !==
    bool eq = strictEqual(exec,v1, v2);
    result = oper == OpStrEq ? eq : !eq;
  }
  return jsBoolean(result);
}

// ------------------------------ BitOperNode ----------------------------------

void BitOperNode::ref()
{
  Node::ref();
  if ( expr1 )
    expr1->ref();
  if ( expr2 )
    expr2->ref();
}

bool BitOperNode::deref()
{
  if ( expr1 && expr1->deref() )
    delete expr1;
  if ( expr2 && expr2->deref() )
    delete expr2;
  return Node::deref();
}

// ECMA 11.10
ValueImp *BitOperNode::evaluate(ExecState *exec)
{
  ValueImp *v1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  ValueImp *v2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  int i1 = v1->toInt32(exec);
  int i2 = v2->toInt32(exec);
  int result;
  if (oper == OpBitAnd)
    result = i1 & i2;
  else if (oper == OpBitXOr)
    result = i1 ^ i2;
  else
    result = i1 | i2;

  return jsNumber(result);
}

// ------------------------------ BinaryLogicalNode ----------------------------

void BinaryLogicalNode::ref()
{
  Node::ref();
  if ( expr1 )
    expr1->ref();
  if ( expr2 )
    expr2->ref();
}

bool BinaryLogicalNode::deref()
{
  if ( expr1 && expr1->deref() )
    delete expr1;
  if ( expr2 && expr2->deref() )
    delete expr2;
  return Node::deref();
}

// ECMA 11.11
ValueImp *BinaryLogicalNode::evaluate(ExecState *exec)
{
  ValueImp *v1 = expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  bool b1 = v1->toBoolean(exec);
  if ((!b1 && oper == OpAnd) || (b1 && oper == OpOr))
    return v1;

  ValueImp *v2 = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return v2;
}

// ------------------------------ ConditionalNode ------------------------------

void ConditionalNode::ref()
{
  Node::ref();
  if ( expr1 )
    expr1->ref();
  if ( expr2 )
    expr2->ref();
  if ( logical )
    logical->ref();
}

bool ConditionalNode::deref()
{
  if ( expr1 && expr1->deref() )
    delete expr1;
  if ( expr2 && expr2->deref() )
    delete expr2;
  if ( logical && logical->deref() )
    delete logical;
  return Node::deref();
}

// ECMA 11.12
ValueImp *ConditionalNode::evaluate(ExecState *exec)
{
  ValueImp *v = logical->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  bool b = v->toBoolean(exec);

  if (b)
    v = expr1->evaluate(exec);
  else
    v = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return v;
}

// ------------------------------ AssignNode -----------------------------------

void AssignNode::ref()
{
  Node::ref();
  if ( left )
    left->ref();
  if ( expr )
    expr->ref();
}

bool AssignNode::deref()
{
  if ( left && left->deref() )
    delete left;
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 11.13
ValueImp *AssignNode::evaluate(ExecState *exec)
{
  Reference l = left->evaluateReference(exec);
  KJS_CHECKEXCEPTIONVALUE
  ValueImp *v;
  if (oper == OpEqual) {
    v = expr->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
  } else {
    ValueImp *v1 = l.getValue(exec);
    ValueImp *v2 = expr->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
    int i1;
    int i2;
    unsigned int ui;
    switch (oper) {
    case OpMultEq:
      v = mult(exec, v1, v2, '*');
      break;
    case OpDivEq:
      v = mult(exec, v1, v2, '/');
      break;
    case OpPlusEq:
      v = add(exec, v1, v2, '+');
      break;
    case OpMinusEq:
      v = add(exec, v1, v2, '-');
      break;
    case OpLShift:
      i1 = v1->toInt32(exec);
      i2 = v2->toInt32(exec);
      v = jsNumber(i1 << i2);
      break;
    case OpRShift:
      i1 = v1->toInt32(exec);
      i2 = v2->toInt32(exec);
      v = jsNumber(i1 >> i2);
      break;
    case OpURShift:
      ui = v1->toUInt32(exec);
      i2 = v2->toInt32(exec);
      v = jsNumber(ui >> i2);
      break;
    case OpAndEq:
      i1 = v1->toInt32(exec);
      i2 = v2->toInt32(exec);
      v = jsNumber(i1 & i2);
      break;
    case OpXOrEq:
      i1 = v1->toInt32(exec);
      i2 = v2->toInt32(exec);
      v = jsNumber(i1 ^ i2);
      break;
    case OpOrEq:
      i1 = v1->toInt32(exec);
      i2 = v2->toInt32(exec);
      v = jsNumber(i1 | i2);
      break;
    case OpModEq: {
      bool d1KnownToBeInteger;
      double d1 = v1->toNumber(exec, d1KnownToBeInteger);
      bool d2KnownToBeInteger;
      double d2 = v2->toNumber(exec, d2KnownToBeInteger);
      v = jsNumber(fmod(d1, d2), d1KnownToBeInteger && d2KnownToBeInteger && d2 != 0);
    }
      break;
    default:
      v = Undefined();
    }
  };
  l.putValue(exec,v);

  KJS_CHECKEXCEPTIONVALUE

  return v;
}

// ------------------------------ CommaNode ------------------------------------

void CommaNode::ref()
{
  Node::ref();
  if ( expr1 )
    expr1->ref();
  if ( expr2 )
    expr2->ref();
}

bool CommaNode::deref()
{
  if ( expr1 && expr1->deref() )
    delete expr1;
  if ( expr2 && expr2->deref() )
    delete expr2;
  return Node::deref();
}

// ECMA 11.14
ValueImp *CommaNode::evaluate(ExecState *exec)
{
  expr1->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE
  ValueImp *v = expr2->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return v;
}

// ------------------------------ StatListNode ---------------------------------

StatListNode::StatListNode(StatementNode *s)
  : statement(s), list(this)
{
  setLoc(s->firstLine(), s->lastLine(), s->sourceId());
}
 
StatListNode::StatListNode(StatListNode *l, StatementNode *s)
  : statement(s), list(l->list)
{
  l->list = this;
  setLoc(l->firstLine(), s->lastLine(), l->sourceId());
}

void StatListNode::ref()
{
  for (StatListNode *n = this; n; n = n->list) {
    n->Node::ref();
    if (n->statement)
      n->statement->ref();
  }
}

bool StatListNode::deref()
{
  StatListNode *next;
  for (StatListNode *n = this; n; n = next) {
    next = n->list;
    if (n->statement && n->statement->deref())
      delete n->statement;
    if (n != this && n->Node::deref())
      delete n;
  }
  return Node::deref();
}

// ECMA 12.1
Completion StatListNode::execute(ExecState *exec)
{
  Completion c = statement->execute(exec);
  KJS_ABORTPOINT
  if (exec->hadException()) {
    ValueImp *ex = exec->exception();
    exec->clearException();
    return Completion(Throw, ex);
  }

  if (c.complType() != Normal)
    return c;
  
  ValueImp *v = c.value();
  
  for (StatListNode *n = list; n; n = n->list) {
    Completion c2 = n->statement->execute(exec);
    KJS_ABORTPOINT
    if (c2.complType() != Normal)
      return c2;

    if (exec->hadException()) {
      ValueImp *ex = exec->exception();
      exec->clearException();
      return Completion(Throw, ex);
    }

    if (c2.isValueCompletion())
      v = c2.value();
    c = c2;
  }

  return Completion(c.complType(), v, c.target());
}

void StatListNode::processVarDecls(ExecState *exec)
{
  for (StatListNode *n = this; n; n = n->list)
    n->statement->processVarDecls(exec);
}

// ------------------------------ AssignExprNode -------------------------------

void AssignExprNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool AssignExprNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 12.2
ValueImp *AssignExprNode::evaluate(ExecState *exec)
{
  return expr->evaluate(exec);
}

// ------------------------------ VarDeclNode ----------------------------------

    
VarDeclNode::VarDeclNode(const Identifier &id, AssignExprNode *in, Type t)
    : varType(t), ident(id), init(in)
{
}

void VarDeclNode::ref()
{
  Node::ref();
  if ( init )
    init->ref();
}

bool VarDeclNode::deref()
{
  if ( init && init->deref() )
    delete init;
  return Node::deref();
}

// ECMA 12.2
ValueImp *VarDeclNode::evaluate(ExecState *exec)
{
  ObjectImp *variable = exec->context().imp()->variableObject();

  ValueImp *val;
  if (init) {
      val = init->evaluate(exec);
      KJS_CHECKEXCEPTIONVALUE
  } else {
      // already declared? - check with getDirect so you can override
      // built-in properties of the global object with var declarations.
      if (variable->getDirect(ident)) 
          return NULL;
      val = Undefined();
  }

#ifdef KJS_VERBOSE
  printInfo(exec,(UString("new variable ")+ident.ustring()).cstring().c_str(),val);
#endif
  // We use Internal to bypass all checks in derived objects, e.g. so that
  // "var location" creates a dynamic property instead of activating window.location.
  int flags = Internal;
  if (exec->context().imp()->codeType() != EvalCode)
    flags |= DontDelete;
  if (varType == VarDeclNode::Constant)
    flags |= ReadOnly;
  variable->put(exec, ident, val, flags);

  return jsString(ident.ustring());
}

void VarDeclNode::processVarDecls(ExecState *exec)
{
  ObjectImp *variable = exec->context().imp()->variableObject();

  // If a variable by this name already exists, don't clobber it -
  // it might be a function parameter
  if (!variable->hasProperty(exec, ident)) {
    int flags = Internal;
    if (exec->context().imp()->codeType() != EvalCode)
      flags |= DontDelete;
    if (varType == VarDeclNode::Constant)
      flags |= ReadOnly;
    variable->put(exec, ident, Undefined(), flags);
  }
}

// ------------------------------ VarDeclListNode ------------------------------

void VarDeclListNode::ref()
{
  for (VarDeclListNode *n = this; n; n = n->list) {
    n->Node::ref();
    if (n->var)
      n->var->ref();
  }
}

bool VarDeclListNode::deref()
{
  VarDeclListNode *next;
  for (VarDeclListNode *n = this; n; n = next) {
    next = n->list;
    if (n->var && n->var->deref())
      delete n->var;
    if (n != this && n->Node::deref())
      delete n;
  }
  return Node::deref();
}


// ECMA 12.2
ValueImp *VarDeclListNode::evaluate(ExecState *exec)
{
  for (VarDeclListNode *n = this; n; n = n->list) {
    n->var->evaluate(exec);
    KJS_CHECKEXCEPTIONVALUE
  }
  return Undefined();
}

void VarDeclListNode::processVarDecls(ExecState *exec)
{
  for (VarDeclListNode *n = this; n; n = n->list)
    n->var->processVarDecls(exec);
}

// ------------------------------ VarStatementNode -----------------------------

void VarStatementNode::ref()
{
  Node::ref();
  if ( list )
    list->ref();
}

bool VarStatementNode::deref()
{
  if ( list && list->deref() )
    delete list;
  return Node::deref();
}

// ECMA 12.2
Completion VarStatementNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  (void) list->evaluate(exec);
  KJS_CHECKEXCEPTION

  return Completion(Normal);
}

void VarStatementNode::processVarDecls(ExecState *exec)
{
  list->processVarDecls(exec);
}

// ------------------------------ BlockNode ------------------------------------

BlockNode::BlockNode(SourceElementsNode *s)
{
  if (s) {
    source = s->elements;
    s->elements = 0;
    setLoc(s->firstLine(), s->lastLine(), s->sourceId());
  } else {
    source = 0;
  }
}

void BlockNode::ref()
{
  Node::ref();
  if ( source )
    source->ref();
}

bool BlockNode::deref()
{
  if ( source && source->deref() )
    delete source;
  return Node::deref();
}

// ECMA 12.1
Completion BlockNode::execute(ExecState *exec)
{
  if (!source)
    return Completion(Normal);

  source->processFuncDecl(exec);

  return source->execute(exec);
}

void BlockNode::processVarDecls(ExecState *exec)
{
  if (source)
    source->processVarDecls(exec);
}

// ------------------------------ EmptyStatementNode ---------------------------

// ECMA 12.3
Completion EmptyStatementNode::execute(ExecState */*exec*/)
{
  return Completion(Normal);
}

// ------------------------------ ExprStatementNode ----------------------------

void ExprStatementNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool ExprStatementNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 12.4
Completion ExprStatementNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  ValueImp *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTION

  return Completion(Normal, v);
}

// ------------------------------ IfNode ---------------------------------------

void IfNode::ref()
{
  Node::ref();
  if ( statement1 )
    statement1->ref();
  if ( statement2 )
    statement2->ref();
  if ( expr )
    expr->ref();
}

bool IfNode::deref()
{
  if ( statement1 && statement1->deref() )
    delete statement1;
  if ( statement2 && statement2->deref() )
    delete statement2;
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 12.5
Completion IfNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  ValueImp *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTION
  bool b = v->toBoolean(exec);

  // if ... then
  if (b)
    return statement1->execute(exec);

  // no else
  if (!statement2)
    return Completion(Normal);

  // else
  return statement2->execute(exec);
}

void IfNode::processVarDecls(ExecState *exec)
{
  statement1->processVarDecls(exec);

  if (statement2)
    statement2->processVarDecls(exec);
}

// ------------------------------ DoWhileNode ----------------------------------

void DoWhileNode::ref()
{
  Node::ref();
  if ( statement )
    statement->ref();
  if ( expr )
    expr->ref();
}

bool DoWhileNode::deref()
{
  if ( statement && statement->deref() )
    delete statement;
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 12.6.1
Completion DoWhileNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  ValueImp *bv;
  Completion c;

  do {
    // bail out on error
    KJS_CHECKEXCEPTION

    exec->context().imp()->seenLabels()->pushIteration();
    c = statement->execute(exec);
    exec->context().imp()->seenLabels()->popIteration();
    if (!((c.complType() == Continue) && ls.contains(c.target()))) {
      if ((c.complType() == Break) && ls.contains(c.target()))
        return Completion(Normal, NULL);
      if (c.complType() != Normal)
        return c;
    }
    bv = expr->evaluate(exec);
    KJS_CHECKEXCEPTION
  } while (bv->toBoolean(exec));

  return Completion(Normal, NULL);
}

void DoWhileNode::processVarDecls(ExecState *exec)
{
  statement->processVarDecls(exec);
}

// ------------------------------ WhileNode ------------------------------------

void WhileNode::ref()
{
  Node::ref();
  if ( statement )
    statement->ref();
  if ( expr )
    expr->ref();
}

bool WhileNode::deref()
{
  if ( statement && statement->deref() )
    delete statement;
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 12.6.2
Completion WhileNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  ValueImp *bv;
  Completion c;
  bool b(false);
  ValueImp *value = NULL;

  while (1) {
    bv = expr->evaluate(exec);
    KJS_CHECKEXCEPTION
    b = bv->toBoolean(exec);

    // bail out on error
    KJS_CHECKEXCEPTION

    if (!b)
      return Completion(Normal, value);

    exec->context().imp()->seenLabels()->pushIteration();
    c = statement->execute(exec);
    exec->context().imp()->seenLabels()->popIteration();
    if (c.isValueCompletion())
      value = c.value();

    if ((c.complType() == Continue) && ls.contains(c.target()))
      continue;
    if ((c.complType() == Break) && ls.contains(c.target()))
      return Completion(Normal, value);
    if (c.complType() != Normal)
      return c;
  }

  return Completion(); // work around gcc 4.0 bug
}

void WhileNode::processVarDecls(ExecState *exec)
{
  statement->processVarDecls(exec);
}

// ------------------------------ ForNode --------------------------------------

void ForNode::ref()
{
  Node::ref();
  if ( statement )
    statement->ref();
  if ( expr1 )
    expr1->ref();
  if ( expr2 )
    expr2->ref();
  if ( expr3 )
    expr3->ref();
}

bool ForNode::deref()
{
  if ( statement && statement->deref() )
    delete statement;
  if ( expr1 && expr1->deref() )
    delete expr1;
  if ( expr2 && expr2->deref() )
    delete expr2;
  if ( expr3 && expr3->deref() )
    delete expr3;
  return Node::deref();
}

// ECMA 12.6.3
Completion ForNode::execute(ExecState *exec)
{
  ValueImp *v, *cval = NULL;

  if (expr1) {
    v = expr1->evaluate(exec);
    KJS_CHECKEXCEPTION
  }
  while (1) {
    if (expr2) {
      v = expr2->evaluate(exec);
      KJS_CHECKEXCEPTION
      if (!v->toBoolean(exec))
	return Completion(Normal, cval);
    }
    // bail out on error
    KJS_CHECKEXCEPTION

    exec->context().imp()->seenLabels()->pushIteration();
    Completion c = statement->execute(exec);
    exec->context().imp()->seenLabels()->popIteration();
    if (c.isValueCompletion())
      cval = c.value();
    if (!((c.complType() == Continue) && ls.contains(c.target()))) {
      if ((c.complType() == Break) && ls.contains(c.target()))
        return Completion(Normal, cval);
      if (c.complType() != Normal)
      return c;
    }
    if (expr3) {
      v = expr3->evaluate(exec);
      KJS_CHECKEXCEPTION
    }
  }
  
  return Completion(); // work around gcc 4.0 bug
}

void ForNode::processVarDecls(ExecState *exec)
{
  if (expr1)
    expr1->processVarDecls(exec);

  statement->processVarDecls(exec);
}

// ------------------------------ ForInNode ------------------------------------

ForInNode::ForInNode(Node *l, Node *e, StatementNode *s)
  : init(0L), lexpr(l), expr(e), varDecl(0L), statement(s)
{
}

ForInNode::ForInNode(const Identifier &i, AssignExprNode *in, Node *e, StatementNode *s)
  : ident(i), init(in), expr(e), statement(s)
{
  // for( var foo = bar in baz )
  varDecl = new VarDeclNode(ident, init, VarDeclNode::Variable);
  lexpr = new ResolveNode(ident);
}

void ForInNode::ref()
{
  Node::ref();
  if ( statement )
    statement->ref();
  if ( expr )
    expr->ref();
  if ( lexpr )
    lexpr->ref();
  if ( init )
    init->ref();
  if ( varDecl )
    varDecl->ref();
}

bool ForInNode::deref()
{
  if ( statement && statement->deref() )
    delete statement;
  if ( expr && expr->deref() )
    delete expr;
  if ( lexpr && lexpr->deref() )
    delete lexpr;
  if ( init && init->deref() )
    delete init;
  if ( varDecl && varDecl->deref() )
    delete varDecl;
  return Node::deref();
}

// ECMA 12.6.4
Completion ForInNode::execute(ExecState *exec)
{
  ValueImp *e;
  ValueImp *retval = NULL;
  ObjectImp *v;
  Completion c;
  ReferenceList propList;

  if ( varDecl ) {
    varDecl->evaluate(exec);
    KJS_CHECKEXCEPTION
  }

  e = expr->evaluate(exec);

  // for Null and Undefined, we want to make sure not to go through
  // the loop at all, because their object wrappers will have a
  // property list but will throw an exception if you attempt to
  // access any property.
  if (e->isUndefinedOrNull()) {
    return Completion(Normal, NULL);
  }

  KJS_CHECKEXCEPTION
  v = e->toObject(exec);
  propList = v->propList(exec);

  ReferenceListIterator propIt = propList.begin();

  while (propIt != propList.end()) {
    Identifier name = propIt->getPropertyName(exec);
    if (!v->hasProperty(exec,name)) {
      propIt++;
      continue;
    }

    Reference ref = lexpr->evaluateReference(exec);
    KJS_CHECKEXCEPTION
    ref.putValue(exec, String(name.ustring()));

    exec->context().imp()->seenLabels()->pushIteration();
    c = statement->execute(exec);
    exec->context().imp()->seenLabels()->popIteration();
    if (c.isValueCompletion())
      retval = c.value();

    if (!((c.complType() == Continue) && ls.contains(c.target()))) {
      if ((c.complType() == Break) && ls.contains(c.target()))
        break;
      if (c.complType() != Normal) {
        return c;
      }
    }

    propIt++;
  }

  // bail out on error
  KJS_CHECKEXCEPTION

  return Completion(Normal, retval);
}

void ForInNode::processVarDecls(ExecState *exec)
{
  statement->processVarDecls(exec);
}

// ------------------------------ ContinueNode ---------------------------------

// ECMA 12.7
Completion ContinueNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  if (ident.isEmpty() && !exec->context().imp()->seenLabels()->inIteration())
    return Completion(Throw,
		      throwError(exec, SyntaxError, "Invalid continue statement."));
  else if (!ident.isEmpty() && !exec->context().imp()->seenLabels()->contains(ident))
    return Completion(Throw,
                      throwError(exec, SyntaxError, "Label %s not found.", ident));
  else
    return Completion(Continue, NULL, ident);
}

// ------------------------------ BreakNode ------------------------------------

// ECMA 12.8
Completion BreakNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  if (ident.isEmpty() && !exec->context().imp()->seenLabels()->inIteration() &&
      !exec->context().imp()->seenLabels()->inSwitch())
    return Completion(Throw,
		      throwError(exec, SyntaxError, "Invalid break statement."));
  else if (!ident.isEmpty() && !exec->context().imp()->seenLabels()->contains(ident))
    return Completion(Throw,
                      throwError(exec, SyntaxError, "Label %s not found.", ident));
  else
    return Completion(Break, NULL, ident);
}

// ------------------------------ ReturnNode -----------------------------------

void ReturnNode::ref()
{
  Node::ref();
  if ( value )
    value->ref();
}

bool ReturnNode::deref()
{
  if ( value && value->deref() )
    delete value;
  return Node::deref();
}

// ECMA 12.9
Completion ReturnNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  CodeType codeType = exec->context().imp()->codeType();
  if (codeType != FunctionCode && codeType != AnonymousCode ) {
    return Completion(Throw, throwError(exec, SyntaxError, "Invalid return statement."));    
  }

  if (!value)
    return Completion(ReturnValue, Undefined());

  ValueImp *v = value->evaluate(exec);
  KJS_CHECKEXCEPTION

  return Completion(ReturnValue, v);
}

// ------------------------------ WithNode -------------------------------------

void WithNode::ref()
{
  Node::ref();
  if ( statement )
    statement->ref();
  if ( expr )
    expr->ref();
}

bool WithNode::deref()
{
  if ( statement && statement->deref() )
    delete statement;
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 12.10
Completion WithNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  ValueImp *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTION
  ObjectImp *o = v->toObject(exec);
  KJS_CHECKEXCEPTION
  exec->context().imp()->pushScope(o);
  Completion res = statement->execute(exec);
  exec->context().imp()->popScope();

  return res;
}

void WithNode::processVarDecls(ExecState *exec)
{
  statement->processVarDecls(exec);
}

// ------------------------------ CaseClauseNode -------------------------------

void CaseClauseNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
  if ( list )
    list->ref();
}

bool CaseClauseNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  if ( list && list->deref() )
    delete list;
  return Node::deref();
}

// ECMA 12.11
ValueImp *CaseClauseNode::evaluate(ExecState *exec)
{
  ValueImp *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTIONVALUE

  return v;
}

// ECMA 12.11
Completion CaseClauseNode::evalStatements(ExecState *exec)
{
  if (list)
    return list->execute(exec);
  else
    return Completion(Normal, Undefined());
}

void CaseClauseNode::processVarDecls(ExecState *exec)
{
  if (list)
    list->processVarDecls(exec);
}

// ------------------------------ ClauseListNode -------------------------------

void ClauseListNode::ref()
{
  for (ClauseListNode *n = this; n; n = n->nx) {
    n->Node::ref();
    if (n->cl)
      n->cl->ref();
  }
}

bool ClauseListNode::deref()
{
  ClauseListNode *next;
  for (ClauseListNode *n = this; n; n = next) {
    next = n->nx;
    if (n->cl && n->cl->deref())
      delete n->cl;
    if (n != this && n->Node::deref())
      delete n;
  }
  return Node::deref();
}

ValueImp *ClauseListNode::evaluate(ExecState */*exec*/)
{
  /* should never be called */
  assert(false);
  return NULL;
}

// ECMA 12.11
void ClauseListNode::processVarDecls(ExecState *exec)
{
  for (ClauseListNode *n = this; n; n = n->nx)
    if (n->cl)
      n->cl->processVarDecls(exec);
}

// ------------------------------ CaseBlockNode --------------------------------

CaseBlockNode::CaseBlockNode(ClauseListNode *l1, CaseClauseNode *d,
                             ClauseListNode *l2)
{
  if (l1) {
    list1 = l1->nx;
    l1->nx = 0;
  } else {
    list1 = 0;
  }

  def = d;

  if (l2) {
    list2 = l2->nx;
    l2->nx = 0;
  } else {
    list2 = 0;
  }
}
 
void CaseBlockNode::ref()
{
  Node::ref();
  if ( def )
    def->ref();
  if ( list1 )
    list1->ref();
  if ( list2 )
    list2->ref();
}

bool CaseBlockNode::deref()
{
  if ( def && def->deref() )
    delete def;
  if ( list1 && list1->deref() )
    delete list1;
  if ( list2 && list2->deref() )
    delete list2;
  return Node::deref();
}

ValueImp *CaseBlockNode::evaluate(ExecState */*exec*/)
{
  /* should never be called */
  assert(false);
  return NULL;
}

// ECMA 12.11
Completion CaseBlockNode::evalBlock(ExecState *exec, ValueImp *input)
{
  ValueImp *v;
  Completion res;
  ClauseListNode *a = list1, *b = list2;
  CaseClauseNode *clause;

    while (a) {
      clause = a->clause();
      a = a->next();
      v = clause->evaluate(exec);
      KJS_CHECKEXCEPTION
      if (strictEqual(exec, input, v)) {
	res = clause->evalStatements(exec);
	if (res.complType() != Normal)
	  return res;
	while (a) {
	  res = a->clause()->evalStatements(exec);
	  if (res.complType() != Normal)
	    return res;
	  a = a->next();
	}
	break;
      }
    }

  while (b) {
    clause = b->clause();
    b = b->next();
    v = clause->evaluate(exec);
    KJS_CHECKEXCEPTION
    if (strictEqual(exec, input, v)) {
      res = clause->evalStatements(exec);
      if (res.complType() != Normal)
	return res;
      goto step18;
    }
  }

  // default clause
  if (def) {
    res = def->evalStatements(exec);
    if (res.complType() != Normal)
      return res;
  }
  b = list2;
 step18:
  while (b) {
    clause = b->clause();
    res = clause->evalStatements(exec);
    if (res.complType() != Normal)
      return res;
    b = b->next();
  }

  // bail out on error
  KJS_CHECKEXCEPTION

  return Completion(Normal);
}

void CaseBlockNode::processVarDecls(ExecState *exec)
{
  if (list1)
    list1->processVarDecls(exec);
  if (def)
    def->processVarDecls(exec);
  if (list2)
    list2->processVarDecls(exec);
}

// ------------------------------ SwitchNode -----------------------------------

void SwitchNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
  if ( block )
    block->ref();
}

bool SwitchNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  if ( block && block->deref() )
    delete block;
  return Node::deref();
}

// ECMA 12.11
Completion SwitchNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  ValueImp *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTION

  exec->context().imp()->seenLabels()->pushSwitch();
  Completion res = block->evalBlock(exec,v);
  exec->context().imp()->seenLabels()->popSwitch();

  if ((res.complType() == Break) && ls.contains(res.target()))
    return Completion(Normal, res.value());
  return res;
}

void SwitchNode::processVarDecls(ExecState *exec)
{
  block->processVarDecls(exec);
}

// ------------------------------ LabelNode ------------------------------------

void LabelNode::ref()
{
  Node::ref();
  if ( statement )
    statement->ref();
}

bool LabelNode::deref()
{
  if ( statement && statement->deref() )
    delete statement;
  return Node::deref();
}

// ECMA 12.12
Completion LabelNode::execute(ExecState *exec)
{
  Completion e;

  if (!exec->context().imp()->seenLabels()->push(label)) {
    return Completion( Throw,
		       throwError(exec, SyntaxError, "Duplicated label %s found.", label));
  };
  e = statement->execute(exec);
  exec->context().imp()->seenLabels()->pop();

  if ((e.complType() == Break) && (e.target() == label))
    return Completion(Normal, e.value());
  return e;
}

void LabelNode::processVarDecls(ExecState *exec)
{
  statement->processVarDecls(exec);
}

// ------------------------------ ThrowNode ------------------------------------

void ThrowNode::ref()
{
  Node::ref();
  if ( expr )
    expr->ref();
}

bool ThrowNode::deref()
{
  if ( expr && expr->deref() )
    delete expr;
  return Node::deref();
}

// ECMA 12.13
Completion ThrowNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  ValueImp *v = expr->evaluate(exec);
  KJS_CHECKEXCEPTION

  return Completion(Throw, v);
}

// ------------------------------ CatchNode ------------------------------------

void CatchNode::ref()
{
  Node::ref();
  if ( block )
    block->ref();
}

bool CatchNode::deref()
{
  if ( block && block->deref() )
    delete block;
  return Node::deref();
}

Completion CatchNode::execute(ExecState */*exec*/)
{
  // should never be reached. execute(exec, arg) is used instead
  assert(0L);
  return Completion();
}

// ECMA 12.14
Completion CatchNode::execute(ExecState *exec, ValueImp *arg)
{
  /* TODO: correct ? Not part of the spec */

  exec->clearException();

  ObjectImp *obj(new ObjectImp());
  obj->put(exec, ident, arg, DontDelete);
  exec->context().imp()->pushScope(obj);
  Completion c = block->execute(exec);
  exec->context().imp()->popScope();

  return c;
}

void CatchNode::processVarDecls(ExecState *exec)
{
  block->processVarDecls(exec);
}

// ------------------------------ FinallyNode ----------------------------------

void FinallyNode::ref()
{
  Node::ref();
  if ( block )
    block->ref();
}

bool FinallyNode::deref()
{
  if ( block && block->deref() )
    delete block;
  return Node::deref();
}

// ECMA 12.14
Completion FinallyNode::execute(ExecState *exec)
{
  return block->execute(exec);
}

void FinallyNode::processVarDecls(ExecState *exec)
{
  block->processVarDecls(exec);
}

// ------------------------------ TryNode --------------------------------------

void TryNode::ref()
{
  Node::ref();
  if ( block )
    block->ref();
  if ( _final )
    _final->ref();
  if ( _catch )
    _catch->ref();
}

bool TryNode::deref()
{
  if ( block && block->deref() )
    delete block;
  if ( _final && _final->deref() )
    delete _final;
  if ( _catch && _catch->deref() )
    delete _catch;
  return Node::deref();
}

// ECMA 12.14
Completion TryNode::execute(ExecState *exec)
{
  KJS_BREAKPOINT;

  Completion c, c2;

  c = block->execute(exec);

  if (!_final) {
    if (c.complType() != Throw)
      return c;
    return _catch->execute(exec,c.value());
  }

  if (!_catch) {
    ValueImp *lastException = exec->exception();
    exec->clearException();
    
    c2 = _final->execute(exec);
    
    if (!exec->hadException())
      exec->setException(lastException);
    
    return (c2.complType() == Normal) ? c : c2;
  }

  if (c.complType() == Throw)
    c = _catch->execute(exec,c.value());

  c2 = _final->execute(exec);
  return (c2.complType() == Normal) ? c : c2;
}

void TryNode::processVarDecls(ExecState *exec)
{
  block->processVarDecls(exec);
  if (_final)
    _final->processVarDecls(exec);
  if (_catch)
    _catch->processVarDecls(exec);
}

// ------------------------------ ParameterNode --------------------------------

void ParameterNode::ref()
{
  for (ParameterNode *n = this; n; n = n->next)
    n->Node::ref();
}

bool ParameterNode::deref()
{
  ParameterNode *next;
  for (ParameterNode *n = this; n; n = next) {
    next = n->next;
    if (n != this && n->Node::deref())
      delete n;
  }
  return Node::deref();
}

// ECMA 13
ValueImp *ParameterNode::evaluate(ExecState */*exec*/)
{
  return Undefined();
}

// ------------------------------ FunctionBodyNode -----------------------------

FunctionBodyNode::FunctionBodyNode(SourceElementsNode *s)
  : BlockNode(s)
{
  setLoc(-1, -1, -1);
  //fprintf(stderr,"FunctionBodyNode::FunctionBodyNode %p\n",this);
}

void FunctionBodyNode::processFuncDecl(ExecState *exec)
{
  if (source)
    source->processFuncDecl(exec);
}

// ------------------------------ FuncDeclNode ---------------------------------

void FuncDeclNode::ref()
{
  Node::ref();
  if ( param )
    param->ref();
  if ( body )
    body->ref();
}

bool FuncDeclNode::deref()
{
  if ( param && param->deref() )
    delete param;
  if ( body && body->deref() )
    delete body;
  return Node::deref();
}

// ECMA 13
void FuncDeclNode::processFuncDecl(ExecState *exec)
{
  ContextImp *context = exec->context().imp();

  // TODO: let this be an object with [[Class]] property "Function"
  FunctionImp *fimp = new DeclaredFunctionImp(exec, ident, body, context->scopeChain());
  ObjectImp *func(fimp); // protect from GC

  ObjectImp *proto = exec->lexicalInterpreter()->builtinObject()->construct(exec, List::empty());
  proto->put(exec, constructorPropertyName, func, ReadOnly|DontDelete|DontEnum);
  func->put(exec, prototypePropertyName, proto, Internal|DontDelete);

  int plen = 0;
  for(ParameterNode *p = param; p != 0L; p = p->nextParam(), plen++)
    fimp->addParameter(p->ident());

  func->put(exec, lengthPropertyName, Number(plen), ReadOnly|DontDelete|DontEnum);

  // ECMA 10.2.2
  context->variableObject()->put(exec, ident, func, Internal | (context->codeType() == EvalCode ? 0 : DontDelete));

  if (body) {
    // hack the scope so that the function gets put as a property of func, and it's scope
    // contains the func as well as our current scope
    ObjectImp *oldVar = context->variableObject();
    context->setVariableObject(func);
    context->pushScope(func);
    body->processFuncDecl(exec);
    context->popScope();
    context->setVariableObject(oldVar);
  }
}

// ------------------------------ FuncExprNode ---------------------------------

void FuncExprNode::ref()
{
  Node::ref();
  if ( param )
    param->ref();
  if ( body )
    body->ref();
}

bool FuncExprNode::deref()
{
  if ( param && param->deref() )
    delete param;
  if ( body && body->deref() )
    delete body;
  return Node::deref();
}


// ECMA 13
ValueImp *FuncExprNode::evaluate(ExecState *exec)
{
  FunctionImp *fimp = new DeclaredFunctionImp(exec, Identifier::null(), body, exec->context().imp()->scopeChain());
  ValueImp *ret(fimp);
  ValueImp *proto = exec->lexicalInterpreter()->builtinObject()->construct(exec, List::empty());
  fimp->put(exec, prototypePropertyName, proto, Internal|DontDelete);

  int plen = 0;
  for(ParameterNode *p = param; p != 0L; p = p->nextParam(), plen++)
    fimp->addParameter(p->ident());

  return ret;
}

// ------------------------------ SourceElementsNode ---------------------------

SourceElementsNode::SourceElementsNode(StatementNode *s1)
  : element(s1), elements(this)
{
  setLoc(s1->firstLine(), s1->lastLine(), s1->sourceId());
}
 
SourceElementsNode::SourceElementsNode(SourceElementsNode *s1, StatementNode *s2)
  : element(s2), elements(s1->elements)
{
  s1->elements = this;
  setLoc(s1->firstLine(), s2->lastLine(), s1->sourceId());
}

void SourceElementsNode::ref()
{
  for (SourceElementsNode *n = this; n; n = n->elements) {
    n->Node::ref();
    if (n->element)
      n->element->ref();
  }
}

bool SourceElementsNode::deref()
{
  SourceElementsNode *next;
  for (SourceElementsNode *n = this; n; n = next) {
    next = n->elements;
    if (n->element && n->element->deref())
      delete n->element;
    if (n != this && n->Node::deref())
      delete n;
  }
  return Node::deref();
}

// ECMA 14
Completion SourceElementsNode::execute(ExecState *exec)
{
  KJS_CHECKEXCEPTION

  Completion c1 = element->execute(exec);
  KJS_CHECKEXCEPTION;
  if (c1.complType() != Normal)
    return c1;
  
  for (SourceElementsNode *n = elements; n; n = n->elements) {
    Completion c2 = n->element->execute(exec);
    if (c2.complType() != Normal)
      return c2;
    // The spec says to return c2 here, but it seems that mozilla returns c1 if
    // c2 doesn't have a value
    if (c2.value())
      c1 = c2;
  }
  
  return c1;
}

// ECMA 14
void SourceElementsNode::processFuncDecl(ExecState *exec)
{
  for (SourceElementsNode *n = this; n; n = n->elements)
    n->element->processFuncDecl(exec);
}

void SourceElementsNode::processVarDecls(ExecState *exec)
{
  for (SourceElementsNode *n = this; n; n = n->elements)
    n->element->processVarDecls(exec);
}

ProgramNode::ProgramNode(SourceElementsNode *s): FunctionBodyNode(s) {
    //fprintf(stderr,"ProgramNode::ProgramNode %p\n",this);
}
