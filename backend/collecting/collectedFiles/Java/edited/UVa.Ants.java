



import java.util.Scanner;


public class Ants {

    public static void main(String args[]) throws Exception {

                Scanner sc = new Scanner(System.in);

                int testCases = sc.nextInt();

                int currentAnt;

        while(testCases > 0) {

                        int length = sc.nextInt();
            int numberOfAnts = sc.nextInt();

                        int min = 0;
            int max = 0;

                        while(numberOfAnts > 0) {

                                currentAnt = sc.nextInt();

                                currentAnt = currentAnt < length - currentAnt ? currentAnt : length - currentAnt;

                                if(currentAnt > min) {

                    min = currentAnt;

                }

                                if(length - currentAnt > max) {

                    max = length - currentAnt;

                }

                                numberOfAnts--;

            }

                        System.out.println(min + " " + max);

                        testCases--;

        }

    }

}

