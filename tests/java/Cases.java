import java.io.IOException;

interface Marked { int value(); }
abstract class Parent { abstract int compute(int x); }

public class Cases extends Parent implements Marked {
    public static final long BIG = 1234567890123L;
    public static final long NEGATIVE = -42L;
    public static final double PI2 = 6.283185307179586;
    public static final float FRACTION = 0.1f;
    public static final String MUTF_TEXT = "café 😀";
    public static final String NUL_TEXT = "\0";
    private String label = "viewer";
    public Cases() { super(); }
    public static int add(int a, int b) { return a + b; }
    @Override public int compute(int x) {
        int[] values = {1, 2, 3};
        int sum = 0;
        for (int v : values) { if (v == 2) sum += x; else sum += v; }
        switch (x) { case 1: sum++; break; case 4: sum += 4; break; default: sum--; }
        return sum;
    }
    @Override public int value() { return label.length(); }
    public void mayFail() throws IOException { throw new IOException(label); }
    public static void main(String[] args) { System.out.println(add(2, 3)); }
}
