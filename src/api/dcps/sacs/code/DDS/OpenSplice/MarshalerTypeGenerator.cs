using System;
using System.Collections.Generic;
using System.Text;
using System.CodeDom.Compiler;
using Microsoft.CSharp;
using System.CodeDom;
using System.Reflection;
using System.IO;

using DDS.OpenSplice.Database;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    public class MarshalerTypeGenerator : IMarshalerTypeGenerator
    {
        public enum InitPhase {
            NothingDefined      = 0x0000,
            CursorDefined       = 0x0001 << 0,
            MemberObjDefined    = 0x0001 << 1
        } 
        
        public MarshalerTypeGenerator()
        { }

        public DatabaseMarshaler CreateMarshaler(IntPtr participant, IntPtr metaData, Type dataType)
        {
            CompilerResults results = null;

            // Generate the interface wrapper here and add to namespace
            CodeTypeDeclaration marshalerClass = CreateMarshalerClass(participant, dataType, metaData);

            try
            {
                CSharpCodeProvider codeProvider = new CSharpCodeProvider();

                List<string> assemLocationList = new List<string>();
                GetReferencedAssemblyLocations(dataType.Assembly, ref assemLocationList);

                CodeCompileUnit compileUnit = new CodeCompileUnit();

                // Declare a new namespace...
                CodeNamespace implNamespace = new CodeNamespace(dataType.Namespace);

                // Add the new namespace to the compile unit.
                compileUnit.Namespaces.Add(implNamespace);

                // Add the new namespace import for the System namespace.
                implNamespace.Imports.Add(new CodeNamespaceImport("System"));
                implNamespace.Imports.Add(new CodeNamespaceImport("System.Runtime.InteropServices"));
                implNamespace.Imports.Add(new CodeNamespaceImport("DDS"));
                implNamespace.Imports.Add(new CodeNamespaceImport("DDS.OpenSplice"));

                implNamespace.Types.Add(marshalerClass);

                // Compile the code in memory
                results = CompileCode(codeProvider, assemLocationList, compileUnit);

                // Load Assembly and Type here
                Type marshalerType = results.CompiledAssembly.GetType(dataType.Namespace + "." + marshalerClass.Name);
                return Activator.CreateInstance(marshalerType, this, participant) as DatabaseMarshaler;
            }
            catch (Exception)
            {
                if (results != null && results.Errors.HasErrors)
                {
                    string errorStr = "There were errors during type creation." + Environment.NewLine;
                    foreach (CompilerError error in results.Errors)
                    {
                        errorStr += "	 " + error.ToString() + Environment.NewLine;
                    }
                    Console.WriteLine(errorStr);
                }
                throw;
            }
        }

        private void GetReferencedAssemblyLocations(Assembly assembly, ref List<string> assemLocationList)
        {
            assemLocationList.Add(assembly.Location);

            AssemblyName[] test = assembly.GetReferencedAssemblies();

            foreach (AssemblyName assem in assembly.GetReferencedAssemblies())
            {
                assemLocationList.Add(Assembly.Load(assem).Location);
            }
        }

        private CompilerResults CompileCode(CodeDomProvider provider, List<string> assemLocationList,
                                                  CodeCompileUnit compileUnit)
        {
            // Configure a CompilerParameters that links referenced assemblies
            // and produces the specified executable file.
            CompilerParameters cp = new CompilerParameters(assemLocationList.ToArray());

            // Generate an executable rather than a DLL file.
            cp.GenerateExecutable = false;

#if DEBUG
            //cp.GenerateInMemory = true;
            cp.IncludeDebugInformation = true;

            CodeGeneratorOptions options = new CodeGeneratorOptions();
            options.BracingStyle = "C";

            string assemblyLoc = Assembly.GetExecutingAssembly().Location;
            string path = Path.Combine(
                Path.GetFullPath(assemblyLoc).Replace(Path.GetFileName(assemblyLoc), string.Empty),
                "DDSGenerated");
            if (!Directory.Exists(path))
            {
                Directory.CreateDirectory(path);
            }
            string sourceFile = Path.Combine(path,
                compileUnit.Namespaces[0].Types[0].Name + ".cs");

            IndentedTextWriter tw = new IndentedTextWriter(new StreamWriter(sourceFile, false), "\t");

            // Generate source code using the code generator.
            provider.GenerateCodeFromCompileUnit(compileUnit, tw, options);

            // Close the output file.
            tw.Close();

            cp.TempFiles = new TempFileCollection(path, true); 
            CompilerResults cr = provider.CompileAssemblyFromFile(cp, sourceFile);
#else
            cp.GenerateInMemory = true;
            cp.IncludeDebugInformation = false;

            // Invoke compilation.
            CompilerResults cr = provider.CompileAssemblyFromDom(cp, compileUnit);
#endif

            // Return the results of compilation.
            return cr;
        }

        private CodeTypeDeclaration CreateMarshalerClass(
                IntPtr participant, 
                Type dataType, 
                IntPtr metaData)
        {
            string marshalerClassName = string.Format("{0}Marshaler", dataType.Name);
            CodeTypeDeclaration implClass = new CodeTypeDeclaration(marshalerClassName);
            implClass.Attributes = MemberAttributes.Public | MemberAttributes.Final;

            implClass.BaseTypes.Add(typeof(DatabaseMarshaler));

            //CreateFieldsAndProperties(implClass);
            CreateConstructor(implClass, dataType, metaData, participant);            

            CreateSampleReaderAlloc(implClass, dataType);
            CreateCopyIn(implClass, dataType, metaData);
            CreateCopyOut(implClass, dataType, metaData);

            return implClass;
        }

        private void CreateSampleReaderAlloc(CodeTypeDeclaration implClass, Type dataType)
        {
            CodeMemberMethod sampleReaderAllocMethod = new CodeMemberMethod();
            sampleReaderAllocMethod.Name = "SampleReaderAlloc";
            sampleReaderAllocMethod.Attributes = MemberAttributes.Public | MemberAttributes.Override;
            sampleReaderAllocMethod.ReturnType = new CodeTypeReference(typeof(object[]));

            CodeParameterDeclarationExpression length =
                new CodeParameterDeclarationExpression(typeof(int), "length");
            sampleReaderAllocMethod.Parameters.Add(length);
            
            CodeMethodReturnStatement methodReturn = new CodeMethodReturnStatement(
                new CodeSnippetExpression(string.Format("new {0}[length]", dataType.FullName)));
            sampleReaderAllocMethod.Statements.Add(methodReturn);
            
            implClass.Members.Add(sampleReaderAllocMethod);
        }

        private string CreateArrayIndex(
                IntPtr collStartType,
                int dimension)
        {
            IntPtr actualType;
            
            // If no dimension specified, return an empty string.
            if (dimension < 0) return string.Empty;
            
            // Assert that startType is always a collection type and has at least 
            // the specified number of dimensions.
            string result = "[i0";
            for (int i = 1; i < dimension; i++)
            {
                actualType = Gapi.MetaData.typeActualType(collStartType);
                
                // Assert actualType is a collection type.
                switch (Gapi.MetaData.collectionTypeKind(actualType))
                {
                case c_collKind.C_SEQUENCE:
                    result += string.Format("][i{0}", i);
                    break;
                case c_collKind.C_ARRAY:
                    result += string.Format(",i{0}", i);
                    break;
                default:
                    throw new Exception("Unsupported Collection type");
                }
                collStartType = Gapi.MetaData.collectionTypeSubType(actualType);
            }
            result += "]";

            return result;
        }

        private void CreateCopyOut(
                CodeTypeDeclaration implClass, 
                Type dataType, 
                IntPtr metaData)
        {
            CodeMemberMethod copyOutMethod = new CodeMemberMethod();
            copyOutMethod.Name = "CopyOut";
            copyOutMethod.Attributes = MemberAttributes.Public | MemberAttributes.Override;

            CodeParameterDeclarationExpression fromPtr =
                new CodeParameterDeclarationExpression(typeof(IntPtr), "from");
            CodeParameterDeclarationExpression toObj =
                new CodeParameterDeclarationExpression(typeof(object), "to");
            toObj.Direction = FieldDirection.Ref;
            CodeParameterDeclarationExpression offset =
                new CodeParameterDeclarationExpression(typeof(int), "offset");
            copyOutMethod.Parameters.Add(fromPtr);
            copyOutMethod.Parameters.Add(toObj);
            copyOutMethod.Parameters.Add(offset);

            CodeSnippetExpression checkInit = new CodeSnippetExpression("if (!initDone) { /* Assert */}");
            copyOutMethod.Statements.Add(checkInit);

            CodeSnippetExpression assignSnippet = new CodeSnippetExpression(
                    string.Format("{0} dataTo = to as {0}", dataType.FullName));
            CodeConditionStatement condition = new CodeConditionStatement(
                    new CodeSnippetExpression("dataTo == null"), 
                    new CodeStatement[] {
                            new CodeExpressionStatement(new CodeSnippetExpression(
                                    string.Format("dataTo = new {0}()", dataType.FullName))),
                            new CodeExpressionStatement(new CodeSnippetExpression("to = dataTo"))
                    } );
            copyOutMethod.Statements.Add(assignSnippet);
            copyOutMethod.Statements.Add(condition);

            if (Gapi.MetaData.baseObjectKind(metaData) == c_metaKind.M_STRUCTURE)
            {
                InitPhase initState = InitPhase.NothingDefined;
                int nrMembers = Gapi.MetaData.structureMemberCount(metaData);
                for (int i = 0; i < nrMembers; i++)
                {
                    IntPtr member = Gapi.MetaData.structureMember(metaData, i);
                    string fieldName = Gapi.MetaData.specifierName(member);
                    FieldInfo field = dataType.GetField(fieldName);
                    CreateStructMemberRead(
                            copyOutMethod, 
                            field.FieldType, 
                            fieldName, member, 
                            i, 
                            ref initState);
                }
            }
            else
            {
                throw new Exception("Non-structured Datatypes not yet supproted.");
            }

            implClass.Members.Add(copyOutMethod);
        }

        private void CreateStructMemberRead(
                CodeMemberMethod copyOutMethod,
                Type fieldType,
                string fieldName,
                IntPtr member, 
                int index,
                ref InitPhase initState)
        {
            string snippet = string.Empty;
            IntPtr memberType = Gapi.MetaData.memberType(member);
            uint offset = Gapi.MetaData.memberOffset(member);

            switch(Gapi.MetaData.baseObjectKind(memberType))
            {
            case c_metaKind.M_STRUCTURE:
                // Handle embedded struct.
                if ((initState & InitPhase.MemberObjDefined) == InitPhase.NothingDefined)
                {
                    snippet = "object ";
                    initState |= InitPhase.MemberObjDefined;
                }
                copyOutMethod.Statements.Add(new CodeSnippetExpression(
                        string.Format("{0}memberVal = dataTo.{1}", snippet, fieldName)));
                copyOutMethod.Statements.Add(new CodeSnippetExpression(
                        string.Format("attr{0}Marshaler.CopyOut(from, ref memberVal, offset + {1})", 
                                index, offset)));
                copyOutMethod.Statements.Add(new CodeConditionStatement(
                        new CodeSnippetExpression(
                                string.Format("dataTo.{0} == null", fieldName)),
                        new CodeExpressionStatement(new CodeSnippetExpression(
                                string.Format("dataTo.{0} = memberVal as {1}", 
                                        fieldName, fieldType.FullName)))));
                break;
            case c_metaKind.M_ENUMERATION:
                // Handle enum.
                copyOutMethod.Statements.Add(new CodeSnippetExpression(
                        string.Format("dataTo.{0} = ({1}) ReadUInt32(from, offset + {2})", 
                                fieldName, fieldType.FullName, offset)));
                break;
            case c_metaKind.M_PRIMITIVE: 
                // Handle primitive.
                switch (Gapi.MetaData.primitiveKind(memberType))
                {
                case c_primKind.P_BOOLEAN:
                case c_primKind.P_CHAR:         case c_primKind.P_OCTET:
                case c_primKind.P_SHORT:        case c_primKind.P_USHORT:
                case c_primKind.P_LONG:         case c_primKind.P_ULONG:
                case c_primKind.P_LONGLONG:     case c_primKind.P_ULONGLONG:
                case c_primKind.P_FLOAT:        case c_primKind.P_DOUBLE:        
//                case "Duration":
//                case "Time":
//                case "InstanceHandle":
//                case "IntPtr":
                    copyOutMethod.Statements.Add(new CodeSnippetExpression(
                            string.Format("dataTo.{0} = Read{1}(from, offset + {2})", 
                                    fieldName, fieldType.Name, offset)));
                    break;
                default:
                    throw new Exception("Unsupported primitive type");
                }
                break; 
            case c_metaKind.M_COLLECTION:
                // Handle Collection type.
                switch (Gapi.MetaData.collectionTypeKind(memberType))
                {
                case c_collKind.C_STRING:
                    copyOutMethod.Statements.Add(new CodeSnippetExpression(
                            string.Format("dataTo.{0} = Read{1}(from, offset + {2})", 
                                    fieldName, fieldType.Name, offset)));
                    break;
                case c_collKind.C_SEQUENCE:
                    // Handle sequences.
                    copyOutMethod.Statements.Add(new CodeSnippetExpression(
                            string.Format("//int length{0} = from.{1}.Length", 
                                    index, fieldName)));
                    break;
                case c_collKind.C_ARRAY:
                    // Handle arrays.
                    if ((initState & InitPhase.CursorDefined) == InitPhase.NothingDefined)
                    {
                        snippet = "int ";
                        initState |= InitPhase.CursorDefined;
                    }
                    copyOutMethod.Statements.Add(new CodeSnippetExpression(
                            string.Format("{0}cursor = {1}", snippet, offset)));
                    copyOutMethod.Statements.AddRange(
                            CreateArrayMemberRead(memberType, fieldType, index, 0, fieldName));
                    break;
                default:
                    throw new Exception("Unsupported Collection type");
                }
                break;
            default:
                throw new Exception("Unsupported Base type");
            }
        }
        
        private CodeStatement[] CreateArrayMemberRead(
                IntPtr type,
                Type elementType,
                int index,
                int dimension,
                string fieldName)
        {
            CodeStatement[] result;
            c_metaKind memberKind = Gapi.MetaData.baseObjectKind(type);
            switch(memberKind)
            {
            case c_metaKind.M_STRUCTURE:
            case c_metaKind.M_ENUMERATION:
            case c_metaKind.M_PRIMITIVE: 
                result = CreateArrayMemberReadInnerLoopBody(type, elementType, index, dimension, fieldName, memberKind);
                break;
            case c_metaKind.M_COLLECTION:
                // Handle Collection type.
                switch (Gapi.MetaData.collectionTypeKind(type))
                {
                case c_collKind.C_STRING:
                    // Handle strings.
                    result = CreateArrayMemberReadInnerLoopBody(type, elementType, index, dimension, fieldName, memberKind);
                    break;
                case c_collKind.C_ARRAY:
                    // Handle arrays.
                    int arrLength = Gapi.MetaData.collectionTypeMaxSize(type);
                    if (elementType.IsArray) 
                        elementType = elementType.GetElementType();
                    IntPtr subType = Gapi.MetaData.collectionTypeSubType(type);
                    CodeStatement[] forBody = CreateArrayMemberRead(
                            subType, 
                            elementType,
                            index,
                            dimension + 1, 
                            fieldName);
                    result = new CodeStatement[1];
                    result[0] = new CodeIterationStatement(
                        //string.Format("int i{0} = 0", dimension)
                            new CodeExpressionStatement(
                                    new CodeSnippetExpression(string.Format("int i{0} = 0", dimension))), 
                            new CodeSnippetExpression(string.Format("i{0} < {1}", dimension, arrLength)),
                            new CodeExpressionStatement(
                                    new CodeSnippetExpression(string.Format("i{0}++", dimension))),
                            forBody);
                    break;
                default:
                    throw new Exception("Unsupported Collection type");
                }
                break;
            default:
                throw new Exception("Unsupported Array type");
            }
                        
            return result;
        }
        
        private CodeStatement[] CreateArrayMemberReadInnerLoopBody(
                IntPtr type,
                Type elementType,
                int index,
                int dimension,
                string fieldName,
                c_metaKind memberKind)
        {
            CodeStatement[] result;
            string arrayBrackets = "[i0";            
            for (int currDim = 1; currDim < dimension; currDim++)
            {
                arrayBrackets += string.Format(",i{0}", currDim);
            }
            arrayBrackets += "]";
            
            switch(memberKind)
            {
            case c_metaKind.M_STRUCTURE:
                result = new CodeStatement[4];
                result[0] = new CodeExpressionStatement(new CodeSnippetExpression(
                        string.Format("object elementObj = dataTo.{0}{1}", fieldName, arrayBrackets)));
                result[1] = new CodeExpressionStatement(new CodeSnippetExpression(
                        string.Format("attr{0}Marshaler.CopyOut(from, ref elementObj, offset + cursor)",
                                index)));
                result[2] = new CodeConditionStatement(
                        new CodeSnippetExpression(
                                string.Format("dataTo.{0}{1} == null", fieldName, arrayBrackets)),
                        new CodeExpressionStatement(new CodeSnippetExpression(
                                string.Format("dataTo.{0}{1} = elementObj as {2}", 
                                        fieldName, arrayBrackets, elementType.FullName)))); 
                break;
            case c_metaKind.M_ENUMERATION:
                result = new CodeStatement[2];
                result[0] = new CodeExpressionStatement(new CodeSnippetExpression(
                        string.Format("dataTo.{0}{1} = ({2}) ReadUInt32(from, offset + cursor)", 
                                fieldName, arrayBrackets, elementType.FullName))); 
                break;
            case c_metaKind.M_PRIMITIVE:
            case c_metaKind.M_COLLECTION: // Assert that this collection is always of type string!!
                result = new CodeStatement[2];
                result[0] = new CodeExpressionStatement(new CodeSnippetExpression(
                        string.Format("dataTo.{0}{1} = Read{2}(from, offset + cursor)", 
                                fieldName, arrayBrackets, elementType.Name))); 
                break;
            default:
                throw new Exception("Unsupported Array type");
            }
            result[result.Length -1] = new CodeExpressionStatement(new CodeSnippetExpression(
                    string.Format("cursor += {0}", Gapi.MetaData.typeSize(type)))); 
            return result;
        }
        
        private void CreateCopyIn(
                CodeTypeDeclaration implClass, 
                Type dataType, 
                IntPtr metaData)
        {
            CodeMemberMethod copyInMethodFromNative = new CodeMemberMethod();
            copyInMethodFromNative.Name = "CopyIn";
            copyInMethodFromNative.Attributes = MemberAttributes.Public | MemberAttributes.Override;
            copyInMethodFromNative.ReturnType = new CodeTypeReference(typeof(bool));

            CodeParameterDeclarationExpression basePtr = 
                new CodeParameterDeclarationExpression(typeof(IntPtr), "basePtr");
            CodeParameterDeclarationExpression fromPtr =
                new CodeParameterDeclarationExpression(typeof(IntPtr), "from");
            CodeParameterDeclarationExpression toPtr =
                new CodeParameterDeclarationExpression(typeof(IntPtr), "to");
            copyInMethodFromNative.Parameters.Add(basePtr);
            copyInMethodFromNative.Parameters.Add(fromPtr);
            copyInMethodFromNative.Parameters.Add(toPtr);

            copyInMethodFromNative.Statements.Add(new CodeSnippetExpression(
                    "GCHandle tmpGCHandle = GCHandle.FromIntPtr(from)"));
            copyInMethodFromNative.Statements.Add(new CodeSnippetExpression(
                    "object fromData = tmpGCHandle.Target"));
            copyInMethodFromNative.Statements.Add(new CodeMethodReturnStatement(
                    new CodeSnippetExpression("CopyIn(basePtr, fromData, to, 0)")));

            implClass.Members.Add(copyInMethodFromNative);

            CodeMemberMethod copyInMethod = new CodeMemberMethod();
            copyInMethod.Name = "CopyIn";
            copyInMethod.Attributes = MemberAttributes.Public | MemberAttributes.Override;;
            copyInMethod.ReturnType = new CodeTypeReference(typeof(bool));

            CodeParameterDeclarationExpression fromType = new CodeParameterDeclarationExpression(
                typeof(object), "untypedFrom");
            CodeParameterDeclarationExpression offset = new CodeParameterDeclarationExpression(
                typeof(int), "offset");
            copyInMethod.Parameters.Add(basePtr);
            copyInMethod.Parameters.Add(fromType);
            copyInMethod.Parameters.Add(toPtr);
            copyInMethod.Parameters.Add(offset);

            copyInMethod.Statements.Add(new CodeSnippetExpression(
                    "if (!initDone) { /* Assert */}"));            
            copyInMethod.Statements.Add(new CodeSnippetExpression(
                    string.Format("{0} from = untypedFrom as {0}", dataType.FullName)));
            copyInMethod.Statements.Add(new CodeSnippetExpression(
                    "if (from == null) return false"));            

            if (Gapi.MetaData.baseObjectKind(metaData) == c_metaKind.M_STRUCTURE)
            {
                int nrMembers = Gapi.MetaData.structureMemberCount(metaData);
                InitPhase initState = InitPhase.NothingDefined;
                for (int i = 0; i < nrMembers; i++)
                {
                	IntPtr member = Gapi.MetaData.structureMember(metaData, i);
                    CreateStructMemberWrite(copyInMethod, member, i, ref initState);
                }
            }
            else
            {
                throw new Exception("Non-structured Datatypes not yet supproted.");
            }

            CodeMethodReturnStatement methodReturn = new CodeMethodReturnStatement(
                new CodePrimitiveExpression(true));
            copyInMethod.Statements.Add(methodReturn);

            implClass.Members.Add(copyInMethod);
        }

        private void CreateStructMemberWrite(
                CodeMemberMethod copyInMethod,
                IntPtr member, 
                int index,
                ref InitPhase initState)
        {
            IntPtr memberType = Gapi.MetaData.memberType(member);
            IntPtr actualType = Gapi.MetaData.typeActualType(memberType);
            string fieldName = Gapi.MetaData.specifierName(member);
            uint offset = Gapi.MetaData.memberOffset(member);

            switch(Gapi.MetaData.baseObjectKind(actualType))
            {
            case c_metaKind.M_STRUCTURE:
                // Handle embedded struct.
                copyInMethod.Statements.Add(new CodeSnippetExpression(string.Format(
                        "if (!attr{0}Marshaler.CopyIn(basePtr, from.{1}, to, offset + {2})) return false", 
                        index, fieldName, offset)));
                break;
            case c_metaKind.M_ENUMERATION:
                // Handle enum.
                copyInMethod.Statements.Add(new CodeSnippetExpression(string.Format(
                        "Write(to, offset + {0}, (uint) from.{1})", 
                        offset, fieldName)));
                break;
            case c_metaKind.M_PRIMITIVE: 
                // Handle primitive.
                switch (Gapi.MetaData.primitiveKind(memberType))
                {
                case c_primKind.P_BOOLEAN:
                case c_primKind.P_CHAR:         case c_primKind.P_OCTET:
                case c_primKind.P_SHORT:        case c_primKind.P_USHORT:
                case c_primKind.P_LONG:         case c_primKind.P_ULONG:
                case c_primKind.P_LONGLONG:     case c_primKind.P_ULONGLONG:
                case c_primKind.P_FLOAT:        case c_primKind.P_DOUBLE:        
//                case "Duration":
//                case "Time":
//                case "InstanceHandle":
//                case "IntPtr":
                    copyInMethod.Statements.Add(new CodeSnippetExpression(
                            string.Format("Write(to, offset + {0}, from.{1})", offset, fieldName)));
                    break;
                default:
                    throw new Exception("Unsupported primitive type");
                }
                break;
            case c_metaKind.M_COLLECTION:
                // Handle Collection type.
                switch (Gapi.MetaData.collectionTypeKind(memberType))
                {
                case c_collKind.C_STRING:
                    // Handle strings.
                    copyInMethod.Statements.Add(new CodeSnippetExpression(
                            string.Format("if (from.{0} == null) return false", fieldName)));
                    copyInMethod.Statements.Add(new CodeSnippetExpression(
                            string.Format("Write(basePtr, to, offset + {0}, ref from.{1})", 
                                    offset,fieldName))); 
                    break;
                case c_collKind.C_SEQUENCE:
                case c_collKind.C_ARRAY:
                    // Handle arrays.
                    string snippet = string.Empty;
                    if ((initState & InitPhase.CursorDefined) == InitPhase.NothingDefined)
                    {
                        snippet = "int ";
                        initState |= InitPhase.CursorDefined;
                    }
                    copyInMethod.Statements.Add(new CodeSnippetExpression(
                            string.Format("{0}cursor0 = offset + {1}", snippet, offset)));
                    copyInMethod.Statements.AddRange(
                            CreateArrayMemberWrite(actualType, actualType, index, 0, 
                                    fieldName, "to", "cursor0"));
                    break;
                default:
                    throw new Exception("Unsupported Collection type");
                }
                break;
            default:
                throw new Exception("Unsupported Base type");
            }
        }
        
        private CodeStatement[] CreateArrayMemberWrite(
                IntPtr collStartType,
                IntPtr arrayType,
                int index,
                int dimension,
                string fieldName,
                string bufName,
                string cursorName)
        {
            CodeStatement[] result;
            int arrLength, seqMaxLength;
            string seqLengthName, seqBufName, seqTypeName, nextCursorName;
            IntPtr subType;
            IntPtr actualType;
            CodeStatement[] forBody;
            
            c_metaKind memberKind = Gapi.MetaData.baseObjectKind(arrayType);
            switch(memberKind)
            {
            case c_metaKind.M_STRUCTURE:
            case c_metaKind.M_ENUMERATION:
            case c_metaKind.M_PRIMITIVE: 
                result = CreateArrayMemberWriteInnerLoopBody(
                        collStartType, arrayType, memberKind, index, dimension, 
                                fieldName, bufName, cursorName);
                break;
            case c_metaKind.M_COLLECTION:
                // Handle Collection type.
                switch (Gapi.MetaData.collectionTypeKind(arrayType))
                {
                case c_collKind.C_STRING:
                    // Handle strings.
                    result = CreateArrayMemberWriteInnerLoopBody(
                            collStartType, arrayType, memberKind, index, dimension, 
                                    fieldName, bufName, cursorName);
                    break;
                case c_collKind.C_ARRAY:
                    // Handle arrays.
                    arrLength = Gapi.MetaData.collectionTypeMaxSize(arrayType);
                    subType = Gapi.MetaData.collectionTypeSubType(arrayType);
                    actualType = Gapi.MetaData.typeActualType(subType);
                    forBody = CreateArrayMemberWrite(collStartType, actualType, index, 
                            dimension + 1, fieldName, bufName, cursorName);
                    result = new CodeStatement[1];
                    result[0] = new CodeIterationStatement(
                            new CodeExpressionStatement(new CodeSnippetExpression(
                                    string.Format("int i{0} = 0", dimension))), 
                            new CodeSnippetExpression(
                                    string.Format("i{0} < {1}", dimension, arrLength)),
                            new CodeExpressionStatement(new CodeSnippetExpression(
                                    string.Format("i{0}++", dimension))),
                            forBody);
                    break;
                case c_collKind.C_SEQUENCE:
                    // Handle sequences.
                    subType = Gapi.MetaData.collectionTypeSubType(arrayType);
                    actualType = Gapi.MetaData.typeActualType(subType);
                    seqLengthName = string.Format("attr{0}Seq{1}Length", index, dimension);
                    seqBufName = string.Format("attr{0}Seq{1}Buf", index, dimension);
                    seqTypeName = string.Format("attr{0}Seq{1}Type", index, dimension);
                    nextCursorName = string.Format("cursor{0}", dimension + 1);
                    seqMaxLength = Gapi.MetaData.collectionTypeMaxSize(arrayType);
                    
                    forBody = CreateArrayMemberWrite(collStartType, actualType, index, 
                            dimension + 1, fieldName, seqBufName, nextCursorName);
                    result = new CodeStatement[6];
                    result[0] = new CodeExpressionStatement(new CodeSnippetExpression(
                            string.Format("int {0} = from.{1}{2}.Length", seqLengthName, fieldName, 
                                    CreateArrayIndex(collStartType, dimension - 1))));
                    if (seqMaxLength == 0)
                    {
                        result[1] = new CodeCommentStatement("Unbounded sequence: bounds check not required...");
                    }
                    else
                    {
                        result[1] = new CodeExpressionStatement(new CodeSnippetExpression(
                                string.Format("if ({0} >= {1}) return false", seqLengthName, seqMaxLength)));
                    }
                    result[2] = new CodeExpressionStatement(new CodeSnippetExpression(
                            string.Format("IntPtr {0} = DDS.OpenSplice.Database.c.newArray({1}, {2})", 
                                    seqBufName, seqTypeName, seqLengthName)));
                    result[3] = new CodeExpressionStatement(new CodeSnippetExpression(
                            string.Format("Write({0}, {1}, {2})", bufName, cursorName, seqBufName)));
                    result[4] = new CodeExpressionStatement(new CodeSnippetExpression(
                            string.Format("int {0} = 0", nextCursorName)));
                    result[5] = new CodeIterationStatement(
                            new CodeExpressionStatement(new CodeSnippetExpression(
                                    string.Format("int i{0} = 0", dimension))), 
                            new CodeSnippetExpression(
                                    string.Format("i{0} < {1}", dimension, seqLengthName)),
                            new CodeExpressionStatement(new CodeSnippetExpression(
                                    string.Format("i{0}++", dimension))), 
                            forBody);
                    break;
                default:
                    throw new Exception("Unsupported Collection type");
                }
                break;
            default:
                throw new Exception("Unsupported Array type");
            }
                        
            return result;
        }

        private CodeStatement[] CreateArrayMemberWriteInnerLoopBody(
                IntPtr collStartType,
                IntPtr memberType,
                c_metaKind memberKind,
                int index,
                int dimension,
                string fieldName,
                string bufName,
                string cursorName)
        {
            string snippet;
            string arrayBrackets = CreateArrayIndex(collStartType, dimension);
            switch(memberKind)
            {
            case c_metaKind.M_STRUCTURE:
                snippet = string.Format(
                        "if (!attr{0}Marshaler.CopyIn(basePtr, from.{1}{2}, {3}, {4})) " +
                        "return false",
                        index, fieldName, arrayBrackets, bufName, cursorName);
                break;
            case c_metaKind.M_ENUMERATION:
                snippet = string.Format("Write({0}, {1}, (uint) from.{2}{3})", 
                        bufName, cursorName, fieldName, arrayBrackets);
                break;
            case c_metaKind.M_PRIMITIVE:
                snippet = string.Format("Write({0}, {1}, from.{2}{3})", 
                        bufName, cursorName, fieldName, arrayBrackets);
                break;
            case c_metaKind.M_COLLECTION:
                // Assert that this collection is always of type string!!
                snippet = string.Format(
                        "Write(basePtr, {0}, {1}, ref from.{2}{3})", 
                        bufName, cursorName, fieldName, arrayBrackets);
                break;
            default:
                throw new Exception("Unsupported Array type");
            }
             
            CodeStatement[] result = new CodeStatement[2];
            result[0] = new CodeExpressionStatement(new CodeSnippetExpression(snippet));
            snippet = string.Format("{0} += {1}", cursorName, Gapi.MetaData.typeSize(memberType)); 
            result[1] = new CodeExpressionStatement(new CodeSnippetExpression(snippet));
            
            return result;
        }
                

//        private void CreateFieldsAndProperties(CodeTypeDeclaration implClass)
//        {
//            CodeMemberField copyInDelegateField = new CodeMemberField(typeof(SampleCopyInDelegate),
//                "copyInDelegate");
//            copyInDelegateField.Attributes = MemberAttributes.Private;

//            CodeMemberField copyOutDelegateField = new CodeMemberField(typeof(SampleCopyOutDelegate),
//                "copyOutDelegate");
//            copyOutDelegateField.Attributes = MemberAttributes.Private;

//            implClass.Members.Add(copyInDelegateField);
//            implClass.Members.Add(copyOutDelegateField);

//            CodeMemberProperty copyInProperty = new CodeMemberProperty();
//            copyInProperty.Name = "CopyInDelegate";
//            copyInProperty.Type = new CodeTypeReference(typeof(SampleCopyInDelegate));
//            copyInProperty.Attributes = MemberAttributes.Public | MemberAttributes.Override;
//            CodeMethodReturnStatement copyInReturn = new CodeMethodReturnStatement(new CodeFieldReferenceExpression(
//                new CodeThisReferenceExpression(), copyInDelegateField.Name));
//            copyInProperty.GetStatements.Add(copyInReturn);
//            implClass.Members.Add(copyInProperty);

//            CodeMemberProperty copyOutProperty = new CodeMemberProperty();
//            copyOutProperty.Name = "CopyOutDelegate";
//            copyOutProperty.Type = new CodeTypeReference(typeof(SampleCopyOutDelegate));
//            copyOutProperty.Attributes = MemberAttributes.Public | MemberAttributes.Override;
//            CodeMethodReturnStatement copyOutReturn = new CodeMethodReturnStatement(new CodeFieldReferenceExpression(
//                new CodeThisReferenceExpression(), copyOutDelegateField.Name));
//            copyOutProperty.GetStatements.Add(copyOutReturn);
//            implClass.Members.Add(copyOutProperty);
//        }

        private void CreateConstructor(
                CodeTypeDeclaration implClass, 
                Type dataType, 
                IntPtr metaData, 
                IntPtr participant)
        {
            string fieldName;
            Type fieldType;
            
            // Constructor
            CodeConstructor defaultConstructor = new CodeConstructor();
            defaultConstructor.Attributes = MemberAttributes.Public;
            CodeParameterDeclarationExpression marshalGenPtr = 
                new CodeParameterDeclarationExpression(typeof(IMarshalerTypeGenerator), "generator");
            defaultConstructor.Parameters.Add(marshalGenPtr);
            CodeParameterDeclarationExpression participantPtr = 
                new CodeParameterDeclarationExpression(typeof(IntPtr), "participant");
            defaultConstructor.Parameters.Add(participantPtr);
            
            if (Gapi.MetaData.baseObjectKind(metaData) == c_metaKind.M_STRUCTURE)
            {
                int nrMembers = Gapi.MetaData.structureMemberCount(metaData);
                for (int i = 0; i < nrMembers; i++)
                {
                	IntPtr member = Gapi.MetaData.structureMember(metaData, i);
                    IntPtr memberType = Gapi.MetaData.memberType(member);
                    IntPtr actualType = Gapi.MetaData.typeActualType(memberType);
                    c_metaKind memberKind = Gapi.MetaData.baseObjectKind(actualType);
                    switch(memberKind)
                    {
                    case c_metaKind.M_STRUCTURE: 
                        // Fetch Marshaler for embedded struct type.
                        fieldName = Gapi.MetaData.specifierName(member);
                        fieldType = dataType.GetField(fieldName).FieldType;
                        implClass.Members.Add(new CodeMemberField(
                                typeof(DatabaseMarshaler), 
                                string.Format("attr{0}Marshaler", i)));
                        defaultConstructor.Statements.Add(new CodeSnippetExpression(
                                string.Format("attr{0}Marshaler = GetMarshaler(participant, typeof({1}))", 
                                        i, fieldType.FullName)));
                        DatabaseMarshaler.Create(participant, actualType, fieldType, this);
                        break;
                    case c_metaKind.M_COLLECTION:
                        // Fetch Marshaler for member type when that turns out to be a struct.
                        fieldName = Gapi.MetaData.specifierName(member);
                        fieldType = dataType.GetField(fieldName).FieldType;
                        
                        // Iterate to the element type of the collection.
                        for (int j = 0; memberKind == c_metaKind.M_COLLECTION; j++)
                        {
                            // For sequences, cache the database type for CopyIn.
                            if (Gapi.MetaData.collectionTypeKind(actualType) == c_collKind.C_SEQUENCE)
                            {
                                implClass.Members.Add(new CodeMemberField(
                                        typeof(IntPtr), 
                                        string.Format("attr{0}Seq{1}Type", i, j)));
                                defaultConstructor.Statements.Add(new CodeSnippetExpression(
                                        string.Format("attr{0}Seq{1}Type = new IntPtr({2})", i, j, actualType)));
                            } 
                            memberType = Gapi.MetaData.collectionTypeSubType(actualType);
                            actualType = Gapi.MetaData.typeActualType(memberType);
                            memberKind = Gapi.MetaData.baseObjectKind(actualType);
                            if (fieldType.IsArray) 
                                fieldType = fieldType.GetElementType();
                        }
                        
                        // If the element type is a structure, cache its Marshaler.
                        if (memberKind == c_metaKind.M_STRUCTURE)
                        {
                            implClass.Members.Add(new CodeMemberField(
                                    typeof(DatabaseMarshaler), 
                                    string.Format("attr{0}Marshaler", i)));
                            defaultConstructor.Statements.Add(new CodeSnippetExpression(
                                    string.Format("attr{0}Marshaler = GetMarshaler(participant, typeof({1}))", 
                                            i, fieldType.FullName)));
                            DatabaseMarshaler.Create(participant, memberType, fieldType, this);
                        }
                        break;
                    default:
                        // Fine: constructor doesn't need to do anything in particular.
                        break;
                    }
                }
            }
            else
            {
                throw new Exception("Non-structured Datatypes not yet supproted.");
            }

            // add the constructor to the class
            implClass.Members.Add(defaultConstructor);
        }
    }
}
