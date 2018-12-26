
package com.alibaba.json.bvtVO;

import java.io.Serializable;
import java.util.ArrayList;


public class IEventDto implements Serializable {

    private static final long serialVersionUID = -3903138261314727539L;

    private String            source;

    private ArrayList<IEvent> eventList = new ArrayList<IEvent>();

    public String getSource() {
        return source;
    }

    public void setSource(String source) {
        this.source = source;
    }
    
    public ArrayList<IEvent> getEventList() {
        return eventList;
    }

    public void setEventList(ArrayList<IEvent> eventList) {
        this.eventList = eventList;
    }

    @Override
    public String toString() {
        return "IEventDto [source=" + source + ", eventList=" + eventList + "]";
    }

}
