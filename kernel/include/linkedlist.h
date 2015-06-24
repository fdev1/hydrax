#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__
#include <kheap.h>

/*
 * helper macros to generate type names
 */
#define LINKED_LIST_STRUCT(type)			struct __##type##_linked_list
#define LINKED_LIST_ITEM_STRUCT(type)		struct __##type##_linked_list_item
#define LINKED_LIST_ITER_STRUCT(type)		struct __##type##_linked_list_iter
#define LINKED_LIST_TYPE(type)			__##type##_linked_list_t
#define LINKED_LIST_ITEM_TYPE(type)		__##type##_linked_list_item_t
#define LINKED_LIST_ITER_TYPE(type)		__##type##_linked_list_iter_t

/*
 * Append an item to the end of the list
 */
#define LINKED_LIST_APPEND(type, list, item)		\
{										\
	(item).next = 0;						\
	(list).last->next = (item);				\
	(list).last = (item);					\
	(list).count++;							\
}



/*
 * declare of the types needed for a linked list of
 * type <type>
 */
#define DEFINE_LINKED_LIST_TYPE(type) 			\
typedef LINKED_LIST_ITEM_STRUCT(type)		\
{									\
	type *val;						\
	LINKED_LIST_ITEM_STRUCT(type) *next;	\
}									\
LINKED_LIST_ITEM_TYPE(type);				\
									\
typedef LINKED_LIST_STRUCT(type)			\
{									\
	LINKED_LIST_ITEM_STRUCT(type) *first;	\
	LINKED_LIST_ITEM_STRUCT(type) *last;	\
	LINKED_LIST_ITEM_STRUCT(type) *current;	\
	LINKED_LIST_ITEM_STRUCT(type) *scratch;	\
	unsigned int count;					\
}									\
LINKED_LIST_TYPE(type);					\
									\
typedef LINKED_LIST_ITER_STRUCT(type)		\
{									\
	LINKED_LIST_ITEM_STRUCT(type) *current;	\
}									\
LINKED_LIST_ITER_TYPE(type);				\
									\
static inline LINKED_LIST_ITEM_TYPE(type) *__##type##_linked_list_append_new ( 	\
	LINKED_LIST_STRUCT(type) *list, type *item)						\
{															\
	list->scratch = (void*) malloc( \
		(uint32_t) sizeof(LINKED_LIST_ITEM_STRUCT(type)));									\
	if (unlikely(list->scratch != NULL))										\
	{																	\
		*(list->scratch) = LINKED_LIST_ITEM_INIT(type);							\
		list->scratch->val = item;											\
		list->scratch->next = 0;						\
		list->last->next = list->scratch;				\
		list->last = list->scratch;					\
		if (list->first == NULL || list->current == NULL) \
		{										\
			list->first = list->scratch;						\
			list->current = list->scratch;					\
		}										\
		list->count++;								\
	}																	\
	return list->scratch;													\
}																		\
static inline void __##type##_linked_list_remove_first_free(						\
	LINKED_LIST_STRUCT(type) *list)											\
{																		\
	list->scratch = list->first;												\
	list->first = list->scratch->next;											\
	free(list->scratch);													\
}

#define LINKED_LIST_ITEM_INIT(type)		(LINKED_LIST_ITEM_STRUCT(type)) { (void*)0, (void*)0 }
#define LINKED_LIST_INIT(type)			((LINKED_LIST_STRUCT(type)) { (void*) 0, (void*) 0, (void*) 0 })
#define LINKED_LIST_ITER_INIT(type, list)	(LINKED_LIST_ITER_STRUCT(type)) { (list).first }

/*
 * Move the list (or iterator) to the next item
 * and return the value of the new current item
 */
#define LINKED_LIST_MOVE_NEXT(list)		\
	((list).current = (list).current->next, ((list).current == NULL) ? NULL : (list).current->val)

/*
 * Move the list (or iterator) to the first item
 * and return it's value
 */
#define LINKED_LIST_MOVE_FIRST(list)		\
	((list).current = (list).first, ((list).current == NULL) ? NULL : (list).current->val)


/*
 * move the list to the last item and return
 * it's value
 */
#define LINKED_LIST_MOVE_LAST(list)		\
	((list).current = (list).last, ((list).current == NULL) ? NULL : (list).current->val)

#define LINKED_LIST_INSERT(type, list, item)		\
{										\
	(item).next = (list.current);				\
	(list).current = (item);					\
	(list).count++;						\
}

/*
 * Allocate a linked_list_item object and append it
 * to the linked list.
 * 
 * NOTE: This macro is NOT thread-safe
 */
#define LINKED_LIST_APPEND_NEW(type, list, item)	__##type##_linked_list_append_new(&(list), (item))
#define LINKED_LIST_REMOVE_FIRST_FREE(type, list)	__##type##_linked_list_remove_first_free(&(list))
/*
 * remove the current item from the list.
 * 
 * NOTE: This macro is NOT thread safe
 */
#define LINKED_LIST_REMOVE_CURRENT(list, iter)	\
(										\
	(list).scratch = (iter).current, 			\
	(iter).current = (iter).current->next, 		\
	(list).first = (unlikely((list).first == (list).scratch)) ? (iter).current : (list).first,	\
	(list).last = (unlikely((list).last == (list).scratch)) ? (iter).current : (list).last,		\
	(list).count--,							\
	(list).scratch							\
)

/*
 * Remove the current item the from list and free it.
 */
#define LINKED_LIST_REMOVE_CURRENT_FREE(list, iter)	\
{											\
	free(LINKED_LIST_REMOVE_CURRENT(list, iter));	\
}

/*
 * move the list forward until the condition is met
 */
#define LINKED_LIST_SEEK_UNTIL(list, cond)		\
	while ((list).current != NULL)			\
	{									\
		if ((cond))						\
			break;						\
		LINKED_LIST_MOVE_NEXT(list);			\
	}

#define DECLARE_LINKED_LIST(type, name)		extern LINKED_LIST_TYPE(type) name
#define DEFINE_LINKED_LIST(type, name)		LINKED_LIST_TYPE(type) name = LINKED_LIST_INIT(type)
#define DEFINE_LINKED_LIST_ITEM(type, name)	LINKED_LIST_ITEM_TYPE(type) name
	
#endif
