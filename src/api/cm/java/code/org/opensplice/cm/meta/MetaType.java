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
package org.opensplice.cm.meta;

import java.io.StringWriter;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.StringTokenizer;

/**
 * Represents a type of a Splice database object (c_type).
 *
 * This object can be used to get information about the structure of data and so
 * it enables the possibility to read/write data of this type.
 */
public class MetaType {
    /**
     * Constructs new MetaType.
     */
    public MetaType(String xmlType){
        isValid = false;
        mayFinalize = true;
        metaField = null;
        typedefs = new LinkedHashMap<MetaField, String>();
        this.xmlType = xmlType;
        this.normalizeXMLType();
    }

    private void normalizeXMLType(){
        boolean inTag = false;
        boolean inCloseTag = false;
        boolean inNodeValue = false;
        StringWriter writer = new StringWriter();
        char[] chars = this.xmlType.toCharArray();
        char cur;

        for(int i=0; i<chars.length; i++){
            cur = chars[i];

            if(inTag){
                if(cur == '>'){
                    inTag = false;

                    if(inCloseTag){
                        inCloseTag = false;
                    } else {
                        inNodeValue = true;
                    }
                } else if(cur == '/'){
                    inCloseTag = true;
                }
                writer.write(cur);
            } else if(inNodeValue){
                if(cur == '<'){
                    inNodeValue = false;
                    inTag = true;
                    writer.write(cur);
                } else {
                    writer.write(cur);
                }
            } else {
                if(cur == ' '){

                } else if(cur == '\n'){

                } else {
                    writer.write(cur);
                }
            }
        }
        this.xmlType = writer.toString();
    }

    /**
     * Adds a typedef to the type.
     *
     * @param typedefName
     *            The name of the typedef.
     * @param field
     *            The field that is 'typedeffed'.
     */
    public void addTypedef(String typedefName, MetaField field){
        typedefs.put(field, typedefName);
    }

    /**
     * Provides access to all typedefs in the type.
     *
     * @return A hashmap that contains all typedefs in the type.
     */
    public LinkedHashMap<MetaField, String> getTypedefs() {
        return typedefs;
    }

    /**
     * Provides access to the name of the typedef in this type.
     *
     * @param field
     *            The field to look the typedef for.
     * @return The name of the typedef if the field is indeed typedeffed or null
     *         otherwise.
     */
    public String getFieldTypedefName(MetaField field){
        return (typedefs.get(field));
    }

    /**
     * Provides access to the isValid boolean.
     *
     * @return true if the type is valid, false otherwise.
     */
    public boolean isValid(){
        return isValid;
    }

    /**
     * Assigns the supplied field to the type.
     *
     * @param field
     *            The field to assign to this type.
     */
    public void setField(MetaField field){
        metaField = field;

    }

    /**
     * Provides access to the field in the type with the supplied name.
     *
     * Nested names are allowed.Example
     *
     * @verbatim IDL: module my_mod{ struct my_structure{ long a; struct
     *           my_inner_struct{ long b; } my_struct; }; };
     * @endverbatim Fields in this type can be accessed as: - my_structure.a -
     *              my_structure.my_struct.b
     *
     * @param fieldName
     *            The name of the field the resolve the value of.
     * @return The field associated with the supplied fieldname or null if it
     *         cannot be found.
     */
    public MetaField getField(String fieldName){
        StringTokenizer tokenizer = new StringTokenizer(fieldName, ".");
        MetaField current = metaField;
        MetaField temp;
        String token;

        while(tokenizer.hasMoreTokens()){
            token = tokenizer.nextToken();

            temp = current.getField(token);

            if((temp != null) && (tokenizer.hasMoreTokens())){
                current = temp;
            }
            else if(temp != null){
                return temp;
            }
            if ((temp != null) && (tokenizer.hasMoreTokens())) {
                current = temp;
            }
            else{
                /* Maybe it is a subType, continue search
                 * it should look like: memberName[i][j]
                 */
                StringTokenizer tokenizer2 = new StringTokenizer(token, "[");
                int depth = tokenizer2.countTokens() - 1;

                if(depth > 0){
                    String token2 = tokenizer2.nextToken();
                    temp = current.getField(token2);

                    for(int i=0; i<depth; i++){
                        if(temp != null){
                            if(temp instanceof MetaCollection){
                                temp = ((MetaCollection)temp).getSubType();
                            }
                            else{
                                return null;
                            }
                        }
                        else{
                            return null;
                        }
                    }
                }
                if (!tokenizer.hasMoreTokens()) {
                    return temp;
                } else {
                    if (temp != null) {
                        current = temp;
                    }
                }
            }
        }
        return null;
    }

    /**
     * Provides access to all fieldnames in the type.
     *
     * @return The array of all fieldnames in the type.
     */
    public String[] getFieldNames(){
        String[] result;
        MetaField[] fields = metaField.getFields();
        ArrayList<Object> alist = new ArrayList<Object>();

        for(int i=0; i<fields.length; i++){
            alist.addAll(fields[i].getFieldNames());

        }
        result = (alist.toArray(new String[1]));

        return result;
    }

    /**
     * Provides access to all fields in the root field.
     *
     * @return The array of fields in the root field.
     */
    public MetaField[] getFields(){
        return metaField.getFields();
    }

    /**
     * Provides access to the root field.
     *
     * @return The root field of the data.
     */
    public MetaField getRootField(){
        return metaField;
    }

    /**
     * Function to notify that the type will not be correct and/or complete
     * after deserialization.
     *
     * After calling this function, a call to finalizeType will NOT result in
     * setting the isValid field to true.
     *
     * @see finalizeType
     */
    public void mayNotBeFinalized(){
        mayFinalize = false;
    }

    /**
     * Finalizes the type by setting the isValid field. If the
     * mayNotBeFinalized() function has been called prior to calling this
     * function, this will not result in a valid type and false will be
     * returned.
     *
     * @return true if the type is valid, false otherwise.
     * @see mayNotBeFinalized()
     */
    public boolean finalizeType(){
        if(mayFinalize){
            isValid = true;
        }
        return isValid;
    }

    public String toXML(){
        return this.xmlType;
    }

    @Override
    public boolean equals(Object obj){
        boolean result = false;

        if(obj instanceof MetaType){
            if(((MetaType)obj).toXML().equals(this.xmlType)){
                result = true;
            }
        }
        return result;
    }

    @Override
    public int hashCode() {
        int var_gen_code;
        int hash = 13;
        var_gen_code = isValid ? 1 : 0;
        var_gen_code += mayFinalize ? 1 : 0;
        var_gen_code += (null == metaField ? 0 : metaField.hashCode());
        var_gen_code += (null == typedefs ? 0 : typedefs.hashCode());
        var_gen_code += (null == xmlType ? 0 : xmlType.hashCode());
        hash = 31 * hash + var_gen_code;
        return hash;
    }

    public LinkedHashMap<String, String> collectAllFieldNames(boolean init) {
        LinkedHashMap<String, String> result = new LinkedHashMap<String, String>();
        MetaField[] fields = this.getFields();
        MetaField field;

        for (int i = 0; i < fields.length; i++) {
            field = fields[i];
            this.walkOverNames(field.getName(), field, result, -1, null, init);
        }
        return result;
    }

    public LinkedHashMap<String, String> collectAllFieldNames(int limit, boolean init) {
        LinkedHashMap<String, String> result = new LinkedHashMap<String, String>();
        MetaField[] fields = this.getFields();
        MetaField field;

        for (int i = 0; i < fields.length; i++) {
            field = fields[i];
            this.walkOverNames(field.getName(), field, result, limit, null, init);
        }
        return result;
    }

    public LinkedHashMap<String, String> collectAllFieldNames(int limit, String startStruct, boolean init) {
        LinkedHashMap<String, String> result = new LinkedHashMap<String, String>();
        MetaField[] fields = this.getFields();
        MetaField field;
        if (startStruct != null) {
            startStruct = removeIndices(startStruct, null);
        }

        for (int i = 0; i < fields.length; i++) {
            field = fields[i];
            this.walkOverNames(field.getName(), field, result, limit, startStruct, init);
        }
        return result;
    }

    private void addFieldName(LinkedHashMap<String, String> result, String fieldName, String startStruct, String value) {
        if (startStruct == null) {
            result.put(fieldName, value);
        } else if (startStruct != null && fieldName.startsWith(startStruct)) {
            result.put(fieldName, value);
        }
    }

    private void walkOverNames(String nestedFieldName, MetaField field, LinkedHashMap<String, String> result,
            int limit, String startStruct, boolean init) {

        if (field instanceof MetaPrimitive) { // Primitive
            this.addPrimitive(nestedFieldName, result, limit, startStruct, (MetaPrimitive) field, init);
        } else if (field instanceof MetaEnum) { // Enumeration
            this.addEnum(nestedFieldName, result, limit, startStruct, (MetaEnum) field, init);
        } else if (field instanceof MetaUnion) { // Union
            this.addUnion(nestedFieldName, (MetaUnion) field, result, limit, startStruct, init);
        } else if (field instanceof MetaStruct) { // Structure
            this.addStruct(nestedFieldName, (MetaStruct) field, result, limit, startStruct, init);
        } else if (field instanceof MetaClass) { // Class
            this.addClass(nestedFieldName, (MetaClass) field, result, limit, startStruct, init);
        } else if (field instanceof MetaCollection) { // Collection
            this.addCollection(nestedFieldName, (MetaCollection) field, result, limit, startStruct, init);
        }
    }

    private void addCollection(String nestedFieldName, MetaCollection field, LinkedHashMap<String, String> result,
            int limit, String startStruct, boolean init) {
        int size;
        String typeName = field.getTypeName();
        String value;
        if (init) {
            value = "test";
        } else {
            value = "N/A";
        }

        if ((typeName.equals("c_string")) || (typeName.equals("c_wstring")) || (typeName.startsWith("C_STRING<"))
                || (typeName.startsWith("C_WSTRING<"))) {
            addFieldName(result, nestedFieldName, startStruct, value);
        } else { // Not a String collection.
            size = field.getMaxSize();
            if (size == -1) {
                addFieldName(result, nestedFieldName, startStruct, "RECURSIVE TYPE NOT SUPPORTED");
            } else {
                if ((limit < 0 && startStruct == null) || (limit > 0 && startStruct == null)) {
                    this.walkOverNames(nestedFieldName, field.getSubType(), result, limit - 1, startStruct, init);
                } else if (nestedFieldName.startsWith(startStruct + ".") && limit > 0) {
                    this.walkOverNames(nestedFieldName, field.getSubType(), result, limit - 1, startStruct, init);
                } else if (limit > 0 && startStruct != null) {
                    this.walkOverNames(nestedFieldName, field.getSubType(), result, limit, startStruct, init);
                }
            }
        }
    }

    private void addPrimitive(String nestedFieldName, LinkedHashMap<String, String> result, int limit,
            String startStruct, MetaPrimitive primType, boolean init) {
        String typeName = primType.getTypeName();

        String value = null;

        if ("c_bool".equals(typeName)) {
            if (init) {
                value = "TRUE";
            } else {
                value = "N/A";
            }
        } else if ("c_char".equals(typeName)) {
            if (init) {
                value = "a";
            } else {
                value = "N/A";
            }
        } else if ("c_voidp".equals(typeName)) {
            if (init) {
                value = "NULL";
            } else {
                value = "N/A";
            }
        } else {
            if (init) {
                value = "0";
            } else {
                value = "N/A";
            }
        }
        addFieldName(result, nestedFieldName, startStruct, value);
    }

    private void addEnum(String nestedFieldName, LinkedHashMap<String, String> result, int limit, String startStruct,
            MetaEnum enumType, boolean init) {
        String value = null;
        if (init) {
            value = enumType.getPosValues()[0];
        } else {
            value = "N/A";
        }

        addFieldName(result, nestedFieldName, startStruct, value);
    }

    private void addUnion(String nestedFieldName, MetaUnion field, LinkedHashMap<String, String> result, int limit,
            String startStruct, boolean init) {
        String value = "N/A";
        MetaUnionCase cases[] = field.getCases();
        if (limit < 0 || limit > 0) {
            addFieldName(result, nestedFieldName + ".switch", startStruct, (String) field.getCases()[0].getLabels()
                    .get(0));
            for (int i = 0; i < cases.length; i++) {
                this.walkOverNames(nestedFieldName + "." + cases[i].getField().getName(), cases[i].getField(), result,
                        limit, startStruct, init);
            }
        } else {
            if (startStruct == null) {
                addFieldName(result, field.getName(), startStruct, value);
            } else {
                addFieldName(result, nestedFieldName + "." + field.getName(), startStruct, value);
            }
        }
    }

    private void addStruct(String nestedFieldName, MetaStruct field, LinkedHashMap<String, String> result, int limit,
            String startStruct, boolean init) {
        MetaField[] fields = field.getFields();
        String value = null;
        if (init) {
            value = "NULL";
        } else {
            value = "N/A";
        }
        if (limit < 0 || limit > 0) {
            for (int i = 0; i < fields.length; i++) {
                this.walkOverNames(nestedFieldName + "." + fields[i].getName(), fields[i], result, limit, startStruct,
                        init);
            }
        } else {
            addFieldName(result, nestedFieldName, startStruct, value);
        }

    }

    private void addClass(String nestedFieldName, MetaClass field, LinkedHashMap<String, String> result, int limit,
            String startStruct, boolean init) {
        MetaField[] fields = field.getFields();
        String value = null;
        if (init) {
            value = "NULL";
        } else {
            value = "N/A";
        }
        if (limit < 0 || limit > 0) {
            for (int i = 0; i < fields.length; i++) {
                this.walkOverNames(nestedFieldName + "." + fields[i].getName(), fields[i], result, limit, startStruct,
                        init);
            }
        } else {
            addFieldName(result, nestedFieldName, startStruct, value);
        }
    }

    public String removeIndices(String name, String struct) {
        String result;
        StringWriter w;
        char c;
        int in;
        /* strip struct off the name if it is set */
        if (struct != null && !struct.equals(name)) {
            name = name.substring(name.indexOf(struct) + 1);
        }

        if (name.indexOf('[') == -1) {
            result = name;
        } else {
            w = new StringWriter();
            int length = name.length();
            in = 0;

            for (int i = 0; i < length; i++) {
                c = name.charAt(i);

                if (c == '[') {
                    in++;
                } else if (c == ']') {
                    in--;
                } else if (in == 0) {
                    w.append(c);
                }
            }
            result = w.toString();
        }

        if (struct != null && !struct.equals(name)) {
            result = struct + "." + result;
        }

        return result;
    }

    public String removeIndicesFromStruct(String name, String struct) {
        String result = name;

        /* strip struct off the name if it is set */
        if (struct != null && name.startsWith(struct) && !struct.equals(result)) {
            String tmp = result.substring(struct.length());
            if (tmp.startsWith("[")) {
                result = tmp.substring(tmp.indexOf("]") + 1);
                result = struct + result;
            }
        }
        return result;
    }

    public String getCollectionFieldName(String name, int colIndex) {
        int index;
        String result = name;

        if (name.endsWith("]")) {
            index = name.lastIndexOf('[');
            result = name.substring(0, index);
            index = result.lastIndexOf('[');

            while ((index != -1) && (result.endsWith("]"))) {
                result = result.substring(0, index);
                index = result.lastIndexOf('[');
            }
            index = result.length();
            result += "[" + colIndex + "]";
            result += name.substring(index);
        } else {
            result += "[" + colIndex + "]";
        }

        return result;
    }

    /**
     * The root field in the type.
     */
    private MetaField metaField;

    /**
     * Boolean that specifies if the type is valid. This field is used by the
     * deserializer, so it can specify if the type could be completely
     * deserialized. If this is not the case, this field will be false.
     */
    private boolean mayFinalize;

    /**
     * Boolean that specifies if the type is complete and correct
     * This is used to catch unknown types in the deserializer. In the
     * future it is not necessary anymore.
     */
    private boolean isValid;

    /**
     * Ordered HashMap that contains all typedefs in the type.
     * (<MetaField field, String typedefName>)
     */
    private final LinkedHashMap<MetaField, String> typedefs;

    private String xmlType;
}
