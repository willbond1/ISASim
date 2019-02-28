import java.util.BitSet;

class CacheLine{
    public BitSet tag;
    public BitSet setNumber;
    public int[] line;
    public boolean dirty;
    public BitSet LRU;

    public CacheLine(){}
}

