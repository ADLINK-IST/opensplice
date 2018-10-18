/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package org.opensplice.cm.transform.xml;

import java.util.ArrayList;
import java.util.HashMap;
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
import org.xml.sax.Locator;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

/**
 * Offers facilities to deserialize a Sample. It is being triggered by the SAX
 * parser.
 *
 * @date May 13, 2004
 */
abstract class CommonHandler extends DefaultHandler {
    /**
     * Constructs a new CommonHandler that is able to deserialize XML to a Java
     * representation.
     *
     * @param type
     *            The type of the userdata.
     */
    public CommonHandler(MetaType type) {
        userDataType = type;
        logger = Logger.getLogger("org.opensplice.api.cm.transform.xml");
    }

    /**
     * Adds a field to the current scope.
     *
     * @param field
     *            The field to add to the scope.
     */
    protected abstract void scopeAdd(String field);

    /**
     * Removes the last field from the scope.
     */
    abstract void scopeRemoveLast();

    /**
     * Provides access to the current scope.
     *
     * @return The current scope. Fieldnames are '.' separated.
     */
    protected abstract String scopeGet();

    /**
     * Provides the current index to address the supplied collection.
     *
     * @verbatim Example:
     *
     *           IDL: long arr[2][3];
     *
     *           Result: The first time "[0][0]" will be returned, the second
     *           time "[0][1]" and so on.
     * @endverbatim
     *
     * @param fieldName
     *            The name of the collection.
     * @param depth
     *            The depth of the index of the collection to look up. This is
     *            used for collections within collections.
     * @return A String representation of the current indices of the collection.
     */
    
    protected String getIndexFieldElement(String fieldName, int depth) {
        ArrayList<String> indices = (fieldElementIndices.get(fieldName));
        String result = "";
        MetaCollection colType = (MetaCollection) (userDataType
                .getField(fieldName));

        if (indices == null) {
            ArrayList<String> newList = new ArrayList<String>(depth);

            StringBuffer buf = new StringBuffer();
            for (int i = 0; i < depth; i++) {
                newList.add("0");
                buf.append("[0]");
            }
            result = buf.toString();
            fieldElementIndices.put(fieldName, newList);
        } else if (colType != null){
            int index, subDepth;
            String strIndex;
            boolean prevAdd = true;
            boolean isAdded = false; // true if one index has already been added
                                     // with 1
            StringBuffer buf = new StringBuffer();
            for (int i = depth - 1; i >= 0; i--) {
                index = Integer.parseInt((indices.get(i)));
                subDepth = colType.getRecursiveMaxSize(i);

                if (index == (subDepth - 1) && (!isAdded)) {
                    strIndex = Integer.toString(0);
                    prevAdd = true;
                } else if (prevAdd && (!isAdded)) {
                    strIndex = Integer.toString(index + 1);
                    isAdded = true;
                    prevAdd = false;
                } else {
                    strIndex = Integer.toString(index);
                }
                indices.set(i, strIndex);
                buf.append("[" + strIndex + "]");
            }
            result = buf.toString();
        }
        return result;
    }

    
    protected void fillUserData(UserData data, ArrayList<FlatElement> flatElements) {
        MetaField[] fields = data.getUserDataType().getFields();

        for (MetaField field : fields) {
            this.fillType(field.getName(), field.getName(), field, data, flatElements);
        }
    }

    protected void fillType(String nestedFieldName,
            String nestedElementFieldName,
            MetaField field, UserData data, ArrayList<FlatElement> flatElements) {
        if (field instanceof MetaPrimitive) { // Primitive
            this.fillPrimitive(nestedFieldName, nestedElementFieldName, data,
                    flatElements);
        } else if (field instanceof MetaEnum) { // Enumeration
            this.fillEnum(nestedFieldName, nestedElementFieldName, data,
                    flatElements);
        } else if (field instanceof MetaUnion) { // Union
            this.fillUnion(nestedFieldName, nestedElementFieldName,
                    (MetaUnion) field, data, flatElements);
        } else if (field instanceof MetaStruct) { // Structure
            this.fillStruct(nestedFieldName, nestedElementFieldName,
                    (MetaStruct) field, data, flatElements);
        } else if (field instanceof MetaClass) { // Class
            this.fillClass(nestedFieldName, nestedElementFieldName,
                    (MetaClass) field, data, flatElements);
        } else if (field instanceof MetaCollection) { // Collection
            this.fillCollection(nestedFieldName, nestedElementFieldName,
                    (MetaCollection) field, data, flatElements);
        }
    }

    protected void fillCollection(String nestedFieldName,
            String nestedElementName, MetaCollection field,
            UserData data, ArrayList<FlatElement> elements) {
        int size;
        String typeName = field.getTypeName();
        FlatElement element = null;
        if ((typeName.equals("c_string")) ||
            (typeName.equals("c_wstring")) ||
            (typeName.startsWith("C_STRING<")) ||
            (typeName.startsWith("C_WSTRING<")))
        {
            element = this.getFirstMatchingElement(elements, nestedElementName);

            if (element != null) {
                data.setData(nestedFieldName, element.flatValue);
            }
        } else { // Not a String collection.
            size = field.getMaxSize();

            element = getFirstMatchingElement(elements, nestedElementName, "size");
            if (element != null) {
                size = Integer.parseInt(element.flatValue);
            }

            if (size != 0) {
                for (int i = 0; i < size; i++) {
                    this.fillType(
                            getCollectionFieldName(nestedFieldName, i),
                            nestedElementName + ".element",
                            field.getSubType(), data, elements);
                }
            }

            /* Collection must remove it's own terminating element */
            element = getFirstMatchingElement(elements, nestedElementName);
        }
    }

    protected String getCollectionFieldName(String name, int colIndex) {
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

    protected void fillClass(String nestedFieldName, String nestedElementName,
            MetaClass field, UserData data, ArrayList<FlatElement> elements) {
        MetaField[] fields = field.getFields();

        for (int i = 0; i < fields.length; i++) {
            this.fillType(nestedFieldName + "." + fields[i].getName(),
                    nestedElementName + "." + fields[i].getName(),
                    fields[i], data, elements);
        }
    }

    protected void fillStruct(String nestedFieldName, String nestedElementName,
            MetaStruct field, UserData data, ArrayList<FlatElement> elements) {
        MetaField[] fields = field.getFields();

        for (int i = 0; i < fields.length; i++) {
            this.fillType(nestedFieldName + "." + fields[i].getName(),
                    nestedElementName + "." + fields[i].getName(),
                    fields[i], data, elements);
        }
    }

    protected void fillPrimitive(String nestedFieldName,
            String nestedElementFieldName, UserData data, ArrayList<FlatElement> elements) {
        FlatElement element;
        element = getFirstMatchingElement(elements, nestedElementFieldName);

        if (element != null) {
            data.setData(nestedFieldName, element.flatValue);
        } else {
            logger.logp(Level.SEVERE, "CommonHandler", "fillPrimitive",
                    "Could not find primitive: " + nestedFieldName);
        }
    }

    protected void fillEnum(String nestedFieldName,
            String nestedElementFieldName,
            UserData data, ArrayList<FlatElement> elements) {
        FlatElement element;
        element = getFirstMatchingElement(elements, nestedElementFieldName);

        if (element != null) {
            data.setData(nestedFieldName, element.flatValue);
        } else {
            logger.logp(Level.SEVERE, "CommonHandler", "fillEnum",
                    "Could not find enum: " + nestedFieldName);
        }
    }

    protected void fillUnion(String nestedFieldName,
            String nestedElementFieldName, MetaUnion field, UserData data,
            ArrayList<FlatElement> elements) {
        FlatElement element;
        element = getFirstMatchingElement(
            elements, nestedElementFieldName + ".switch");

        if (element != null) {
            data.setData(nestedFieldName + ".switch", element.flatValue);
            MetaUnionCase c = field.getCase(element.flatValue);
            this.fillType(nestedFieldName + "." + c.getField().getName(),
                    nestedElementFieldName + "." + c.getField().getName(),
                    c.getField(), data, elements);

        } else {
            logger.logp(Level.SEVERE, "CommonHandler", "fillUnion",
                    "Could not find union case: " + nestedFieldName + ".switch");
        }
    }

    private static final int FIND = -3;
    private static final int LOOP = -2;
    private static final int BAIL = -1;

    protected FlatElement getFirstMatchingElement(
        ArrayList<FlatElement> elements,
        String path)
    {
        return getFirstMatchingElement(elements, path, null);
    }

    protected FlatElement getFirstMatchingElement(
        ArrayList<FlatElement> elements,
        String base,
        String name)
    {
        FlatElement element;
        int found = FIND;
        int index = 0;
        int length = elements.size();
        int range = 0;
        String path;

        if (name != null) {
            path = base + "." + name;
        } else {
            path = base;
        }

        for (index = 0; index < length && found < BAIL; index++) {
            element = elements.get(index);
            if (element.flatName.equals(path)) {
                range = index;
                found = index;
            } else if (element.flatName.startsWith(base)) {
                /* Finding the base element first (base equals flatName) or
                   (flatName startsWith path) could be considered a grave bug */
                if (found == FIND) {
                    range = index;
                    found = LOOP;
                }
            } else {
                if (found == LOOP) {
                    found = BAIL;
                }
            }
        }

        if (found < 0) {
            element = null;
        } else {
            element = elements.remove(found);
        }

        for (index = 0; index < range; index++) {
            elements.remove(0);
        }

        return element;
    }

    /**
     * Triggered by the parser when data has been found. Data is added to the
     * data buffer.
     *
     * @see org.xml.sax.ContentHandler#characters(char[], int, int)
     */
    @Override
    public void characters(char[] ch, int start, int length)
            throws SAXException {
        String s = new String(ch, start, length);

        if (dataBuffer == null) {
            dataBuffer = s;
        } else {
            dataBuffer += s;
        }
    }

    /**
     * Not implemented, because it is not being used.
     *
     * @see org.xml.sax.ContentHandler#ignorableWhitespace(char[], int, int)
     */
    @Override
    public void ignorableWhitespace(char[] ch, int start, int length)
            throws SAXException {
    }

    /**
     * Not implemented, because it is not being used.
     *
     * @see org.xml.sax.ContentHandler#skippedEntity(java.lang.String)
     */
    @Override
    public void skippedEntity(String name) throws SAXException {
    }

    /**
     * Not implemented, because it is not being used.
     *
     * @see org.xml.sax.ContentHandler#setDocumentLocator(org.xml.sax.Locator)
     */
    @Override
    public void setDocumentLocator(Locator locator) {
    }

    /**
     * Not implemented, because it is not being used.
     *
     * @see org.xml.sax.ContentHandler#processingInstruction(java.lang.String,
     *      java.lang.String)
     */
    @Override
    public void processingInstruction(String target, String data)
            throws SAXException {
    }

    /**
     * Not implemented, because it is not being used.
     *
     * @see org.xml.sax.ContentHandler#startPrefixMapping(java.lang.String,
     *      java.lang.String)
     */
    @Override
    public void startPrefixMapping(String prefix, String uri)
            throws SAXException {
    }

    /**
     * Not implemented, because it is not being used.
     *
     * @see org.xml.sax.ContentHandler#endPrefixMapping(java.lang.String)
     */
    @Override
    public void endPrefixMapping(String prefix) throws SAXException {
    }

    static class FlatElement {
        public String flatName;
        public String flatValue;

        public FlatElement(String flatName, String flatValue) {
            this.flatName = flatName;
            this.flatValue = flatValue;
        }
    }


    protected ArrayList<FlatElement>             flatData;

    /**
     * The type of the userdata in the message.
     */
    protected final MetaType                     userDataType;

    /**
     * Keeps track of a collection index while processing it. (<String
     * fieldName, ArrayList indices>).
     *
     * This is done to address the right index of a collection.
     */
    protected HashMap<String, ArrayList<String>> fieldElementIndices;

    /**
     * Logging facilities for the handler.
     */
    protected final Logger                       logger;

    protected String                           dataBuffer;
}
