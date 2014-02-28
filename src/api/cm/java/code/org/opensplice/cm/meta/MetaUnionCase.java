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

import java.util.ArrayList;

/**
 * Represents a union case within a union of a Splice database type.
 * 
 * @date Jun 14, 2004
 */
public class MetaUnionCase extends MetaField{
    /**
     * Constructs a new union case.
     * 
     * @param name
     *            The name of the case field.
     * @param typeName
     *            The type name of the case field.
     * @param _caseField
     *            The case field.
     * @param _labels
     *            List of all labels, that are associated with the case field.
     */
    public MetaUnionCase(String name, String typeName, MetaField _caseField, ArrayList<String> _labels) {
        super(name, typeName);
        caseField = _caseField;
        labels = _labels;
    }

    /**
     * Provides access to the case field.
     * 
     * @return The case field of this case.
     */
    public MetaField getField(){
        return caseField;
    }

    @Override
    public MetaField getField(String fieldName){
        if(fieldName.equals(caseField.getName())){
            return caseField;
        }
        return null;
    }

    @Override
    public MetaField[] getFields(){
        MetaField[] result = {caseField};

        return result;
    }

    @Override
    public String toString(){
        StringBuffer buf = new StringBuffer();
        buf.append(super.toString());
        buf.append(caseField.toString());
        for (int i = 0; i < labels.size(); i++) {
            buf.append("\nLabel: " + labels.get(i));
        }
        return buf.toString();
    }

    @Override
    public ArrayList<String> getFieldNames() {
        ArrayList<String> result = new ArrayList<String>();
        ArrayList<String> names;

        names = caseField.getFieldNames();

        for(int j=0; j<names.size(); j++){
            result.add(names.get(j));
        }
        return result;
    }

    /**
     * Provides access to all labels associated with the case field.
     * 
     * @return The list of case labels.
     */
    public ArrayList<String> getLabels() {
        return labels;
    }

    /**
     * Validates whether the supplied label is matches this case.
     * 
     * @param label
     *            The label of the case, which must be matched.
     * @return true if it matches, false otherwise.
     */
    public boolean containsLabel(String label){
        String curLabel;

        for(int i=0; i<labels.size(); i++){
            curLabel = (labels.get(i));

            if(curLabel.equals(label)){
                return true;
            }
        }
        return false;
    }

    /**
     * The case field.
     */
    private final MetaField caseField;

    /**
     * List of all labels, that are associated with the case field.
     */
    private final ArrayList<String> labels;
}
