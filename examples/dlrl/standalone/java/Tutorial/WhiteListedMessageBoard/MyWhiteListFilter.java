package WhiteListedMessageBoard;

/**
 * The DLRL offers multiple ways to filter out objects, by means of content
 * filtering and by means of dynamic filtering. With content filtering the
 * objects that do not match the filter are never created and not managed at
 * all, with dynamic filtering the objects are created and are managed, but
 * subsets of the objects can be filtered out.
 * The latter approach is used within this example. The DLRL offers dynamic
 * filter by means of so called Selection objects which filter out objects that
 * do not match the SelectionCriterion criteria. A SelectionCriterion can be
 * an SQL query or an application defined filter object contains some
 * application specific logic to determine which objects pass the filter and
 * which do not. The example uses this latter approach for filtering out data.
 * This class represents the filter object used within the selection object to
 * filter out objects. Each time an object is created, modified or deleted the
 * filter 'check_object' operation is invoke and the object is evaluated against
 * the logic contained within that operation.
 */
public class MyWhiteListFilter extends DLRLChat.WhiteListFilter
{
    private String nameFilter;

    /**
     * This constructor creates a filter that filters WhiteList objects
     * based upon matching or not matching the specified WhiteList name.
     *
     * @param name the whitelist name to filter on.
     */
    public MyWhiteListFilter(
        String name)
    {
        nameFilter = name;
    }

    /**
     * This operation is invoked by the WhiteListSelection to find out whether
     * the specified WhiteList object matches the filter or not. Based on the
     * outcome of this operation, the Selection will or will not include
     * the specified object.
     *
     * @param an_object the object that is matched against the filter.
     * @param membership_state for this filter this attribute is irrelevant.
     * @return true if the object matches the filter, false if it doesn't.
     */
    public boolean check_object(
        DLRLChat.WhiteList an_object,
        DDS.MembershipState membership_state)
    {
        boolean passesFilter = false;
        /* It is very important to catch ALL exceptions within the scope of the
         * filter operation. If this is not done the exception will be
         * propagated into the DLRL kernel, where the DLRL will report the
         * error to an error log and shutdown the thread handling updates
         * due to the exception being a fatal error, as any exception thrown
         * by this operation is by definition an application induced exception
         * for which there is no generic way of recovery.
         */
        try
        {
            if (an_object.get_name().equals(nameFilter))
            {
                passesFilter = true;
            }
        }
        catch (Exception e)
        {
            /* Any sort of exception handling may occur here, this is
             * application driven. In our example the exception will be
             * simply caught and ignored.
             */
        }

        return passesFilter;
    }

}
