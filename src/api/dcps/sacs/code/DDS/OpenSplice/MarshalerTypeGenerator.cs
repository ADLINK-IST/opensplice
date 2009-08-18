﻿using System;
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
            NothingDefined      = 0x00,
            CursorDefined       = 0x01,
            MemberObjDefined    = 0x02
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

            CodeSnippetExpression assignSnippet =
                new CodeSnippetExpression(string.Format("{0} dataTo = to as {0}", dataType.FullName));
            CodeConditionStatement condition =
                new CodeConditionStatement(new CodeSnippetExpression("to == null"), 
                    new CodeExpressionStatement(new CodeSnippetExpression(
                        string.Format("dataTo = new {0}()", dataType.FullName))));
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
                    CreateTypeSupportRead(
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

            CodeSnippetExpression endAssign = new CodeSnippetExpression("to = dataTo");
            copyOutMethod.Statements.Add(endAssign);

            implClass.Members.Add(copyOutMethod);
        }

        private void CreateTypeSupportRead(
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
                copyOutMethod.Statements.Add(new CodeSnippetExpression(
                        string.Format("if (dataTo.{0} == null) dataTo.{0} = memberVal as {1}", fieldName, fieldType.FullName)));
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
                case c_primKind.P_CHAR:
                case c_primKind.P_OCTET:
                case c_primKind.P_SHORT:
                case c_primKind.P_USHORT:
                case c_primKind.P_LONG:
                case c_primKind.P_ULONG:
                case c_primKind.P_LONGLONG:
                case c_primKind.P_ULONGLONG:
                case c_primKind.P_FLOAT:
                case c_primKind.P_DOUBLE:
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
                            string.Format("int length{0} = from.{1}.Length", 
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
                            CreateArrayMemberRead(memberType, fieldType, 0, fieldName));
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
                int dimension,
                string fieldName)
        {
            CodeStatement[] result;
            switch(Gapi.MetaData.baseObjectKind(type))
            {
            case c_metaKind.M_PRIMITIVE: 
                // Handle primitive.
                switch (Gapi.MetaData.primitiveKind(type))
                {
                case c_primKind.P_BOOLEAN:
                case c_primKind.P_CHAR:
                case c_primKind.P_OCTET:
                case c_primKind.P_SHORT:
                case c_primKind.P_USHORT:
                case c_primKind.P_LONG:
                case c_primKind.P_ULONG:
                case c_primKind.P_LONGLONG:
                case c_primKind.P_ULONGLONG:
                case c_primKind.P_FLOAT:
                case c_primKind.P_DOUBLE:
//                case "Duration":
//                case "Time":
//                case "InstanceHandle":
//                case "IntPtr":
                    result = CreateArrayMemberReadInnerLoopBody(type, elementType, dimension, fieldName);
                    break;
                default:
                    throw new Exception("Unsupported primitive type");
                }
                break;
            case c_metaKind.M_COLLECTION:
                // Handle Collection type.
                switch (Gapi.MetaData.collectionTypeKind(type))
                {
                case c_collKind.C_STRING:
                    // Handle strings.
                    result = CreateArrayMemberReadInnerLoopBody(type, elementType, dimension, fieldName);
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
                int dimension,
                string fieldName)
        {
            string snippet = "dataTo." + fieldName + "[i0"; 
            for (int currDim = 1; currDim < dimension; currDim++)
            {
                snippet += string.Format(",i{0}", currDim);
            }
            snippet += string.Format("] = Read{0}(from, offset + cursor)", elementType.Name);
            CodeStatement[] result = new CodeStatement[2];
            result[0] = new CodeExpressionStatement(new CodeSnippetExpression(snippet));
            snippet = string.Format("cursor += {0}", Gapi.MetaData.typeSize(type)); 
            result[1] = new CodeExpressionStatement(new CodeSnippetExpression(snippet));
            
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

            CodeSnippetExpression handleSnippet = new CodeSnippetExpression("GCHandle tmpGCHandle = GCHandle.FromIntPtr(from)");
            CodeSnippetExpression castSnippet = new CodeSnippetExpression("object fromData = tmpGCHandle.Target");
            CodeSnippetExpression returnSnippet = new CodeSnippetExpression("CopyIn(basePtr, fromData, to, 0)");
            CodeMethodReturnStatement returnStatement = new CodeMethodReturnStatement(returnSnippet);
            copyInMethodFromNative.Statements.Add(handleSnippet);
            copyInMethodFromNative.Statements.Add(castSnippet);
            copyInMethodFromNative.Statements.Add(returnStatement);

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

            CodeSnippetExpression checkInit = new CodeSnippetExpression("if (!initDone) { /* Assert */}");
            copyInMethod.Statements.Add(checkInit);
            
            CodeSnippetExpression castType = new CodeSnippetExpression(
                    string.Format("{0} from = untypedFrom as {0}", dataType.FullName));
            copyInMethod.Statements.Add(castType);

            if (Gapi.MetaData.baseObjectKind(metaData) == c_metaKind.M_STRUCTURE)
            {
                int nrMembers = Gapi.MetaData.structureMemberCount(metaData);
                InitPhase initState = InitPhase.NothingDefined;
                for (int i = 0; i < nrMembers; i++)
                {
                	IntPtr member = Gapi.MetaData.structureMember(metaData, i);
                    CreateTypeSupportWrite(copyInMethod, member, i, ref initState);
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

        private void CreateTypeSupportWrite(
                CodeMemberMethod copyInMethod,
                IntPtr member, 
                int index,
                ref InitPhase initState)
        {
            string snippet = string.Empty;
            IntPtr memberType = Gapi.MetaData.memberType(member);
            string fieldName = Gapi.MetaData.specifierName(member);
            uint offset = Gapi.MetaData.memberOffset(member);

            switch(Gapi.MetaData.baseObjectKind(memberType))
            {
            case c_metaKind.M_STRUCTURE:
                // Handle embedded struct.
                snippet = string.Format(
                        "attr{0}Marshaler.CopyIn(basePtr, from.{1}, to, offset + {2})", 
                        index, fieldName, offset);
                copyInMethod.Statements.Add(new CodeSnippetExpression(snippet));
                break;
            case c_metaKind.M_ENUMERATION:
                // Handle enum.
                snippet = string.Format(
                        "Write(to, offset + {0}, (uint) from.{1})", 
                        offset, fieldName);
                copyInMethod.Statements.Add(new CodeSnippetExpression(snippet));
                break;
            case c_metaKind.M_PRIMITIVE: 
                // Handle primitive.
                switch (Gapi.MetaData.primitiveKind(memberType))
                {
                case c_primKind.P_BOOLEAN:
                case c_primKind.P_CHAR:
                case c_primKind.P_OCTET:
                case c_primKind.P_SHORT:
                case c_primKind.P_USHORT:
                case c_primKind.P_LONG:
                case c_primKind.P_ULONG:
                case c_primKind.P_LONGLONG:
                case c_primKind.P_ULONGLONG:
                case c_primKind.P_FLOAT:
                case c_primKind.P_DOUBLE:
//                case "Duration":
//                case "Time":
//                case "InstanceHandle":
//                case "IntPtr":
                    snippet = string.Format("Write(to, offset + {0}, from.{1})", offset, fieldName);
                    copyInMethod.Statements.Add(new CodeSnippetExpression(snippet));
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
                    snippet = string.Format("if (from.{0} == null) return false", fieldName);
                    copyInMethod.Statements.Add(new CodeSnippetExpression(snippet));
                    snippet = string.Format(
                            "Write(basePtr, to, offset + {0}, ref from.{1})", 
                            offset, 
                            fieldName); 
                    copyInMethod.Statements.Add(new CodeSnippetExpression(snippet));
                    break;
                case c_collKind.C_SEQUENCE:
                    // Handle sequences.
                    snippet = string.Format("int length{0} = from.{1}.Length", index, fieldName);
                    copyInMethod.Statements.Add(new CodeSnippetExpression(snippet));
                    break;
                case c_collKind.C_ARRAY:
                    // Handle arrays.
                    if ((initState & InitPhase.CursorDefined) == InitPhase.NothingDefined)
                    {
                        snippet = "int ";
                        initState |= InitPhase.CursorDefined;
                    }
                    copyInMethod.Statements.Add(new CodeSnippetExpression(
                            string.Format("{0}cursor = {1}", snippet, offset)));
                    copyInMethod.Statements.AddRange(CreateArrayMemberWrite(memberType, 0, fieldName));
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
                IntPtr type,
                int dimension,
                string fieldName)
        {
            CodeStatement[] result;
            switch(Gapi.MetaData.baseObjectKind(type))
            {
            case c_metaKind.M_PRIMITIVE: 
                // Handle primitive.
                switch (Gapi.MetaData.primitiveKind(type))
                {
                case c_primKind.P_BOOLEAN:
                case c_primKind.P_CHAR:
                case c_primKind.P_OCTET:
                case c_primKind.P_SHORT:
                case c_primKind.P_USHORT:
                case c_primKind.P_LONG:
                case c_primKind.P_ULONG:
                case c_primKind.P_LONGLONG:
                case c_primKind.P_ULONGLONG:
                case c_primKind.P_FLOAT:
                case c_primKind.P_DOUBLE:
//                case "Duration":
//                case "Time":
//                case "InstanceHandle":
//                case "IntPtr":
                    result = CreateArrayMemberWriteInnerLoopBody(type, dimension, fieldName, false);
                    break;
                default:
                    throw new Exception("Unsupported primitive type");
                }
                break;
            case c_metaKind.M_COLLECTION:
                // Handle Collection type.
                switch (Gapi.MetaData.collectionTypeKind(type))
                {
                case c_collKind.C_STRING:
                    // Handle strings.
                    result = CreateArrayMemberWriteInnerLoopBody(type, dimension, fieldName, true);
                    break;
                case c_collKind.C_ARRAY:
                    // Handle arrays.
                    int arrLength = Gapi.MetaData.collectionTypeMaxSize(type);
                    IntPtr subType = Gapi.MetaData.collectionTypeSubType(type);
                    CodeStatement[] forBody = CreateArrayMemberWrite(subType, dimension + 1, fieldName);
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

        private CodeStatement[] CreateArrayMemberWriteInnerLoopBody(
                IntPtr type,
                int dimension,
                string fieldName,
                bool byRef)
        {
            string snippet = string.Format("Write({0}to, offset + cursor, {1}from.{2}[i0", 
                    byRef ? "basePtr, " : "",
                    byRef ? "ref " : "",
                    fieldName); 
            for (int currDim = 1; currDim < dimension; currDim++)
            {
                snippet += string.Format(",i{0}", currDim);
            }
            snippet += "])";
            CodeStatement[] result = new CodeStatement[2];
            result[0] = new CodeExpressionStatement(new CodeSnippetExpression(snippet));
            snippet = string.Format("cursor += {0}", Gapi.MetaData.typeSize(type)); 
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
                    switch(Gapi.MetaData.baseObjectKind(memberType))
                    {
                    case c_metaKind.M_STRUCTURE: 
                        // Handle class.
                        string fieldName = Gapi.MetaData.specifierName(member);
                        FieldInfo field = dataType.GetField(fieldName);
                        Type fieldType = field.FieldType;
                        CodeMemberField attrMarshaler = new CodeMemberField(
                                typeof(DatabaseMarshaler), 
                                string.Format("attr{0}Marshaler", i));
                        CodeSnippetExpression assignMarshaler = new CodeSnippetExpression(
                                string.Format("attr{0}Marshaler = GetMarshaler(participant, typeof({1}))", 
                                                      i, fieldType.FullName));
                        implClass.Members.Add(attrMarshaler);
                        defaultConstructor.Statements.Add(assignMarshaler);
                        DatabaseMarshaler.Create(participant, memberType, fieldType, this);
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
