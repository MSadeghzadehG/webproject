package com.alibaba.json.bvtVO;



import java.util.HashMap;
import java.util.Map;


public class QueueEntity {

    private int                         id;
    private String                      description;

    private Map<Integer, PhysicalQueue> pqMap = new HashMap<Integer, PhysicalQueue>();

    
    public Map<Integer, PhysicalQueue> getPqMap() {
        return pqMap;
    }

    
    public void setPqMap(Map<Integer, PhysicalQueue> pqMap) {
        this.pqMap = pqMap;
    }

    public QueueEntity(){
    }

    
    public Map<Integer, PhysicalQueue> getPhysicalQueueMap() {
        return pqMap;
    }

    
    public void setPhysicalQueueMap(Map<Integer, PhysicalQueue> pqMap) {
        this.pqMap = pqMap;
    }

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

}
