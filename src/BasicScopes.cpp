//
//  BasicScopes.cpp
//  Turing Compiler
//
//  Created by Tristan Hume on 12-02-07.
//  Copyright 2012 15 Norwich Way. All rights reserved.
//

#include "BasicScopes.h"

#include <llvm/Support/IRBuilder.h>
#include <llvm/Constant.h>

#include "Message.h"

using namespace llvm;

Symbol *BasicScope::resolveVarThis(std::string name) {
    if (symbols.find(name) == symbols.end()) {
        throw Message::Exception(Twine("Could not find variable or function \"") + name + "\".");
    }
    return symbols[name];
}

//! sets a variable name to reference a specific value
void BasicScope::setVar(std::string name, llvm::Value *val, TuringType *type) {
    // TODO check if it already exists?
    symbols[name] = new VarSymbol(val,type);
}

void BasicScope::setVar(std::string name, Symbol *val) {
    // TODO check if it already exists?
    symbols[name] = val;
}

bool BasicScope::isDeclaredThis(std::string name) {
    return symbols.find(name) != symbols.end();
}

Symbol *BasicScope::declareVar(std::string name, TuringType *type) {
    if (isDeclaredThis(name)) {
        throw Message::Exception(Twine("Variable ") + name + " is already defined.");
    }
    
    // prefix the scope name to avoid clashes at the LLVM level
    std::string llvmName = (Twine(getScopeName()) + name).str();
    Value *var = allocateSpace(type, llvmName);
    
    Symbol *sym = new VarSymbol(var,type);
    
    // store in the symbol table
    symbols[name] = sym;
    
    return sym;
}

GlobalScope::GlobalScope(llvm::Module *mod, Scope *parent) : BasicScope(parent), TheModule(mod) {
    
}

Scope *GlobalScope::createChildScope() {
    return new GlobalScope(TheModule,this);
}

Value *GlobalScope::allocateSpace(TuringType *type, const std::string &name) {
    return new GlobalVariable(/*Module=*/*TheModule, 
                              /*Type=*/type->getLLVMType(false),
                              /*isConstant=*/false,
                              /*Linkage=*/GlobalValue::CommonLinkage,
                              /*Initializer=*/Constant::getNullValue(type->getLLVMType(false)), // has initializer, specified below
                              /*Name=*/name);
}

LocalScope::LocalScope(llvm::Function *func, Scope *parent) : BasicScope(parent), TheFunction(func)  {}

Scope *LocalScope::createChildScope() {
    return new LocalScope(TheFunction,this);
}

Value *LocalScope::allocateSpace(TuringType *type, const std::string &name) {
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                     TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(type->getLLVMType(false), 0,name);
}