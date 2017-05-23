import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;


public class Filter<TYPE> {
    private String expression;
    private String[] params;

    public Filter(String expression, String... params) {
        this.expression = expression;
        this.params = params;
    }

    public String getExpression() {
        return this.expression;
    }

    public List<String> getParameters() {
        return new ArrayList<String>(Arrays.asList(this.params));
    }
}
