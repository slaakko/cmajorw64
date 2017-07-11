// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_SYMBOL_INCLUDED
#include <cmajor/ast/Specifier.hpp>
#include <cmajor/parsing/Scanner.hpp>
#include <stdint.h>

namespace cmajor { namespace symbols {

using cmajor::parsing::Span;
using namespace cmajor::ast;

class SymbolWriter;
class SymbolReader;
class ContainerScope;
class ContainerSymbol;
class TypeSymbol;
class ClassTypeSymbol;
class InterfaceTypeSymbol;
class NamespaceSymbol;
class FunctionSymbol;
class SymbolTable;

enum class SymbolType : uint8_t
{
    boolTypeSymbol, sbyteTypeSymbol, byteTypeSymbol, shortTypeSymbol, ushortTypeSymbol, intTypeSymbol, uintTypeSymbol, longTypeSymbol, ulongTypeSymbol, floatTypeSymbol, doubleTypeSymbol, 
    charTypeSymbol, wcharTypeSymbol, ucharTypeSymbol, voidTypeSymbol,
    derivedTypeSymbol,
    namespaceSymbol, functionSymbol, staticConstructorSymbol, constructorSymbol, destructorSymbol, memberFunctionSymbol, functionGroupSymbol, classTypeSymbol, interfaceTypeSymbol, 
    delegateTypeSymbol, classDelegateTypeSymbol, declarationBlock, typedefSymbol, constantSymbol, enumTypeSymbol, enumConstantSymbol,
    templateParameterSymbol,  parameterSymbol, localVariableSymbol, memberVariableSymbol,
    maxSymbol
};

std::string SymbolTypeStr(SymbolType symbolType);

enum class SymbolAccess : uint8_t
{
    private_ = 0, protected_ = 1, internal_ = 2, public_ = 3
};

enum class SymbolFlags : uint8_t
{
    none = 0, 
    access = 1 << 0 | 1 << 1,
    static_ = 1 << 2,
    nothrow_ = 1 << 3,
    project = 1 << 4,
    bound = 1 << 5
};

inline SymbolFlags operator&(SymbolFlags left, SymbolFlags right)
{
    return SymbolFlags(uint8_t(left) & uint8_t(right));
}

inline SymbolFlags operator|(SymbolFlags left, SymbolFlags right)
{
    return SymbolFlags(uint8_t(left) | uint8_t(right));
}

inline SymbolFlags operator~(SymbolFlags flags)
{
    return SymbolFlags(~uint8_t(flags));
}

std::string SymbolFlagStr(SymbolFlags symbolFlags);

class Symbol
{
public:
    Symbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_);
    virtual ~Symbol();
    virtual void Write(SymbolWriter& writer);
    virtual void Read(SymbolReader& reader);
    virtual void EmplaceType(TypeSymbol* typeSymbol, int index) {}
    virtual bool IsExportSymbol() const { return IsProject(); }
    virtual bool IsContainerSymbol() const { return false; }
    virtual bool IsFunctionSymbol() const { return false; }
    virtual bool IsClassTypeSymbol() const { return false; }
    virtual const ContainerScope* GetContainerScope() const { return nullptr; }
    virtual ContainerScope* GetContainerScope() { return nullptr; }
    virtual std::u32string FullName() const;
    virtual std::u32string FullNameWithSpecifiers() const;
    void SetAccess(SymbolAccess access_) { flags = flags | SymbolFlags(access_); }
    void SetAccess(Specifiers accessSpecifiers);
    SymbolType GetSymbolType() const { return symbolType; }
    const Span& GetSpan() const { return span; }
    void SetSpan(const Span& span_) { span = span_; }
    const std::u32string& Name() const { return name; }
    void SetName(const std::u32string& name_) { name = name_; }
    SymbolFlags GetSymbolFlags() const { return flags; }
    bool IsStatic() const { return GetFlag(SymbolFlags::static_); }
    void SetStatic() { SetFlag(SymbolFlags::static_); }
    bool IsNothrow() const { return GetFlag(SymbolFlags::nothrow_); }
    void SetNothrow() { SetFlag(SymbolFlags::nothrow_); }
    bool IsProject() const { return GetFlag(SymbolFlags::project); }
    void SetProject() { SetFlag(SymbolFlags::project); }
    bool IsBound() const { return GetFlag(SymbolFlags::bound); }
    void SetBound() { SetFlag(SymbolFlags::bound); }
    bool GetFlag(SymbolFlags flag) const { return (flags & flag) != SymbolFlags::none; }
    void SetFlag(SymbolFlags flag) { flags = flags | flag; }
    void ResetFlag(SymbolFlags flag) { flags = flags & ~flag; }
    const Symbol* Parent() const { return parent; }
    void SetParent(Symbol* parent_) { parent = parent_; }
    const NamespaceSymbol* Ns() const;
    NamespaceSymbol* Ns();
    const ClassTypeSymbol* ClassNoThrow() const;
    ClassTypeSymbol* ClassNoThrow();
    const ClassTypeSymbol* Class() const;
    ClassTypeSymbol* Class();
    const ClassTypeSymbol* ContainingClassNoThrow() const;
    ClassTypeSymbol* ContainingClassNoThrow();
    const InterfaceTypeSymbol* InterfaceNoThrow() const;
    InterfaceTypeSymbol* InterfaceNoThrow();
    const InterfaceTypeSymbol* ContainingInterfaceNoThrow() const;
    InterfaceTypeSymbol* ContainingInterfaceNoThrow() ;
    const FunctionSymbol* Function() const;
    FunctionSymbol* Function();
    const SymbolTable* GetSymbolTable() const { return symbolTable; }
    SymbolTable* GetSymbolTable() { return symbolTable; }
    void SetSymbolTable(SymbolTable* symbolTable_) { symbolTable = symbolTable_; }
private:
    SymbolType symbolType;
    Span span;
    std::u32string name;
    SymbolFlags flags;
    Symbol* parent;
    SymbolTable* symbolTable;
};

class SymbolCreator
{
public:
    virtual ~SymbolCreator();
    virtual Symbol* CreateSymbol(const Span& span, const std::u32string& name) = 0;
};

class SymbolFactory
{
public:
    static void Init();
    static void Done();
    static SymbolFactory& Instance() { Assert(instance, "symbol factory not initialized"); return *instance; }
    Symbol* CreateSymbol(SymbolType symbolType, const Span& span, const std::u32string& name);
    void Register(SymbolType symbolType, SymbolCreator* creator);
private:
    static std::unique_ptr<SymbolFactory> instance;
    std::vector<std::unique_ptr<SymbolCreator>> symbolCreators;
    SymbolFactory();
};

void InitSymbol();
void DoneSymbol();

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_SYMBOL_INCLUDED
