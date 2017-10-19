// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/BasicTypeSymbol.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/symbols/SymbolCollector.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

BasicTypeSymbol::BasicTypeSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_) : 
    TypeSymbol(symbolType_, span_, name_), 
    defaultConstructor(nullptr), copyConstructor(nullptr), moveConstructor(nullptr), copyAssignment(nullptr), moveAssignment(nullptr), returnFun(nullptr), equalityOp(nullptr)
{
}

void BasicTypeSymbol::Write(SymbolWriter& writer)
{
    TypeSymbol::Write(writer);
    uint32_t defaultConstructorId = 0;
    if (defaultConstructor)
    {
        defaultConstructorId = defaultConstructor->FunctionId();
    }
    writer.GetBinaryWriter().Write(defaultConstructorId);
    uint32_t copyConstructorId = 0;
    if (copyConstructor)
    {
        copyConstructorId = copyConstructor->FunctionId();
    }
    writer.GetBinaryWriter().Write(copyConstructorId);
    uint32_t moveConstructorId = 0;
    if (moveConstructor)
    {
        moveConstructorId = moveConstructor->FunctionId();
    }
    writer.GetBinaryWriter().Write(moveConstructorId);
    uint32_t copyAssignmentId = 0;
    if (copyAssignment)
    {
        copyAssignmentId = copyAssignment->FunctionId();
    }
    writer.GetBinaryWriter().Write(copyAssignmentId);
    uint32_t moveAssignmentId = 0;
    if (moveAssignment)
    {
        moveAssignmentId = moveAssignment->FunctionId();
    }
    writer.GetBinaryWriter().Write(moveAssignmentId);
    uint32_t returnId = 0;
    if (returnFun)
    {
        returnId = returnFun->FunctionId();
    }
    writer.GetBinaryWriter().Write(returnId);
    uint32_t equalityOpId = 0;
    if (equalityOp)
    {
        equalityOpId = equalityOp->FunctionId();
    }
    writer.GetBinaryWriter().Write(equalityOpId);
}

void BasicTypeSymbol::Read(SymbolReader& reader)
{
    TypeSymbol::Read(reader);
    uint32_t defaultConstructorId = reader.GetBinaryReader().ReadUInt();
    if (defaultConstructorId != 0)
    {
        GetSymbolTable()->EmplaceFunctionRequest(this, defaultConstructorId, 0);
    }
    uint32_t copyConstructorId = reader.GetBinaryReader().ReadUInt();
    if (copyConstructorId != 0)
    {
        GetSymbolTable()->EmplaceFunctionRequest(this, copyConstructorId, 1);
    }
    uint32_t moveConstructorId = reader.GetBinaryReader().ReadUInt();
    if (moveConstructorId != 0)
    {
        GetSymbolTable()->EmplaceFunctionRequest(this, moveConstructorId, 2);
    }
    uint32_t copyAssignmentId = reader.GetBinaryReader().ReadUInt();
    if (copyAssignmentId != 0)
    {
        GetSymbolTable()->EmplaceFunctionRequest(this, copyAssignmentId, 3);
    }
    uint32_t moveAssignmentId = reader.GetBinaryReader().ReadUInt();
    if (moveAssignmentId != 0)
    {
        GetSymbolTable()->EmplaceFunctionRequest(this, moveAssignmentId, 4);
    }
    uint32_t returnId = reader.GetBinaryReader().ReadUInt();
    if (returnId != 0)
    {
        GetSymbolTable()->EmplaceFunctionRequest(this, returnId, 5);
    }
    uint32_t equalityOpId = reader.GetBinaryReader().ReadUInt();
    if (equalityOpId != 0)
    {
        GetSymbolTable()->EmplaceFunctionRequest(this, equalityOpId, 6);
    }
}

void BasicTypeSymbol::EmplaceFunction(FunctionSymbol* functionSymbol, int index)
{
    switch (index)
    {
        case 0: defaultConstructor = functionSymbol; break;
        case 1: copyConstructor = functionSymbol; break;
        case 2: moveConstructor = functionSymbol; break;
        case 3: copyAssignment = functionSymbol; break;
        case 4: moveAssignment = functionSymbol; break;
        case 5: returnFun = functionSymbol; break;
        case 6: equalityOp = functionSymbol; break;
        default:
        {
            Assert(false, "invalid emplace function index");
        }
    }
}

void BasicTypeSymbol::Accept(SymbolCollector* collector)
{
    if (IsProject())
    {
        collector->AddBasicType(this);
    }
}

void BasicTypeSymbol::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine(ToUtf8(Name()));
    formatter.WriteLine("typeid: " + std::to_string(TypeId()));
}

BoolTypeSymbol::BoolTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::boolTypeSymbol, span_, name_)
{
}

SByteTypeSymbol::SByteTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::sbyteTypeSymbol, span_, name_)
{
}

ByteTypeSymbol::ByteTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::byteTypeSymbol, span_, name_)
{
}

ShortTypeSymbol::ShortTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::shortTypeSymbol, span_, name_)
{
}

UShortTypeSymbol::UShortTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::ushortTypeSymbol, span_, name_)
{
}

IntTypeSymbol::IntTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::intTypeSymbol, span_, name_)
{
}

UIntTypeSymbol::UIntTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::uintTypeSymbol, span_, name_)
{
}

LongTypeSymbol::LongTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::longTypeSymbol, span_, name_)
{
}

ULongTypeSymbol::ULongTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::ulongTypeSymbol, span_, name_)
{
}

FloatTypeSymbol::FloatTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::floatTypeSymbol, span_, name_)
{
}

DoubleTypeSymbol::DoubleTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::doubleTypeSymbol, span_, name_)
{
}

CharTypeSymbol::CharTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::charTypeSymbol, span_, name_)
{
}

WCharTypeSymbol::WCharTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::wcharTypeSymbol, span_, name_)
{
}

UCharTypeSymbol::UCharTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::ucharTypeSymbol, span_, name_)
{
}

VoidTypeSymbol::VoidTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::voidTypeSymbol, span_, name_)
{
}

} } // namespace cmajor::symbols
