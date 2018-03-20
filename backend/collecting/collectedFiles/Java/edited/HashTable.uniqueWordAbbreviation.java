









import java.util.ArrayList;

public class ValidWordAbbr {

    HashMap<String, String> map;

    public ValidWordAbbr(String[] dictionary) {
        
        this.map = new HashMap<String, String>();
        
        for(String word : dictionary) {
            
            String key = getKey(word);
            
            if(map.containsKey(key)) {
                
                if(!map.get(key).equals(word)) {
                    
                    map.put(key, "");
                    
                }
                
            }
            
            else {
                
                map.put(key, word);
                
            }
            
            
        }
        
        
        
        
        
    }

    public boolean isUnique(String word) {
        
        return !map.containsKey(getKey(word))||map.get(getKey(word)).equals(word);
        
    }
    
    public String getKey(String word) {
        
        if(word.length() <= 2) return word;
        
        return word.charAt(0) + Integer.toString(word.length() - 2) + word.charAt(word.length() - 1);
        
    }
    
}


