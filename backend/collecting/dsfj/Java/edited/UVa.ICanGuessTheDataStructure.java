

						




import java.util.*;
import java.io.*;

public class ICanGuessTheDataStructure {

    public static void main(String args[]) throws Exception {

                Stack<Integer> stack = new Stack<Integer>();
        Queue<Integer> queue = new LinkedList<Integer>();
                PriorityQueue<Integer> priorityQueue = new PriorityQueue<Integer>(Collections.reverseOrder());

                BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        String line;

                while ((line = br.readLine()) != null) {

                        int stackRemovals = 0;
            int queueRemovals = 0;
            int priorityQueueRemovals = 0;
            int totalRemovals = 0;

                        int numberOfCases = Integer.parseInt(line);

                        queue.clear();
            priorityQueue.clear();
            stack.clear();

                        for (int i = 0; i < numberOfCases; i++) {

                String[] currentLineSplit = br.readLine().split(" ");
                int command = Integer.parseInt(currentLineSplit[0]);
                int number = Integer.parseInt(currentLineSplit[1]);

                                if (command == 1) {

                    stack.push(number);
                    queue.add(number);
                    priorityQueue.add(number);

                } else {

                                        if (!stack.isEmpty() && stack.peek() == number && stackRemovals == totalRemovals) {

                        stackRemovals++;
                        stack.pop();

                    }

                    if (!queue.isEmpty() && queue.peek() == number && queueRemovals == totalRemovals) {

                        queueRemovals++;
                        queue.remove();

                    }

                    if (!priorityQueue.isEmpty() && priorityQueue.peek() == number && priorityQueueRemovals == totalRemovals) {

                        priorityQueueRemovals++;
                        priorityQueue.remove();

                    }

                    totalRemovals++;


                }

            }

                        if ((stackRemovals == totalRemovals && queueRemovals == totalRemovals) || (stackRemovals == totalRemovals && stackRemovals == priorityQueueRemovals) || (queueRemovals == totalRemovals && priorityQueueRemovals == totalRemovals)) {

                System.out.println("not sure");

            } else if (stackRemovals == totalRemovals) {

                System.out.println("stack");

            } else if (queueRemovals == totalRemovals) {

                System.out.println("queue");

            } else if (priorityQueueRemovals == totalRemovals) {

                System.out.println("priority queue");

            } else {

                System.out.println("impossible");

            }

        }


    }

}
