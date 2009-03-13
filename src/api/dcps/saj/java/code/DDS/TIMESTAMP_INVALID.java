
package DDS;

public interface TIMESTAMP_INVALID
{
  public static final DDS.Time_t value = new DDS.Time_t(
      TIMESTAMP_INVALID_SEC.value, TIMESTAMP_INVALID_NSEC.value);
}
