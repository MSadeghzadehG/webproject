


import static java.lang.Integer.parseInt;
import static java.lang.System.exit;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.Scanner;
import java.util.StringTokenizer;

public class DigitCounting {

	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);
		int numberOfTestCases = input.nextInt();
		while (numberOfTestCases != 0) {
			int[] numbers = new int[10];
			int number = input.nextInt();
			for (int i = number; i > 0; i--) {
				int j = i;
				while (j != 0) {
					numbers[j % 10]++;
					j = j / 10;
				}
			}
			for (int i = 0; i < 10; i++) {
				if (i != 0) {
					System.out.print(" ");
				}
				System.out.print(numbers[i]);
			}
			System.out.println();
			numberOfTestCases--;
		}
	}
}
