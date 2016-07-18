/**
 * @file linkedList.c
 * @author Adrian Rasmussen
 *
 * @brief Provides set of functions to manage a linked list, with {@link pa.h} as the data
 * @see error.h which creates the ERROR_CODE enum used as return values
 * @version 15/05/2013
 * @since 05/05/2013
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "linkedList.h"
#include "error.h"

/**
 * Method:    newLinkedList
 * @brief     allocates space for a linked list
 * @param 	  list - address of place to save linked list to
 * @return    ERROR_CODE enum value: E_SUCCESS if successful
 *									 E_OUT_OF_MEMORY if malloc failed
 **/
ERROR_CODE newLinkedList( LinkedList **list )
{
	(*list) = (LinkedList *) malloc(sizeof **list);
	if ( (*list) == NULL)
	{
		printf("Could not allocate memory for list\n");
		return E_OUT_OF_MEMORY;
	}
	(*list)->head = NULL;
	(*list)->tail = NULL;

	return E_SUCCESS;
}

/**
 * Method:    enqueue
 * @brief     Adds the PA to the end of the linked list
 * @param 	  list - Existing list of PAs, must not be NULL
 * @param 	  data - Pointer to the PA
 * @return    ERROR_CODE enum value: E_SUCCESS if successful
 *									 E_INVALID_INPUT if list is NULL
 *									 E_OUT_OF_MEMORY if malloc failed
 **/
ERROR_CODE enqueue( LinkedList *list, Pa *data )
{
	LinkedListNode *newNode;

	assert( list != NULL);
	if ( list == NULL)
	{
		return E_INVALID_INPUT;
	}
	newNode = (LinkedListNode*) malloc( sizeof *newNode);
	if ( newNode == NULL)
	{
		printf("Could not malloc newNode\n");
		return E_OUT_OF_MEMORY;
	}

	newNode->data = data;
	newNode->next = NULL;

	if ( list->head == NULL) /*Empty list*/
	{
		list->head = newNode;
		list->tail = newNode;
	}
	else
	{
		list->tail->next = newNode;
		list->tail = newNode;
	}

	return E_SUCCESS;
}

/**
 * Method:    dequeue
 * @brief     Removes the first element from the list, saving pointer to data but freeing node
 * @param 	  list - linked list, must not be NULL
 * @param 	  data - location to save data to
 * @return    ERROR_CODE enum value: E_SUCCESS if successful
 *									 E_EMPTY_LIST if no elements in list, but list itself is not NULL
 **/
ERROR_CODE dequeue( LinkedList *list, Pa **data)
{
	LinkedListNode *temp;

	assert(list != NULL);
	if ( list->head == NULL) /*Empty list*/
	{
		return E_EMPTY_LIST;
	}

	/*Retrieve PA and remove node*/
	temp = list->head;
	*data = temp->data;
	list->head = list->head->next;
	free(temp);

	if ( list->head == NULL) /*The list is now empty*/
	{
		list->tail = NULL;
	}

	return E_SUCCESS;
}

/**
 * Method:    deleteLinkedList
 * @brief     Deallocates memory for the list, including any data if still allocated
 * @param 	  list - pointer to allocated linked list
 * @return    ERROR_CODE enum value: E_SUCCESS 
 **/
ERROR_CODE deleteLinkedList( LinkedList *list )
{
	LinkedListNode *current, *previous;

	/*If already NULL, don't need to deallocate*/
	if (list != NULL)
	{
		current = list->head;

		while ( current != NULL)
		{
			previous = current;
			current = current->next;

			free(previous->data->times);
			free(previous->data);
			free(previous);
			previous = NULL;
		}
		free(list);
		list = NULL;
	}

	return E_SUCCESS;
}

/**
 * Method:    isEmpty
 * @brief     Checks if the list is empty, by seeing if the head points to NULL
 * @param 	  list - Pointer to the linked list, must not be NULL
 * @return    1 if empty, 0 otherwise
 **/
int isEmpty(LinkedList *list)
{
	return (list->head == NULL);
}