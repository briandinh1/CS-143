/** FCFSSchedulingAlgorithm.java
 * 
 * A first-come first-served scheduling algorithm.
 *
 * @author: Charles Zhu
 * Spring 2016
 *
 */
package com.jimweller.cpuscheduler;

import java.util.*;

public class FCFSSchedulingAlgorithm extends BaseSchedulingAlgorithm {

    private ArrayList<Process> queue;

    FCFSSchedulingAlgorithm(){
        // Fill in this method
        /*------------------------------------------------------------*/
        this.queue = new ArrayList<Process>();
        /*------------------------------------------------------------*/
    }

    /** Add the new job to the correct queue.*/
    public void addJob(Process p){
        // Remove the next lines to start your implementation
        // throw new UnsupportedOperationException();
        
        // Fill in this method
        /*------------------------------------------------------------*/
        queue.add(p);
        /*------------------------------------------------------------*/
    }
    
    /** Returns true if the job was present and was removed. */
    public boolean removeJob(Process p){
        // Remove the next lines to start your implementation
        // throw new UnsupportedOperationException();

        // Fill in this method
        /*------------------------------------------------------------*/
        return queue.remove(p);
        /*------------------------------------------------------------*/
    }

    /** Transfer all the jobs in the queue of a SchedulingAlgorithm to another, such as
    when switching to another algorithm in the GUI */
    public void transferJobsTo(SchedulingAlgorithm otherAlg) {
        throw new UnsupportedOperationException();
        // leave blank
    }

    /** Returns the next process that should be run by the CPU, null if none available.*/
    public Process getNextJob(long currentTime){
        // Remove the next lines to start your implementation
        // throw new UnsupportedOperationException();
        
        // Fill in this method
        /*------------------------------------------------------------*/
        return queue.peek();
        /*------------------------------------------------------------*/
    }

    public String getName(){
        return "First-Come First-Served";
    }
    
}