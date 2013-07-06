/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
package DCG.DCGUtilities;

import org.openorb.compiler.object.IdlArray;
import org.openorb.compiler.object.IdlAttribute;
import org.openorb.compiler.object.IdlConst;
import org.openorb.compiler.object.IdlEnum;
import org.openorb.compiler.object.IdlExcept;
import org.openorb.compiler.object.IdlFactory;
import org.openorb.compiler.object.IdlFixed;
import org.openorb.compiler.object.IdlIdent;
import org.openorb.compiler.object.IdlInterface;
import org.openorb.compiler.object.IdlModule;
import org.openorb.compiler.object.IdlNative;
import org.openorb.compiler.object.IdlObject;
import org.openorb.compiler.object.IdlOp;
import org.openorb.compiler.object.IdlParam;
import org.openorb.compiler.object.IdlSequence;
import org.openorb.compiler.object.IdlSimple;
import org.openorb.compiler.object.IdlStateMember;
import org.openorb.compiler.object.IdlString;
import org.openorb.compiler.object.IdlStruct;
import org.openorb.compiler.object.IdlStructMember;
import org.openorb.compiler.object.IdlTypeDef;
import org.openorb.compiler.object.IdlUnion;
import org.openorb.compiler.object.IdlUnionMember;
import org.openorb.compiler.object.IdlValue;
import org.openorb.compiler.object.IdlValueBox;
import org.openorb.compiler.object.IdlWString;
import org.openorb.compiler.idl.reflect.idlInterface;
import org.openorb.compiler.idl.reflect.idlParameter;

/**
 * This interface defines the methods invoked by the IDLTraverser and must be
 * implemented by any class that uses the IDLTraverser class.
 */
public interface IDLTraversable {

    /**
     * This method will be called by the IDLTraverser each time an IDL array object is
     * found. The corresponding traverse method in the IDLTraverser (if present)
     * should be called from the implementation of this process method, unless of
     * course other functionality is desired. Note that functionality can be inserted
     * before AND after the corresponding traverse method is called. This allows for
     * maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90A3003A
     */
    public void processIDLArray(IdlArray object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL attribute
     * object is found. The corresponding traverse method in the IDLTraverser (if
     * present) should be called from the implementation of this process method,
     * unless of course other functionality is desired. Note that functionality can be
     * inserted before AND after the corresponding traverse method is called. This
     * allows for maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90D3000C
     */
    public void processIDLAttribute(IdlAttribute object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL const object is
     * found. The corresponding traverse method in the IDLTraverser (if present)
     * should be called from the implementation of this process method, unless of
     * course other functionality is desired. Note that functionality can be inserted
     * before AND after the corresponding traverse method is called. This allows for
     * maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90D402CB
     */
    public void processIDLConst(IdlConst object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time a context string is
     * found. The corresponding traverse method in the IDLTraverser (if present)
     * should be called from the implementation of this process method, unless of
     * course other functionality is desired. Note that functionality can be inserted
     * before AND after the corresponding traverse method is called. This allows for
     * maximum flexibility when traversing the IDL tree structure.
     *
     * @param context The String object that needs to be processed and traversed
     * further (if possible)
     * @throws Exception
     * @roseuid 40DA90D603E4
     */
    public void processIDLContext(String context) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL declarator
     * object is found. The corresponding traverse method in the IDLTraverser (if
     * present) should be called from the implementation of this process method,
     * unless of course other functionality is desired. Note that functionality can be
     * inserted before AND after the corresponding traverse method is called. This
     * allows for maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90D802DB
     */
    public void processIDLDeclarator(IdlObject object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL enum object is
     * found. The corresponding traverse method in the IDLTraverser (if present)
     * should be called from the implementation of this process method, unless of
     * course other functionality is desired. Note that functionality can be inserted
     * before AND after the corresponding traverse method is called. This allows for
     * maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90DA01A2
     */
    public void processIDLEnum(IdlEnum object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time a string enum member
     * is found. The corresponding traverse method in the IDLTraverser (if present)
     * should be called from the implementation of this process method, unless of
     * course other functionality is desired. Note that functionality can be inserted
     * before AND after the corresponding traverse method is called. This allows for
     * maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The String object that needs to be processed and traversed
     * further (if possible)
     * @throws Exception
     * @roseuid 40DA90DC00D7
     */
    public void processIDLEnumMember(String object, int indexOfMember, int totalMembers) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL exception
     * object is found. The corresponding traverse method in the IDLTraverser (if
     * present) should be called from the implementation of this process method,
     * unless of course other functionality is desired. Note that functionality can be
     * inserted before AND after the corresponding traverse method is called. This
     * allows for maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90DD03A6
     */
    public void processIDLExcept(IdlExcept object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL factory object
     * (also known as init) is found. The corresponding traverse method in the
     * IDLTraverser (if present) should be called from the implementation of this
     * process method, unless of course other functionality is desired. Note that
     * functionality can be inserted before AND after the corresponding traverse
     * method is called. This allows for maximum flexibility when traversing the IDL
     * tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90DF024E
     */
    public void processIDLFactory(IdlFactory object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL fixed object is
     * found. The corresponding traverse method in the IDLTraverser (if present)
     * should be called from the implementation of this process method, unless of
     * course other functionality is desired. Note that functionality can be inserted
     * before AND after the corresponding traverse method is called. This allows for
     * maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90E100F6
     */
    public void processIDLFixed(IdlFixed object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL interface
     * object is found. The corresponding traverse method in the IDLTraverser (if
     * present) should be called from the implementation of this process method,
     * unless of course other functionality is desired. Note that functionality can be
     * inserted before AND after the corresponding traverse method is called. This
     * allows for maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90E30125
     */
    public void processIDLInterface(IdlInterface object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL interface
     * inheritance list is found. The corresponding traverse method in the
     * IDLTraverser (if present) should be called from the implementation of this
     * process method, unless of course other functionality is desired. Note that
     * functionality can be inserted before AND after the corresponding traverse
     * method is called. This allows for maximum flexibility when traversing the IDL
     * tree structure.
     *
     * @param inheritanceList The IDL object that needs to be processed and traversed
     * further (if possible)
     * @throws Exception
     * @roseuid 40DA90E502CB
     */
    public void processIDLInterfaceInheritance(idlInterface[] inheritanceList) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL struct member
     * (also used to represent member object from other IDL objects then structs)
     * object is found. The corresponding traverse method in the IDLTraverser (if
     * present) should be called from the implementation of this process method,
     * unless of course other functionality is desired. Note that functionality can be
     * inserted before AND after the corresponding traverse method is called. This
     * allows for maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90E70358
     */
    public void processIDLMember(IdlStructMember object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL module object
     * is found. The corresponding traverse method in the IDLTraverser (if present)
     * should be called from the implementation of this process method, unless of
     * course other functionality is desired. Note that functionality can be inserted
     * before AND after the corresponding traverse method is called. This allows for
     * maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90E901D1
     */
    public void processIDLModule(IdlObject object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL native object
     * is found. The corresponding traverse method in the IDLTraverser (if present)
     * should be called from the implementation of this process method, unless of
     * course other functionality is desired. Note that functionality can be inserted
     * before AND after the corresponding traverse method is called. This allows for
     * maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90EB0135
     */
    public void processIDLNative(IdlNative object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL operation
     * object is found. The corresponding traverse method in the IDLTraverser (if
     * present) should be called from the implementation of this process method,
     * unless of course other functionality is desired. Note that functionality can be
     * inserted before AND after the corresponding traverse method is called. This
     * allows for maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90ED030A
     */
    public void processIDLOperation(IdlOp object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL parameter
     * object is found. The corresponding traverse method in the IDLTraverser (if
     * present) should be called from the implementation of this process method,
     * unless of course other functionality is desired. Note that functionality can be
     * inserted before AND after the corresponding traverse method is called. This
     * allows for maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90EF0377
     */
    public void processIDLParameter(idlParameter object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL raises object
     * is found. The corresponding traverse method in the IDLTraverser (if present)
     * should be called from the implementation of this process method, unless of
     * course other functionality is desired. Note that functionality can be inserted
     * before AND after the corresponding traverse method is called. This allows for
     * maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90F2022F
     */
    public void processIDLRaisesException(IdlExcept object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL sequence object
     * is found. The corresponding traverse method in the IDLTraverser (if present)
     * should be called from the implementation of this process method, unless of
     * course other functionality is desired. Note that functionality can be inserted
     * before AND after the corresponding traverse method is called. This allows for
     * maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90FD0387
     */
    public void processIDLSequence(IdlSequence object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL statemember
     * object is found. The corresponding traverse method in the IDLTraverser (if
     * present) should be called from the implementation of this process method,
     * unless of course other functionality is desired. Note that functionality can be
     * inserted before AND after the corresponding traverse method is called. This
     * allows for maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90F40358
     */
    public void processIDLStateMember(IdlStateMember object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL string object
     * is found. The corresponding traverse method in the IDLTraverser (if present)
     * should be called from the implementation of this process method, unless of
     * course other functionality is desired. Note that functionality can be inserted
     * before AND after the corresponding traverse method is called. This allows for
     * maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90F601C2
     */
    public void processIDLString(IdlString object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL struct object
     * is found. The corresponding traverse method in the IDLTraverser (if present)
     * should be called from the implementation of this process method, unless of
     * course other functionality is desired. Note that functionality can be inserted
     * before AND after the corresponding traverse method is called. This allows for
     * maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90F80174
     */
    public void processIDLStruct(IdlStruct object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL typedef object
     * is found. The corresponding traverse method in the IDLTraverser (if present)
     * should be called from the implementation of this process method, unless of
     * course other functionality is desired. Note that functionality can be inserted
     * before AND after the corresponding traverse method is called. This allows for
     * maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90FA0099
     */
    public void processIDLTypeDef(IdlTypeDef object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL typespec object
     * is found. The corresponding traverse method in the IDLTraverser (if present)
     * should be called from the implementation of this process method, unless of
     * course other functionality is desired. Note that functionality can be inserted
     * before AND after the corresponding traverse method is called. This allows for
     * maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90FC004B
     */
    public void processIDLTypeSpec(IdlObject object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL union object is
     * found. The corresponding traverse method in the IDLTraverser (if present)
     * should be called from the implementation of this process method, unless of
     * course other functionality is desired. Note that functionality can be inserted
     * before AND after the corresponding traverse method is called. This allows for
     * maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA90FF026E
     */
    public void processIDLUnion(IdlUnion object) throws Exception;

	public void processIDLUnionDiscriminant(IdlObject object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL union member
     * object is found. The corresponding traverse method in the IDLTraverser (if
     * present) should be called from the implementation of this process method,
     * unless of course other functionality is desired. Note that functionality can be
     * inserted before AND after the corresponding traverse method is called. This
     * allows for maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA9101022F
     */
    public void processIDLUnionMember(IdlUnionMember object) throws Exception;
    public void processIDLUnionBranch(java.util.Vector cases) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL valuetype
     * object is found. The corresponding traverse method in the IDLTraverser (if
     * present) should be called from the implementation of this process method,
     * unless of course other functionality is desired. Note that functionality can be
     * inserted before AND after the corresponding traverse method is called. This
     * allows for maximum flexibility when traversing the IDL tree structure.
     * Special rule: calling the traverse method corresponding to this process method
     * will cause the process method to be called of the value abstract def, valuedef
     * or valueforward def to be called. This feature is implemented to allow more
     * control over the behaviour of traverser, as the developer can choose to use the
     * corresponding traverse method or process each IDL value object exactly the
     * same, before continueing to a specific implementation, as valuetype represent
     * alot of the same information.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA91030210
     */
    public void processIDLValue(IdlValue object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL abstract
     * valuetype object is found. The corresponding traverse method in the
     * IDLTraverser (if present) should be called from the implementation of this
     * process method, unless of course other functionality is desired. Note that
     * functionality can be inserted before AND after the corresponding traverse
     * method is called. This allows for maximum flexibility when traversing the IDL
     * tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA910501F1
     */
    public void processIDLValueAbstractDef(IdlValue object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL boxed valuetype
     * object is found. The corresponding traverse method in the IDLTraverser (if
     * present) should be called from the implementation of this process method,
     * unless of course other functionality is desired. Note that functionality can be
     * inserted before AND after the corresponding traverse method is called. This
     * allows for maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA910700F7
     */
    public void processIDLValueBox(IdlValueBox object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL def. valuetype
     * object is found. The corresponding traverse method in the IDLTraverser (if
     * present) should be called from the implementation of this process method,
     * unless of course other functionality is desired. Note that functionality can be
     * inserted before AND after the corresponding traverse method is called. This
     * allows for maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA9109022F
     */
    public void processIDLValueDef(IdlValue object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL forward
     * valuetype object is found. The corresponding traverse method in the
     * IDLTraverser (if present) should be called from the implementation of this
     * process method, unless of course other functionality is desired. Note that
     * functionality can be inserted before AND after the corresponding traverse
     * method is called. This allows for maximum flexibility when traversing the IDL
     * tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA910B008A
     */
    public void processIDLValueForwardDef(IdlValue object) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL value
     * inheritance list is found. The corresponding traverse method in the
     * IDLTraverser (if present) should be called from the implementation of this
     * process method, unless of course other functionality is desired. Note that
     * functionality can be inserted before AND after the corresponding traverse
     * method is called. This allows for maximum flexibility when traversing the IDL
     * tree structure.
     *
     * @param inheritanceList An array containing IDL valuetype objects that need to
     * be processed and traversed further (if desired)
     * @throws Exception
     * @roseuid 40DA910C029D
     */
    public void processIDLValueInheritance(IdlValue[] inheritanceList) throws Exception;

    /**
     * This method will be called by the IDLTraverser each time an IDL wide string
     * object is found. The corresponding traverse method in the IDLTraverser (if
     * present) should be called from the implementation of this process method,
     * unless of course other functionality is desired. Note that functionality can be
     * inserted before AND after the corresponding traverse method is called. This
     * allows for maximum flexibility when traversing the IDL tree structure.
     *
     * @param object The IDL object that needs to be processed and traversed further
     * (if possible)
     * @throws Exception
     * @roseuid 40DA910E01D2
     */
    public void processIDLWString(IdlWString object) throws Exception;
}
