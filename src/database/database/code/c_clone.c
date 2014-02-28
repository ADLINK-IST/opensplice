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
#include "os.h"
#include "c__base.h"
#include "c__metabase.h"
#include "os_report.h"
#include "c__scope.h"
#include "c_collection.h"
#include "ut_collection.h"
#include "c_clone.h"

#define c_array(c) ((c_array)(c))

/* to enable debugging tracing for C_CLONE, change below statement to
 * #define C_CLONE_TRACE(p) p
 */
#define C_CLONE_TRACE(p)

/*
 * parameter t the type of object o
 * parameter o the object of which must be determined whether it's a type
 * Returns TRUE if o is a type. The type of o is needed, because o could be
 * an inline object, and then c_getType will not work.
 */
#define isType(t, o) (c_isBaseObjectType(t) && c_objectIsType(o))
#define isModule(t, o) (c_isBaseObjectType(t) && c_baseObjectKind(o) == M_MODULE) /*todo: finish this */

/*
 * This object contains a reference to the destination database and a collection
 * of processed objects for resolving. This collection contains all 'source
 * object'-'destination object' pointer pairs, so for each processed object in
 * the source database, there is a matching object in the destination database.
 */
struct c_clone_s {
    ut_collection processed_objects;
    c_base destination;
    /* If cls is a c_metaObject or a c_type, something special has to be done
     * (which is explained there). Therefore we cache the following variables
     * for quick comparison:
     */
    c_type type_c_metaObject;
    c_type type_c_type;
};

/*
 * This value is used for indentation during debugging tracing. This is not
 * thread safe, as it's a shared static variable for the entire process.
 */
C_CLONE_TRACE(static c_long indent = 0;)

/*
 * This function does the actual cloning. A more detailed explanation is given
 * at the implementation.
 */
c_object
_c_cloneAction(
    c_clone _this,
    c_type type,
    c_object srcObj,
    c_object dstObj);

/*
 * Returns the string representation of a c_metaKind as a hardcoded string.
 */
static const c_char *
metaKindImage (
    c_metaKind kind)
{
#define _CASE_(o) case o: return #o
    switch (kind) {
    _CASE_(M_UNDEFINED);
    _CASE_(M_ATTRIBUTE);
    _CASE_(M_CLASS);
    _CASE_(M_COLLECTION);
    _CASE_(M_CONSTANT);
    _CASE_(M_CONSTOPERAND);
    _CASE_(M_ENUMERATION);
    _CASE_(M_EXCEPTION);
    _CASE_(M_EXPRESSION);
    _CASE_(M_INTERFACE);
    _CASE_(M_LITERAL);
    _CASE_(M_MEMBER);
    _CASE_(M_MODULE);
    _CASE_(M_OPERATION);
    _CASE_(M_PARAMETER);
    _CASE_(M_PRIMITIVE);
    _CASE_(M_RELATION);
    _CASE_(M_BASE);
    _CASE_(M_STRUCTURE);
    _CASE_(M_TYPEDEF);
    _CASE_(M_UNION);
    _CASE_(M_UNIONCASE);
    _CASE_(M_COUNT);
    default:
        return "Unknown metaKind specified";
            }
#undef _CASE_
}


/*
 * Compares two pointers.
 * Returns:
 *  - OS_EQ when pointers are equal
 *  - OS_GT when o1 is bigger than o2
 *  - OS_LT when o1 is littler than o2
 */
static os_equality
c_cloneComparePointers(
        void *o1,
        void *o2,
        void *args)
{
    os_equality result = OS_EQ;

    OS_UNUSED_ARG(args);

    if(o1 > o2) {
        result = OS_GT;
    }
    else if(o1 < o2)
    {
        result = OS_LT;
    }
    return result;
}

/*
 * This method is used by c_clone to free the c_objects
 * when the c_clone->processed_objects collection is freed.
 */
static void
c_clonePointerFree(
        void *o,
        void *arg)
{
    OS_UNUSED_ARG(arg);

    c_free(o);
}

/*
 * The method c_clone uses this method to check whether
 * certain objects have already been cloned or are already being cloned.
 * It's also a faster way to resolve types (faster than c_metaResolveType),
 * when the to be resolved type is already cloned or is already being cloned.
 * If the object is successfully resolved, the pointer will be stored in the
 * dstObj parameter. If the object is not present, dstObj will be set to NULL.
 * Returns true is no error has occurred, false otherwise and then the value of
 * dstObj will be undetermined. If an error occurs, this method will generate a
 * message in the error log. The object *dstObj will be c_keep'd.
 *
 * - Notes:
 *    - The resolved object (*dstOBj) is c_keep'ed.
 *    - It does not matter from which database the object comes.
 */
static c_bool
c_cloneResolve(
        c_clone _this,
        c_object srcObj,
        c_object *dstObj)
{
    c_bool result = TRUE;
    C_CLONE_TRACE(indent++;)
    C_CLONE_TRACE(printf("%*s==========\n%*s_resolve\n%*s==========\n", indent*2, " ", indent*2, " ", indent*2, " "));
    C_CLONE_TRACE(printf("%*sresolving a %s\n", indent*2, " ", c_metaObject(c_getType(srcObj))->name));

    *dstObj = ut_get(_this->processed_objects, srcObj);

    /*
     * If the object has not been resolved, we check whether it's a type
     * or a module, because then we can try to resolve it using
     * c_metaResolve.
     */
    if(!(*dstObj)
            && (isType(c_getType(srcObj), srcObj) || isModule(c_getType(srcObj), srcObj)))
    {
        c_string type_name = c_metaScopedName(c_metaObject(srcObj));

        C_CLONE_TRACE(printf("%*sresolving %s\n", indent*2, " ", type_name));

        *dstObj = c_metaResolve(c_metaObject(_this->destination), type_name);
        os_free(type_name); /* c_metaScopedName() allocates on heap */
        if(*dstObj) {
            C_CLONE_TRACE(printf("%*sfound by meta resolving\n", indent*2, " "));
            if(c_metaCompare(srcObj, *dstObj) == E_UNEQUAL) {
                OS_REPORT_1(
                        OS_ERROR,
                        "c_cloneResolve",
                        0,
                        "Source type and destination type both exist (type name is '%s'), but are not equal.",
                        c_metaObject(srcObj)->name);
                assert(FALSE);
                *dstObj = NULL;
                result = FALSE;
            }
        }
    }
    else
    {
        C_CLONE_TRACE(printf("%*sfound in adm tree\n", indent*2, " "));
        c_keep(*dstObj);
    }

    C_CLONE_TRACE(if(!(*dstObj)) printf("%*snot found!\n", indent*2, " "));
    C_CLONE_TRACE(indent--);
    return result;
}

/*
 * This method recursively clones all (parent) modules of the
 * module given. It returns the cloned (and c_keep'ed) module of the module given.
 * This is done separately from the rest of the clone process, as cloning a module
 * is the only exception to not cloning all of it's children.
 *
 * The exception is, because if you clone a struct A in module M, you want A to be
 * completely cloned, including its namespace, but any other types in module M
 * should not be cloned. Returns a c_keep'd c_module.
 *
 */
static c_module
c_cloneModules(
        c_clone _this,
        c_module module)
{
    c_module dstModule;
    c_base srcDB;

    assert(module);

    srcDB = c_getBase(module);

    if(!c_cloneResolve(_this, module, (c_object*)&dstModule))
    {
        return NULL; /* error */
    }

    if(!dstModule){
        if(c_metaObject(module)->definedIn == c_metaObject(srcDB)) /* 0 case */
        {
            dstModule = c_module(c_metaDeclare(c_metaObject(_this->destination), c_metaObject(module)->name, M_MODULE));
        }
        else
        {
            c_module definedIn = c_cloneModules(_this, c_module(c_metaObject(module)->definedIn));
            assert(c_baseObjectKind(definedIn) == M_MODULE);
            dstModule = c_module(c_metaDeclare(c_metaObject(definedIn), c_metaObject(module)->name, M_MODULE));
        }

        c_keep(module); /* when put into administration, it will ultimately be freed again */
        c_keep(dstModule);
        ut_tableInsert(ut_table(_this->processed_objects), module, dstModule);
    }

    return dstModule;
}

/*
 * This struct is used by c_cloneCloneCollection, which is a helper function
 * for cloning collections. The dstCollection pointer contains a reference
 * to the collection to be cloned, the c parameter contains a reference to the
 * c_clone object.
 */
typedef struct c_cloneCloneCollectionArg_s {
   c_object dstCollection;
   c_clone c;
} * c_cloneCloneCollectionArg;

/*
* Used by _c_cloneAction in a collection walk to clone the objects in a collection.
* First the c_clone->processed_objects is checked whether it contains the object
* (which would mean the object has already been cloned, or is being cloned).
* If the object has not been found, then the type of the object will be resolved,
* after which the object is cloned and inserted into the collection contained by the
* arg parameter.
*/
static c_bool
c_cloneCloneCollection (
        c_object o,
        c_voidp actionArg)
{
   c_clone c;
   c_object clonedObj = NULL;
   c_cloneCloneCollectionArg arg = (c_cloneCloneCollectionArg)actionArg;

   c = arg->c;

   /*
    * First try to resolve the object.
    */
   if(c_cloneResolve(c, o, &clonedObj))
   {
       if(!clonedObj){
           c_type oType = c_getType(o);
           c_type clonedObjType = NULL;
           /*
            * Try to resolve the type of the object.
            */
           if(c_cloneResolve(c, oType, (void*)&clonedObjType))
           {
               /*
                * If the type of the to-be cloned object is not resolved, the type
                * must be cloned too.
                */
               if(!clonedObjType)
               {
                   clonedObjType = _c_cloneAction(c, c_getBase(arg->dstCollection)->metaType[c_baseObjectKind(oType)], oType, NULL);
               }

               /*
                * Now we have all we need to clone the object.
                */
               assert(clonedObjType);
               clonedObj = _c_cloneAction(c, clonedObjType, o, NULL);
               c_free(clonedObjType);
           }
           else
           {
               /* error */
               return FALSE;
           }
       }

       if(clonedObj){
           if(c_collectionTypeKind(c_getType(arg->dstCollection)) == C_SCOPE){
               c_scopeInsert((c_scope)arg->dstCollection, c_metaObject(clonedObj));
           }
           else /* this is any other collection */
           {
               c_insert((c_collection)arg->dstCollection, clonedObj);
           }

           c_free(clonedObj);
       }
   }
   else
   {
       /* error */
       return FALSE;
   }

   return TRUE;
}

/*
 * Used by _c_clone in a c_scopeWalk to clone the objects in a c_scope.
 */
static void
c_cloneCloneScopeObject (
        c_metaObject o,
        c_scopeWalkActionArg actionArg)
{
    c_cloneCloneCollection((c_object)o, (c_voidp)actionArg);
}


/*
 * Creates a new c_clone object which contains a reference to the destination
 * database and holds the administration of processed objects.
 */
c_clone
c_cloneNew(c_base destination)
{
    c_clone _this;

    assert(destination);

    _this = (c_clone)os_malloc(sizeof(struct c_clone_s));

    _this->destination = destination;
    _this->processed_objects = NULL;
    _this->type_c_metaObject = c_type(c_metaResolveType(c_metaObject(destination), "c_metaObject"));
    _this->type_c_type = c_type(c_metaResolveType(c_metaObject(destination), "c_type"));

    return _this;
}

/*
 * Cleans up the c_clone object.
 */
void
c_cloneFree(c_clone _this)
{
    assert(_this);
    assert(_this->processed_objects == NULL);

    c_free(_this->type_c_metaObject);
    c_free(_this->type_c_type);
    _this->type_c_metaObject = NULL;
    _this->type_c_type = NULL;

    os_free(_this);
}

/*
 * This function is for cloning objects. The c_clone object _this contains the reference to
 * the destination database where obj should be cloned into. This is a deep-copy, so
 * all references are cloned too.
 *
 * Pre-condition: all basic meta-types are available in the destination database
 * Result: obj and anything it's dependent on (types, references) is cloned to the destination database.
 * Returns: the cloned object from the destination database.
 */
c_object
c_cloneCloneObject(
        c_clone _this,
        c_object obj)
{
    c_object o;
    c_type type = NULL;

    /*
     * Before each action a clean administration of the processed objects is created,
     * and after the clone, destroyed.
     * This is done, because of various problems if we don't do this:
     *  - cloning from different databases
     *  - the c_keep on objects in both the source and destination databases gives many
     *      problems:
     *      - state changes in databases
     *      - destroying of databases
     *
     * If it is decided that the administration should be kept over multiple clone
     * actions, then the creation of the administration should be moved to c_cloneNew,
     * and the destruction to c_cloneFree.
     *
     * What to keep in mind before deciding this:
     *  - the clone object is meant for 1-1 relationship between source and destination
     *    database. Because the clone object is created with the destination db as
     *    parameter, following this constraint for the destination db is no problem.
     *    But the source database is also not allowed to change. This should be enforced.
     *  - if a database is destroyed (source or destination), then the clone object
     *    should also be destroyed, where all references within the administration
     *    to all types and objects are released.
     *  - if the last reference keeper outside the clone object frees the object,
     *    then it will not be deleted yet, because the clone object's administration
     *    still contains a reference. This is probably not desirable.
     */
    _this->processed_objects = ut_tableNew(c_cloneComparePointers, NULL);

    if(c_cloneResolve(_this, c_getType(obj), (c_object*)&type)){
        if(!type) {
            _c_cloneAction(_this,
                           _this->destination->metaType[c_baseObjectKind(c_getType(c_getType(obj)))],
                           c_getType(obj),
                           &type);
        }
    }
    else
    {
        /* error */
        return NULL;
    }

    assert(type && c_getBase(type) == _this->destination);

    o = _c_cloneAction(_this, type, obj, NULL);

    ut_tableFree(ut_table(_this->processed_objects), c_clonePointerFree, NULL, c_clonePointerFree, NULL);
    _this->processed_objects = NULL;

    if(isType(type, obj))
    {
        assert(c_metaCompare(obj, o) == E_EQUAL);
    }

    return o;
}

/*
 * This method does the actual cloning. The c_clone _this object contains the reference
 * to the c_clone object containing the administration of processed objects (to be
 * able to handle cyclic references, and so objects are not cloned multiple times),
 * and a reference to the destination database. The c_type type is the type of
 * the object to be cloned. This type is expected to reside in the destination
 * database. The parameter srcObj contains the reference to the object to be cloned,
 * and dstObj optionally the reference to the already mallocced memory to clone
 * the object to. This is internally used for cloning inline objects. If this
 * parameter is NULL, then the object is mallocced here, cloned and returned as a result.
 *
 * Preconditions:
 *  - parameter type is in the destination's database.
 *
 * Result:
 *  - successfully clones srcObj
 *  - returns a reference to the cloned object (c_keep'd if applicable, i.e. in case of reference objects)
 */
c_object
_c_cloneAction(
    c_clone _this,
    c_type type,
    c_object srcObj,
    c_object dstObj)
{
    assert(type);
    assert(_this);
    C_CLONE_TRACE(indent++);

    if(srcObj == NULL) /* easy copy; Null in source database, is Null in target database */
    {
        return NULL;
    }

    if(srcObj == c_getBase(srcObj)) /* easy copy: apparently srcObj == database, so return destination database */
    {
        return _this->destination;
    }

    C_CLONE_TRACE(
        if(isType(type, srcObj)){
            printf("%*sCLONING TYPE '%s'\n", indent*2, " ", c_metaObject(srcObj)->name);
        } else {
            printf("%*sCLONING OBJECT of type '%s'\n", indent*2, " ", c_metaObject(type)->name);
        }
    )

    /*
     * Check whether dstObj is NULL or not. If it isn't, it's an inline object
     * to which the srcObj data must be cloned. If dstObj is NULL, we need
     * to see if it already exists (and return it if true), or
     * allocate memory depending on what kind of object it is.
     */
    if(!dstObj){
        if(c_cloneResolve(_this, srcObj, &dstObj)) /* if no error during resolving */
        {
            if(dstObj)  /* already cloned or already busy with cloning it */
            {
                C_CLONE_TRACE(indent--);
                return dstObj;
            }
            else if (c_baseObjectKind(type) == M_COLLECTION)
            {
                switch(c_collectionTypeKind(type)){
                    case C_ARRAY:
                    case C_SEQUENCE:
                        dstObj = c_newBaseArrayObject(c_collectionType(type), c_arraySize(srcObj));
                        break;
                    case C_SCOPE:
                        dstObj = c_new(type);
                        break;
                    case C_STRING:
                        dstObj = c_stringMalloc(type->base, strlen(srcObj) + 1); /* + 1 for the '\0' character */
                        break;
                    case C_WSTRING:
                        dstObj = c_wstringMalloc(type->base, strlen(srcObj) + 1); /* + 1 for the '\0' character */
                        break;
                    case C_LIST:
                    case C_BAG:
                    case C_SET:
                    case C_DICTIONARY:
                    case C_QUERY:
                    case C_MAP:
                        dstObj = c_new(type);
                        break;
                    default:
                        assert(FALSE);
                        break;
                }
            }
            else
            {
                /* at this moment dstObj has not yet been set, so we can
                 * create is using the c_new operator.
                 */

                dstObj = c_new(type);
            }
            assert(dstObj);

            /* now insert it into the processed_objects administration,
             * so future (recursive ones especially) c_clone calls know it is
             * being cloned or is cloned.
             */
            ut_tableInsert(ut_table(_this->processed_objects), c_keep(srcObj), c_keep(dstObj));
        }
        else
        {
            C_CLONE_TRACE(indent--);
            return NULL; /* error */
        }
    }


    switch(c_baseObjectKind(type)){
        case M_CLASS:
        case M_INTERFACE:
        {
            c_object *dstRef;
            c_object srcRef;
            c_class cls = c_class(type);
            /*
             * we are going to handle cls and each type it extends. This is done as
             * follows:
             * while(cls){
             *      ... do what is necessary ...
             *      cls = cls->extends;
             * }
             */

            /*
             * First copy the whole memory value, this includes all references which is
             * incorrect but will be corrected in following loop that also recursively clones referenced objects.
             */
            memcpy(dstObj, srcObj, c_type(type)->size);

            while(cls){
                C_CLONE_TRACE(printf("%*shandling '%s'\n", indent*2, " ", c_metaObject(cls)->name));
                if(c_typeHasRef(c_type(cls))){
                    int i = 0;
                    c_long size = c_arraySize(c_interface(cls)->references);
                    for(i=0; i<size; i++)
                    {
                        c_type srcType, dstType = NULL;
                        srcRef = *(c_object*)C_DISPLACE(srcObj, c_property(c_interface(cls)->references[i])->offset);
                        dstRef = (c_object*)C_DISPLACE(dstObj, c_property(c_interface(cls)->references[i])->offset);
                        /* the type of the object srcRef described in the references array
                         * could be a (abstract) supertype of the actual type. Because we
                         * know that it's a reference, we can get the type from the
                         * header of the object by using c_getType(). This
                         * way c_clone has the proper type to clone the object.
                         */

                        if(srcRef == NULL){
                            /* if srcRef is NULL, the *dstRef is also NULL.
                             */
                            *dstRef = NULL;
                        }
                        else
                        {
                            srcType = c_getType(srcRef);
                            if(!c_cloneResolve(_this, c_object(srcType), (c_object*)&dstType)){
                                /* error */
                                C_CLONE_TRACE(indent--);
                                return NULL;
                            }

                            assert(dstType);
                            assert(c_typeIsRef(c_property(c_interface(cls)->references[i])->type));

                            *dstRef = _c_cloneAction(_this,
                                              dstType,
                                              srcRef,
                                              NULL);
                        }

                    }
                }

                /* if the class is a c_type, then its object has a pointer to the database (type->base). This
                 * reference is not referenced by the type's interface->references array, and so must be
                 * pointed to as a separate case.
                 */
                if(c_object(cls) == c_object(_this->type_c_metaObject))
                {
                    c_type(dstObj)->base = _this->destination;
                }

                /* if the class is a c_metaObject, then its object has a pointer
                 * to the object that object is defined in (c_metaObject->definedIn). This
                 * reference is not referenced by the type's interface->references array, and so must be
                 * pointed to as a seperate case.
                 */
                else if(c_object(cls) == c_object(_this->type_c_type))
                {
                    C_CLONE_TRACE(int i = 0;)
                    C_CLONE_TRACE(printf("%*ssize of references in c_metaObject: %d\n", indent*2, " ", c_arraySize(c_interface(cls)->references)));
                    C_CLONE_TRACE(
                        for(i = 0; i<c_arraySize(c_interface(cls)->references); i++)
                        {
                            printf("%*s  - ref (cnt: %d) %d: %s\n",
                                    indent*2, " ",
                                    c_refCount(c_interface(cls)->references[i]),
                                    i, c_metaObject(c_interface(cls)->references[i])->name);
                        }
                    )

                    /*
                     * Modules are handled as a special case, because we don't want
                     * every object and type in a module cloned. This is done, because
                     * e.g. when we clones some struct in module M in the source database,
                     * the result should be a struct in module M in the destination database.
                     * All other objects and types in module M in the source database
                     * is of no interest to us.
                     */
                    if(c_baseObjectKind(c_metaObject(srcObj)->definedIn) == M_MODULE)
                    {
                        c_metaObject(dstObj)->definedIn = c_metaObject(
                                                                c_cloneModules(
                                                                        _this,
                                                                        c_module(c_metaObject(srcObj)->definedIn))
                                                          );
                        /* Object only needs to be inserted into the scope when the scope of the
                         * dstObj is a module, as these are not completely cloned. Other objects
                         * are automatically inserted into the scope's scope, as the scope is cloned
                         * too.
                         */
                        c_scopeInsert(c_module(c_metaObject(dstObj)->definedIn)->scope, dstObj);
                    }
                    else
                    {
                        /*
                         * This object is not definedIn a module, but some other
                         * metaObject. First see if the definedIn metaObject is
                         * in the destination database.
                         */
                        if(!c_cloneResolve(_this,
                                           c_metaObject(srcObj)->definedIn,
                                           (c_object*)&c_metaObject(dstObj)->definedIn))
                        {
                            C_CLONE_TRACE(indent--);
                            return NULL; /* error */
                        }

                        /* if the definedIn metaObject is not in the destination
                         * database..
                         */
                        if(!c_metaObject(dstObj)->definedIn)
                        {
                            c_type definedInType = NULL;

                            /* resolve the type of the definedIn metaObject in the
                             * destination database.
                             */
                            if(!c_cloneResolve(_this,
                                               c_getType(c_metaObject(srcObj)->definedIn),
                                         (c_object*)&definedInType))
                            {
                                C_CLONE_TRACE(indent--);
                                return NULL; /* error */
                            }

                            /*
                             * If the type of the definedIn metaObject cannot be
                             * resolved, clone it.
                             */
                            if(!definedInType)
                            {
                                definedInType = _c_cloneAction(_this,
                                        type->base->metaType[c_baseObjectKind(c_getType(c_metaObject(srcObj)->definedIn))],
                                        c_getType(c_metaObject(srcObj)->definedIn),
                                        NULL);
                            }

                            /*
                             * Clone the definedIn metaObject to the destination
                             * database.
                             */
                            c_metaObject(dstObj)->definedIn = _c_cloneAction(_this,
                                                                       definedInType,
                                                                       c_metaObject(srcObj)->definedIn,
                                                                       NULL);
                            c_free(definedInType);
                        }
                    }
                }

                cls = cls->extends;
            }
        }
        break;
        case M_STRUCTURE:
        case M_EXCEPTION:
            memcpy(dstObj, srcObj, c_type(type)->size);
            if(c_typeHasRef(type)){
                int i = 0;
                c_long size = c_arraySize(c_structure(type)->references);
                for(i=0; i< size; i++)
                {
                    if(!c_typeIsRef(c_specifier(c_structure(type)->references[i])->type))
                    {
                       if(c_typeHasRef(c_specifier(c_structure(type)->references[i])->type))
                       {
                           _c_cloneAction(_this,
                                    c_specifier(c_structure(type)->references[i])->type,
                                    C_DISPLACE(srcObj, c_member(c_structure(type)->references[i])->offset),
                                    C_DISPLACE(dstObj, c_member(c_structure(type)->references[i])->offset));
                       }
                    }
                    else
                    {
                        /* this is a reference to an object, so get the type
                         * from the header using c_getType.
                         * srcType the type in the source database
                         * dstType the matching type in the destination database
                         */
                        c_type srcType, dstType;

                        c_object* ref = (c_object*)C_DISPLACE(dstObj, c_member(c_structure(type)->references[i])->offset);
                        c_object src = *(c_object*)C_DISPLACE(srcObj, c_member(c_structure(type)->references[i])->offset);

                        srcType = c_getType(src);

                        dstType = _c_cloneAction(_this, c_getBase(type)->metaType[c_baseObjectKind(c_getType(srcType))], srcType, NULL);

                        assert(dstType);

                        *ref = _c_cloneAction(_this,
                                        dstType,
                                        src,
                                        NULL);

                        c_free(dstType);
                    }
                }
            }
        break;
        case M_COLLECTION:
            switch(c_collectionTypeKind(type)){
                case C_SEQUENCE:
                case C_ARRAY:
                {
                    int i, size = 0;
                    c_type subtype;

                    subtype = c_collectionTypeSubType(type);

                    if(c_collectionTypeKind(type) == C_ARRAY && c_collectionTypeMaxSize(type) > 0) {
                        size = c_collectionTypeMaxSize(type);
                    }
                    else
                    {
                        size = c_arraySize(srcObj);
                    }
                    /*
                     * c_typeHasRef returns true when c_typeIsRef or c_typeHasRef is true for
                     * any object it contains. I.e. if an array with inline structs (no ref)
                     * contains a pointer (ref), c_typeHasRef also returns TRUE.
                     *
                     * Therefore, the subtype should be checked if it is a ref
                     * or not, to know how to clone it (memory is already allocated
                     * or not).
                     */
                    if(c_typeHasRef(type))
                    {
                        if(c_typeIsRef(subtype)){
                            c_object src;
                            c_type srcType, dstType;
                            for(i=0; i<size; i++){
                                    src = (c_object)c_array(srcObj)[i];

                                    srcType = c_getType(src);

                                    dstType = _c_cloneAction(_this, c_getBase(type)->metaType[c_baseObjectKind(c_getType(srcType))], srcType, NULL);

                                    assert(dstType);

                                    c_array(dstObj)[i] =
                                            _c_cloneAction(_this,
                                                     dstType,
                                                     src,
                                                     NULL);

                                    c_free(dstType);
                            }
                        }
                        else
                        {
                            for(i=0; i<size; i++){
                                _c_cloneAction(_this,
                                         c_collectionType(type)->subType,
                                         &c_array(srcObj)[i],
                                         &c_array(dstObj)[i]);
                            }
                        }
                    }
                    else
                    {
                        memcpy(dstObj, srcObj, size * c_type(c_collectionType(type)->subType)->size);
                    }
                }
                break;
                case C_STRING:
                    memcpy(dstObj, srcObj, (strlen(srcObj)+1) * sizeof(c_char)); /* copy the '\0' character also */
                    break;
                case C_WSTRING:
                    memcpy(dstObj, srcObj, (strlen(srcObj)+1) * sizeof(c_wchar)); /* copy the '\0' character also */
                    break;
                case C_SCOPE:
                case C_LIST:
                case C_BAG:
                case C_SET:
                case C_DICTIONARY:
                case C_QUERY:
                {
                    struct c_cloneCloneCollectionArg_s arg;
                    arg.dstCollection = (c_object)dstObj;
                    arg.c = _this;

                    if(c_collectionTypeKind(type) == C_SCOPE){
                        c_scopeWalk(c_scope(srcObj), c_cloneCloneScopeObject, &arg);
                    }
                    else
                    {
                        if(!c_walk((c_collection)srcObj, c_cloneCloneCollection, (c_voidp)&arg))
                        {
                            /* error, already reported */
                            C_CLONE_TRACE(indent--);
                            return NULL;
                        }
                    }
                }
                break;
                case C_MAP:
                    /* c_map? */
                    assert(FALSE);
                    break;
                default:
                    assert(FALSE);
                    break;
            }
        break;
        case M_TYPEDEF:
            _c_cloneAction(_this, c_typeActualType(type), srcObj, dstObj);
            break;
        case M_PRIMITIVE:
        case M_ENUMERATION:
            memcpy(dstObj, srcObj, c_type(type)->size);
            break;
        case M_UNION:
        {
            c_value v;
            c_array references;

            memcpy(dstObj, srcObj, c_type(type)->size);
#define _CASE_(k,t) case k: v = t##Value(*((t *)srcObj)); break
            switch (c_metaValueKind(c_metaObject(c_union(srcObj)->switchType))) {
            _CASE_(V_BOOLEAN,   c_bool);
            _CASE_(V_OCTET,     c_octet);
            _CASE_(V_SHORT,     c_short);
            _CASE_(V_LONG,      c_long);
            _CASE_(V_LONGLONG,  c_longlong);
            _CASE_(V_USHORT,    c_ushort);
            _CASE_(V_ULONG,     c_ulong);
            _CASE_(V_ULONGLONG, c_ulonglong);
            _CASE_(V_CHAR,      c_char);
            _CASE_(V_WCHAR,     c_wchar);
            default:
                OS_REPORT(OS_ERROR,
                          "c_clone",0,
                          "illegal union switch type detected");
                assert(FALSE);
                C_CLONE_TRACE(indent--);
                return NULL;
            break;
            }
#undef _CASE_
            references = c_union(srcObj)->references;
            if (references != NULL) {
                int i=0, j=0;
                c_bool done = FALSE;
                c_long nrOfRefs, nrOfLabs;
                c_array labels;

                nrOfRefs = c_arraySize(references);
                for (i=0; (i<nrOfRefs) && !done; i++)
                {
                    labels = c_unionCase(references[i])->labels;
                    j=0;
                    nrOfLabs = c_arraySize(labels);
                    for (j=0; (j<nrOfLabs) && !done; j++)
                    {
                        if (c_valueCompare(v,c_literal(labels[j])->value) == C_EQ) {
                            if(c_typeIsRef(c_specifier(references[i])->type)){
                                c_type srcType, dstType;

                                c_object src = *(c_object*)C_DISPLACE(srcObj, c_type(c_union(srcObj)->switchType)->size);

                                srcType = c_getType(src);

                                dstType = _c_cloneAction(_this, c_getBase(type)->metaType[c_baseObjectKind(c_getType(srcType))], srcType, NULL);

                                assert(dstType);

                                *(c_object*)C_DISPLACE(dstObj, c_type(c_union(dstObj)->switchType)->size)
                                        = _c_cloneAction(_this,
                                                   dstType,
                                                   src,
                                                   NULL);

                                c_free(dstType);
                            }
                            else
                            {
                                _c_cloneAction(_this,
                                         c_specifier(references[i])->type,
                                         C_DISPLACE(srcObj, c_type(c_union(srcObj)->switchType)->size),
                                         C_DISPLACE(dstObj, c_type(c_union(dstObj)->switchType)->size));
                            }
                            done = TRUE;
                        }
                    }
                }
            }
        }
        break;
        default:
            OS_REPORT_1(OS_ERROR, "c_clone", 0,
                        "Could not clone object of type %s",
                        metaKindImage(c_baseObjectKind(type)));
            dstObj = NULL;
            assert(FALSE);
    }

    C_CLONE_TRACE(indent--);
    return dstObj;
}
