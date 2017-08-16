// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/BoundCompileUnit.hpp>
#include <cmajor/binder/BoundNodeVisitor.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/ClassTypeSymbol.hpp>
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
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
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
}

void ClassTypeConversion::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
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
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
private:
    TypeSymbol* nullPtrType;
    TypeSymbol* targetPointerType;;
};

NullPtrToPtrConversion::NullPtrToPtrConversion(TypeSymbol* nullPtrType_, TypeSymbol* targetPointerType_) :
    FunctionSymbol(Span(), U"nullptr2ptr"), nullPtrType(nullPtrType_), targetPointerType(targetPointerType_)
{
    SetConversion();
    SetGroupName(U"@conversion");
}

void NullPtrToPtrConversion::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    llvm::Value* value = emitter.Stack().Pop();
    emitter.Stack().Push(emitter.Builder().CreateBitCast(value, targetPointerType->IrType(emitter)));
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
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
private:
    TypeSymbol* voidPtrType;
    TypeSymbol* targetPointerType;;
};

VoidPtrToPtrConversion::VoidPtrToPtrConversion(TypeSymbol* voidPtrType_, TypeSymbol* targetPointerType_) :
    FunctionSymbol(Span(), U"voidPtr2ptr"), voidPtrType(voidPtrType_), targetPointerType(targetPointerType_)
{
    SetConversion();
    SetGroupName(U"@conversion");
}

void VoidPtrToPtrConversion::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
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
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
private:
    TypeSymbol* sourcePtrType;
    TypeSymbol* voidPtrType;
};

PtrToVoidPtrConversion::PtrToVoidPtrConversion(TypeSymbol* sourcePtrType_, TypeSymbol* voidPtrType_) :
    FunctionSymbol(Span(), U"ptr2voidPtr"), sourcePtrType(sourcePtrType_), voidPtrType(voidPtrType_)
{
    SetConversion();
    SetGroupName(U"@conversion");
}

void PtrToVoidPtrConversion::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    llvm::Value* value = emitter.Stack().Pop();
    emitter.Stack().Push(emitter.Builder().CreateBitCast(value, voidPtrType->IrType(emitter)));
}

BoundCompileUnit::BoundCompileUnit(Module& module_, CompileUnitNode* compileUnitNode_) : 
    BoundNode(Span(), BoundNodeType::boundCompileUnit), module(module_), symbolTable(module.GetSymbolTable()), compileUnitNode(compileUnitNode_), hasGotos(false), 
    operationRepository(*this), functionTemplateRepository(*this), classTemplateRepository(*this)
{
    boost::filesystem::path fileName = boost::filesystem::path(compileUnitNode->FilePath()).filename();
    boost::filesystem::path directory = module.DirectoryPath();
    boost::filesystem::path llfp = (directory / fileName).replace_extension(".ll");
    boost::filesystem::path optllfp = (directory / fileName).replace_extension(".opt.ll");
    boost::filesystem::path objfp = (directory / fileName).replace_extension(".obj");
    llFilePath = GetFullPath(llfp.generic_string());
    optLLFilePath = GetFullPath(optllfp.generic_string());
    objectFilePath = GetFullPath(objfp.generic_string());
}

void BoundCompileUnit::Load(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot load from compile unit", GetSpan());
}

void BoundCompileUnit::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to compile unit", GetSpan());
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
        throw Exception("cannot remove last file scope from empty list of file scopes", GetSpan());
    }
    fileScopes.erase(fileScopes.end() - 1);
}

void BoundCompileUnit::AddBoundNode(std::unique_ptr<BoundNode>&& boundNode)
{
    boundNodes.push_back(std::move(boundNode));
}

FunctionSymbol* BoundCompileUnit::GetConversion(TypeSymbol* sourceType, TypeSymbol* targetType, const Span& span)
{
    FunctionSymbol* conversion = symbolTable.GetConversion(sourceType, targetType, span);
    if (!conversion)
    {
        conversion = conversionTable.GetConversion(sourceType, targetType, span);
        if (!conversion)
        {
            if (sourceType->IsNullPtrType() && targetType->IsPointerType())
            {
                std::unique_ptr<FunctionSymbol> nullPtrToPtrConversion(new NullPtrToPtrConversion(symbolTable.GetTypeByName(U"@nullptr_type"), targetType));
                conversion = nullPtrToPtrConversion.get();
                conversionTable.AddConversion(conversion);
                conversionTable.AddGeneratedConversion(std::move(nullPtrToPtrConversion));
                return conversion;
            }
            else if (sourceType->IsVoidPtrType() && targetType->IsPointerType())
            {
                std::unique_ptr<FunctionSymbol> voidPtrToPtrConversion(new VoidPtrToPtrConversion(symbolTable.GetTypeByName(U"void")->AddPointer(span), targetType));
                conversion = voidPtrToPtrConversion.get();
                conversionTable.AddConversion(conversion);
                conversionTable.AddGeneratedConversion(std::move(voidPtrToPtrConversion));
                return conversion;
            }
            else if (sourceType->IsPointerType() && targetType->RemoveConst(span)->IsVoidPtrType())
            {
                std::unique_ptr<FunctionSymbol> ptrToVoidPtrConversion(new PtrToVoidPtrConversion(sourceType, symbolTable.GetTypeByName(U"void")->AddPointer(span)));
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
                        std::u32string conversionName = sourceType->FullName() + U"2" + targetType->FullName();
                        std::unique_ptr<FunctionSymbol> implicitClassTypeConversion(new ClassTypeConversion(conversionName, ConversionType::implicit_, conversionDistance, sourceType, targetType));
                        conversion = implicitClassTypeConversion.get();
                        conversionTable.AddConversion(conversion);
                        conversionTable.AddGeneratedConversion(std::move(implicitClassTypeConversion));
                        return conversion;
                    }
                    else
                    {
                        uint8_t conversionDistance = 0;
                        if (targetClassType->HasBaseClass(sourceClassType, conversionDistance))
                        {
                            std::u32string conversionName = sourceType->FullName() + U"2" + targetType->FullName();
                            std::unique_ptr<FunctionSymbol> explicitClassTypeConversion(new ClassTypeConversion(conversionName, ConversionType::explicit_, conversionDistance, sourceType, targetType));
                            conversion = explicitClassTypeConversion.get();
                            conversionTable.AddConversion(conversion);
                            conversionTable.AddGeneratedConversion(std::move(explicitClassTypeConversion));
                            return conversion;
                        }
                    }
                }
            }

        }
    }
    return conversion;
}

void BoundCompileUnit::CollectViableFunctions(const std::u32string& groupName, ContainerScope* containerScope, std::vector<std::unique_ptr<BoundExpression>>& arguments, 
    BoundFunction* currentFunction, std::unordered_set<FunctionSymbol*>& viableFunctions, std::unique_ptr<Exception>& exception, const Span& span)
{
    operationRepository.CollectViableFunctions(groupName, containerScope, arguments, currentFunction, viableFunctions, exception, span);
}

FunctionSymbol* BoundCompileUnit::Instantiate(FunctionSymbol* functionTemplate, const std::unordered_map<TemplateParameterSymbol*, TypeSymbol*>& templateParameterMapping, const Span& span)
{
    return functionTemplateRepository.Instantiate(functionTemplate, templateParameterMapping, span);
}

void BoundCompileUnit::Instantiate(FunctionSymbol* memberFunction, ContainerScope* containerScope, const Span& span)
{
    classTemplateRepository.Instantiate(memberFunction, containerScope, span);
}

int BoundCompileUnit::Install(const std::string& str)
{
    return stringRepository.Install(str);
}

const std::string& BoundCompileUnit::GetString(int stringId) const
{
    return stringRepository.GetString(stringId);
}

} } // namespace cmajor::binder
