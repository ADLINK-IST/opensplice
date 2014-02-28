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
 * Represents a union field in a Splice database type.
 * 
 * @date Jun 10, 2004
 */
public class MetaUnion extends MetaField{
    /**
     * Constructs a new union field.
     * 
     * @param name
     *            The name of the union.
     * @param typeName
     *            The type name of the union.
     * @param _discriminator
     *            The discriminator of the union.
     * @param _cases
     *            The cases within the union.
     */
    public MetaUnion(String name, String typeName, MetaField _discriminator, ArrayList<MetaUnionCase> _cases) {
        super(name, typeName);
        discriminator = _discriminator;
        cases = _cases;
    }

    /**
     * Provides access to the discriminator of the union.
     * 
     * @return The discriminator field of the union.
     */
    public MetaField getDiscriminator(){
        return discriminator;
    }

    /**
     * Provides access to all cases within the union.
     * 
     * @return The array of cases within the union.
     */
    public MetaUnionCase[] getCases(){
        MetaUnionCase[] result = (cases.toArray(new MetaUnionCase[1]));

        return result;
    }

    /**
     * Provides access to the union case associated with the supplied
     * discriminator value.
     * 
     * @param discriminatorValue
     *            The value of the discriminator.
     * @return The union case that is associated with the supplied discriminator
     *         value.
     */

    public MetaUnionCase getCase(String discriminatorValue){
        MetaUnionCase labelCase;
        String realValue = null;

        if(discriminatorValue == null){
            realValue = "0";
        }
        else if(discriminator instanceof MetaEnum){
            /*
             * String[] posValues = ((MetaEnum)discriminator).getPosValues();
             * String result; for(int i=0; i<posValues.length; i++){ result =
             * posValues[i]; if(result.equals(discriminatorValue)){ realValue =
             * Integer.toString(i); } }
             */
            realValue = discriminatorValue;
        }
        else if(discriminator instanceof MetaPrimitive){
            realValue = discriminatorValue;
        }
        else{
            realValue = "0";
        }

        for(int i=0; i<cases.size(); i++){
            labelCase = (cases.get(i));

            if(labelCase.containsLabel(realValue)){
                return labelCase;
            }
        }
        return (cases.get(cases.size() - 1));
    }

    @Override
    public MetaField[] getFields(){
        MetaField[] result = new MetaField[cases.size() + 1];

        result[0] = discriminator;

        for(int i=0; i<cases.size(); i++){
            result[i+1] = (cases.get(i)).getField();
        }
        return result;
    }

    @Override
    public String toString(){
        StringBuffer buf = new StringBuffer();
        buf.append(super.toString());
        buf.append("\ndiscriminator: " + discriminator.toString());
        for (int i = 0; i < cases.size(); i++) {
            buf.append("\ncase: " + cases.get(i).toString());
        }
        return buf.toString();
    }

    @Override
    public MetaField getField(String fieldName){
        MetaUnionCase result;

        for(int i=0; i<cases.size(); i++){
            result = (cases.get(i));

            if(fieldName.equals(result.getName())){
                return result.getField();
            }
        }
        if("switch".equals(fieldName)){
            return discriminator;
        }
        return null;
    }

    @Override
    public ArrayList<String> getFieldNames() {
        MetaField field;
        ArrayList<String> result = new ArrayList<String>();
        ArrayList<String> names;

        result.add(name + ".switch");

        for(int i=0; i<cases.size(); i++){
            field = (cases.get(i));

            names = field.getFieldNames();

            for(int j=0; j<names.size(); j++){
                result.add(name + "." + (names.get(j)));
            }
        }
        return result;
    }

    public boolean labelExists(String label){
        boolean result;
        MetaUnionCase labelCase;

        result = false;

        if(label != null){
            for(int i=0; (i<cases.size()) && (!result); i++){
                labelCase = (cases.get(i));

                if(labelCase.containsLabel(label)){
                    result = true;
                } else if(labelCase.containsLabel("")){ /*default label*/
                    result = true;
                }
            }
        }
        return result;
    }

    /**
     * The discriminator of the union.
     */
    private final MetaField discriminator;

    /**
     * The cases of the union.(<MetaUnionCase>)
     * 
     */
    private final ArrayList<MetaUnionCase> cases;
}
