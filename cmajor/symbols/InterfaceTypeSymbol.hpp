// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_INTERFACE_TYPE_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_INTERFACE_TYPE_SYMBOL_INCLUDED
#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/ClassTypeSymbol.hpp>

namespace cmajor { namespace symbols {

class MemberFunctionSymbol;

class InterfaceTypeSymbol : public TypeSymbol
{
public:
    InterfaceTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "interface"; }
    void AddMember(Symbol* member) override;
    void Accept(SymbolCollector* collector) override;
    void SetSpecifiers(Specifiers specifiers);
    llvm::Type* IrType(Emitter& emitter) override;
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override;
    const std::vector<MemberFunctionSymbol*>& MemberFunctions() const { return memberFunctions; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags, MemberFunctionSymbol* interfaceMemberFunction);
private:
    std::vector<MemberFunctionSymbol*> memberFunctions;
    llvm::Type* irType;
};

class InterfaceTypeDefaultConstructor : public FunctionSymbol
{
public:
    InterfaceTypeDefaultConstructor(InterfaceTypeSymbol* interfaceType_, const Span& span_);
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
};

class InterfaceTypeCopyConstructor : public FunctionSymbol
{
public:
    InterfaceTypeCopyConstructor(InterfaceTypeSymbol* interfaceType_, const Span& span_);
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
};

class InterfaceTypeMoveConstructor : public FunctionSymbol
{
public:
    InterfaceTypeMoveConstructor(InterfaceTypeSymbol* interfaceType_, const Span& span_);
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
};

class InterfaceTypeCopyAssignment : public FunctionSymbol
{
public:
    InterfaceTypeCopyAssignment(InterfaceTypeSymbol* interfaceType_, const Span& span_);
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
};

class InterfaceTypeMoveAssignment : public FunctionSymbol
{
public:
    InterfaceTypeMoveAssignment(InterfaceTypeSymbol* interfaceType_, const Span& span_);
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
};

class ClassToInterfaceConversion : public FunctionSymbol
{
public:
    ClassToInterfaceConversion(ClassTypeSymbol* sourceClassType_, InterfaceTypeSymbol* targetInterfaceType_, LocalVariableSymbol* temporaryInterfaceObjectVar_, int32_t interfaceIndex_, const Span& span_);
    ConversionType GetConversionType() const override { return ConversionType::implicit_; }
    uint8_t ConversionDistance() const override { return 1; }
    TypeSymbol* ConversionSourceType() const override { return sourceClassType->AddLvalueReference(Span()); }
    TypeSymbol* ConversionTargetType() const override { return targetInterfaceType; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
    bool IsClassToInterfaceTypeConversion() const override {return true; }
private:
    ClassTypeSymbol* sourceClassType;
    InterfaceTypeSymbol* targetInterfaceType;
    LocalVariableSymbol* temporaryInterfaceObjectVar;
    int32_t interfaceIndex;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_INTERFACE_TYPE_SYMBOL_INCLUDED
