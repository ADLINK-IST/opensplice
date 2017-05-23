.. _`Appendix A`:


##########
Appendix A
##########

.. _`Scripting BNF`:

Scripting BNF
*************

*This Appendix gives the formal description of the Tester Scripting language.*

TOKENS
======

::

   TOKENS
   <DEFAULT> SKIP : {
   " "
   | "\t"
   | "\n"
   | "\r"
   | <"//" (~["\n","\r"])* ("\n" | "\r" | "\r\n")>
   | <"--" (~["\n","\r"])* ("\n" | "\r" | "\r\n")>
   | <"/*" (~["*"])* "*" ("*" | ~["*","/"] (~["*"])* "*")* "/">
   }

   <DEFAULT> TOKEN : {
   <INTEGER_LITERAL: <DECIMAL_LITERAL> (["l","L"])? 
   | <HEX_LITERAL> (["l","L"])? 
   | <OCTAL_LITERAL> (["l","L"])?>
   | <#DECIMAL_LITERAL: (["+","-"])? ["0"-"9"] (["0"-"9"])*>
   | <#HEX_LITERAL: "0" ["x","X"] (["0"-"9","a"-"f","A"-"F"])+>
   | <#OCTAL_LITERAL: "0" (["0"-"7"])*>
   | <FLOATING_POINT_LITERAL: (["+","-"])? (["0"-"9"])+ "." (["0"-"9"])* 
	   (<EXPONENT>)? (["f","F","d","D"])? 
   | "." (["0"-"9"])+ (<EXPONENT>)? (["f","F","d","D"])? 
   | (["0"-"9"])+ <EXPONENT> (["f","F","d","D"])? 
   | (["0"-"9"])+ (<EXPONENT>)? ["f","F","d","D"]>
   | <#EXPONENT: ["e","E"] (["+","-"])? (["0"-"9"])+>
   | <CHARACTER_LITERAL: "\'" (~["\'","\\","\n","\r"] 
   | "\\" (["n","t","b","r","f","\\","\'","\""] 
   | ["0"-"7"] (["0"-"7"])? 
   | ["0"-"3"] ["0"-"7"] ["0"-"7"])) "\'">
   | <STRING_LITERAL: "\"" (~["\"","\\","\n","\r"] 
   | "\\" (["n","t","b","r","f","\\","\'","\""] 
   | ["0"-"7"] (["0"-"7"])? 
   | ["0"-"3"] ["0"-"7"] ["0"-"7"] 
   | ["\n","\r"] 
   | "\r\n"))* "\"">
   | <HASH_LITERAL: "#"(["0"-"9"])+>
   | <JAVASCRIPT: "`" (~["`"])* "`">
   }

   <DEFAULT> TOKEN : {
   <SEND: "send">
   | <REPEAT: "repeat">
   | <PERIODIC: "periodic">
   | <MACRO: "macro">
   | <DISPOSE: "dispose">
   | <WRITEDISPOSE: "writedispose">
   | <WAIT: "wait">
   | <WAITABS: "waitabs">
   | <CALL: "call">
   | <RUN: "run">
   | <CHECK: "check">
   | <CHECK_LAST: "check_last">
   | <CHECK_ANY: "check_any">
   | <RECHECK_LAST: "recheck_last">
   | <DISPOSED: "disposed">
   | <MARK: "mark">
   | <MISS: "miss">
   | <MARKMSG: "markmsg">
   | <MISSMSG: "missmsg">
   | <SCENARIO: "scenario">
   | <UNIQID: "uniqid">
   | <VAR: "var">
   | <END: "end">
   | <MSG: "message">
   | <LOG: "log">
   | <FAIL: "fail">
   | <CLEAR: "clear">
   | <IF: "if">
   | <THEN: "then">
   | <ELSE: "else">
   | <ENDIF: "endif">
   | <FOR: "for">
   | <IN: "in">
   | <LOOP: "loop">
   | <ENDLOOP: "endloop">
   | <WHILE: "while">
   | <READER: "reader">
   | <WRITE: "write">
   | <READ: "read">
   | <CONNECT: "connect">
   | <DISCONNECT: "disconnect">
   | <EXEC: "execute">
   | <CONTROL: "control">
   | <SET: "set">
   | <COLUMN: "column">
   | <GRAPH: "graph">
   | <REVERSE_FAIL: "reverse_fail">
   | <EXIT: "exit">
   }

   <DEFAULT> TOKEN : {
   <IDENTIFIER: <LETTER> (<LETTER> | <DIGIT>)*>
   | <#LETTER: ["$","A"-"Z","_","a"-"z"]>
   | <DIGIT: ["0"-"9"]>
   }


NON-TERMINALS
=============

::

   NON-TERMINALS
      Scenario := <SCENARIO> <IDENTIFIER> ( InstructionList )? <END> 
		   <SCENARIO>
      Macro := <MACRO> <IDENTIFIER> "(" ( ArgumentList )? ")" 
		   ( InstructionList )? <END> <MACRO>
         |  <SCENARIO> <IDENTIFIER> ( InstructionList )? <END> <SCENARIO>
      InstructionList   := ( Instruction )+
      Instruction := SendInstruction
         |  RepeatInstruction
         |  PeriodicInstruction
         |  DisposeInstruction
         |  WriteDisposeInstruction
         |  WaitInstruction
         |  WaitabsInstruction
         |  CheckInstruction
         |  CheckLastInstruction
         |  CheckAnyInstruction
         |  RecheckLastInstruction
         |  DisposedInstruction
         |  MarkInstruction
         |  MarkMsgInstruction
         |  MissInstruction
         |  MissMsgInstruction
         |  CallInstruction
         |  ForInstruction
         |  WhileInstruction
         |  SetInstruction
         |  VarDeclaration
         |  IfInstruction
         |  MessageInstruction
         |  ClearInstruction
         |  LogInstruction
         |  FailInstruction
         |  ReaderInstruction
         |  WriteInstruction
         |  ReadInstruction
         |  ConnectInstruction
         |  DisconnectInstruction
         |  ExecuteInstruction
         |  ControlInstruction
         |  ColumnInstruction
         |  GraphInstruction
         |  ReverseFailInstruction
         |  ExitInstruction
         |  ScriptInvocation
      ReaderInstruction := <READER> ( <DISPOSE> )? "(" Constant 
		   (<HASH_LITERAL>)? ( "," <IDENTIFIER> ( "," Constant 
			( "," Constant )? )? )? ");"
      ColumnInstruction := <COLUMN> ( <CLEAR> )? "(" Constant 
		   ( "," Constant )? ");"
      GraphInstruction  := <GRAPH> "(" ParameterList ");"
      MessageInstruction   := <MSG> "(" <STRING_LITERAL> ( Constant )? ");"
      LogInstruction := <LOG> "(" <STRING_LITERAL> ( Constant )? ");"
      FailInstruction   := <FAIL> "(" <STRING_LITERAL> ( Constant )? ");"
      ControlInstruction   := <CONTROL> <IDENTIFIER> "." <IDENTIFIER> ( 
		   ( "(" ParameterList ( ( ");" ) | ( ")" ";" ) ) ) | ( ";" ) )
      ClearInstruction  := <CLEAR> ";"
      ExitInstruction   := <EXIT> ( <IF> <FAIL> )? ";"
      ScriptInvocation  := Script ";"
      SendInstruction   := <SEND> <IDENTIFIER> ( ( "." <IDENTIFIER> ) )? 
		   "(" ( ParameterList )? ");"
      RepeatInstruction := <REPEAT> <IDENTIFIER> FloatValue IntValue "(" 
		   ( ParameterList )? ");"
      PeriodicInstruction  := <PERIODIC> <IDENTIFIER> <IDENTIFIER> 
		   FloatValue IntValue "(" ( ParameterList )? ");"
      WriteInstruction  := <WRITE> <IDENTIFIER> "." <IDENTIFIER> "(" 
		   ( ParameterList )? ");"
      VarDeclaration := <VAR> FieldName "=>" Constant ";"
      DisposeInstruction   := <DISPOSE> <IDENTIFIER> ( ( "." 
		   <IDENTIFIER> ) )? "(" ( ParameterList )? ");"
      WriteDisposeInstruction := <WRITEDISPOSE> <IDENTIFIER> ( ( "." 
		   <IDENTIFIER> ) )? "(" ( ParameterList )? ");"
      CheckInstruction  := <CHECK> <IDENTIFIER> ( ( "." <IDENTIFIER> ) )? 
		   "(" ( ChkParameterList )? ");"
      CheckLastInstruction := <CHECK_LAST> <IDENTIFIER> ( ( "." 
		   <IDENTIFIER> ) )? "(" ( ChkParameterList )? ");"
      CheckAnyInstruction  := <CHECK_ANY> <IDENTIFIER> ( ( "." 
		   <IDENTIFIER> ) )? "(" ( ChkParameterList )? ");"
      RecheckLastInstruction  := <RECHECK_LAST> <IDENTIFIER> ( ( "." 
		   <IDENTIFIER> ) )? "(" ( ChkParameterList )? ");"
      ReadInstruction   := <READ> <IDENTIFIER> "." <IDENTIFIER> "(" 
		   ( ChkParameterList )? ");"
      MarkMsgInstruction   := <MARKMSG> <IDENTIFIER> "." <IDENTIFIER> "(" 
		   ( ChkParameterList )? ");"
      MissMsgInstruction   := <MISSMSG> <IDENTIFIER> "." <IDENTIFIER> "(" 
		   ( ChkParameterList )? ");"
      ConnectInstruction   := <CONNECT> <IDENTIFIER> ( Constant )? ";"
      DisconnectInstruction   := <DISCONNECT> <IDENTIFIER> ";"
      DisposedInstruction  := <DISPOSED> <IDENTIFIER> ( ( "." 
		   <IDENTIFIER> ) )? "(" ( ChkParameterList )? ");"
      MissInstruction   := <MISS> <IDENTIFIER> ( ( "." <IDENTIFIER> ) )? 
		   "(" ( ChkParameterList )? ");"
      MarkInstruction   := <MARK> <IDENTIFIER> ( ( "." <IDENTIFIER> ) )? 
		   "(" ( ChkParameterList )? ");"
      CallInstruction   := <CALL> <IDENTIFIER> ( ( "." <IDENTIFIER> ) )? 
		   "(" ( ParameterList )? ");"
      SetInstruction := <SET> <IDENTIFIER> "(" ( ParameterList )? ")" 
		   "(" "(" ParamHeaderList ")" ParamSetList ");"
      ParamHeaderList   := <IDENTIFIER> ( "," ParamHeaderList )?
      ParamSetList   := "," ParamSet ( ParamSetList )?
      ParamSet := "(" ParamValueList ")"
      ParamValueList := Constant ( "," ParamValueList )?
      IfInstruction  := <IF> "(" CompareExpression ")" <THEN> 
		   InstructionList ( <ELSE> InstructionList )? <ENDIF> ";"
      CompareExpression := CalcExpression ( CompareOperator 
		   CompareExpression )?
      CalcExpression := PrimaryExpression ( CalcOperator CalcExpression )?
      PrimaryExpression := Constant
         |  "(" CompareExpression ")"
      CompareOperator   := "=="
         |  "!="
         |  ">"
         |  "<"
         |  ">="
         |  "<="
         |  "||"
         |  "&&"
      CalcOperator   := "|"
         |  "&"
         |  "+"
         |  "-"
         |  "*"
         |  "/"
      ForInstruction := <FOR> <IDENTIFIER> <IN> ( 
		   ( IntValue ".." IntValue ) 
         | "(" VarList ")" ) <LOOP> InstructionList <ENDLOOP> ";"
      WhileInstruction  := <WHILE> "(" CompareExpression ")" <LOOP> 
		   InstructionList <ENDLOOP> ";"
      VarList  := Constant ( "," VarList )?
      WaitInstruction   := <WAIT> "(" Constant ");"
      WaitabsInstruction   := <WAITABS> "(" Constant ");"
      ExecuteInstruction   := <EXEC> ( <WAIT> )? ( <LOG> )? 
		   <STRING_LITERAL> ";"
      ReverseFailInstruction  := <REVERSE_FAIL> ";"
      ParameterList  := Parameter ( "," Parameter )* ( "," )?
      Parameter   := FieldName "=>" Constant
      ChkParameterList  := ChkParameter ( "," ChkParameter )* ( "," )?
      ChkParameter   := FieldName "=>" ( "!" )? Constant ( ":" Constant )?
      ArgumentList   := Argument ( Argument )*
      Argument := FieldName ":" FieldName ( ":=" Constant )? ";"
      FieldName   := <IDENTIFIER> ( "[" <INTEGER_LITERAL> "]" )? 
         ( ( "." FieldName ) )?
      IntValue := <INTEGER_LITERAL>
         |  "<<" <IDENTIFIER>
         |  <IDENTIFIER>
      FloatValue  := <FLOATING_POINT_LITERAL>
         |  "<<" <IDENTIFIER>
         |  <IDENTIFIER>
      Constant := <INTEGER_LITERAL>
         |  <FLOATING_POINT_LITERAL>
         |  <CHARACTER_LITERAL>
         |  <STRING_LITERAL>
         |  ">>" <IDENTIFIER>
         |  ">>" <JAVASCRIPT>
         |  "<<" <IDENTIFIER> ( "." <IDENTIFIER> )?
         |  <IDENTIFIER>
         |  <UNIQID>
         |  <JAVASCRIPT>
      Script   := <JAVASCRIPT>


.. END
