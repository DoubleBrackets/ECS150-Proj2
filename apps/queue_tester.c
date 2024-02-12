#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

#define TEST_ASSERT(assert)                 \
	do                                      \
	{                                       \
		printf("ASSERT: " #assert " ... "); \
		if (assert)                         \
		{                                   \
			printf("PASS\n");               \
		}                                   \
		else                                \
		{                                   \
			printf("FAIL\n");               \
			exit(1);                        \
		}                                   \
	} while (0)

/* Create */
void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}

void test_destroy(void)
{
	fprintf(stderr, "*** TEST destroy ***\n");

	queue_t q = queue_create();

	int a = 1;
	queue_enqueue(q, &a);
	TEST_ASSERT(queue_destroy(q) == -1);

	queue_delete(q, &a);

	TEST_ASSERT(queue_destroy(q) == 0);
}

void test_enqueue(void)
{
	fprintf(stderr, "*** TEST enqueue ***\n");

	// Null parameters
	int a = 1;
	queue_t q = queue_create();
	TEST_ASSERT(queue_enqueue(NULL, &a) == -1);
	TEST_ASSERT(queue_enqueue(q, NULL) == -1);
}

void test_dequeue(void)
{
	fprintf(stderr, "*** TEST dequeue ***\n");

	int *output;
	int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};

	queue_t q = queue_create();
	for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++)
	{
		TEST_ASSERT(queue_enqueue(q, &data[i]) == 0);
	}

	for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++)
	{
		TEST_ASSERT(queue_dequeue(q, &output) == 0);
		TEST_ASSERT(output == &data[i]);
	}

	// Empty queue
	queue_t empty = queue_create();
	TEST_ASSERT(queue_dequeue(empty, &output) == -1);

	// Null parameters
	TEST_ASSERT(queue_dequeue(NULL, &output) == -1);
	TEST_ASSERT(queue_dequeue(q, NULL) == -1);
}

void test_delete(void)
{
	fprintf(stderr, "*** TEST delete ***\n");

	queue_t q = queue_create();
	int data1 = 1;
	queue_enqueue(q, &data1);
	int data2 = 2;
	queue_enqueue(q, &data2);
	int data3 = 3;
	queue_enqueue(q, &data3);

	TEST_ASSERT(queue_delete(q, &data2) == 0);

	// Test when middle element is deleted
	TEST_ASSERT(q->head->next_node->data == &data3);
	TEST_ASSERT(q->tail->prev_node->data == &data1);
	TEST_ASSERT(q->length == 2);

	queue_delete(q, &data3);

	// Test when there is only one element left
	TEST_ASSERT(q->head->data == &data1);
	TEST_ASSERT(q->tail->data == &data1);
	TEST_ASSERT(q->head->prev_node == NULL);
	TEST_ASSERT(q->head->next_node == NULL);
	TEST_ASSERT(q->tail->prev_node == NULL);
	TEST_ASSERT(q->tail->next_node == NULL);
	TEST_ASSERT(q->length == 1);

	// Test when all elements have been deleted
	queue_delete(q, &data1);
	TEST_ASSERT(q->head == NULL);
	TEST_ASSERT(q->length == 0);

	// Null parameters
	TEST_ASSERT(queue_delete(NULL, &data1) == -1);
	TEST_ASSERT(queue_delete(q, NULL) == -1);

	// Unable to be found
	int data4 = 4;
	TEST_ASSERT(queue_delete(q, &data4) == -1);
}

void iterator_inc(queue_t q, void *data)
{
	int *a = (int *)data;

	if (*a == 42)
		queue_delete(q, data);
	else
		*a += 1;
}

void test_iterator(void)
{
	fprintf(stderr, "*** TEST iterator ***\n");

	queue_t q;
	int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
	size_t i;

	/* Initialize the queue and enqueue items */
	q = queue_create();
	for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
		queue_enqueue(q, &data[i]);

	/* Increment every item of the queue, delete item '42' */
	int result = queue_iterate(q, iterator_inc);
	TEST_ASSERT(data[0] == 2);
	TEST_ASSERT(result == 0);
	TEST_ASSERT(queue_length(q) == 9);

	// Null parameters
	TEST_ASSERT(queue_iterate(q, NULL) == -1);
	TEST_ASSERT(queue_iterate(NULL, iterator_inc) == -1);
}

void test_length(void)
{
	fprintf(stderr, "*** TEST length ***\n");

	queue_t q = queue_create();

	TEST_ASSERT(queue_length(q) == 0);

	int a = 1;
	queue_enqueue(q, &a);
	TEST_ASSERT(queue_length(q) == 1);

	int b = 1;
	queue_enqueue(q, &b);
	TEST_ASSERT(queue_length(q) == 2);

	queue_delete(q, &b);
	TEST_ASSERT(queue_length(q) == 1);

	// Null parameters
	TEST_ASSERT(queue_length(NULL) == -1);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void **)&ptr);
	TEST_ASSERT(ptr == &data);
}

int main(void)
{
	// Test individual methods
	test_create();
	test_destroy();
	test_enqueue();
	test_dequeue();
	test_delete();
	test_iterator();
	test_length();

	// scenario tests
	test_queue_simple();

	fprintf(stderr, "\n┬─┬ノ( º _ ºノ) Big Success\n");

	return 0;
}
