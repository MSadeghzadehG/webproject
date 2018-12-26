
                
        

import java.io.*;
import java.util.Arrays;
import java.util.HashMap;


public class VirtualFriends {

    public static int[] people = new int[1000001];
    public static int[] relationships = new int[1000001];

    public static void main(String args[]) throws Exception {
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        int cases = Integer.parseInt(br.readLine());
        while(cases-- > 0) {
            startUnion(people.length);
            HashMap<String, Integer> map = new HashMap<String, Integer>();
            int friendships = Integer.parseInt(br.readLine());
            int numberOfPeople = 0;
            for(int i = 0; i < friendships; i++) {
                String[] line = br.readLine().split("\\s+");
                String x = line[0];
                String y = line[1];
                if (x.equals(y)) {
                    System.out.println(1);
                    continue;
                }
                if (!map.containsKey(x)) {
                    map.put(x, ++numberOfPeople);
                }
                if (!map.containsKey(y)) {
                    map.put(y, ++numberOfPeople);
                }

                                System.out.println(union(map.get(x), map.get(y)));
            }
        }
    }

    public static void startUnion(int numberOfPeople) {
        for(int i = 0; i < numberOfPeople; i++) {
                        people[i] = i;

                        relationships[i] = 1;
        }
    }

    public static int union(int person, int friend) {
                person = find(person);
        friend = find(friend);

        if(person != friend) {
                                    if(relationships[person] > relationships[friend]) {
                relationships[person] += relationships[friend];
                people[friend] = person;
                return relationships[person];
            }

            else {
                relationships[friend] += relationships[person];
                people[person] = friend;
                return relationships[friend];
            }
        }
        return relationships[person];
    }

    public static int find(int person) {
                if(people[person] != person) {
            people[person] = find(people[person]);
        }
        return people[person];
    }
}

