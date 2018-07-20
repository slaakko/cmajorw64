// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/BoundCompileUnit.hpp>
#include <cmajor/binder/BoundNamespace.hpp>
#include <cmajor/binder/BoundNodeVisitor.hpp>
#include <cmajor/binder/StatementBinder.hpp>
#include <cmajor/binder/BoundStatement.hpp>
#include <cmajor/binder/BoundFunction.hpp>
#include <cmajor/binder/OverloadResolution.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/ClassTypeSymbol.hpp>
#include <cmajor/symbols/GlobalFlags.hpp>
#include <cmajor/symbols/DelegateSymbol.hpp>
#include <cmajor/symbols/InterfaceTypeSymbol.hpp>
#include <cmajor/util/Path.hpp>
#include <boost/filesystem.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::util;

class ClassTypeConversion : public FunctionSymbol
{
public:
    ClassTypeConversion(const std::u32string& name_, ConversionType conversionType_, uint8_t conversionDistance_, TypeSymbol* sourceType_, TypeSymbol* targetType_);
    ConversionType GetConversionType() const override { return conversionType; }
    uint8_t ConversionDistance() const override { return conversionDistance; }
    TypeSymbol* ConversionSourceType() const { return sourceType; }
    TypeSymbol* ConversionTargetType() const { return targetType; }
    bool IsBasicTypeOperation() const override { return true; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags, const Span& span) override;
    const char* ClassName() const override { return "ClassTypeConversion"; }
private:
    ConversionType conversionType;
    uint8_t conversionDistance;
    TypeSymbol* sourceType;
    TypeSymbol* targetType;
};

ClassTypeConversion::ClassTypeConversion(const std::u32string& name_, ConversionType conversionType_, uint8_t conversionDistance_, TypeSymbol* sourceType_, TypeSymbol* targetType_) : 
    FunctionSymbol(Span(), name_), conversionType(conversionType_), conversionDistance(conversionDistance_), sourceType(sourceType_), targetType(targetType_)
{
    SetConversion();
    SetGroupName(U"@conversion");
    SetAccess(SymbolAccess::public_);
}

void ClassTypeConversion::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags, const Span& span)
{
    emitter.SetCurrentDebugLocation(span);
    llvm::Value* value = emitter.Stack().Pop();
    emitter.Stack().Push(emitter.Builder().CreateBitCast(value, targetType->IrType(emitter)));
}

class NullPtrToPtrConversion : public FunctionSymbol
{
public:
    NullPtrToPtrConversion(TypeSymbol* nullPtrType_, TypeSymbol* targetPointerType_);
    ConversionType GetConversionType() const override { return ConversionType::implicit_; }
    uint8_t ConversionDistance() const override { return 1; }
    TypeSymbol* ConversionSourceType() const { return nullPtrType; }
    TypeSymbol* ConversionTargetType() const { return targetPointerType; }
    bool IsBasicTypeOperation() const override { return true; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags, const Span& span) override;
    std::unique_ptr<Value> ConvertValue(const std::unique_ptr<Value>& value) const override;
    const char* ClassName() const override { return "NullPtrToPtrConversion"; }
private:
    TypeSymbol* nullPtrType;
    TypeSymbol* targetPointerType;
};

NullPtrToPtrConversion::NullPtrToPtrConversion(TypeSymbol* nullPtrType_, TypeSymbol* targetPointerType_) :
    FunctionSymbol(Span(), U"nullptr2ptr"), nullPtrType(nullPtrType_), targetPointerType(targetPointerType_)
{
    SetConversion();
    SetGroupName(U"@conversion");
    SetAccess(SymbolAccess::public_);
}

void NullPtrToPtrConversion::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags, const Span& span)
{
    emitter.SetCurrentDebugLocation(span);
    llvm::Value* value = emitter.Stack().Pop();
    emitter.Stack().Push(emitter.Builder().CreateBitCast(value, targetPointerType->IrType(emitter)));
}

std::unique_ptr<Value> NullPtrToPtrConversion::ConvertValue(const std::unique_ptr<Value>& value) const
{
    TypeSymbol* type = value->GetType(nullPtrType->GetSymbolTable());
    if (type->IsPointerType())
    {
        return std::unique_ptr<Value>(new PointerValue(value->GetSpan(), type, nullptr));
    }
    else
    {
        return std::unique_ptr<Value>();
    }
}

class VoidPtrToPtrConversion : public FunctionSymbol
{
public:
    VoidPtrToPtrConversion(TypeSymbol* voidPtrType_, TypeSymbol* targetPointerType_);
    ConversionType GetConversionType() const override { return ConversionType::explicit_; }
    uint8_t ConversionDistance() const override { return 255; }
    TypeSymbol* ConversionSourceType() const { return voidPtrType; }
    TypeSymbol* ConversionTargetType() const { return targetPointerType; }
    bool IsBasicTypeOperation() const override { return true; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags, const Span& span) override;
    const char* ClassName() const override { return "VoidPtrToPtrConversion"; }
private:
    TypeSymbol* voidPtrType;
    TypeSymbol* targetPointerType;
};

VoidPtrToPtrConversion::VoidPtrToPtrConversion(TypeSymbol* voidPtrType_, TypeSymbol* targetPointerType_) :
    FunctionSymbol(Span(), U"voidPtr2ptr"), voidPtrType(voidPtrType_), targetPointerType(targetPointerType_)
{
    SetConversion();
    SetGroupName(U"@conversion");
    SetAccess(SymbolAccess::public_);
}

void VoidPtrToPtrConversion::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags, const Span& span)
{
    emitter.SetCurrentDebugLocation(span);
    llvm::Value* value = emitter.Stack().Pop();
    emitter.Stack().Push(emitter.Builder().CreateBitCast(value, targetPointerType->IrType(emitter)));
}

class PtrToVoidPtrConversion : public FunctionSymbol
{
public:
    PtrToVoidPtrConversion(TypeSymbol* sourcePtrType_, TypeSymbol* voidPtrType_);
    ConversionType GetConversionType() const override { return ConversionType::implicit_; }
    uint8_t ConversionDistance() const override { return 10; }
    TypeSymbol* ConversionSourceType() const { return sourcePtrType; }
    TypeSymbol* ConversionTargetType() const { return voidPtrType; }
    bool IsBasicTypeOperation() const override { return true; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags, const Span& span) override;
    const char* ClassName() const override { return "PtrToVoidPtrConversion"; }
private:
    TypeSymbol* sourcePtrType;
    TypeSymbol* voidPtrType;
};

PtrToVoidPtrConversion::PtrToVoidPtrConversion(TypeSymbol* sourcePtrType_, TypeSymbol* voidPtrType_) :
    FunctionSymbol(Span(), U"ptr2voidPtr"), sourcePtrType(sourcePtrType_), voidPtrType(voidPtrType_)
{
    SetConversion();
    SetGroupName(U"@conversion");
    SetAccess(SymbolAccess::public_);
}

void PtrToVoidPtrConversion::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags, const Span& span)
{
    emitter.SetCurrentDebugLocation(span);
    llvm::Value* value = emitter.Stack().Pop();
    emitter.Stack().Push(emitter.Builder().CreateBitCast(value, voidPtrType->IrType(emitter)));
}

class PtrToULongConversion : public FunctionSymbol
{
public:
    PtrToULongConversion(TypeSymbol* otrType_, TypeSymbol* ulongType_);
    ConversionType GetConversionType() const override { return ConversionType::explicit_; }
    uint8_t ConversionDistance() const override { return 255; }
    TypeSymbol* ConversionSourceType() const { return ptrType; }
    TypeSymbol* ConversionTargetType() const { return ulongType; }
    bool IsBasicTypeOperation() const override { return true; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags, const Span& span) override;
    const char* ClassName() const override { return "PtrToULongConversion"; }
private:
    TypeSymbol* ptrType;
    TypeSymbol* ulongType;
};

PtrToULongConversion::PtrToULongConversion(TypeSymbol* ptrType_, TypeSymbol* ulongType_) :
    FunctionSymbol(Span(), U"ptr2ulong"), ptrType(ptrType_), ulongType(ulongType_)
{
    SetConversion();
    SetGroupName(U"@conversion");
    SetAccess(SymbolAccess::public_);
}

void PtrToULongConversion::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags, const Span& span)
{
    emitter.SetCurrentDebugLocation(span);
    llvm::Value* value = emitter.Stack().Pop();
    emitter.Stack().Push(emitter.Builder().CreatePtrToInt(value, ulongType->IrType(emitter)));
}

BoundCompileUnit::BoundCompileUnit(Module& module_, CompileUnitNode* compileUnitNode_, AttributeBinder* attributeBinder_) :
    BoundNode(&module_, Span(), BoundNodeType::boundCompileUnit), module(module_), symbolTable(module.GetSymbolTable()), compileUnitNode(compileUnitNode_), attributeBinder(attributeBinder_), currentNamespace(nullptr), 
    hasGotos(false), operationRepository(*this), functionTemplateRepository(*this), classTemplateRepository(*this), inlineFunctionRepository(*this), constExprFunctionRepository(*this), bindingTypes(false),
    compileUnitIndex(-2)
{
    boost::filesystem::path fileName = boost::filesystem::path(compileUnitNode->FilePath()).filename();
    boost::filesystem::path directory = module.DirectoryPath();
    boost::filesystem::path llfp = (directory / fileName).replace_extension(".ll");
    boost::filesystem::path optllfp = (directory / fileName).replace_extension(".opt.ll");
#ifdef _WIN32
    boost::filesystem::path objfp = (directory / fileName).replace_extension(".obj");
#else
    boost::filesystem::path objfp = (directory / fileName).replace_extension(".o");
#endif
    llFilePath = GetFullPath(llfp.generic_string());
    optLLFilePath = GetFullPath(optllfp.generic_string());
    objectFilePath = GetFullPath(objfp.generic_string());
}

void BoundCompileUnit::Load(Emitter& emitter, OperationFlags flags)
{
    throw Exception(&GetModule(), "cannot load from compile unit", GetSpan());
}

void BoundCompileUnit::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception(&GetModule(), "cannot store to compile unit", GetSpan());
}

void BoundCompileUnit::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

void BoundCompileUnit::AddFileScope(FileScope* fileScope)
{
    fileScopes.push_back(std::unique_ptr<FileScope>(fileScope));
}

void BoundCompileUnit::RemoveLastFileScope()
{
    if (fileScopes.empty())
    {
        throw Exception(&GetModule(), "file scopes of bound compile unit is empty", GetSpan());
    }
    fileScopes.erase(fileScopes.end() - 1);
}

FileScope* BoundCompileUnit::ReleaseLastFileScope()
{
    if (fileScopes.empty())
    {
        throw Exception(&GetModule(), "file scopes of bound compile unit is empty", GetSpan());
    }
    FileScope* fileScope = fileScopes.back().release();
    RemoveLastFileScope();
    return fileScope;
}

void BoundCompileUnit::AddBoundNode(std::unique_ptr<BoundNode>&& boundNode)
{
    if (currentNamespace)
    {
        currentNamespace->AddMember(std::move(boundNode));
    }
    else
    {
        boundNodes.push_back(std::move(boundNode));
    }
}

FunctionSymbol* BoundCompileUnit::GetConversion(TypeSymbol* sourceType, TypeSymbol* targetType, ContainerScope* containerScope, BoundFunction* currentFunction, const Span& span, ArgumentMatch& argumentMatch)
{
    FunctionSymbol* conversion = symbolTable.GetConversion(sourceType, targetType, span);
    if (conversion && conversion->GetSymbolType() == SymbolType::conversionFunctionSymbol)
    {
        argumentMatch.preReferenceConversionFlags = OperationFlags::addr;
    }
    if (!conversion)
    {
        conversion = conversionTable.GetConversion(sourceType, targetType, span);
        if (!conversion)
        {
            if (sourceType->IsNullPtrType() && targetType->IsPointerType() && !targetType->IsReferenceType())
            {
                std::unique_ptr<FunctionSymbol> nullPtrToPtrConversion(new NullPtrToPtrConversion(symbolTable.GetTypeByName(U"@nullptr_type"), targetType));
                conversion = nullPtrToPtrConversion.get();
                conversionTable.AddConversion(conversion);
                conversionTable.AddGeneratedConversion(std::move(nullPtrToPtrConversion));
                return conversion;
            }
            else if (sourceType->IsVoidPtrType() && targetType->IsPointerType() && !targetType->IsReferenceType())
            {
                std::unique_ptr<FunctionSymbol> voidPtrToPtrConversion(new VoidPtrToPtrConversion(symbolTable.GetTypeByName(U"void")->AddPointer(span), targetType));
                conversion = voidPtrToPtrConversion.get();
                conversionTable.AddConversion(conversion);
                conversionTable.AddGeneratedConversion(std::move(voidPtrToPtrConversion));
                return conversion;
            }
            else if (sourceType->PlainType(span)->IsPointerType() && targetType == symbolTable.GetTypeByName(U"ulong"))
            {
                std::unique_ptr<FunctionSymbol> ptrToULongConversion(new PtrToULongConversion(sourceType->PlainType(span), symbolTable.GetTypeByName(U"ulong")));
                conversion = ptrToULongConversion.get();
                conversionTable.AddConversion(conversion);
                conversionTable.AddGeneratedConversion(std::move(ptrToULongConversion));
                return conversion;
            }
            else if (sourceType->PlainType(span)->IsPointerType() && targetType->RemoveConst(span)->IsVoidPtrType())
            {
                std::unique_ptr<FunctionSymbol> ptrToVoidPtrConversion(new PtrToVoidPtrConversion(sourceType->PlainType(span), symbolTable.GetTypeByName(U"void")->AddPointer(span)));
                conversion = ptrToVoidPtrConversion.get();
                conversionTable.AddConversion(conversion);
                conversionTable.AddGeneratedConversion(std::move(ptrToVoidPtrConversion));
                return conversion;
            }
            else if (sourceType->BaseType()->IsClassTypeSymbol() && targetType->BaseType()->IsClassTypeSymbol())
            {
                if (sourceType->PointerCount() == targetType->PointerCount() &&
                    (!sourceType->IsArrayType() && !targetType->IsArrayType()))
                {
                    ClassTypeSymbol* sourceClassType = static_cast<ClassTypeSymbol*>(sourceType->BaseType());
                    ClassTypeSymbol* targetClassType = static_cast<ClassTypeSymbol*>(targetType->BaseType());
                    uint8_t conversionDistance = 0;
                    if (sourceClassType->HasBaseClass(targetClassType, conversionDistance))
                    {
                        if (targetType->IsLvalueReferenceType() && !sourceType->IsReferenceType())
                        {
                            argumentMatch.preReferenceConversionFlags = OperationFlags::addr;
                            sourceType = sourceType->AddLvalueReference(span);
                            if (targetType->IsConstType())
                            {
                                sourceType = sourceType->AddConst(span);
                            }
                        }
                        std::u32string conversionName = sourceType->FullName() + U"2" + targetType->FullName();
                        std::unique_ptr<FunctionSymbol> implicitClassTypeConversion(new ClassTypeConversion(conversionName, ConversionType::implicit_, conversionDistance, sourceType, targetType));
                        conversion = implicitClassTypeConversion.get();
                        // do not add entry to the conversion table
                        conversionTable.AddGeneratedConversion(std::move(implicitClassTypeConversion));
                        return conversion;
                    }
                    else
                    {
                        uint8_t conversionDistance = 0;
                        if (targetClassType->HasBaseClass(sourceClassType, conversionDistance))
                        {
                            if (targetType->IsLvalueReferenceType() && !sourceType->IsReferenceType())
                            {
                                argumentMatch.preReferenceConversionFlags = OperationFlags::addr;
                                sourceType = sourceType->AddLvalueReference(span);
                                if (targetType->IsConstType())
                                {
                                    sourceType = sourceType->AddConst(span);
                                }
                            }
                            std::u32string conversionName = sourceType->FullName() + U"2" + targetType->FullName();
                            std::unique_ptr<FunctionSymbol> explicitClassTypeConversion(new ClassTypeConversion(conversionName, ConversionType::explicit_, conversionDistance, sourceType, targetType));
                            conversion = explicitClassTypeConversion.get();
                            // do not add entry to the conversion table
                            conversionTable.AddGeneratedConversion(std::move(explicitClassTypeConversion));
                            return conversion;
                        }
                    }
                }
            }
            else if ((sourceType->GetSymbolType() == SymbolType::functionGroupTypeSymbol || sourceType->GetSymbolType() == SymbolType::memberExpressionTypeSymbol) && 
                targetType->GetSymbolType() == SymbolType::delegateTypeSymbol)
            {
                FunctionGroupSymbol* functionGroupSymbol = nullptr;
                BoundMemberExpression* boundMemberExpression = nullptr;
                if (sourceType->GetSymbolType() == SymbolType::functionGroupTypeSymbol)
                {
                    FunctionGroupTypeSymbol* functionGroupTypeSymbol = static_cast<FunctionGroupTypeSymbol*>(sourceType);
                    functionGroupSymbol = functionGroupTypeSymbol->FunctionGroup();
                }
                else if (sourceType->GetSymbolType() == SymbolType::memberExpressionTypeSymbol)
                {
                    MemberExpressionTypeSymbol* memberExpressionTypeSymbol = static_cast<MemberExpressionTypeSymbol*>(sourceType);
                    boundMemberExpression = static_cast<BoundMemberExpression*>(memberExpressionTypeSymbol->BoundMemberExpression());
                    if (boundMemberExpression->Member()->GetBoundNodeType() == BoundNodeType::boundFunctionGroupExpression)
                    {
                        BoundFunctionGroupExpression* boundFunctionGroupExpression = static_cast<BoundFunctionGroupExpression*>(boundMemberExpression->Member());
                        functionGroupSymbol = boundFunctionGroupExpression->FunctionGroup();
                    }
                }
                if (functionGroupSymbol)
                {
                    DelegateTypeSymbol* delegateTypeSymbol = static_cast<DelegateTypeSymbol*>(targetType);
                    int arity = delegateTypeSymbol->Arity();
                    std::unordered_set<FunctionSymbol*> viableFunctions;
                    functionGroupSymbol->CollectViableFunctions(arity, viableFunctions);
                    for (FunctionSymbol* viableFunction : viableFunctions)
                    {
                        if (viableFunction->GetSymbolType() == SymbolType::memberFunctionSymbol && !viableFunction->IsStatic()) continue;
                        bool found = true;
                        for (int i = 0; i < arity; ++i)
                        {
                            ParameterSymbol* sourceParam = viableFunction->Parameters()[i];
                            ParameterSymbol* targetParam = delegateTypeSymbol->Parameters()[i];
                            if (!TypesEqual(sourceParam->GetType(), targetParam->GetType()))
                            {
                                found = false;
                                break;
                            }
                        }
                        if (found)
                        {
                            found = TypesEqual(viableFunction->ReturnType(), delegateTypeSymbol->ReturnType());
                        }
                        if (found)
                        {
                            if (boundMemberExpression)
                            {
                                boundMemberExpression->ResetClassPtr();
                            }
                            std::unique_ptr<FunctionSymbol> functionToDelegateConversion(new FunctionToDelegateConversion(sourceType, delegateTypeSymbol, viableFunction));
                            conversion = functionToDelegateConversion.get();
                            conversionTable.AddConversion(conversion);
                            conversionTable.AddGeneratedConversion(std::move(functionToDelegateConversion));
                            return conversion;
                        }
                    }
                }
            }
            else if ((sourceType->GetSymbolType() == SymbolType::functionGroupTypeSymbol || sourceType->GetSymbolType() == SymbolType::memberExpressionTypeSymbol) && 
                targetType->PlainType(span)->GetSymbolType() == SymbolType::classDelegateTypeSymbol && currentFunction)
            {
                ClassDelegateTypeSymbol* classDelegateType = static_cast<ClassDelegateTypeSymbol*>(targetType->PlainType(span));
                FunctionGroupSymbol* functionGroup = nullptr;
                if (sourceType->GetSymbolType() == SymbolType::functionGroupTypeSymbol)
                {
                    FunctionGroupTypeSymbol* functionGroupTypeSymbol = static_cast<FunctionGroupTypeSymbol*>(sourceType);
                    BoundFunctionGroupExpression* boundFunctionGroup = static_cast<BoundFunctionGroupExpression*>(functionGroupTypeSymbol->BoundFunctionGroup());
                    functionGroup = functionGroupTypeSymbol->FunctionGroup();
                }
                else if (sourceType->GetSymbolType() == SymbolType::memberExpressionTypeSymbol)
                {
                    MemberExpressionTypeSymbol* memberExpressionType = static_cast<MemberExpressionTypeSymbol*>(sourceType);
                    BoundMemberExpression* boundMemberExpr = static_cast<BoundMemberExpression*>(memberExpressionType->BoundMemberExpression());
                    if (boundMemberExpr->Member()->GetBoundNodeType() == BoundNodeType::boundFunctionGroupExpression)
                    {
                        BoundFunctionGroupExpression* boundFunctionGroup = static_cast<BoundFunctionGroupExpression*>(boundMemberExpr->Member());
                        functionGroup = boundFunctionGroup->FunctionGroup();
                    }
                }
                if (functionGroup)
                {
                    int arity = classDelegateType->Arity();
                    std::unordered_set<FunctionSymbol*> viableFunctions;
                    functionGroup->CollectViableFunctions(arity + 1, viableFunctions);
                    for (FunctionSymbol* viableFunction : viableFunctions)
                    {
                        bool found = true;
                        for (int i = 1; i < arity + 1; ++i)
                        {
                            ParameterSymbol* sourceParam = viableFunction->Parameters()[i];
                            ParameterSymbol* targetParam = classDelegateType->Parameters()[i - 1];
                            if (!TypesEqual(sourceParam->GetType(), targetParam->GetType()))
                            {
                                found = false;
                                break;
                            }
                        }
                        if (found)
                        {
                            found = TypesEqual(viableFunction->ReturnType(), classDelegateType->ReturnType());
                        }
                        if (found)
                        {
                            LocalVariableSymbol* objectDelegatePairVariable = currentFunction->GetFunctionSymbol()->CreateTemporary(classDelegateType->ObjectDelegatePairType(), span);
                            std::unique_ptr<FunctionSymbol> memberFunctionToClassDelegateConversion(new MemberFunctionToClassDelegateConversion(span, sourceType, classDelegateType, viableFunction,
                                objectDelegatePairVariable));
                            conversion = memberFunctionToClassDelegateConversion.get();
                            conversionTable.AddConversion(conversion);
                            conversionTable.AddGeneratedConversion(std::move(memberFunctionToClassDelegateConversion));
                            return conversion;
                        }
                    }
                }
            }
            else if (targetType->PlainType(span)->GetSymbolType() == SymbolType::interfaceTypeSymbol && currentFunction)
            {
                InterfaceTypeSymbol* targetInterfaceType = static_cast<InterfaceTypeSymbol*>(targetType->PlainType(span));
                if (sourceType->IsClassTypeSymbol()) 
                {
                    ClassTypeSymbol* sourceClassType = static_cast<ClassTypeSymbol*>(sourceType);
                    int32_t n = sourceClassType->ImplementedInterfaces().size();
                    for (int32_t i = 0; i < n; ++i)
                    {
                        InterfaceTypeSymbol* sourceInterfaceType = sourceClassType->ImplementedInterfaces()[i];
                        if (TypesEqual(targetInterfaceType, sourceInterfaceType))
                        {
                            LocalVariableSymbol* temporaryInterfaceObjectVar = currentFunction->GetFunctionSymbol()->CreateTemporary(targetInterfaceType, span);
                            std::unique_ptr<FunctionSymbol> classToInterfaceConversion(new ClassToInterfaceConversion(sourceClassType, targetInterfaceType, temporaryInterfaceObjectVar, i, span));
                            classToInterfaceConversion->SetModule(&GetModule());
                            classToInterfaceConversion->SetSymbolTable(&symbolTable);
                            conversion = classToInterfaceConversion.get();
                            conversionTable.AddConversion(conversion);
                            conversionTable.AddGeneratedConversion(std::move(classToInterfaceConversion));
                            return conversion;
                        }
                    }
                }
            }
        }
    }
    if (conversion)
    {
        if (conversion->Parent() && !conversion->IsGeneratedFunction() && conversion->Parent()->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol)
        {
            InstantiateClassTemplateMemberFunction(conversion, containerScope, currentFunction, span);
        }
        else if (GetGlobalFlag(GlobalFlags::release) && conversion->IsInline())
        {
            InstantiateInlineFunction(conversion, containerScope, span);
        }
    }
    return conversion;
}

void BoundCompileUnit::CollectViableFunctions(const std::u32string& groupName, ContainerScope* containerScope, std::vector<std::unique_ptr<BoundExpression>>& arguments, 
    BoundFunction* currentFunction, std::unordered_set<FunctionSymbol*>& viableFunctions, std::unique_ptr<Exception>& exception, const Span& span)
{
    operationRepository.CollectViableFunctions(groupName, containerScope, arguments, currentFunction, viableFunctions, exception, span);
}

FunctionSymbol* BoundCompileUnit::InstantiateFunctionTemplate(FunctionSymbol* functionTemplate, const std::unordered_map<TemplateParameterSymbol*, TypeSymbol*>& templateParameterMapping, 
    const Span& span)
{
    return functionTemplateRepository.Instantiate(functionTemplate, templateParameterMapping, span);
}

void BoundCompileUnit::InstantiateClassTemplateMemberFunction(FunctionSymbol* memberFunction, ContainerScope* containerScope, BoundFunction* currentFunction, const Span& span)
{
    classTemplateRepository.Instantiate(memberFunction, containerScope, currentFunction, span);
}

void BoundCompileUnit::InstantiateInlineFunction(FunctionSymbol* inlineFunction, ContainerScope* containerScope, const Span& span)
{
    inlineFunctionRepository.Instantiate(inlineFunction, containerScope, span);
}

FunctionNode* BoundCompileUnit::GetFunctionNodeFor(FunctionSymbol* constExprFunctionSymbol)
{
    return constExprFunctionRepository.GetFunctionNodeFor(constExprFunctionSymbol);
}

void BoundCompileUnit::GenerateCopyConstructorFor(ClassTypeSymbol* classTypeSymbol, ContainerScope* containerScope, BoundFunction* currentFunction, const Span& span)
{
    operationRepository.GenerateCopyConstructorFor(classTypeSymbol, containerScope, currentFunction, span);
}

void BoundCompileUnit::GenerateCopyConstructorFor(InterfaceTypeSymbol* interfaceTypeSymbol, ContainerScope* containerScope, BoundFunction* currentFunction, const Span& span)
{
    operationRepository.GenerateCopyConstructorFor(interfaceTypeSymbol, containerScope, currentFunction, span);
}

int BoundCompileUnit::Install(const std::string& str)
{
    return utf8StringRepository.Install(str);
}

int BoundCompileUnit::Install(const std::u16string& str)
{
    return utf16StringRepository.Install(str);
}

int BoundCompileUnit::Install(const std::u32string& str)
{
    return utf32StringRepository.Install(str);
}

int BoundCompileUnit::Install(const boost::uuids::uuid& uuid)
{
    return uuidRepository.Install(uuid);
}

const std::string& BoundCompileUnit::GetUtf8String(int stringId) const
{
    return utf8StringRepository.GetString(stringId);
}

const std::u16string& BoundCompileUnit::GetUtf16String(int stringId) const
{
    return utf16StringRepository.GetString(stringId);
}

const std::u32string& BoundCompileUnit::GetUtf32String(int stringId) const
{
    return utf32StringRepository.GetString(stringId);
}

const unsigned char* BoundCompileUnit::GetUtf8CharPtr(int stringId) const
{
    return utf8StringRepository.CharPtr(stringId);
}

const char16_t* BoundCompileUnit::GetUtf16CharPtr(int stringId) const
{
    return utf16StringRepository.CharPtr(stringId);
}

const char32_t* BoundCompileUnit::GetUtf32CharPtr(int stringId) const
{
    return utf32StringRepository.CharPtr(stringId);
}

const boost::uuids::uuid& BoundCompileUnit::GetUuid(int uuidId) const
{
    return uuidRepository.GetUuid(uuidId);
}

void BoundCompileUnit::AddConstantArray(ConstantSymbol* constantArraySymbol)
{
    constantArrayRepository.AddConstantArray(constantArraySymbol);
}

void BoundCompileUnit::AddConstantStructure(ConstantSymbol* constantStructureSymbol)
{
    constantStructureRepository.AddConstantStructure(constantStructureSymbol);
}

void BoundCompileUnit::PushBindingTypes()
{
    bindingTypesStack.push(bindingTypes);
    bindingTypes = true;
}

void BoundCompileUnit::PopBindingTypes()
{
    bindingTypes = bindingTypesStack.top();
    bindingTypesStack.pop();
}

void BoundCompileUnit::FinalizeBinding(ClassTypeSymbol* classType)
{
    if (classType->GetSymbolType() != SymbolType::classTemplateSpecializationSymbol) return;
    ClassTemplateSpecializationSymbol* classTemplateSpecialization = static_cast<ClassTemplateSpecializationSymbol*>(classType);
    if (classTemplateSpecialization->StatementsNotBound())
    {
        classTemplateSpecialization->ResetStatementsNotBound();
        FileScope* fileScope = classTemplateSpecialization->ReleaseFileScope();
        bool fileScopeAdded = false;
        if (fileScope)
        {
            AddFileScope(fileScope);
            fileScopeAdded = true;
        }
        StatementBinder statementBinder(*this);
        classTemplateSpecialization->GlobalNs()->Accept(statementBinder);
        if (fileScopeAdded)
        {
            RemoveLastFileScope();
        }
    }
}

void BoundCompileUnit::PushNamespace(BoundNamespace* ns)
{
    namespaceStack.push(currentNamespace);
    currentNamespace = ns;
}

void BoundCompileUnit::PopNamespace()
{
    currentNamespace = namespaceStack.top();
    namespaceStack.pop();
}

} } // namespace cmajor::binder
