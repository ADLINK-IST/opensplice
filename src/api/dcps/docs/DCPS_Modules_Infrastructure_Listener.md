Listener            {#DCPS_Modules_Infrastructure_Listener}
=========

![Class model of supported DCPS Listeners] (@ref InfrastructureModule_Listener_UML.png)

Listeners provide a mechanism for the middleware to asynchronously alert the application of the occurrence of relevant \ref DCPS_Modules_Infrastructure_Status "status changes".

All Entities support a listener, which type of which is specialised to the specific type of the related Entity (e.g., DataReaderListener for the DataReader).
Listeners are interfaces that the application must implement. Each dedicated listener presents a list of operations that correspond to the relevant communication status changes (i.e., that the
application may react to).

By default, status callbacks are handled by the most dedicated Listener (i.e. the Listener closest to the source of the status change).

- If a status change is not handled by an Entity, it is propagated to its parent.
- If an Entity does handle the status change, it is not propagated to its parent.

A Listener-callback resets its status, as it implicitly accesses the status when it is passed as a parameter to the corresponding callback operation:

- The corresponding status flag will be reset to FALSE.
- The corresponding ‘change’ counters will be reset to 0.

![Conceptional Listener DataReader hook] (@ref Listeners.png)
