package org.opensplice.dds.dcps;


/**@class QueryCondition
 * @date Jun 4, 2004
 * @brief The DCPS QueryCondition object.
 * 
 * QueryCondition objects are specialized ReadCondition objects that allow
 * the application to also specify a fielter on the locally available data.
 */
public class QueryCondition extends Entity{
    /**The query expression that is associated with the QueryCondition.*/
    private String query_expression;
    /**The query arguments in the expression.*/
    private String[] query_arguments;
    
    /**@brief Creates a new QueryCondition.
     * 
     * @param _query_expression The 'WHERE' clause of an OQL query expression.
     * @param _query_arguments The arguments for the supplied expression.
     */
    QueryCondition(String _query_expression, String[] _query_arguments){
        query_expression = _query_expression;
        query_arguments = _query_arguments;
    }
    
    /**@brief Gives access to the query arguments.
     * 
     * @return The array of arguments.
     */
    public String[] get_query_arguments(){
        return query_arguments;
    }
    
    /**@brief Changes the query arguments.
     * 
     * Currently not implemented.
     * 
     * @param args The query arguments.
     * @return ReturnCode.OK if succeeded, any other ReturnCode otherwise.
     */
    public ReturnCode set_query_arguments(String[] args){
        query_arguments = args;
        return ReturnCode.OK;
    }
    
    /**@brief Gives access to the query expression.
     * 
     * @return The query expression.
     */
    public String get_query_expression(){
        return query_expression;
    }
}

