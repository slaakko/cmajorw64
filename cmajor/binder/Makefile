include ../Makefile.common

OBJECTS = Access.o AttributeBinder.o BoundClass.o BoundCompileUnit.o BoundConstraint.o BoundExpression.o BoundFunction.o BoundNode.o \
BoundNodeVisitor.o BoundStatement.o ClassTemplateRepository.o Concept.o ConceptRepository.o ConstantArrayRepository.o ConstantStructureRepository.o \
ConstExprFunctionRepository.o ControlFlowAnalyzer.o Evaluator.o ExpressionBinder.o FunctionTemplateRepository.o InlineFunctionRepository.o \
JsonAttributeProcessor.o ModuleBinder.o OperationRepository.o OverloadResolution.o StatementBinder.o StringRepository.o TypeBinder.o TypeResolver.o

%o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

../lib/$(config)/libbinder.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	rm -f *.o
	rm -f ../lib/$(config)/libbinder.a

