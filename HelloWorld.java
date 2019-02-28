import java.util.BitSet;

class HelloWorld {
    public static void main(String[] args) {
        int n = 63;
        BitSet bs = BitSet.valueOf(new long[]{n});
        bs.clear();
        System.out.println(bs); // Display the string.
    }
}
