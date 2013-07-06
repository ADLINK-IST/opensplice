package org.opensplice.cm.transform.xml;

import java.util.logging.Logger;
import java.util.logging.Level;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import java.lang.StringBuilder;
import java.text.CharacterIterator;
import java.text.StringCharacterIterator;

import org.w3c.dom.Document;
import org.w3c.dom.Text;

import org.opensplice.cm.transform.StorageSerializer;
import org.opensplice.cm.transform.TransformationException;

public class StorageSerializerXML implements StorageSerializer {
    private static final String STORAGE_STRING = "rr_storage";
    private static final String TYPENAME_STRING = "rr_storageTypeName";
    private Logger logger;

    public StorageSerializerXML()  {
        logger = Logger.getLogger("org.opensplice.api.cm.transform.xml");
    }

    public String serializeStorage(Object storage) throws TransformationException {
        String xmlStorage = null;

        if(storage instanceof String){
            xmlStorage = String.format("<" + STORAGE_STRING + ">%s</" + STORAGE_STRING + ">", (String)storage);
        } else {
            logger.logp(Level.SEVERE,  "StorageSerializerXML",
                    "serializeStorage",
                    "Storage could not be serialized, opaque type not as expected (expected instanceof String)");
            throw new TransformationException("Storage could not be serialized, opaque type not as expected (expected instanceof String)");
        }

        return xmlStorage;
    }

    public String serializeTypeName(String typeName) throws TransformationException {
        return String.format("<%s>%s</%s>", TYPENAME_STRING, (typeName != null) ? xmlEncodeString(typeName) : "", TYPENAME_STRING);
    }

    private String xmlEncodeString(String str){
        StringBuilder b = new StringBuilder(str.length());
        CharacterIterator strIter = new StringCharacterIterator(str);

        for(char c = strIter.first(); c != CharacterIterator.DONE; c = strIter.next()) {
            switch(c){
            case '<':
                b.append("&lt;");
                break;
            case '>':
                b.append("&gt;");
                break;
            case '&':
                b.append("&amp;");
                break;
            case '"':
                b.append("&quot;");
                break;
            case '\'':
                b.append("&apos;");
                break;
            default:
                b.append(c);
                break;
            }
        }
        return b.toString();
    }
}
