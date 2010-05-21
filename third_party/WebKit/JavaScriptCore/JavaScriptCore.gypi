{
    'variables': {
        'javascriptcore_files': [
            'API/APICast.h',
            'API/JavaScript.h',
            'API/JavaScriptCore.h',
            'API/JSBase.cpp',
            'API/JSBase.h',
            'API/JSBasePrivate.h',
            'API/JSCallbackConstructor.cpp',
            'API/JSCallbackConstructor.h',
            'API/JSCallbackFunction.cpp',
            'API/JSCallbackFunction.h',
            'API/JSCallbackObject.cpp',
            'API/JSCallbackObject.h',
            'API/JSCallbackObjectFunctions.h',
            'API/JSClassRef.cpp',
            'API/JSClassRef.h',
            'API/JSContextRef.cpp',
            'API/JSContextRef.h',
            'API/JSContextRefPrivate.h',
            'API/JSObjectRef.cpp',
            'API/JSObjectRef.h',
            'API/JSProfilerPrivate.cpp',
            'API/JSProfilerPrivate.h',
            'API/JSRetainPtr.h',
            'API/JSStringRef.cpp',
            'API/JSStringRef.h',
            'API/JSStringRefBSTR.cpp',
            'API/JSStringRefBSTR.h',
            'API/JSStringRefCF.cpp',
            'API/JSStringRefCF.h',
            'API/JSValueRef.cpp',
            'API/JSValueRef.h',
            'API/OpaqueJSString.cpp',
            'API/OpaqueJSString.h',
            'API/tests/JSNode.h',
            'API/tests/JSNodeList.h',
            'API/tests/Node.h',
            'API/tests/NodeList.h',
            'API/WebKitAvailability.h',
            'assembler/AbstractMacroAssembler.h',
            'assembler/ARMv7Assembler.h',
            'assembler/AssemblerBuffer.h',
            'assembler/CodeLocation.h',
            'assembler/MacroAssembler.h',
            'assembler/MacroAssemblerARMv7.h',
            'assembler/MacroAssemblerCodeRef.h',
            'assembler/MacroAssemblerX86.h',
            'assembler/MacroAssemblerX86_64.h',
            'assembler/MacroAssemblerX86Common.h',
            'assembler/X86Assembler.h',
            'bytecode/CodeBlock.cpp',
            'bytecode/CodeBlock.h',
            'bytecode/EvalCodeCache.h',
            'bytecode/Instruction.h',
            'bytecode/JumpTable.cpp',
            'bytecode/JumpTable.h',
            'bytecode/Opcode.cpp',
            'bytecode/Opcode.h',
            'bytecode/SamplingTool.cpp',
            'bytecode/SamplingTool.h',
            'bytecode/StructureStubInfo.cpp',
            'bytecode/StructureStubInfo.h',
            'bytecompiler/BytecodeGenerator.cpp',
            'bytecompiler/BytecodeGenerator.h',
            'bytecompiler/NodesCodegen.cpp',
            'bytecompiler/Label.h',
            'bytecompiler/LabelScope.h',
            'bytecompiler/RegisterID.h',
            'config.h',
            'debugger/Debugger.cpp',
            'debugger/Debugger.h',
            'debugger/DebuggerActivation.cpp',
            'debugger/DebuggerActivation.h',
            'debugger/DebuggerCallFrame.cpp',
            'debugger/DebuggerCallFrame.h',
            'icu/unicode/parseerr.h',
            'icu/unicode/platform.h',
            'icu/unicode/putil.h',
            'icu/unicode/uchar.h',
            'icu/unicode/ucnv.h',
            'icu/unicode/ucnv_err.h',
            'icu/unicode/ucol.h',
            'icu/unicode/uconfig.h',
            'icu/unicode/uenum.h',
            'icu/unicode/uiter.h',
            'icu/unicode/uloc.h',
            'icu/unicode/umachine.h',
            'icu/unicode/unorm.h',
            'icu/unicode/urename.h',
            'icu/unicode/uset.h',
            'icu/unicode/ustring.h',
            'icu/unicode/utf.h',
            'icu/unicode/utf16.h',
            'icu/unicode/utf8.h',
            'icu/unicode/utf_old.h',
            'icu/unicode/utypes.h',
            'icu/unicode/uversion.h',
            'interpreter/CachedCall.h',
            'interpreter/CallFrame.cpp',
            'interpreter/CallFrame.h',
            'interpreter/CallFrameClosure.h',
            'interpreter/Interpreter.cpp',
            'interpreter/Interpreter.h',
            'interpreter/Register.h',
            'interpreter/RegisterFile.cpp',
            'interpreter/RegisterFile.h',
            'JavaScriptCorePrefix.h',
            'jit/ExecutableAllocator.cpp',
            'jit/ExecutableAllocator.h',
            'jit/ExecutableAllocatorFixedVMPool.cpp',
            'jit/ExecutableAllocatorPosix.cpp',
            'jit/ExecutableAllocatorWin.cpp',
            'jit/JIT.cpp',
            'jit/JIT.h',
            'jit/JITArithmetic.cpp',
            'jit/JITArithmetic32_64.cpp',
            'jit/JITCall.cpp',
            'jit/JITCall32_64.cpp',
            'jit/JITCode.h',
            'jit/JITInlineMethods.h',
            'jit/JITOpcodes.cpp',
            'jit/JITOpcodes32_64.cpp',
            'jit/JITPropertyAccess.cpp',
            'jit/JITPropertyAccess32_64.cpp',
            'jit/JITStubCall.h',
            'jit/JITStubs.cpp',
            'jit/JITStubs.h',
            'jsc.cpp',
            'os-win32/stdbool.h',
            'os-win32/stdint.h',
            'parser/Lexer.cpp',
            'parser/Lexer.h',
            'parser/NodeConstructors.h',
            'parser/NodeInfo.h',
            'parser/Nodes.cpp',
            'parser/Nodes.h',
            'parser/Parser.cpp',
            'parser/Parser.h',
            'parser/ParserArena.cpp',
            'parser/ParserArena.h',
            'parser/ResultType.h',
            'parser/SourceCode.h',
            'parser/SourceProvider.h',
            'pcre/pcre.h',
            'pcre/pcre_compile.cpp',
            'pcre/pcre_exec.cpp',
            'pcre/pcre_internal.h',
            'pcre/pcre_tables.cpp',
            'pcre/pcre_ucp_searchfuncs.cpp',
            'pcre/pcre_xclass.cpp',
            'pcre/ucpinternal.h',
            'pcre/ucptable.cpp',
            'profiler/CallIdentifier.h',
            'profiler/Profile.cpp',
            'profiler/Profile.h',
            'profiler/ProfileGenerator.cpp',
            'profiler/ProfileGenerator.h',
            'profiler/ProfileNode.cpp',
            'profiler/ProfileNode.h',
            'profiler/Profiler.cpp',
            'profiler/Profiler.h',
            'profiler/ProfilerServer.h',
            'runtime/ArgList.cpp',
            'runtime/ArgList.h',
            'runtime/Arguments.cpp',
            'runtime/Arguments.h',
            'runtime/ArrayConstructor.cpp',
            'runtime/ArrayConstructor.h',
            'runtime/ArrayPrototype.cpp',
            'runtime/ArrayPrototype.h',
            'runtime/BatchedTransitionOptimizer.h',
            'runtime/BooleanConstructor.cpp',
            'runtime/BooleanConstructor.h',
            'runtime/BooleanObject.cpp',
            'runtime/BooleanObject.h',
            'runtime/BooleanPrototype.cpp',
            'runtime/BooleanPrototype.h',
            'runtime/CallData.cpp',
            'runtime/CallData.h',
            'runtime/ClassInfo.h',
            'runtime/Collector.cpp',
            'runtime/Collector.h',
            'runtime/CollectorHeapIterator.h',
            'runtime/CommonIdentifiers.cpp',
            'runtime/CommonIdentifiers.h',
            'runtime/Completion.cpp',
            'runtime/Completion.h',
            'runtime/ConstructData.cpp',
            'runtime/ConstructData.h',
            'runtime/DateConstructor.cpp',
            'runtime/DateConstructor.h',
            'runtime/DateConversion.cpp',
            'runtime/DateConversion.h',
            'runtime/DateInstance.cpp',
            'runtime/DateInstance.h',
            'runtime/DateInstanceCache.h',
            'runtime/DatePrototype.cpp',
            'runtime/DatePrototype.h',
            'runtime/Error.cpp',
            'runtime/Error.h',
            'runtime/ErrorConstructor.cpp',
            'runtime/ErrorConstructor.h',
            'runtime/ErrorInstance.cpp',
            'runtime/ErrorInstance.h',
            'runtime/ErrorPrototype.cpp',
            'runtime/ErrorPrototype.h',
            'runtime/ExceptionHelpers.cpp',
            'runtime/ExceptionHelpers.h',
            'runtime/FunctionConstructor.cpp',
            'runtime/FunctionConstructor.h',
            'runtime/FunctionPrototype.cpp',
            'runtime/FunctionPrototype.h',
            'runtime/GetterSetter.cpp',
            'runtime/GetterSetter.h',
            'runtime/GlobalEvalFunction.cpp',
            'runtime/GlobalEvalFunction.h',
            'runtime/Identifier.cpp',
            'runtime/Identifier.h',
            'runtime/InitializeThreading.cpp',
            'runtime/InitializeThreading.h',
            'runtime/InternalFunction.cpp',
            'runtime/InternalFunction.h',
            'runtime/JSActivation.cpp',
            'runtime/JSActivation.h',
            'runtime/JSArray.cpp',
            'runtime/JSArray.h',
            'runtime/JSByteArray.cpp',
            'runtime/JSByteArray.h',
            'runtime/JSCell.cpp',
            'runtime/JSCell.h',
            'runtime/JSFunction.cpp',
            'runtime/JSFunction.h',
            'runtime/JSGlobalData.cpp',
            'runtime/JSGlobalData.h',
            'runtime/JSGlobalObject.cpp',
            'runtime/JSGlobalObject.h',
            'runtime/JSGlobalObjectFunctions.cpp',
            'runtime/JSGlobalObjectFunctions.h',
            'runtime/JSImmediate.cpp',
            'runtime/JSImmediate.h',
            'runtime/JSLock.cpp',
            'runtime/JSLock.h',
            'runtime/JSNotAnObject.cpp',
            'runtime/JSNotAnObject.h',
            'runtime/JSNumberCell.cpp',
            'runtime/JSNumberCell.h',
            'runtime/JSObject.cpp',
            'runtime/JSObject.h',
            'runtime/JSObjectWithGlobalObject.cpp',
            'runtime/JSObjectWithGlobalObject.h',
            'runtime/JSONObject.cpp',
            'runtime/JSONObject.h',
            'runtime/JSPropertyNameIterator.cpp',
            'runtime/JSPropertyNameIterator.h',
            'runtime/JSStaticScopeObject.cpp',
            'runtime/JSStaticScopeObject.h',
            'runtime/JSString.cpp',
            'runtime/JSString.h',
            'runtime/JSType.h',
            'runtime/JSTypeInfo.h',
            'runtime/JSValue.cpp',
            'runtime/JSValue.h',
            'runtime/JSVariableObject.cpp',
            'runtime/JSVariableObject.h',
            'runtime/JSWrapperObject.cpp',
            'runtime/JSWrapperObject.h',
            'runtime/LiteralParser.cpp',
            'runtime/LiteralParser.h',
            'runtime/Lookup.cpp',
            'runtime/Lookup.h',
            'runtime/MarkStack.cpp',
            'runtime/MarkStack.h',
            'runtime/MarkStackWin.cpp',
            'runtime/MathObject.cpp',
            'runtime/MathObject.h',
            'runtime/NativeErrorConstructor.cpp',
            'runtime/NativeErrorConstructor.h',
            'runtime/NativeErrorPrototype.cpp',
            'runtime/NativeErrorPrototype.h',
            'runtime/NativeFunctionWrapper.h',
            'runtime/NumberConstructor.cpp',
            'runtime/NumberConstructor.h',
            'runtime/NumberObject.cpp',
            'runtime/NumberObject.h',
            'runtime/NumberPrototype.cpp',
            'runtime/NumberPrototype.h',
            'runtime/ObjectConstructor.cpp',
            'runtime/ObjectConstructor.h',
            'runtime/ObjectPrototype.cpp',
            'runtime/ObjectPrototype.h',
            'runtime/Operations.cpp',
            'runtime/Operations.h',
            'runtime/PropertyDescriptor.cpp',
            'runtime/PropertyDescriptor.h',
            'runtime/PropertyMapHashTable.h',
            'runtime/PropertyNameArray.cpp',
            'runtime/PropertyNameArray.h',
            'runtime/PropertySlot.cpp',
            'runtime/PropertySlot.h',
            'runtime/Protect.h',
            'runtime/PrototypeFunction.cpp',
            'runtime/PrototypeFunction.h',
            'runtime/PutPropertySlot.h',
            'runtime/RegExp.cpp',
            'runtime/RegExp.h',
            'runtime/RegExpConstructor.cpp',
            'runtime/RegExpConstructor.h',
            'runtime/RegExpMatchesArray.h',
            'runtime/RegExpObject.cpp',
            'runtime/RegExpObject.h',
            'runtime/RegExpPrototype.cpp',
            'runtime/RegExpPrototype.h',
            'runtime/ScopeChain.cpp',
            'runtime/ScopeChain.h',
            'runtime/ScopeChainMark.h',
            'runtime/SmallStrings.cpp',
            'runtime/SmallStrings.h',
            'runtime/StringConstructor.cpp',
            'runtime/StringConstructor.h',
            'runtime/StringObject.cpp',
            'runtime/StringObject.h',
            'runtime/StringObjectThatMasqueradesAsUndefined.h',
            'runtime/StringPrototype.cpp',
            'runtime/StringPrototype.h',
            'runtime/Structure.cpp',
            'runtime/Structure.h',
            'runtime/StructureChain.cpp',
            'runtime/StructureChain.h',
            'runtime/StructureTransitionTable.h',
            'runtime/SymbolTable.h',
            'runtime/Terminator.h',
            'runtime/TimeoutChecker.cpp',
            'runtime/TimeoutChecker.h',
            'runtime/Tracing.h',
            'runtime/UString.cpp',
            'runtime/UString.h',
            'runtime/WeakRandom.h',
            'wtf/AlwaysInline.h',
            'wtf/ASCIICType.h',
            'wtf/Assertions.cpp',
            'wtf/Assertions.h',
            'wtf/Atomics.h',
            'wtf/AVLTree.h',
            'wtf/ByteArray.cpp',
            'wtf/ByteArray.h',
            'wtf/chromium/ChromiumThreading.h',
            'wtf/chromium/MainThreadChromium.cpp',
            'wtf/CrossThreadRefCounted.h',
            'wtf/CurrentTime.cpp',
            'wtf/CurrentTime.h',
            'wtf/DateMath.cpp',
            'wtf/DateMath.h',
            'wtf/Deque.h',
            'wtf/DisallowCType.h',
            'wtf/dtoa.cpp',
            'wtf/dtoa.h',
            'wtf/FastAllocBase.h',
            'wtf/FastMalloc.cpp',
            'wtf/FastMalloc.h',
            'wtf/Forward.h',
            'wtf/GetPtr.h',
            'wtf/gobject/GOwnPtr.cpp',
            'wtf/gobject/GOwnPtr.h',
            'wtf/gtk/MainThreadGtk.cpp',
            'wtf/gtk/ThreadingGtk.cpp',
            'wtf/HashCountedSet.h',
            'wtf/HashFunctions.h',
            'wtf/HashIterators.h',
            'wtf/HashMap.h',
            'wtf/HashSet.h',
            'wtf/HashTable.cpp',
            'wtf/HashTable.h',
            'wtf/HashTraits.h',
            'wtf/ListHashSet.h',
            'wtf/ListRefPtr.h',
            'wtf/Locker.h',
            'wtf/MD5.cpp',
            'wtf/MD5.h',
            'wtf/MainThread.cpp',
            'wtf/MainThread.h',
            'wtf/MallocZoneSupport.h',
            'wtf/MathExtras.h',
            'wtf/MessageQueue.h',
            'wtf/Noncopyable.h',
            'wtf/NotFound.h',
            'wtf/OwnArrayPtr.h',
            'wtf/OwnFastMallocPtr.h',
            'wtf/OwnPtr.h',
            'wtf/OwnPtrCommon.h',
            'wtf/PassOwnPtr.h',
            'wtf/PassRefPtr.h',
            'wtf/Platform.h',
            'wtf/PtrAndFlags.h',
            'wtf/RandomNumber.cpp',
            'wtf/RandomNumber.h',
            'wtf/RandomNumberSeed.h',
            'wtf/RefCounted.h',
            'wtf/RefCountedLeakCounter.cpp',
            'wtf/RefCountedLeakCounter.h',
            'wtf/RefPtr.h',
            'wtf/RefPtrHashMap.h',
            'wtf/RetainPtr.h',
            'wtf/SegmentedVector.h',
            'wtf/SizeLimits.cpp',
            'wtf/StaticConstructors.h',
            'wtf/StdLibExtras.h',
            'wtf/StringExtras.h',
            'wtf/StringHashFunctions.h',
            'wtf/TCPackedCache.h',
            'wtf/qt/MainThreadQt.cpp',
            'wtf/qt/StringQt.cpp',
            'wtf/qt/ThreadingQt.cpp',
            'wtf/TCPageMap.h',
            'wtf/TCSpinLock.h',
            'wtf/TCSystemAlloc.cpp',
            'wtf/TCSystemAlloc.h',
            'wtf/ThreadIdentifierDataPthreads.cpp',
            'wtf/ThreadIdentifierDataPthreads.h',
            'wtf/Threading.cpp',
            'wtf/Threading.h',
            'wtf/ThreadingNone.cpp',
            'wtf/ThreadingPrimitives.h',
            'wtf/ThreadingPthreads.cpp',
            'wtf/ThreadingWin.cpp',
            'wtf/ThreadSafeShared.h',
            'wtf/ThreadSpecific.h',
            'wtf/ThreadSpecificWin.cpp',
            'wtf/TypeTraits.cpp',
            'wtf/TypeTraits.h',
            'wtf/text/AtomicString.cpp',
            'wtf/text/AtomicString.h',
            'wtf/text/AtomicStringImpl.h',
            'wtf/text/CString.cpp',
            'wtf/text/CString.h',
            'wtf/text/StringHash.h',
            'wtf/text/StringImpl.cpp',
            'wtf/text/StringImpl.h',
            'wtf/text/StringStatics.cpp',
            'wtf/text/WTFString.cpp',
            'wtf/text/WTFString.h',
            'wtf/unicode/Collator.h',
            'wtf/unicode/CollatorDefault.cpp',
            'wtf/unicode/glib/UnicodeGLib.cpp',
            'wtf/unicode/glib/UnicodeGLib.h',
            'wtf/unicode/glib/UnicodeMacrosFromICU.h',
            'wtf/unicode/icu/CollatorICU.cpp',
            'wtf/unicode/icu/UnicodeIcu.h',
            'wtf/unicode/qt4/UnicodeQt4.h',
            'wtf/unicode/Unicode.h',
            'wtf/unicode/UTF8.cpp',
            'wtf/unicode/UTF8.h',
            'wtf/UnusedParam.h',
            'wtf/ValueCheck.h',
            'wtf/Vector.h',
            'wtf/VectorTraits.h',
            'wtf/VMTags.h',
            'wtf/WTFThreadData.cpp',
            'wtf/WTFThreadData.h',
            'wtf/win/MainThreadWin.cpp',
            'wtf/win/OwnPtrWin.cpp',
            'wtf/wx/MainThreadWx.cpp',
            'yarr/RegexCompiler.cpp',
            'yarr/RegexCompiler.h',
            'yarr/RegexInterpreter.cpp',
            'yarr/RegexInterpreter.h',
            'yarr/RegexJIT.cpp',
            'yarr/RegexJIT.h',
            'yarr/RegexParser.h',
            'yarr/RegexPattern.h',
        ]
    }
}
