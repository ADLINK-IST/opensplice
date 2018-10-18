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
package org.opensplice.dds.core;

/**
 * This type is implemented by all OpenSplice {@link org.omg.dds.core.Entity}
 * classes to allow setting and getting of OpenSplice-specific properties on
 * them. See {@link #setProperty(String, String)} for all properties that are
 * currently supported.
 *
 * @see #setProperty(String, String)
 */
public interface Properties {
    /**
     * This method sets the property specified by the key to the value given by
     * the value. Currently, this method is only supported on a
     * {@link org.omg.dds.sub.DataReader} and that supports the following
     * properties:
     * <p>
     * <b>parallelReadThreadCount</b> - By default, the demarshalling of data
     * into Java objects by a single read or take operation happens only in the
     * calling thread. The parallelReadThreadCount property can be used to
     * control the number of parallel threads to be used for this demarshalling.
     * When reading multiple of samples takes a significant amount of time,
     * increasing the number of threads on a multi-core machine can provide a
     * significant benefit. The value is interpreted as the number of parallel
     * threads to use (i.e., the value is a string representing a natural
     * integer in decimal notation, so for example the string '4' will cause 4
     * threads to be used). The value '0' is allowed and selects the default
     * behavior. If the call was successful, successive read/take operations on
     * that {@link org.omg.dds.sub.DataReader} will use the specified number of
     * threads for the demarshalling step of the respective operations until the
     * value of this property is changed again.
     * <p>
     * <b>CDRCopy</b> - The default demarshalling technique constructs the Java
     * object directly from the shared memory, but for some types, it is more
     * efficient to marshal the value in shared memory into CDR representation,
     * and then demarshal the CDR representation from Java. The value is
     * interpreted as a boolean (i.e., value must be either 'true' or 'false'),
     * with 'false' selecting the default technique and 'true' selecting the
     * alternative, CDR-based technique. The CDR-based technique requires JacORB
     * to be configured as the default ORB in the JVM, and furthermore requires
     * that the JacORB IDL preprocessor has been used to generate a Helper class
     * for the topic type of this {@link org.omg.dds.sub.DataReader} and that
     * this helper class can be found in the class path. The ORB initialization
     * and Helper class lookup is all done at run-time to avoid introducing a
     * compile-time dependency, but this means that attempting to set the
     * CDRCopy property to 'true' can fail in complicated ways because of these
     * dependencies. When set to true, the CDR-based technique will be used for
     * successive read/take operations on the data reader, until the property is
     * set to false again.
     *
     * @param key
     *            The key of the property
     * @param value
     *            The value to assign to the property.
     */
    public void setProperty(String key, String value);

    /**
     * Provides access to the current value for a given property.
     *
     * @param key
     *            The key of the property to obtain the value for.
     * @return The value for the specified property, or null if it has not been
     *         set.
     */
    public String getProperty(String key);
}
