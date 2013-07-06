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
package DCG.DCGUtilities;

import java.util.*;
import java.io.*;

public class StreamHandlerUtil extends Thread
{
    private InputStream inputStream;
    private String type;
    private Exception e = null;
    private boolean terminate = false;

    public StreamHandlerUtil(
        InputStream inputStream,
        String type)
    {
        this.inputStream = inputStream;
        this.type = type;
    }

    public Exception getException(
        )
    {
        return e;
    }
    
    public void finished() {
    	this.terminate = true;
    }

    public void run(
        )
    {
        InputStreamReader inputReader;
        BufferedReader bufferedReader = null;
        String line = null;

        try
        {
            inputReader = new InputStreamReader(inputStream);
            bufferedReader = new BufferedReader(inputReader);

            while(!this.terminate)
            {
                line = bufferedReader.readLine();
                if(line != null) {
	                if(type != null)
	                {
	                    System.out.println(type+": "+line);
	                } else {
	                    System.out.println(line);
	                }
                }
                Thread.sleep(50);
            }
            bufferedReader.close();
        } catch (Exception e)
        {
            this.e = e;
        }
    }
}
