
            
        



import java.util.*;
import java.io.*;

public class SplittingNumbers {

    public static void main(String args[]) throws Exception {

        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        String line;

        while((line = br.readLine()) != null) {

                        int number = Integer.parseInt(line);

                        if(number == 0) break;

                        int count = 0;
            int a = 0;
            int b = 0;

            while(number > 0) {


                                int currentBit = number ^ (number & (number - 1));


                                if(count % 2 == 0) {

                    a |= currentBit;

                }

                                else {

                    b |= currentBit;

                }

                                count++;

                                number &= (number - 1);

            }

                        System.out.println(a + " " + b);

        }


    }

}
