class Cache{
    public int wordSize = 32;
    public int capacity;
    public int ways;
    public int lineSize;
    public int latency;
    
    public Cache(int capacity, int ways, int lineSize, int latency){
        this.capacity = capacity;
        this.ways = ways;
        this.lineSize = lineSize;
        this.latency = latency;
    }



}