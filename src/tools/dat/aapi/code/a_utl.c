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



/* Initialises the context and returns a new pointer to it.
 * It must be called before using any of the stop watch functions.
 * Remember to de-init after use!
 */
a_utlContext
a_utlInit()
{
	a_utlContext context = a_memAlloc(sizeof(struct a_utlContext_s));
	context->sw_start = os_timeGet();
	context->sw_stop  = context->sw_start;
	return context;
}



/* De-initialises the context
 */
void
a_utlDeInit(
	a_utlContext context)
{
	if (context) {
		a_memFree(context);
	}
}



/* Starts the Stop Watch
 */
void
a_utlStopWatchStart(
	a_utlContext context)
{
	context->sw_start = os_timeGet();
}



/* Stops the Stop Watch
 */
void
a_utlStopWatchStop(
	a_utlContext context)
{
	context->sw_stop = os_timeGet();
}



/* Returns the elapsed time, between starting and
 * stopping the stop watch, in microseconds.
 */
unsigned long
a_utlGetStopWatchTimeMicroSecs(
	a_utlContext context)
{
	unsigned long result;
	os_time os_time_result = os_timeSub(context->sw_stop, context->sw_start);
	result = (unsigned long)((os_time_result.tv_sec) * 1000000 + os_time_result.tv_nsec / 1000);
	return result;
}



/* Returns the elapsed time, between starting and
 * stopping the stop watch, in milliseconds.
 */
unsigned long
a_utlGetStopWatchTimeMilSecs(
	a_utlContext context)
{
	unsigned long result;
	os_time os_time_result = os_timeSub(context->sw_stop, context->sw_start);
	result = (unsigned long)((os_time_result.tv_sec) * 1000 + os_time_result.tv_nsec / 1000000);
	return result;
}





/* Returns a (static) string representation of a
 * c_metaKind value.
 */
char *
a_utlMetaKindStr(
	c_metaKind kind)
{
    switch(kind) {
		case M_UNDEFINED:    return "M_UNDEFINED";    break;
		case M_ATTRIBUTE:    return "M_ATTRIBUTE";    break;
		case M_CLASS:        return "M_CLASS";        break;
		case M_COLLECTION:   return "M_COLLECTION";   break;
		case M_CONSTANT:     return "M_CONSTANT";     break;
		case M_CONSTOPERAND: return "M_CONSTOPERAND"; break;
		case M_ENUMERATION:  return "M_ENUMERATION";  break;
		case M_EXCEPTION:    return "M_EXCEPTION";    break;
		case M_EXPRESSION:   return "M_EXPRESSION";   break;
		case M_INTERFACE:    return "M_INTERFACE";    break;
		case M_LITERAL:      return "M_LITERAL";      break;
		case M_MEMBER:       return "M_MEMBER";       break;
		case M_MODULE:       return "M_MODULE";       break;
		case M_OPERATION:    return "M_OPERATION";    break;
		case M_PARAMETER:    return "M_PARAMETER";    break;
		case M_PRIMITIVE:    return "M_PRIMITIVE";    break;
		case M_RELATION:     return "M_RELATION";     break;
		case M_BASE:         return "M_BASE";         break;
		case M_STRUCTURE:    return "M_STRUCTURE";    break;
		case M_TYPEDEF:      return "M_TYPEDEF";      break;
		case M_UNION:        return "M_UNION";        break;
		case M_UNIONCASE:    return "M_UNIONCASE";    break;
		case M_COUNT:        return "M_COUNT";        break;
		default:             return "(unknown)";      break;
    }
}



/* Returns a (static) string representation of a
 * c_collKind value.
 */
char *
a_utlCollKindStr(
	c_collKind kind)
{
	switch(kind) {
		case C_UNDEFINED:   return "C_UNDEFINED";  break;
		case C_LIST:        return "C_LIST";       break;
		case C_ARRAY:       return "C_ARRAY";      break;
		case C_BAG:         return "C_BAG";        break;
		case C_SET:         return "C_SET";        break;
		case C_MAP:         return "C_MAP";        break;
		case C_DICTIONARY:  return "C_DICTIONARY"; break;
		case C_SEQUENCE:    return "C_SEQUENCE";   break;
		case C_STRING:      return "C_STRING";     break;
		case C_WSTRING:     return "C_WSTRING";    break;
		case C_QUERY:       return "C_QUERY";      break;
		case C_SCOPE:       return "C_SCOPE";      break;
		case C_COUNT:       return "C_COUNT";      break;
		default:            return "(unknown)";    break;
	}
}
		


/* Returns a (static) string representation of a
 * c_primKind value.
 */
char *
a_utlPrimKindStr(
	c_primKind kind)
{
	switch(kind) {
		case P_UNDEFINED: return "P_UNDEFINED"; break;
		case P_BOOLEAN:   return "P_BOOLEAN";   break;
		case P_CHAR:      return "P_CHAR";      break;
		case P_WCHAR:     return "P_WCHAR";     break;
		case P_OCTET:     return "P_OCTET";     break;
		case P_SHORT:     return "P_SHORT";     break;
		case P_USHORT:    return "P_USHORT";    break;
		case P_LONG:      return "P_LONG";      break;
		case P_ULONG:     return "P_ULONG";     break;
		case P_LONGLONG:  return "P_LONGLONG";  break;
		case P_ULONGLONG: return "P_ULONGLONG"; break;
		case P_FLOAT:     return "P_FLOAT";     break;
		case P_DOUBLE:    return "P_DOUBLE";    break;
		case P_VOIDP:     return "P_VOIDP";     break;
		case P_MUTEX:     return "P_MUTEX";     break;
		case P_LOCK:      return "P_LOCK";      break;
		case P_COND:      return "P_COND";      break;
		case P_COUNT:     return "P_COUNT";     break;
		default:          return "(unknown)";   break;
	}
}



/* Returns a (static) string representation of a
 * c_exprKind value.
 */
char *
a_utlExprKindStr(
	c_exprKind kind)
{
	switch(kind) {
		case E_UNDEFINED:  return "E_UNDEFINED";  break;
		case E_OR:         return "E_OR";         break;
		case E_XOR:        return "E_XOR";        break;
		case E_AND:        return "E_AND";        break;
		case E_SHIFTRIGHT: return "E_SHIFTRIGHT"; break;
		case E_SHIFTLEFT:  return "E_SHIFTLEFT";  break;
		case E_PLUS:       return "E_PLUS";       break;
		case E_MINUS:      return "E_MINUS";      break;
		case E_MUL:        return "E_MUL";        break;
		case E_DIV:        return "E_DIV";        break;
		case E_MOD:        return "E_MOD";        break;
		case E_NOT:        return "E_NOT";        break;
		case E_COUNT:      return "E_COUNT";      break;
		default:           return "(unknown)";    break;
	}
}



/* Returns a (static) string representation of a
 * c_direction value.
 */
char *
a_utlDirectionStr(
	c_direction dir)
{
	switch(dir) {
		case D_UNDEFINED: return "D_UNDEFINED"; break;
		case D_IN:        return "D_IN";        break;
		case D_OUT:       return "D_OUT";       break;
		case D_INOUT:     return "D_INOUT";     break;
		case D_COUNT:     return "D_COUNT";     break;
		default:          return "(unknown)";   break;
	}
}



/* Returns a (static) string representation of a
 * c_valueKind value.
 */
char *
a_utlValueKindStr(
	c_valueKind kind)
{
	switch (kind) {
		case V_UNDEFINED: return "(Undefined)"; break;
		case V_BOOLEAN:   return "Boolean";     break;
		case V_OCTET:     return "Octet";       break;
		case V_SHORT:     return "Short";       break;
		case V_LONG:      return "Long";        break;
		case V_LONGLONG:  return "LongLong";    break;
		case V_USHORT:    return "UShort";      break;
		case V_ULONG:     return "ULong";       break;
		case V_ULONGLONG: return "ULongLong";   break;
		case V_FLOAT:     return "Float";       break;
		case V_DOUBLE:    return "Double";      break;
		case V_CHAR:      return "Char";        break;
		case V_STRING:    return "String";      break;
		case V_WCHAR:     return "WChar";       break;
		case V_WSTRING:   return "WString";     break;
		case V_FIXED:     return "Fixed";       break;
		case V_OBJECT:    return "Object";      break;
		default:          return "(unknown)";   break;
	}
}



//END a_utl.c
