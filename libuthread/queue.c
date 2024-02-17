#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

typedef struct node *node_t;

typedef struct node
{
	node_t next_node;
	node_t prev_node;
	void *data;
} node;

typedef struct queue
{
	/* TODO Phase 1 */
	unsigned int length;
	node_t head;
	node_t tail;
} queue;

queue_t queue_create(void)
{
	/* TODO Phase 1 */
	queue_t new_queue = malloc(sizeof(queue));
	if (new_queue == NULL)
	{
		return NULL;
	}

	new_queue->length = 0;
	new_queue->head = NULL;
	new_queue->tail = NULL;

	return new_queue;
}

int queue_destroy(queue_t queue)
{
	if (queue == NULL || queue->length > 0)
	{
		return -1;
	}

	free(queue);
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	if (queue == NULL || data == NULL)
	{
		return -1;
	}

	// create new node
	node_t new_node = malloc(sizeof(node));

	if (new_node == NULL)
	{
		return -1;
	}

	new_node->data = data;
	new_node->next_node = NULL;
	new_node->prev_node = NULL;

	if (queue->length == 0)
	{
		queue->head = new_node;
		queue->tail = new_node;
	}
	else
	{
		queue->tail->next_node = new_node;
		new_node->prev_node = queue->tail;
		queue->tail = new_node;
	}

	queue->length++;

	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	if (queue == NULL || queue->length == 0 || data == NULL)
	{
		return -1;
	}

	node_t dequeued_node = queue->head;
	*data = dequeued_node->data;

	queue->head = queue->head->next_node;
	queue->length--;

	free(dequeued_node);

	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	if (queue == NULL || data == NULL)
	{
		return -1;
	}

	node_t current = queue->head;

	while (current != NULL)
	{
		if (current->data == data)
		{
			node_t prev = current->prev_node;
			node_t next = current->next_node;

			if (prev != NULL)
			{
				prev->next_node = next;
			}
			if (next != NULL)
			{
				next->prev_node = prev;
			}

			// special case if deleted node is head or tail
			if (queue->head == current)
			{
				queue->head = current->next_node;
			}

			if (queue->tail == current)
			{
				queue->tail = current->prev_node;
			}

			free(current);

			queue->length--;

			return 0;
		}
		current = current->next_node;
	}

	return -1;
}

int queue_iterate(queue_t queue, queue_func_t func)
{
	if (queue == NULL || func == NULL)
	{
		return -1;
	}

	node_t current = queue->head;

	while (current != NULL)
	{
		// Need to save in case the provided function removes our current node
		node_t next = current->next_node;
		func(queue, current->data);
		current = next;
	}

	return 0;
}

int queue_length(queue_t queue)
{
	if (queue == NULL)
	{
		return -1;
	}

	return queue->length;
}
