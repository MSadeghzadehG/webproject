  


import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Scanner;

public class PrimeFactors {

	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);
		int number = input.nextInt();
		boolean[] isPrime = generatePrimeNumbers();
		while (number != 0) {
			boolean isNegative = false;
			if (number < 0) {
				isNegative = true;
				number = Math.abs(number);
			}
			int originalNumber = number;
			formatOutput(originalNumber, sieveOfEratosthenes(isPrime, originalNumber), isNegative);
			number = input.nextInt();
		}
	}

	public static List<Integer> sieveOfEratosthenes(boolean[] isPrime, int number) {
		List<Integer> primeFactors = new ArrayList<Integer>();
		int squareRootOfOriginalNumber = (int) Math.sqrt(number);
		for (int i = 2; i <= squareRootOfOriginalNumber; i++) {
			if (isPrime[i]) {
				while (number % i == 0) {
					primeFactors.add(i);
					number = number / i;
				}
			}
		}
		if (number != 1) {
			primeFactors.add(number);
		}
		return primeFactors;
	}

	static void formatOutput(int number, List<Integer> primeFactors, boolean isNegative) {
		if (isNegative) {
			number *= -1;
		}
		StringBuilder output = new StringBuilder(number + " = ");
		int numberOfPrimeFactors = primeFactors.size();
		if (numberOfPrimeFactors == 1) {
			if (isNegative) {
				output.append("-1 x " + (number * (-1)));
			} else {
				output.append(number);
			}
		} else {
			Collections.sort(primeFactors);
			if (isNegative) {
				output.append("-1 x ");
			}
			for (int i = 0; i < numberOfPrimeFactors - 1; i++) {
				output.append(primeFactors.get(i) + " x ");
			}
			output.append(primeFactors.get(numberOfPrimeFactors - 1));
		}
		System.out.println(output);
	}

	static boolean[] generatePrimeNumbers() {
		int number = (int) Math.sqrt(Integer.MAX_VALUE);
		boolean[] isPrime = new boolean[number + 1];
		for (int i = 2; i < number + 1; i++) {
			isPrime[i] = true;
		}
		for (int factor = 2; factor * factor < number + 1; factor++) {
			if (isPrime[factor]) {
				for (int j = factor; j * factor < number + 1; j++) {
					isPrime[j * factor] = false;
				}
			}
		}
		return isPrime;
	}

}