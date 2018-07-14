/** PrioritySchedulingAlgorithm.java
 * 
 * A single-queue priority scheduling algorithm.
 *
 * @author: Charles Zhu
 * Spring 2016
 *
 */
package com.jimweller.cpuscheduler;

import java.util.*;
import com.jimweller.cpuscheduler.Process;



public class PrioritySchedulingAlgorithm extends BaseSchedulingAlgorithm implements OptionallyPreemptiveSchedulingAlgorithm {
    

    private MyPriorityQueue<Process> queue;
    private Process current;
    private boolean preemptive;



    PrioritySchedulingAlgorithm(){
        // Fill in this method
        /*------------------------------------------------------------*/
       	this.queue = new MyPriorityQueue<Process>();
       	this.current = null;
        /*------------------------------------------------------------*/
    }

    /** Add the new job to the correct queue.*/
    public void addJob(Process p){
        // Remove the next lines to start your implementation
        // throw new UnsupportedOperationException();
        
        // Fill in this method
        /*------------------------------------------------------------*/
        if (preemptive && (current != null) && (p.priority < current.priority)) 
        {
        	queue.add(current, current.priority);
        	current = p;
        }
        else queue.add(p, p.priority);
        /*------------------------------------------------------------*/
    }
    
    /** Returns true if the job was present and was removed. */
    public boolean removeJob(Process p){
        // Remove the next lines to start your implementation
        // throw new UnsupportedOperationExce,ption();
        
        // Fill in this method
        /*------------------------------------------------------------*/
        if (current == null) return false;

        if (current.PID == p.PID)
        {
        	current = null;
        	return true;
        }
        
        return false;
        /*------------------------------------------------------------*/
    }

    /** Transfer all the jobs in the queue of a SchedulingAlgorithm to another, such as
    when switching to another algorithm in the GUI */
    public void transferJobsTo(SchedulingAlgorithm otherAlg) {
        throw new UnsupportedOperationException();
    }


    /** Returns the next process that should be run by the CPU, null if none available.*/
    public Process getNextJob(long currentTime){
        // Remove the next lines to start your implementation
        // throw new UnsupportedOperationException();
        
        // Fill in this method
        /*------------------------------------------------------------*/
        if (current == null) current = queue.remove();
        return current;
        /*------------------------------------------------------------*/
    }

    public String getName(){
        return "Single-Queue Priority";
    }

    /**
     * @return Value of preemptive.
     */
    public boolean isPreemptive(){
        // Remove the next lines to start your implementation
        // throw new UnsupportedOperationException();
        
        // Fill in this method
        /*------------------------------------------------------------*/
        return preemptive;
        /*------------------------------------------------------------*/
    }
    
    /**
     * @param v Value to assign to preemptive.
     */
    public void setPreemptive(boolean v){
        // Remove the next lines to start your implementation
        // throw new UnsupportedOperationException();
        
        // Fill in this method
        /*------------------------------------------------------------*/
        preemptive = v;
        /*------------------------------------------------------------*/
    }
    
}