package hudson.model;

import hudson.ExtensionList;
import hudson.ExtensionPoint;
import java.util.ArrayList;
import java.util.List;



public abstract class ComputerPanelBox implements ExtensionPoint{
    
    private Computer computer;
    
    
    public void setComputer(Computer computer){
        this.computer = computer;
    }
    
    public Computer getComputer(){
        return computer;
    }
    
    
    public static List<ComputerPanelBox> all(Computer computer) {
        List<ComputerPanelBox> boxs = new ArrayList<ComputerPanelBox>();
        for(ComputerPanelBox box:  ExtensionList.lookup(ComputerPanelBox.class)){
            box.setComputer(computer);
            boxs.add(box);
        }
        return boxs;
    }


}
