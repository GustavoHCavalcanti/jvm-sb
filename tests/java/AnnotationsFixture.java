import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.TYPE, ElementType.FIELD, ElementType.METHOD,
        ElementType.PARAMETER, ElementType.TYPE_USE})
@interface Mark {
    int value() default 37;
}

@Mark(11)
public class AnnotationsFixture<T> {
    @Deprecated
    @Mark(12)
    public static final int CONSTANT = 7;

    @Mark(13)
    public T field;

    @Mark(14)
    public String run(@Mark(15) String input)
            throws IllegalArgumentException {
        Runnable action = () -> System.out.println(input);
        class Local { }
        action.run();
        @Mark(16) String value = input == null ? "null" : input;
        return value;
    }

    public static class Nested { }
}
