
package DDS;

public interface DURATION_INFINITE
{
  public static final DDS.Duration_t value = new DDS.Duration_t(
      DURATION_INFINITE_SEC.value, DURATION_INFINITE_NSEC.value );
}
