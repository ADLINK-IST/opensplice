using System;
using System.Collections.Generic;
using System.Text;
using System.CodeDom.Compiler;
using Microsoft.CSharp;
using System.CodeDom;
using System.Reflection;
using System.IO;

namespace DDS.OpenSplice
{
    public class MarshalerTypeGenerator : IMarshalerTypeGenerator
    {
        //private Type dataType;
        //private string[] names;
        //private int[] offsets;
        //private CodeTypeDeclaration marshalerClass;

        public MarshalerTypeGenerator()
        { }

        public BaseMarshaler CreateMarshaler(IntPtr participant, Type dataType, string typeName, string[] names, int[] offsets)
        {
            CompilerResults results = null;

            //this.dataType = dataType;
            //this.names = names;
            //this.offsets = offsets;

            // Generate the interface wrapper here and add to namespace
            CodeTypeDeclaration marshalerClass = CreateMarshalerClass(participant, dataType, names, offsets);

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
                Type marshaler_type = results.CompiledAssembly.GetType(dataType.Namespace + "." + marshalerClass.Name);
                return Activator.CreateInstance(marshaler_type, this, participant) as BaseMarshaler;
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
                string[] names, 
                int[] offsets)
        {
            string marshalerClassName = string.Format("{0}Marshaler", dataType.Name);
            CodeTypeDeclaration implClass = new CodeTypeDeclaration(marshalerClassName);
            implClass.Attributes = MemberAttributes.Public | MemberAttributes.Final;

            implClass.BaseTypes.Add(typeof(BaseMarshaler));

            //CreateFieldsAndProperties(implClass);
            CreateConstructor(participant, implClass, dataType, names);            

            CreateSampleReaderAlloc(implClass, dataType);
            CreateCopyIn(implClass, dataType, names, offsets);
            CreateCopyOut(implClass, dataType, names, offsets);

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
                new CodeSnippetExpression(string.Format("new {0}[length]", dataType.Name)));
            sampleReaderAllocMethod.Statements.Add(methodReturn);
            
            implClass.Members.Add(sampleReaderAllocMethod);
        }



        private void CreateCopyOut(
                CodeTypeDeclaration implClass, 
                Type dataType, 
                string[] names, 
                int[] offsets)
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
                new CodeSnippetExpression(string.Format("{0} dataTo = to as {0}", dataType.Name));
            CodeConditionStatement condition =
                new CodeConditionStatement(new CodeSnippetExpression("dataTo == null"), 
                    new CodeExpressionStatement(new CodeSnippetExpression(
                        string.Format("dataTo = new {0}()", dataType.Name))));
            copyOutMethod.Statements.Add(assignSnippet);
            copyOutMethod.Statements.Add(condition);

            for (int i = 0; i < names.Length; i++)
            {
                FieldInfo field = dataType.GetField(names[i]);
                copyOutMethod.Statements.Add(CreateTypeSupportRead(field.FieldType, i, field.Name, offsets[i]));
            }

            CodeSnippetExpression endAssign = new CodeSnippetExpression("to = dataTo");
            copyOutMethod.Statements.Add(endAssign);

            implClass.Members.Add(copyOutMethod);
        }

        private CodeExpression CreateTypeSupportRead(
                Type fieldType, 
                int index, 
                string fieldName, 
                int offset)
        {
            string snippet = string.Empty;
            if (fieldType.IsClass && fieldType.Name != "String")
            {
                // Handle class.
                snippet = string.Format(
                        "attr{0}Marshaler.CopyOut(from, ref to, offset + {1})", 
                        index, offset);
                return new CodeSnippetExpression(snippet);
            }
            else if (fieldType.IsEnum)
            {
                // handle enum.
                snippet = string.Format(
                        "dataTo.{0} = ({1}) ReadUInt32(from, offset + {2})", 
                        fieldName, fieldType.FullName, offset);
                return new CodeSnippetExpression(snippet);
            }
            else
            {
                switch (fieldType.Name)
                {
                    case "UInt16":
                    case "Int16":
                    case "UInt32":
                    case "Int32":
                    case "UInt64":
                    case "Int64":
                    case "Byte":
                    case "Char":
                    case "Boolean":
                    case "Double":
                    case "Single":
                    case "Duration":
                    case "Time":
                    case "InstanceHandle":
                    case "IntPtr":
                    case "String":
                        snippet = string.Format("dataTo.{0} = Read{1}(from, offset + {2})", fieldName, fieldType.Name, offset);
                        return new CodeSnippetExpression(snippet);
                    default:
                        break;
                }
            }
            return null;
        }

        private void CreateCopyIn(
                CodeTypeDeclaration implClass, 
                Type dataType, 
                string[] names, 
                int[] offsets)
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

            for (int i = 0; i < names.Length; i++)
            {
                FieldInfo field = dataType.GetField(names[i]);
                copyInMethod.Statements.Add(CreateTypeSupportWrite(field.FieldType, i, field.Name, offsets[i]));
            }

            CodeMethodReturnStatement methodReturn = new CodeMethodReturnStatement(
                new CodePrimitiveExpression(true));
            copyInMethod.Statements.Add(methodReturn);

            implClass.Members.Add(copyInMethod);
        }

        private CodeExpression CreateTypeSupportWrite(
                Type fieldType, 
                int index, 
                string fieldName, 
                int offset)
        {
            string snippet = string.Empty;
            if (fieldType.IsClass && fieldType.Name != "String")
            {
                // Handle class.
                snippet = string.Format(
                        "attr{0}Marshaler.CopyIn(basePtr, from.{1}, to, offset + {2})", 
                        index, fieldName, offset);
                return new CodeSnippetExpression(snippet);
            }
            else if (fieldType.IsEnum)
            {
                // handle enum.
                snippet = string.Format(
                        "Write(to, offset + {0}, (uint) from.{1})", 
                        offset, fieldName);
                return new CodeSnippetExpression(snippet);
            }
            else 
            {
                switch (fieldType.Name)
                {
                    case "UInt16":
                    case "Int16":
                    case "UInt32":
                    case "Int32":
                    case "UInt64":
                    case "Int64":
                    case "Byte":
                    case "Char":
                    case "Boolean":
                    case "Double":
                    case "Single":
                    case "Duration":
                    case "Time":
                    case "InstanceHandle":
                    case "IntPtr":
                        snippet = string.Format("Write(to, offset + {0}, from.{1})", offset, fieldName);
                        return new CodeSnippetExpression(snippet);
                    case "String":
                        snippet = string.Format("Write(basePtr, to, offset + {0}, ref from.{1})", offset, fieldName);
                        return new CodeSnippetExpression(snippet);
                    default:
                        break;
                }
            }
            return null;
        }

//        private void InitType(DomainParticipant participant, string typeName)
//        {
//            TypeSupport.GetNamesAndOffsets(participant.GapiPeer, typeName, out nameArray, out offsetArray)
//            CodeMemberMethod initTypeMethod = new CodeMemberMethod();
//            initTypeMethod.Name = "InitType";
//            initTypeMethod.Attributes = MemberAttributes.Public | MemberAttributes.Override;
//
//            CodeParameterDeclarationExpression participant = 
//                new CodeParameterDeclarationExpression(typeof(IntPtr), "participant");
//            initTypeMethod.Parameters.Add(participant);
            
//            string typeName = string.Format("{0}::{1}", dataType.Namespace, dataType.Name);

//            CodeSnippetExpression initNameAndOffsetArray = new CodeSnippetExpression(
//                string.Format("TypeSupport.GetNamesAndOffsets(participant, \"{0}\", out nameArray, out offsetArray)", typeName));
//            initTypeMethod.Statements.Add(initNameAndOffsetArray);

            // TODO: handle nested
            
//            CodeAssignStatement assignInitDone = new CodeAssignStatement(
//                new CodeFieldReferenceExpression(new CodeThisReferenceExpression(), "initDone"),
//                new CodePrimitiveExpression(true));
//            initTypeMethod.Statements.Add(assignInitDone);

//            implClass.Members.Add(initTypeMethod);
//        }

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
                IntPtr participant, 
                CodeTypeDeclaration implClass, 
                Type dataType, 
                string[] names)
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

            for (int i = 0; i < names.Length; i++)
            {
                Type fieldType = dataType.GetField(names[i]).FieldType;
                if (fieldType.IsClass && fieldType.Name != "String")
                {
                    // Handle class.
                    CodeMemberField attrMarshaler = new CodeMemberField(typeof(BaseMarshaler), 
                            string.Format("attr{0}Marshaler", i));
                    CodeSnippetExpression assignMarshaler = new CodeSnippetExpression(
                            string.Format("attr{0}Marshaler = DDS.OpenSplice.BaseMarshaler.Create(" +
                                                  "generator, typeof({1}), \"Data::Erik::Test\", participant)", 
                                                  i, fieldType.FullName));
                    implClass.Members.Add(attrMarshaler);
                    defaultConstructor.Statements.Add(assignMarshaler);
                    DDS.OpenSplice.BaseMarshaler.Create(this, fieldType, "Data::Erik::Test", participant);
                }
            }

            // add the constructor to the class
            implClass.Members.Add(defaultConstructor);
        }
    }
}
