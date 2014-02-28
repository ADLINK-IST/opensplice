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
package org.opensplice.cm.transform.xml;

import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.StringTokenizer;
import java.util.logging.Level;
import java.util.logging.Logger;

import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaClass;
import org.opensplice.cm.meta.MetaCollection;
import org.opensplice.cm.meta.MetaEnum;
import org.opensplice.cm.meta.MetaField;
import org.opensplice.cm.meta.MetaPrimitive;
import org.opensplice.cm.meta.MetaStruct;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.meta.MetaUnion;
import org.opensplice.cm.meta.MetaUnionCase;
import org.opensplice.cm.transform.TransformationException;
import org.opensplice.cm.transform.UserDataSerializer;

/**
 * The XML implementation of an UserDataSerializer. It is capable of
 * transforming a UserData object into a serialized XML representation.
 *
 * @date Jun 2, 2004
 */
public class UserDataSerializerXML implements UserDataSerializer {

    /**
     * Creates a new serializer, that is capable of transforming UserData to its
     * XML representation.
     */
    public UserDataSerializerXML() {
        type = null;
        logger = Logger.getLogger("org.opensplice.api.cm.transform.xml");

    }

    @Override
    public synchronized String serializeUserData(UserData data)
            throws TransformationException {
        if (data == null) {
            throw new TransformationException("Supplied UserData is not valid.");
        }
        StringWriter writer = new StringWriter();
        type = data.getUserDataType();
        MetaField[] fields = type.getFields();
        MetaField field;

        writer.write("<object>");

        for (int i = 0; i < fields.length; i++) {
            field = fields[i];
            this.serializeType(writer, field.getName(), field, data, true);
        }

        writer.write("</object>");
        writer.flush();
        /*
         * logger.logp(java.util.logging.Level.FINEST, "UserDataSerializerXML",
         * "serializeUserData", "Serialized data:\n" + writer.toString());
         */
        return writer.toString();
    }

    private void serializeType(StringWriter writer, String nestedFieldName,
            MetaField field, UserData data, boolean printField) {
        if (printField) {
            writer.write("<" + field.getName() + ">");
        }

        if (field instanceof MetaPrimitive) { // Primitive
            this.serializePrimitive(writer, nestedFieldName, data, field);
        } else if (field instanceof MetaEnum) { // Enumeration
            this.serializeEnum(writer, nestedFieldName, data);
        } else if (field instanceof MetaUnion) { // Union
            this.serializeUnion(writer, nestedFieldName, (MetaUnion) field,
                    data);
        } else if (field instanceof MetaStruct) { // Structure
            this.serializeStruct(writer, nestedFieldName, (MetaStruct) field,
                    data);
        } else if (field instanceof MetaClass) { // Class
            this.serializeClass(writer, nestedFieldName, (MetaClass) field,
                    data);
        } else if (field instanceof MetaCollection) { // Collection
            this.serializeCollection(writer, nestedFieldName,
                    (MetaCollection) field,
                    data);
        }
        if (printField) {
            writer.write("</" + field.getName() + ">");
        }
    }

    private void serializeCollection(StringWriter writer,
            String nestedFieldName, MetaCollection field, UserData data) {
        int size, maxSize;
        String typeName = field.getTypeName();

        if ((typeName.equals("c_string")) ||
                (typeName.equals("c_wstring")) ||
                (typeName.startsWith("C_STRING<")) ||
                (typeName.startsWith("C_WSTRING<"))) {
            this.serializeString(writer, nestedFieldName, data);
        } else { // Not a String collection.
            maxSize = field.getMaxSize();

            if (maxSize == 0) {
                this.serializeUnboundedSequence(writer, nestedFieldName, field,
                        data);
            } else if (maxSize == -1) { /* recursive type */
                this.serializeRecursiveType(writer, nestedFieldName, field,
                        data);
            } else {
                /* This is an array or bounded sequence. */
                if (typeName.startsWith("C_ARRAY")) {
                    /* When dealing with an array, then the actual size
                     * is the same as the maximum size. */
                    size = maxSize;
                } else {
                    /* When dealing with a bounded sequence, then the actual
                     * size can differ from the maximum size. */
                    size = getSequenceSize(nestedFieldName, field, data);
                    /* Also, we need this size within the data. */
                    writer.write("<size>" + size + "</size>");
                }
                /* Now, fill up the elements. */
                for (int i = 0; i < size; i++) {
                    writer.write("<element>");
                    this.serializeType(writer,
                            getCollectionFieldName(nestedFieldName, i),
                            field.getSubType(), data, false);
                    writer.write("</element>");
                }
            }
        }
    }

    private String getCollectionFieldName(String name, int colIndex) {
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

    private void serializeString(StringWriter writer, String nestedFieldName, UserData data) {
        String strData = data.getFieldValue(nestedFieldName);
        if (strData != null) {
            strData = strData.replaceAll("&", "&amp;").replaceAll("<", "&lt;").replaceAll(">", "&gt;");
        }
        writer.write(strData);
    }

    private void serializeClass(StringWriter writer, String nestedFieldName,
            MetaClass field, UserData data) {
        MetaField[] fields = field.getFields();

        for (int i = 0; i < fields.length; i++) {
            this.serializeType(writer,
                    nestedFieldName + "." + fields[i].getName(),
                    fields[i], data, true);
        }
    }

    private void serializeStruct(StringWriter writer, String nestedFieldName,
            MetaStruct field, UserData data) {
        MetaField[] fields = field.getFields();

        for (int i = 0; i < fields.length; i++) {
            this.serializeType(writer,
                    nestedFieldName + "." + fields[i].getName(),
                    fields[i], data, true);
        }
    }

    private void serializePrimitive(StringWriter writer, String nestedFieldName, UserData data, MetaField mf) {
        String value = data.getFieldValue(nestedFieldName);

        if (value == null) {

            String typeName = ((MetaPrimitive) mf).getTypeName();

            if ("c_bool".equals(typeName)) {
                    value = "TRUE";
            } else if ("c_char".equals(typeName)) {
                    value = "a";
            } else if ("c_voidp".equals(typeName)) {
                    value = "NULL";
            } else {
                value = "0";
            }
            logger.logp(Level.SEVERE, "UserDataSerializerXML", "serializePrimitive", "Could not find value for field: "
                    + nestedFieldName + " initializing with default value: " + value);
        }
        writer.write(value);
    }

    private void serializeEnum(StringWriter writer, String nestedFieldName,
            UserData data) {
        writer.write(data.getFieldValue(nestedFieldName));
    }

    private void serializeUnion(StringWriter writer, String nestedFieldName,
            MetaUnion field, UserData data) {
        String value = data.getFieldValue(nestedFieldName + ".switch");

        MetaUnionCase c = field.getCase(value);

        writer.write("<switch>" + value + "</switch>");
        this.walkType(writer, nestedFieldName + "." + c.getField().getName(),
                c.getField(), data);
    }

    /**
     * Serializes the supplied field that has the supplied nested name.
     *
     * @param nestedFieldName
     *            The nested field name (that means: including scope) of the
     *            field.
     * @param field
     *            The field to serialize.
     * @param data
     *            The data to serialize.
     */
    private void walkType(StringWriter writer, String nestedFieldName,
            MetaField field, UserData data) {

        writer.write("<" + field.getName() + ">");

        if (field instanceof MetaPrimitive) { // Primitive
            this.serializeMemberContents(writer, nestedFieldName, data);
        } else if (field instanceof MetaEnum) { // Enumeration
            this.serializeMemberContents(writer, nestedFieldName, data);
        } else if (field instanceof MetaCollection) { // Collection
            if (field.getTypeName().startsWith("C_SEQUENCE")) {
                int size = ((MetaCollection) field).getMaxSize();

                if (size == 0) {
                    this.serializeUnboundedSequence(writer, nestedFieldName,
                            (MetaCollection) field, data);
                } else {
                    writer.write("<size>" + size + "</size>");
                    this.serializeMemberContents(writer, nestedFieldName, data);
                }
            } else if (field.getTypeName().startsWith("C_ARRAY")) {
                int size = ((MetaCollection) field).getMaxSize();

                if (size == 0) {
                    writer.write("<size>" + size + "</size>");
                    this.serializeUnboundedSequence(writer, nestedFieldName,
                            (MetaCollection) field, data);
                } else {
                    this.serializeMemberContents(writer, nestedFieldName, data);
                }
            } else {
                this.serializeMemberContents(writer, nestedFieldName, data);
            }
        } else if (field instanceof MetaUnion) { // Union
            this.serializeUnionContents(writer, nestedFieldName,
                    (MetaUnion) field, data);
        } else { // Class or Structure
            MetaField field2;
            MetaField[] fields = field.getFields();

            for (int i = 0; i < fields.length; i++) {
                field2 = fields[i];
                this.walkType(writer, nestedFieldName + "." + field2.getName(),
                        field2, data);
            }
        }
        writer.write("</" + field.getName() + ">");
    }

    /**
     * Serializes the contents of the supplied union.
     *
     * @param nestedFieldName
     *            The name of the union, including its scope.
     * @param field
     *            The union field to serialize the contents of.
     * @param data
     *            The data to serialize.
     */
    private void serializeUnionContents(StringWriter writer,
            String nestedFieldName, MetaUnion field, UserData data) {
        String value = data.getFieldValue(nestedFieldName + ".switch");

        MetaUnionCase c = field.getCase(value);

        writer.write("<switch>" + value + "</switch>");
        this.walkType(writer, nestedFieldName + "." + c.getField().getName(),
                c.getField(), data);

    }

    /**
     * Serializes the contents of the supplied field. This is done be looking up
     * the field in the type of the data and retrieving the value of the field
     * in the userdata.
     *
     * @param nestedFieldName
     *            The name of the field including its scope.
     * @param data
     *            The data to serialize.
     */
    private void serializeMemberContents(StringWriter writer,
            String nestedFieldName, UserData data) {
        boolean isString = false;
        String nestedFieldTypeName = this.removeIndices(nestedFieldName);
        MetaField typeField = type.getField(nestedFieldTypeName);
        MetaField originalType = typeField;
        String value = null;

        value = data.getFieldValue(nestedFieldName);
        logger.logp(java.util.logging.Level.FINEST, "UserDataSerializerXML",
                "serializeMemberContents", "Fieldname: " + nestedFieldName
                        + " Value: " + value);

        if (typeField instanceof MetaCollection) {
            MetaCollection colType = (MetaCollection) typeField;

            while ((typeField instanceof MetaCollection) && (!isString)) {
                String typeName = typeField.getTypeName();

                if ((typeName.equals("c_string")) ||
                        (typeName.equals("c_wstring")) ||
                        (typeName.startsWith("C_STRING<")) ||
                        (typeName.startsWith("C_WSTRING<"))) {
                    value = this.getSerializedStringCollection(nestedFieldName,
                            colType, data);
                    isString = true;
                } else {
                    typeField = ((MetaCollection) typeField).getSubType();

                }
            }

            if (!isString) { // Type is not a string and does not contain
                             // strings
                if ((originalType instanceof MetaCollection)
                        && (typeField instanceof MetaStruct)) {
                    MetaField field2;
                    MetaField[] fields = typeField.getFields();
                    int size = ((MetaCollection) originalType).getMaxSize();

                    for (int i = 0; i < size; i++) {
                        writer.write("<element>");

                        for (int j = 0; j < fields.length; j++) {
                            field2 = fields[j];
                            this.walkType(writer, nestedFieldName +
                                    "[" + i + "]." + field2.getName(),
                                    field2, data);
                        }
                        writer.write("</element>");
                    }
                    value = "";
                } else if ((originalType instanceof MetaCollection) &&
                        (originalType.getTypeName().startsWith("C_SEQUENCE<"))) {
                    value = this.getSerializedStringCollection(nestedFieldName,
                            colType, data);
                } else {
                    value = value.replaceAll("\\[", "<element>");
                    value = value.replaceAll("\\]", "</element>");
                    value = value.replaceAll(",", "</element><element>");
                }
            } else {
                // Do nothing.
            }
        }
        writer.write(value);
    }

    /**
     * Serializes the contents of the supplied collection.
     *
     * @param nestedFieldName
     *            The name of the collection field in the data.
     * @param colType
     *            The type of the collection.
     * @param data
     *            The data to serialize.
     * @return The serialized representation of the collection data.
     */
    private String getSerializedStringCollection(String nestedFieldName,
            MetaCollection colType, UserData data) {
        String result = null;
        String size = null;
        String typeName = colType.getTypeName();
        int index = nestedFieldName.indexOf('[');
        String temp = null;

        if ((typeName.equals("c_string")) ||
                (typeName.equals("c_wstring")) ||
                (typeName.startsWith("C_STRING<")) ||
                (typeName.startsWith("C_WSTRING<"))) {
            String strData = data.getFieldValue(nestedFieldName);
            result = strData.replaceAll("&", "&amp;").replaceAll("<", "&lt;").replaceAll(">", "&gt;");
        } else if (colType.getSubType() instanceof MetaCollection) {
            result = "";

            if (colType.getSubType().getTypeName().startsWith("C_SEQUENCE<")) {
                size = "<size>"
                        + ((MetaCollection) colType.getSubType()).getMaxSize()
                        + "</size>";
            }

            if (index != -1) {
                temp = nestedFieldName.substring(0, index);

                for (int i = 0; i < colType.getMaxSize(); i++) {
                    result += "<element>";

                    if (size != null) {
                        result += size;
                    }
                    result += getSerializedStringCollection(
                            temp + "[" + i + "]"
                                    + nestedFieldName.substring(index),
                                                (MetaCollection) colType
                                                        .getSubType(),
                                                data);
                    result += "</element>";
                }
            } else {
                for (int i = 0; i < colType.getMaxSize(); i++) {
                    result += "<element>";

                    if (size != null) {
                        result += size;
                    }
                    result += getSerializedStringCollection(nestedFieldName
                            + "[" + i + "]",
                                                (MetaCollection) colType
                                                        .getSubType(),
                                                data);
                    result += "</element>";
                }
            }
        } else {
            result = "";

            if (index != -1) {
                temp = nestedFieldName.substring(0, index);

                for (int i = 0; i < colType.getMaxSize(); i++) {
                    result += "<element>";
                    result += data.getFieldValue(temp + "[" + i + "]"
                            + nestedFieldName.substring(index));
                    result += "</element>";
                }
            } else {
                for (int i = 0; i < colType.getMaxSize(); i++) {
                    result += "<element>";
                    result += data.getFieldValue(nestedFieldName + "[" + i
                            + "]");
                    result += "</element>";
                }
            }
        }
        return result;
    }

    /**
     * Acquires the number of elements of the supplied sequence.
     *
     * @param nestedFieldName
     *            The name of the field (including scope)
     * @param seqType
     *            The type of the sequence.
     * @param data
     *            The data that is being serialized.
     * @return The number of elements in the given sequence.
     */
    private int getSequenceSize(String nestedFieldName,
            MetaCollection seqType, UserData data) {
        /*
         * This is very much based on how serializeUnboundedSequence get
         * the size of its unbounded sequence.
         * The same actions can be used on bounded sequences as well to
         * get their actual size (which can be less then their maximum
         * size).
         */
        int size = 0;

        StringTokenizer tokenizer;
        String typeName;
        String value;
        String token;
        MetaField subType = seqType.getSubType();

        if (subType instanceof MetaPrimitive) {
            value = data.getFieldValue(nestedFieldName);

            if (value != null) {
                if (!(value.equals("NULL") || value.equals("[]"))) {
                    tokenizer = new StringTokenizer(value.substring(1,
                            value.length() - 1), ",");
                    size = tokenizer.countTokens();
                }
            } else {
                // also check for the alternative notation
                value = data.getFieldValue(nestedFieldName + "[0]");
                if (value != null) {
                    List<String> values = new ArrayList<String>();
                    int index = 1;
                    while (value != null) {
                        values.add(value);
                        value = data.getFieldValue(nestedFieldName + "["
                                + index + "]");
                        index++;
                    }
                    size = values.size();
                }
            }
        } else if (subType instanceof MetaCollection) {
            typeName = subType.getTypeName();

            if ((typeName.equals("c_string")) ||
                    (typeName.equals("c_wstring")) ||
                    (typeName.startsWith("C_STRING<")) ||
                    (typeName.startsWith("C_WSTRING<"))) {
                value = data.getFieldValue(nestedFieldName);

                if (value != null) {
                    if (!(value.equals("NULL") || value.equals("[]"))) {
                        tokenizer = new StringTokenizer(value.substring(1,
                                value.length() - 1), ",");
                        size = tokenizer.countTokens();
                    }
                }
            }
        } else if ((subType instanceof MetaStruct) ||
                   (subType instanceof MetaUnion ) ) {
            // Verify if member "size" exists, if so, then serialize and
            // increment
            LinkedHashSet<String> fields = data.getFieldNames();
            boolean found = true;
            while (found) {
                String entryFieldName = nestedFieldName + "[" + size + "]";
                boolean fieldExists = false;
                Iterator<String> it = fields.iterator();
                while (!fieldExists && it.hasNext()) {
                    if (it.next().startsWith(entryFieldName)) {
                        fieldExists = true;
                    }
                }
                if (fieldExists) {
                    size++;
                } else {
                    found = false;
                }
            }
        }
        return size;
    }

    /**
     * Serializes an unbounded sequence.
     *
     * @param nestedFieldName
     *            The name of the field (including scope)
     * @param seqType
     *            The type of the sequence.
     * @param data
     *            The data to serialize.
     */
    private void serializeUnboundedSequence(StringWriter writer,
            String nestedFieldName, MetaCollection seqType, UserData data) {
        String value;
        String typeName;
        StringTokenizer tokenizer;
        MetaField subType = seqType.getSubType();
        String token;
        String seqTypeName;

        seqTypeName = seqType.getTypeName();

        if (subType instanceof MetaPrimitive) {
            value = data.getFieldValue(nestedFieldName);

            if (value != null) {
                if (value.equals("NULL")) {
                    writer.write("&lt;NULL&gt;");
                } else if (value.equals("[]")) {
                    writer.write("<size>0</size>");
                } else {
                    tokenizer = new StringTokenizer(value.substring(1,
                            value.length() - 1), ",");
                    int size = tokenizer.countTokens();

                    writer.write("<size>" + size + "</size>");

                    for (int i = 0; i < size; i++) {
                        token = tokenizer.nextToken();
                        writer.write("<element>" + token + "</element>");
                    }
                }
            } else {
                // also check for the alternative notation
                value = data.getFieldValue(nestedFieldName + "[0]");
                if (value != null) {
                    List<String> values = new ArrayList<String>();
                    int index = 1;
                    while (value != null) {
                        values.add(value);
                        value = data.getFieldValue(nestedFieldName + "["
                                + index + "]");
                        index++;
                    }
                    writer.write("<size>" + values.size() + "</size>");
                    Iterator<String> it = values.iterator();
                    while (it.hasNext()) {
                        writer.write("<element>" + it.next() + "</element>");
                    }
                } else {
                    writer.write("&lt;NULL&gt;");
                }
            }
        } else if (subType instanceof MetaCollection) {
            typeName = subType.getTypeName();

            if ((typeName.equals("c_string")) ||
                    (typeName.equals("c_wstring")) ||
                    (typeName.startsWith("C_STRING<")) ||
                    (typeName.startsWith("C_WSTRING<"))) {
                value = data.getFieldValue(nestedFieldName);

                if (value != null) {
                    if (value.equals("NULL")) {
                        writer.write("&lt;NULL&gt;");
                    } else if (value.equals("[]")) {
                        if (!(seqTypeName.startsWith("C_ARRAY"))) {
                            writer.write("<size>0</size>");
                        }
                    } else {
                        tokenizer = new StringTokenizer(value.substring(1,
                                value.length() - 1), ",");
                        int size = tokenizer.countTokens();

                        if (!(seqTypeName.startsWith("C_ARRAY"))) {
                            writer.write("<size>" + size + "</size>");
                        }
                        for (int i = 0; i < size; i++) {
                            token = tokenizer.nextToken();
                            writer.write("<element>" + token + "</element>");
                        }
                    }
                } else {
                    writer.write("&lt;NULL&gt;");
                }
            }
        } else if (subType instanceof MetaStruct) {
            int size = 0;
            StringWriter localWriter = new StringWriter();
            // Verify if member "size" exists, if so, then serialize and
            // increment
            LinkedHashSet<String> fields = data.getFieldNames();
            boolean found = true;
            while (found) {
                String entryFieldName = nestedFieldName + "[" + size + "]";
                boolean fieldExists = false;
                Iterator<String> it = fields.iterator();
                while (!fieldExists && it.hasNext()) {
                    if (it.next().startsWith(entryFieldName)) {
                        fieldExists = true;
                    }
                }
                if (fieldExists) {
                    localWriter.write("<element>");
                    serializeStruct(localWriter, entryFieldName,
                            (MetaStruct) subType, data);
                    localWriter.write("</element>");
                    size++;
                } else {
                    found = false;
                }
            }
            if (size == 0) {
                writer.write("&lt;NULL&gt;");
            } else {
                writer.write("<size>" + size + "</size>");
            }
            writer.write(localWriter.toString());
        } else if (subType instanceof MetaUnion) {
            int size = 0;
            StringWriter localWriter = new StringWriter();
            // Verify if member "size" exists, if so, then serialize and
            // increment
            LinkedHashSet<String> fields = data.getFieldNames();
            boolean found = true;
            while (found) {
                String entryFieldName = nestedFieldName + "[" + size + "]";
                boolean fieldExists = false;
                Iterator<String> it = fields.iterator();
                while (!fieldExists && it.hasNext()) {
                    if (it.next().startsWith(entryFieldName)) {
                        fieldExists = true;
                    }
                }
                if (fieldExists) {
                    localWriter.write("<element>");
                    this.serializeUnion(localWriter, entryFieldName,
                            (MetaUnion) subType,
                            data);
                    localWriter.write("</element>");
                    size++;
                } else {
                    found = false;
                }
            }
            if (size == 0) {
                writer.write("&lt;NULL&gt;");
            } else {
                writer.write("<size>" + size + "</size>");
            }
            writer.write(localWriter.toString());

        } else {
            writer.write("&lt;NULL&gt;");
        }
        return;
    }

    private void serializeRecursiveType(StringWriter writer,
            String nestedFieldName, MetaCollection seqType, UserData data) {
        // String value = data.getFieldValue(nestedFieldName);
        String typeName = seqType.getTypeName();

        if (!((typeName.startsWith("C_ARRAY")))) {
            writer.write("&lt;NULL&gt;");
        }
        return;
    }

    private String removeIndices(String name) {
        String result;
        StringWriter w;
        char c;
        int in;

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

        return result;
    }

    /**
     * The type of the UserData to serialize.
     */
    private MetaType type;

    /**
     * The logging facilities.
     */
    private final Logger logger;
}
