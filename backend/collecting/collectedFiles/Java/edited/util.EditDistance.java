
package hudson.util;

import java.util.Collection;
import java.util.Arrays;


public class EditDistance {

    
    public static int editDistance( String a, String b ) {
        return new EditDistance(a,b).calc();
    }

    
    public static String findNearest( String key, String[] group ) {
        return findNearest(key,Arrays.asList(group));
    }

    public static String findNearest( String key, Collection<String> group ) {
        int c = Integer.MAX_VALUE;
        String r = null;

        for (String g : group) {
            int ed = editDistance(key, g);
            if (c > ed) {
                c = ed;
                r = g;
            }
        }
        return r;
    }

    
    private int[] cost;
    
    private int[] back;

    
    private final String a,b;

    private EditDistance( String a, String b ) {
        this.a=a;
        this.b=b;
        cost = new int[a.length()+1];
        back = new int[a.length()+1]; 
        for( int i=0; i<=a.length(); i++ )
            cost[i] = i;
    }

    
    private void flip() {
        int[] t = cost;
        cost = back;
        back = t;
    }

    private int min(int a,int b,int c) {
        return Math.min(a,Math.min(b,c));
    }

    private int calc() {
        for( int j=0; j<b.length(); j++ ) {
            flip();
            cost[0] = j+1;
            for( int i=0; i<a.length(); i++ ) {
                int match = (a.charAt(i)==b.charAt(j))?0:1;
                cost[i+1] = min( back[i]+match, cost[i]+1, back[i+1]+1 );
            }
        }
        return cost[a.length()];
    }
}
