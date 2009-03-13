
#include "os.h"
#include "os_report.h"

#include "v__keyType.h"

#include "c_metabase.h"
#include "c_iterator.h"
#include "c_stringSupport.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/

c_type
v_keyTypeNew(
    c_type type,
    const c_char *keyList)
{
    c_type instKeyType;
    c_type foundType;
    c_base base;
    c_iter list;
    c_string name, str;
    c_metaObject o;
    c_long totalKeyNames;

    base = c_getBase(type);
    instKeyType = c_type(c_metaDefine(c_metaObject(base),M_CLASS));

    /* define key attributes */
    totalKeyNames = 0;

    list = c_splitString(keyList, ", \t");
    name = c_iterTakeFirst(list);
    while (name != NULL) {
        totalKeyNames += strlen(name);
        
        foundType = c_type(c_metaResolve(c_metaObject(type), name));
        assert(foundType != NULL);
        /* replace '.' by '_' */
        str = name;
        while (*str != '\0') {
            if (*str == '.') {
                *str = '_';
            }
            str++;
        }
        o = c_metaDeclare(c_metaObject(instKeyType), name, M_ATTRIBUTE);
        switch (c_baseObject(foundType)->kind) {
        case M_ATTRIBUTE:
        case M_RELATION:
            c_property(o)->type = c_keep(c_property(foundType)->type);
        break;
        case M_MEMBER:
            c_property(o)->type = c_keep(c_specifier(foundType)->type);
        break;
        default:
            OS_REPORT(OS_ERROR, "v_keyType", 0, "Unsupported key field");
            assert(FALSE);
        break;
        }
        
        c_free(o);
        os_free(name);
        name = c_iterTakeFirst(list);
    };
    c_iterFree(list);

    c_metaObject(instKeyType)->definedIn = c_keep(base);
    c_metaFinalize(c_metaObject(instKeyType));

    /* bind the type to a name with format <type>(<key>*), where
       <type> is the typename
       <key>* is comma-seperated list of key names
    */
    str = c_metaScopedName(c_metaObject(type));
    list = c_splitString(keyList, ", \t");

    name = (c_string)os_malloc(totalKeyNames + strlen(str) + c_iterLength(list) + strlen("()"));
    strcpy(name, str);
    strcat(name, "(");
    os_free(str);
    str = c_iterTakeFirst(list);
    while (str != NULL) {
        strcat(name, str);
        os_free(str);
        str = c_iterTakeFirst(list);
    }
    strcat(name, ")");
    c_iterFree(list);

    foundType = c_type(c_metaBind(c_metaObject(base), name, c_metaObject(instKeyType)));
printf("bound keytype %s\n", name);
    os_free(name);
    c_free(instKeyType);
    
    return foundType;
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
