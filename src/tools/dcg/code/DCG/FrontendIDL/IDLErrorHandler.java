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
package DCG.FrontendIDL;

public interface IDLErrorHandler
{

    public void error(
        String fileName,
        int line,
        String message);

    public void warning(
        String fileName,
        int line,
        String message);

    /* Invoked when the maximum number of errors is exceeded, after which
     * compilation of the IDL file is stopped.
     */
    public void maxErrorsExceeded(
        String fileName,
        int maxErrors);

}
