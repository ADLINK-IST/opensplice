/**@package org.opensplice.dds.dcps.jni
 * @brief Provides facilities to plug in a communication mechanism into
 * the DCPS API, to be able to communicate in multiple ways without changing
 * the implementation of the API.
 * 
 * It allows:
 * - Plugging in a self implemented communication protocol.
 * - Handles API calls to communication interface.
 */
package org.opensplice.dds.dcps.jni;

/**@brief An exception that occurs when the DCPS API cannot communicate
 * with other Splice2v3 components. 
 */
