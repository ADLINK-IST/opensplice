
package DDS;

public interface DURATION_ZERO
{
  public static final DDS.Duration_t value = new DDS.Duration_t(
      DURATION_ZERO_SEC.value, DURATION_ZERO_NSEC.value);
}
