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
package DDS;

/**
 * <P>This class represents the DLRL Object Identifier, which is the unique
 * identifier for a DLRL Object. Three <code>long</code> attributes are used to form
 * the unique identification.</P>
 */
public final class DLRLOid implements Cloneable{

    /* An integer array with length 3. The integers
     * together create a unique identifier
     * @deprecated
     */
	public int value[] = new int[3];//NOT IN DESIGN

    public int systemId;
    public int localId;
    public int serial;

    /* The default constructor */
	public DLRLOid (){}

    /* Create a DLRLOid with an integer value array as provided
     * as parameter. If the value array is not of length 3 then
     * the DLRLOid will be created, but will cause PreconditionNotMet
     * exceptions when used for DLRL specific operations.
     * @deprecated
     */
    public DLRLOid (int value[]){
        this.value = value;
    }

    /* Create a DLRLOid with three integers provided
     * as parameter. The (0,0,0) variant is reserved as NIL pointer.
     */
    public DLRLOid (int systemId, int localId, int serial){
        this.systemId = systemId;
        this.localId = localId;
        this.serial = serial;
    }

    public Object clone(){
        try{
            DDS.DLRLOid object = (DDS.DLRLOid)super.clone();
            if(this.value != null){
                object.value = (int[])this.value.clone();
                for(int count = 0; count < this.value.length; count++){
                    int var = this.value[count];
                    object.value[count] = var;
                }
            }
            return object;
        } catch(java.lang.CloneNotSupportedException e){
            return null;
        }
    }

} // class DLRLOid
