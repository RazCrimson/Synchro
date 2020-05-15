#pragma once

#include "Event\Event.h"

using namespace System;
using namespace System::Collections::Generic;


public ref class EventsQueue : LinkedList<Event^>
{
public:

	void Enqueue(Event^ element)
	{
		AddLast(element);
	}

	Event^ dequeue()
	{
		if (this->First == nullptr)
			throw gcnew InvalidOperationException("Queue is Empty");
		Event^ event = First->Value;
		RemoveFirst();
		return event;
	}

	Event^ Peek()
	{
		if (this->First == nullptr)
			throw gcnew InvalidOperationException("Queue is Empty");
		Event^ event = First->Value;
		return event;
	}

	// Rate limit the event additions in milliseconds
	void rateLimitedEnqueue(Event^ event, Int32 rateLimitMilliSeconds) 
	{
		if (Contains(event))
		{
			auto node = FindLast(event);
			if (node != nullptr)
			{
				Event^ prevEvent = node->Value;
				if (event->getTimeElapsed() - prevEvent->getTimeElapsed() < rateLimitMilliSeconds)
					return;
			}
		}
		Enqueue(event);
	}

};