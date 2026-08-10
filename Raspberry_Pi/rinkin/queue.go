package main

import "time"

type Slot struct {
	value     string
	timestamp time.Time
}

type Queue struct {
	data map[string]*Slot
}

type QueueResult struct {
	slot, value string
	timestamp   time.Time
}

func NewQueue() *Queue {
	return &Queue{
		data: make(map[string]*Slot),
	}
}

func (q *Queue) Push(slot, value string) {
	q.data[slot] = &Slot{value: value, timestamp: time.Now()}
}

func (q *Queue) Pop() *QueueResult {
	var oldest *QueueResult
	for k, v := range q.data {
		if oldest == nil || oldest.timestamp.After(v.timestamp) {
			oldest = &QueueResult{
				slot:      k,
				value:     v.value,
				timestamp: v.timestamp,
			}
		}
	}
	if oldest != nil {
		delete(q.data, oldest.slot)
	}
	return oldest
}
