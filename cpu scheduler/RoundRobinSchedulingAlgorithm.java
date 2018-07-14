/** RoundRobinSchedulingAlgorithm.java
 * 
 * A scheduling algorithm that randomly picks the next job to go.
 *
 * @author: Kyle Benson
 * Winter 2013
 *
 */
package com.jimweller.cpuscheduler;

import java.util.*;

public class RoundRobinSchedulingAlgorithm extends BaseSchedulingAlgorithm {

    /** the time slice each process gets */
    private int quantum;
    private ArrayList<Process> processes;
    private int counter;
    private int currentJob;


    RoundRobinSchedulingAlgorithm() {
        // Fill in this method
        /*------------------------------------------------------------*/
        counter = quantum;
        currentJob = -1; 
        processes = new ArrayList<Process>();
        /*------------------------------------------------------------*/
    }

    /** Add the new job to the correct queue. */
    public void addJob(Process p) {
        // Remove the next lines to start your implementation
        // throw new UnsupportedOperationException();
        
        // Fill in this method
        /*------------------------------------------------------------*/
        processes.add(p);
        /*------------------------------------------------------------*/
    }

    /** Returns true if the job was present and was removed. */
    public boolean removeJob(Process p) {
        // Remove the next lines to start your implementation
        // throw new UnsupportedOperationException();
        
        // Fill in this method
        /*------------------------------------------------------------*/
        int processIndex = processes.indexOf(p);
        boolean removeResult = processes.remove(p);

        if ((currentJob >= processIndex) && (processIndex >= 0))
        {
        	--currentJob;

        	try
        	{
        		activeJob = processes.get(currentJob);
        	}
        	catch (IndexOutOfBoundsException e)
        	{
        		activeJob = null;
        	}
        }
        counter = 0;
        return removeResult;
        /*------------------------------------------------------------*/
    }

    /** Transfer all the jobs in the queue of a SchedulingAlgorithm to another, such as
    when switching to another algorithm in the GUI */
    public void transferJobsTo(SchedulingAlgorithm otherAlg) {
        throw new UnsupportedOperationException();
    }

    /**
     * Get the value of quantum.
     * 
     * @return Value of quantum.
     */
    public int getQuantum() {
        return quantum;
    }

    /**
     * Set the value of quantum.
     * 
     * @param v
     *            Value to assign to quantum.
     */
    public void setQuantum(int v) {
        this.quantum = v;
    }

    /**
     * Returns the next process that should be run by the CPU, null if none
     * available.
     */
    public Process getNextJob(long currentTime) {
        // Remove the next lines to start your implementation
        // throw new UnsupportedOperationException();
        
        // Fill in this method
        /*------------------------------------------------------------*/
        Process currentProc = null;
        --counter;

        if ((currentJob >= 0) && !isJobFinished() && (counter >= 0)) return activeJob;

        if (processes.isEmpty())
        {
        	currentJob = -1;
        	return null;
        }

        int i;
        if (currentJob >= processes.size()-1 || currentJob < 0) i = 0;
        else i = ++currentJob;

        counter = quantum;
        currentJob = i;
        activeJob = currentProc = processes.get(i);
        return currentProc;
        /*------------------------------------------------------------*/
    }

    public String getName() {
        return "Round Robin";
    }
    
}