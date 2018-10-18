package chatroom;

import java.util.ArrayList;
import java.util.List;

import org.omg.dds.core.event.DataAvailableEvent;
import org.omg.dds.pub.DataWriter;
import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.DataReaderAdapter;
import org.omg.dds.sub.Sample;

import Chat.ChatMessage;
import Chat.NameService;
import Chat.NamedMessage;

public class DataReaderListenerClass extends DataReaderAdapter<ChatMessage>
{
    DataReader<ChatMessage> chatMessageDataReader = null;
    DataReader<NameService> nameServiceDataReader = null;
    DataWriter<NamedMessage> namedMessageDataWriter = null;

    public DataReaderListenerClass(DataReader<ChatMessage> chatMessageDataReader,
            DataReader<NameService> nameServiceDataReader, DataWriter<NamedMessage> namedMessageDataWriter) {
        this.chatMessageDataReader = chatMessageDataReader;
        this.nameServiceDataReader = nameServiceDataReader;
        this.namedMessageDataWriter = namedMessageDataWriter;
    }

    @Override
    public void onDataAvailable(DataAvailableEvent<ChatMessage> status) {

        try {
            List<Sample<ChatMessage>> cmSamples = new ArrayList<Sample<ChatMessage>>();
            List<Sample<NameService>> nsSamples = new ArrayList<Sample<NameService>>();

            /* We want to take all samples (ANY, ANY, ANY) so no need to
             * use a DataState filter. */
            chatMessageDataReader.take(cmSamples);
            for (Sample<ChatMessage> sample : cmSamples) {
                ChatMessage message = sample.getData();
                if (message != null) {
                    NamedMessage nm = new NamedMessage();
                    nm.content = message.content;
                    nm.index = message.index;
                    nm.userID = message.userID;

                    /* Get the name for the given userID by reading from the
                     * NameService topic with the given ID. */
                    String expr = "userID = %0";
                    List<String> params = new ArrayList<String>();
                    params.add(new Integer(message.userID).toString());
                    /* We want to read all samples (ANY, ANY, ANY) so no need to
                     * use a DataState filter. */
                    nameServiceDataReader.read(nsSamples);
                    for (Sample<NameService> s : nsSamples) {
                        NameService ns = s.getData();
                        if (ns != null) {
                            nm.userName = ns.name;
                        }
                    }
                    namedMessageDataWriter.write(nm);
                }
            }
        } catch(Exception e) {
            e.printStackTrace();
        }
    }
}


