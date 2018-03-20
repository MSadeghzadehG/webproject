


import java.math.BigInteger;
import java.util.Scanner;

public class BasicallySpeaking {

	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);
		while (input.hasNext()) {
			String numberAsString = input.next();
			int fromBase = input.nextInt();
			int toBase = input.nextInt();
			BigInteger number = new BigInteger(numberAsString, fromBase);
			String numberThatIsPrinted = number.toString(toBase);
			String answer = numberThatIsPrinted.toUpperCase();
			if (numberThatIsPrinted.length() > 7) {
				answer = "ERROR";
			}
			System.out.printf("%7s\n", answer);
		}
	}
}
