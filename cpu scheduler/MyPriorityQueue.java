
package com.jimweller.cpuscheduler;

import java.util.*;

public class MyPriorityQueue<element>
{
    
    private int size;
    private ArrayList<Node<element>>  queue;


    private class Node<element>
    {
        public element data;
        public long priority;
        public int leftChild;
        public int rightChild;

        Node(element data, long priority)
        {
            this.data = data;
            this.priority = priority;
            leftChild = rightChild = -1;  
        }
    }


    MyPriorityQueue()
    {
        this.size = 0;
        this.queue = new ArrayList<Node<element>>();
    }


    public void add(element data, long priority)
    {
        Node<element> newNode = new Node<element>(data, priority);
        queue.add(newNode);
        int newNodeIndex = queue.indexOf(newNode);

        if (newNodeIndex != 0)
        {
            Node<element> parent = queue.get(newNodeIndex / 2);
            
            if (parent.leftChild <= -1) parent.leftChild = newNodeIndex;
            else if (parent.rightChild <= -1) parent.rightChild = newNodeIndex;

            percolateUp(newNodeIndex);
        }

        ++size;
    }


    public element remove()
    {
        if (size <= 0) throw new IllegalArgumentException();

        if (size == 1)
        {
            Node<element> node = queue.get(0);
            queue.remove(0);
            --size;
            return node.data;
        }

        int lastIndex = queue.size() - 1;
        swap(0, lastIndex);
        Node<element> node = queue.get(lastIndex);
        queue.remove(lastIndex);

        Node<element> finalNodeParent = queue.get(lastIndex / 2);

        if (finalNodeParent.rightChild >= 0) finalNodeParent.rightChild = -1;
        else finalNodeParent.leftChild = -1;

        percolateDown(0);
        --size;
        return node.data;
    }


    private void swap(int parentIndex, int childIndex)
    {
        Node<element> parent = queue.get(parentIndex);
        Node<element> child = queue.get(childIndex);
        element tempData = parent.data;
        long tempPriority = parent.priority;
        parent.data = child.data;
        parent.priority = child.priority;
        child.data = tempData;
        child.priority = tempPriority;
    }
    

    private void percolateUp(int i)
    {
        if (i > 0)
        {
            int parentIndex = i / 2;
            if (queue.get(i).priority < queue.get(parentIndex).priority)
            {
                swap(parentIndex, i);
                percolateUp(parentIndex);
            }
        }
    }


    private void percolateDown(int i)
    {
        Node<element> parent = queue.get(i);

        if (parent.leftChild >= 0)
        {
            int minChild = getSmallestNode(i, parent.priority);
            if (minChild != i)
            {
                swap(i, minChild );
                percolateDown(minChild);
            }
        }

    }


    private int getSmallestNode(int parentIndex, long priority)
    {
        Node<element> parent = queue.get(parentIndex);
        long minPriority = priority;
        int minChild = parentIndex;
        long leftPriority = queue.get(parent.leftChild).priority;

        if (minPriority > leftPriority)
        {
            minChild = parent.leftChild;
            minPriority = leftPriority;
        }

        if (parent.rightChild >= 0)
            if (minPriority > queue.get(parent.rightChild).priority)
                minChild = parent.rightChild;

        return minChild;

    }
}