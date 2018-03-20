package hudson.model;

import hudson.ExtensionList;
import hudson.ExtensionPoint;

import java.util.ArrayList;
import java.util.List;


public abstract class TransientViewActionFactory implements ExtensionPoint {

	
	public abstract List<Action> createFor(View v);
	
    
	public static ExtensionList<TransientViewActionFactory> all() {
		return ExtensionList.lookup(TransientViewActionFactory.class);
	}
	
    
	public static List<Action> createAllFor(View v) {
		List<Action> result = new ArrayList<Action>();
		for (TransientViewActionFactory f: all()) {
			result.addAll(f.createFor(v));
		}
		return result;
	}

}
