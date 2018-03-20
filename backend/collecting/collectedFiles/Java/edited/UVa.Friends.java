



import java.io.*;
import java.util.*;



public class Friends {

        public static int[] people = new int[30001];
    public static int[] relationships = new int[50001];

    public static void main(String args[]) throws Exception {

                BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        String line = br.readLine();

                int testCases = Integer.parseInt(line);

        for(int i = 0; i < testCases; i++) {

                        String[] info = br.readLine().split(" ");
            int numberOfPeople = Integer.parseInt(info[0]);
            int numberOfRelationship = Integer.parseInt(info[1]);

            startUnion(numberOfPeople, people, relationships);

                        for(int j = 0; j < numberOfRelationship ; j++) {

                                String[] currentLine = br.readLine().split(" ");
                int person = Integer.parseInt(currentLine[0]);
                int friend = Integer.parseInt(currentLine[1]);
                union(person, friend);

            }

                        int maxGroup = 1;

                        for(int j = 0; j <= numberOfPeople; j++) {

                                maxGroup = relationships[j] > maxGroup ? relationships[j] : maxGroup;

            }

                        System.out.println(maxGroup);

        }

    }

    public static void startUnion(int numberOfPeople, int[] people, int[] relationships) {

        for(int i = 0; i <= numberOfPeople; i++) {

                        people[i] = i;

                        relationships[i] = 1;

        }

    }

    public static void union(int person, int friend) {

                person = find(person);
        friend = find(friend);

        if(person != friend) {

                        join(person, friend);

        }

    }

    public static int find(int person) {

                if(people[person] != person) {

            people[person] = find(people[person]);

        }

        return people[person];

    }

    public static void join(int person, int friend) {

                if(relationships[person] > relationships[friend]) {

            relationships[person] += relationships[friend];
            people[friend] = person;

        }

        else {

            relationships[friend] += relationships[person];
            people[person] = friend;

        }

    }

}




