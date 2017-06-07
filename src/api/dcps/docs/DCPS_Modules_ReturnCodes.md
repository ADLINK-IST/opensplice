Return Codes            {#DCPS_Modules_ReturnCodes}
============

At the PIM (**P** latform **I** ndependent **M** odel) level errors on operations are modelled as return codes. Each PSM may map these to either return codes or exceptions.

Return Code             |   Brief
------------------------|--------------------------------
OK                      | Successful return
ERROR                   | Generic, unspecified error
BAD_PARAMETER           | Illegal parameter value
UNSUPPORTED             | Unsupported operation. Can only be returned by operations that are optional
ALREADY_DELETED         | The object target of this operation has already been deleted
OUT_OF_RESOURCES        | Service ran out of resources need to complete the operation
NOT_ENABLED             | Operation Invoked on an Entity that is not yet enabled
IMMUTABLE_POLICY        | Application attempted to modify an immutable QoSPolicy
INCONSISTENT_POLICY     | Application specified a set of policies that are not consistent with each other
PRECONDITION_NOT_MET    | A pre-condition for the operation was not met
TIMEOUT                 | The operation timed out
ILLEGAL_OPERATION       | An operation was invoked on an inappropriate object or at an inappropriate time
NO_DATA                 | Indicates a transient situation where the operation did not return any data but there is no inherent error

Any operation with return type ReturnCode_t may return OK, ERROR, or ILLEGAL_OPERATION. Any operation that takes an input parameter may additionally return BAD_PARAMETER. Any operation on an object created from any of the factories may additionally return ALREADY_DELETED. Any operation that is stated as optional may additionally return UNSUPPORTED. The return codes OK, ERROR, ILLEGAL_OPERATION, ALREADY_DELETED, UNSUPPORTED, and BAD_PARAMETER are the standard return codes and the specification won’t mention them explicitly for each operation. Operations that may return any of the additional (non-standard) error codes above will state so explicitly.

It is an error for an application to use an Entity that has already been deleted by means of the corresponding delete operation on the factory. If an application does this, the result is unspecified and will depend on the implementation and the PSM. In the cases where the implementation can detect the use of a deleted entity, the operation should fail and return ALREADY_DELETED.
