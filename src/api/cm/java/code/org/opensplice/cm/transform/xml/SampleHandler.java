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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.logging.Level;

import org.opensplice.cm.data.GID;
import org.opensplice.cm.data.Message;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.State;
import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.qos.MessageQoS;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Offers facilities to deserialize a Sample. It is being triggered by the SAX
 * parser.
 *
 * @date May 13, 2004
 */
public class SampleHandler extends CommonHandler {
    /**
     * Constructs a new SampleHandler that is able to deserialize a Sample from
     * XML to a Java representation.
     *
     * @param type
     *            The type of the userdata in the Sample.
     */
    public SampleHandler(MetaType type) {
        super(type);
        sample = null;
        message = null;
        userData = null;
    }

    /**
     * Adds a field to the current scope.
     *
     * @param field
     *            The field to add to the scope.
     */
    @Override
    protected void scopeAdd(String field) {
        fieldScope.add(field);
    }

    /**
     * Removes the last field from the scope.
     */
    @Override
    protected void scopeRemoveLast() {
        fieldScope.removeLast();
    }

    /**
     * Provides access to the current scope.
     *
     * @return The current scope. Fieldnames are '.' separated.
     */
    @Override
    protected String scopeGet() {
        boolean first = true;
        ListIterator<String> li = fieldScope.listIterator();
        StringBuffer buf = new StringBuffer();
        while (li.hasNext()) {
            if (first) {
                buf.append(li.next());
                first = false;
            } else {
                buf.append("." + li.next());
            }
        }
        return buf.toString();
    }

    /**
     * Triggered by the parser to notify that the end of the XML input has been
     * reached.
     *
     * @see org.xml.sax.ContentHandler#endDocument()
     */
    @Override
    public void endDocument() throws SAXException {
        Sample s = sample;
        UserData ud;

        for (int i = 0; i < history.size(); i++) {
            s.setPrevious(history.get(i));
            s = s.getPrevious();
            ud = s.getMessage().getUserData();
            fillUserData(ud, flatDataHistory.get(i));

        }
        ud = s.getMessage().getUserData();
        fillUserData(ud, flatData);
    }

    /**
     * Triggered by the parser to notify that the parsing has been started. All
     * data will be initialized.
     *
     * @see org.xml.sax.ContentHandler#startDocument()
     */
    @Override
    public void startDocument() throws SAXException {
        MessageQoS msgQos = null;
        userData = new UserData(userDataType);
        writerGid = new GID(-1, -1);
        instanceGid = new GID(-1, -1);
        State state = new State(State.WRITE);
        message = new Message(state, -1, -1, writerGid, instanceGid, 0, msgQos,
                userData);
        sample = new Sample(-1, -1, new State(0), -1, -1, message);
        fieldScope = new LinkedList<String>();
        fieldElementIndices = new HashMap<String, ArrayList<String>>();
        flatData = new ArrayList<FlatElement>();
        flatDataHistory = new ArrayList<ArrayList<FlatElement>>();
        history = new ArrayList<Sample>();
        dataBuffer = null;
    }

    /**
     * Triggered by the parser to notify a start tag. The name of the tag will
     * be added to the current scope.
     *
     * @see org.xml.sax.ContentHandler#startElement(java.lang.String,
     *      java.lang.String, java.lang.String, org.xml.sax.Attributes)
     */
    @Override
    public void startElement(String namespaceURI, String localName,
            String qName, Attributes atts) throws SAXException {
        this.scopeAdd(qName);
    }

    /**
     * Triggered by the parser to notify an end tag. The name of the tag is
     * being removed from the current scope. Data that is currently in the
     * buffer, is assigned now. After that, the data buffer is cleared.
     *
     * The following data is supported:
     * - object.insertTime.seconds -> insert time seconds
     * - object.insertTime.nanoseconds -> insert time nanoseconds
     * - object.sampleState -> sample state
     * - object.previous.* -> history
     * - object.message.nodeState -> node state
     * - object.message.writeTime.seconds -> write time seconds
     * - object.message.writeTime.nanoseconds -> write time nanoseconds
     * - object.message.userData.* -> all userdata fields.
     * - object.message.writerGID.* -> writer GID.
     * - object.message.instanceGID.*-> instance GID.
     * - object.message.sampleSequenceNumber -> message sequence number.
     *
     * @todo TODO: Implement QoS support.
     *
     * @see org.xml.sax.ContentHandler#endElement(java.lang.String,
     *      java.lang.String, java.lang.String)
     */
    @Override
    public void endElement(String namespaceURI, String localName, String qName)
            throws SAXException {
        String s = dataBuffer;
        String scope = this.scopeGet();
        int historyDepth = -1;
        ArrayList<FlatElement> flatDH = flatData;
        Sample sam = sample;
        Message mes = message;

        if (s == null) {
            s = "";
        }

        if (s != null) {
            if ((scope.startsWith("object.previous.")) && (!("NULL".equals(s)))) {
                historyDepth++;
                String temp = scope.substring(16);

                while (temp.startsWith("previous.")) {
                    historyDepth++;
                    temp = temp.substring(9);
                }

                while (history.size() < (historyDepth + 1)) {
                    UserData hUserData = new UserData(userDataType);
                    GID wGid = new GID(-1, -1);
                    GID iGid = new GID(-1, -1);
                    MessageQoS msgQos = null;
                    State state = new State(State.WRITE);
                    Message hMessage = new Message(state, -1, -1, wGid, iGid,
                            0, msgQos, hUserData);
                    Sample hSample = new Sample(-1, -1, new State(0), -1, -1,
                            hMessage);
                    history.add(hSample);
                    ArrayList<FlatElement> hFlatData = new ArrayList<FlatElement>();
                    flatDataHistory.add(hFlatData);
                }
                sam = (history.get(historyDepth));
                mes = sam.getMessage();
                flatDH = flatDataHistory.get(historyDepth);
                scope = "object." + temp;
            }

            if (scope.startsWith("object.message.userData.")) {
                String fieldName = scope.substring(24);
                FlatElement fe = new FlatElement(fieldName, s);
                flatDH.add(fe);
            } else if (scope.equals("object.insertTime.seconds")) {
                sam.setInsertTimeSec(Long.parseLong(s));
            } else if (scope.equals("object.insertTime.nanoseconds")) {
                sam.setInsertTimeNanoSec(Long.parseLong(s));
            } else if (scope.equals("object.sampleState")) {
                sam.setState(new State(Integer.parseInt(s)));
            } else if (scope.equals("object.disposeCount")) {
                sam.setDisposeCount(Long.parseLong(s));
            } else if (scope.equals("object.noWritersCount")) {
                sam.setNoWritersCount(Long.parseLong(s));
            } else if (scope.equals("object.message.nodeState")) {
                mes.setNodeState(new State(Integer.parseInt(s)));
            } else if (scope.equals("object.message.writeTime.seconds")) {
                mes.setWriteTimeSec(Long.parseLong(s));
            } else if (scope.equals("object.message.writeTime.nanoseconds")) {
                mes.setWriteTimeNanoSec(Long.parseLong(s));
            } else if (scope.equals("object.message.writerGID.localId")) {
                mes.getWriterGid().setLocalId(Long.parseLong(s));
            } else if (scope.equals("object.message.writerGID.systemId")) {
                mes.getWriterGid().setSystemId(Long.parseLong(s));
            } else if (scope.equals("object.message.writerInstanceGID.localId")) {
                mes.getInstanceGid().setLocalId(Long.parseLong(s));
            } else if (scope.equals("object.message.writerInstanceGID.systemId")) {
                mes.getInstanceGid().setSystemId(Long.parseLong(s));
            } else if (scope.equals("object.message.sampleSequenceNumber")) {
                mes.setSampleSequenceNumber(Long.parseLong(s));
            } else if (scope.equals("object.message.qos.size")) {
                mes.setQos(new MessageQoS(new int[Integer.parseInt(s)]));
            } else if (scope.equals("object.message.qos.element")) {
                mes.getQos().addElement(Integer.parseInt(s));
            } else {
                logger.logp(Level.FINE, "SampleHandler", "endElement",
                        "UNKNOWN scope: " + scope + " = " + s);
            }
            dataBuffer = null;
        }
        this.scopeRemoveLast();

        if ("previous".equals(qName)) {
            if (this.scopeGet().indexOf("userData") == -1) {
                fieldElementIndices = new HashMap<String, ArrayList<String>>();
            }
        }
    }


    /**
     * Provides access to the parsed Sample.
     *
     * @return The parsed Sample.
     */
    public Sample getSample() {
        return sample;
    }

    private ArrayList<ArrayList<FlatElement>> flatDataHistory; /* <ArrayList<FlatElement>> */

    private ArrayList<Sample> history;

    /**
     * The deserialized sample.
     */
    private Sample sample;

    /**
     * The deserialized message in the sample.
     */
    private Message message;

    /**
     * The deserialized userdata in the message.
     */
    private UserData userData;

    /**
     * Used to keep track of the current scope. Nested fields are added to the
     * scope until handled.
     */
    private LinkedList<String> fieldScope;


    private GID writerGid;

    private GID instanceGid;

}
