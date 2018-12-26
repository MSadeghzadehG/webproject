



import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.*;
import static java.lang.Character.isUpperCase;



public class OpenSource {

    public static void main(String args[]) throws Exception {

        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        String line;

        while(!(line = br.readLine()).equals("0")) {
            HashMap<String, Integer> projects = new HashMap<String, Integer>();
            HashMap<String, String> students = new HashMap<String, String>();
            String project = line;
            projects.put(project, 0);

            while(!(line = br.readLine()).equals("1")) {
                if(isUpperCase(line.charAt(0))) {
                    project = line;
                    projects.put(project, 0);
                }

                else {
                    if(students.containsKey(line)) {
                        if(students.get(line).equals("")) {
                            continue;
                        }

                        else {
                            if(!students.get(line).equals(project)) {
                                projects.put(students.get(line), projects.get(students.get(line)) - 1);
                                students.put(line, "");
                            }
                        }
                    }

                    else {
                        projects.put(project, projects.get(project) + 1);
                        students.put(line, project);
                    }
                }
            }

            List<Pair> pairs = new ArrayList<Pair>();
            int count = 0;

            for(String s : projects.keySet()) {

                pairs.add(new Pair(s, projects.get(s)));

            }

            Collections.sort(pairs,new Comparator<Pair>() {

                public int compare(Pair o1, Pair o2) {
                    if(-Integer.compare(o1.total, o2.total) == 0) {
                        return o1.name.compareTo(o2.name);
                    }
                    return -Integer.compare(o1.total, o2.total);
                }
            });

            for(Pair p : pairs) {

                System.out.println(p.name + " " + p.total);

            }

        }

    }

}

class Pair {
    String name;
    int total;

    Pair(String name, int total) {
        this.name = name;
        this.total = total;
    }
}