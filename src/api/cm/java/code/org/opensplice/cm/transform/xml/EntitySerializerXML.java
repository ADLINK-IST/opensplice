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

import org.opensplice.cm.*;
import org.opensplice.cm.transform.EntitySerializer;
import org.opensplice.cm.transform.TransformationException;

/**
 * The XML implementation of an EntitySerializer. It is capable of 
 * transforming an Entity object into a serialized XML representation.
 * 
 * @date Sep 17, 2004
 */
public class EntitySerializerXML implements EntitySerializer {
    public String serializeEntity(Entity e) throws TransformationException {
        if(e == null){
            throw new TransformationException("Supplied Entity is not valid.");
        }
        StringWriter writer = new StringWriter();
        writeEntity(writer, e);
        writer.flush();
        return writer.toString();
    }
    
	@Override
	public String serializeEntities(Entity[] entities) throws TransformationException {
	       if(entities == null || entities.length == 0){
	            throw new TransformationException("Supplied Entities are not valid.");
	        }
	        StringWriter writer = new StringWriter();
	        writer.write("<entityList>");
	        for(Entity e : entities){
	        	writeEntity(writer, e);
	        }
	        writer.write("</entityList>");
	        writer.flush();
	        return writer.toString();
	} 
	
    /**
     * Helper method to serialize a supplied Entity into a serialized representation.
     * 
     * @param writer The string buffer used to store the serialized entity 
     * @param e The entity that is serialized to the writer
     */
	private void writeEntity(StringWriter writer, Entity e) throws TransformationException{
    	if(e == null){
            throw new TransformationException("Supplied Entity is not valid.");
        }
        writer.write("<entity><pointer>" + e.getPointer() + 
                        "</pointer><handle_index>" + e.getIndex() + 
                        "</handle_index><handle_serial>" + e.getSerial() +
                        "</handle_serial><name>" + e.getName() + "</name>" +
                        "<enabled>" + Boolean.toString(e.isEnabled()).toUpperCase() + 
                        "</enabled>");

        if(e instanceof Topic){
            writer.write("<keyList>" + ((Topic)e).getKeyList() + "</keyList>");
            writer.write("<kind>TOPIC</kind><typeName>" + ((Topic)e).getTypeName() + "</typeName>");
        } else if(e instanceof ServiceState){
            writer.write("<kind>SERVICESTATE</kind><statename>" + 
                            ((ServiceState)e).getStateName() + 
                            "</statename><state>" + 
                            ServiceStateKind.getString(((ServiceState)e).getServiceStateKind()) +
                            "</state>"
                            );
        } else if(e instanceof Partition){
            writer.write("<kind>DOMAIN</kind>");
        } else if(e instanceof Participant){
            if(e instanceof Service){
                writer.write("<kind>SERVICE</kind>");
            } else{
                writer.write("<kind>PARTICIPANT</kind>");
            }
        } else if(e instanceof Publisher){
            writer.write("<kind>PUBLISHER</kind>");
        } else if(e instanceof Subscriber){
            writer.write("<kind>SUBSCRIBER</kind>");
        } else if(e instanceof Writer){
            writer.write("<kind>WRITER</kind>");
        } else if(e instanceof Reader){
            if(e instanceof DataReader){
                writer.write("<kind>DATAREADER</kind>");
            } else if(e instanceof Queue){
                writer.write("<kind>QUEUE</kind>");
            } else if(e instanceof NetworkReader){
                writer.write("<kind>NETWORKREADER</kind>");
            } else if(e instanceof GroupQueue){
                writer.write("<kind>GROUPQUEUE</kind>");
            } else if(e instanceof Query){
                writer.write("<kind>QUERY</kind>");
                
                if(((Query)e).getExpression() != null){
                    writer.write("<expression><![CDATA[" + ((Query)e).getExpression() + "]]></expression>");
                } else {
                    writer.write("<expression></expression>");
                }
                if(((Query)e).getExpressionParams() != null){
                    writer.write("<params><![CDATA[" + ((Query)e).getExpressionParams() + "]]><params>");
                } else {
                    writer.write("<params></params>");
                }
                
                writer.write("<instanceState>" + ((Query)e).getInstanceState().getValue() + "</instanceState>");
                writer.write("<sampleState>" + ((Query)e).getSampleState().getValue() + "</sampleState>");
                writer.write("<viewState>" + ((Query)e).getViewState().getValue() + "</viewState>");
            }else {
                assert false: "Serialize entity XML: received unknown entity";
            }
        } else if(e instanceof Waitset){
            writer.write("<kind>WAITSET</kind>");
        } else {
            assert false: "Serialize entity XML: received unknown entity";
        }
        writer.write("</entity>");	
	}
}
