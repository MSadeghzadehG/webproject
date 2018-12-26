package org.elasticsearch.painless;

import java.util.List;
import java.util.function.Function;




public class FeatureTest {
    private int x;
    private int y;
    public int z;

    
    public FeatureTest() {
    }

    
    public FeatureTest(int x, int y) {
        this.x = x;
        this.y = y;
    }

    
    public int getX() {
        return x;
    }

    
    public void setX(int x) {
        this.x = x;
    }

    
    public int getY() {
        return y;
    }

    
    public void setY(int y) {
        this.y = y;
    }

    
    public static boolean overloadedStatic() {
        return true;
    }

    
    public static boolean overloadedStatic(boolean whatToReturn) {
        return whatToReturn;
    }

    
    public Object twoFunctionsOfX(Function<Object,Object> f, Function<Object,Object> g) {
        return f.apply(g.apply(x));
    }

    public void listInput(List<Object> list) {

    }
}
