/**
 * @file list.h
 * @author ylp
 * @brief Refer to the Linux kernel source code include/linux/list.h (5.10.0)
 * @version 0.1
 * @date 2021-12-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef LIST_H
#define LIST_H

#include "kernel.h"
#include "compiler_attributes.h"
#include "types.h"
#include "poison.h"

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 * Static initialization
 */

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

/**
 * INIT_LIST_HEAD - Initialize a list_head structure
 * @list: list_head structure to be initialized.
 *
 * Initializes the list_head to point to itself.  If it is a list header,
 * the result is an empty list.
 * Dynamic initialization
 */
static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

/*
 * Adds a new node between two neighboring nodes after they are known  
 * (Private function)
 */
static inline void _list_add_between(struct list_head *new_node, struct list_head *prev, struct list_head *next)
{
    next->prev = new_node;
    new_node->next = next;
    new_node->prev = prev;
    prev->next = new_node;
}

/*
 * Adds a new node to the head of the list
 */
static inline void list_add(struct list_head *new_node, struct list_head *head)
{
	_list_add_between(new_node, head, head->next);
}

/*
 * Adds a new node to the end of the list
 */
static inline void list_add_tail(struct list_head *new_node, struct list_head *head)
{
    _list_add_between(new_node, head->prev, head);
}

/*
 * For the node to be deleted, if two adjacent nodes are known, you can do this by connecting the two nodes across the node
 */
static inline void _list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    WRITE_ONCE(prev->next, next);
}

/*
 * Deletes a linked list node entry
 */
static inline void _list_del_entry(struct list_head *entry)
{
    _list_del(entry->prev, entry->next);
}

/*
 * Deletes the specified element from the linked list
 */
static inline void list_del(struct list_head *entry)
{
    _list_del_entry(entry);
    entry->next = (struct list_head *)LIST_POISON1;
    entry->prev = (struct list_head *)LIST_POISON2;
}

/*
 * Replaces the specified element in a linked list
 */
static inline void list_replace(struct list_head *old_node, struct list_head *new_node)
{
    new_node->next = old_node->next;
    new_node->next->prev = new_node;
    new_node->prev = old_node->prev;
    new_node->prev->next = new_node;
}

/*
 * Replaces the specified element in the list and points the front and back of the old element to itself (initialization) 
 */
static inline void list_replace_init(struct list_head *old_node, struct list_head *new_node)
{
    list_replace(old_node, new_node);
    INIT_LIST_HEAD(old_node);
}

/*
 * Swap two nodes in a linked list
 * Node2 replaces node1, and then joins node1 where node2 was 
 */
static inline void list_swap(struct list_head *node1, struct list_head *node2)
{
    struct list_head *ptr = node2->prev;
    list_del(node2);
    list_replace(node1,node2);
    if (ptr == node1)
        ptr = node2;

    list_add(node1,ptr);
}

/*
 * Remove a node from the original list and place it at the head of another list 
 */
static inline void list_move(struct list_head *entry, struct list_head *head)
{
    _list_del_entry(entry);
    list_add(entry,head);
}

/*
 * Remove a node from the original list and place it at the end of another list
 */
static inline void list_move_tail(struct list_head *entry, struct list_head *head)
{
    _list_del_entry(entry);
    list_add_tail(entry,head);
}

/*
 * Adds a list to the end of the new list
 */
static inline void list_bulk_move_tail(struct list_head *head, struct list_head *first, struct list_head *last)
{
    // Remove from the original list
    first->prev->next = last->next;
    last->next->prev = first->prev;

    // Adds this segment to the new list
    head->prev->next = first;
    first->prev = head->prev;
    last->next = head;
    head->prev = last;
}

/*
 * Determines whether the given node is the first element in the list
 */
static inline bool list_is_first(const struct list_head *entry, const struct list_head *head)
{
    return entry->prev == head;
}

/*
 * Determines whether the given element is a tail element
 */
static inline bool list_is_last(const struct list_head *entry, const struct list_head *head)
{
    return entry->next == head;
}

/*
 * Determines whether the given linked list is empty
 */
static inline bool list_is_empty(const struct list_head *head)
{
    return READ_ONCE(head->next) == head;
}

/*
 * The left-hand list
 */
static inline void list_rotate_left(struct list_head *head)
{
	struct list_head *first;

	if (!list_is_empty(head)) {
		first = head->next;
		list_move_tail(first, head);
	}
}

/*
 * Places the specified element at the end of the list (or "front" of the list) 
 */
static inline void list_rotate_to_front(struct list_head *entry, struct list_head *head)
{
    list_move_tail(entry, head);
}

/*
 * Determines whether a given linked list has only one element
 */
static inline bool list_is_singular(struct list_head *head)
{
    return !list_is_empty(head) && (head->next == head->prev);
}

static inline void _list_cut_position(struct list_head *list,struct list_head *head, struct list_head *entry)
{
	struct list_head *new_first = entry->next;
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = new_first;
	new_first->prev = head;
}

/*
 * Adds the element after the specified position in the head list to the list. The list is cleared 
 */
static inline void list_cut_position(struct list_head *list, struct list_head *head, struct list_head *entry)
{
	if (list_is_empty(head))
		return;
	if (list_is_singular(head) &&
		(head->next != entry && head != entry))
		return;
	if (entry == head)
		INIT_LIST_HEAD(list);
	else
		_list_cut_position(list, head, entry);
}

/*
 * Add the element before the head position to the list, and the list is cleared 
 */
static inline void list_cut_before(struct list_head *list, struct list_head *head, struct list_head *entry)
{
    if (head->next == entry) {
        INIT_LIST_HEAD(list);
        return;
    }
    list->next = head->next;
    list->next->prev = list;
    list->prev = entry->prev;
    list->prev->next = list;
    head->next = entry;
    entry->prev = head;
}

static inline void _list_splice(const struct list_head *list,
				 struct list_head *prev,
				 struct list_head *next)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/*
 * Join two linked lists. List will be added after head
 */
static inline void list_splice(const struct list_head *list, struct list_head *head)
{
    if(!list_is_empty(list))
        _list_splice(list, head, head->next);
}

/*
 * Join two linked lists list will be added before head
 */
static inline void list_splice_tail(const struct list_head *list, struct list_head *head)
{
    if(!list_is_empty(list))
        _list_splice(list, head->prev, head);

}

/*
 * Join two lists,list will be added after head,list will be reinitialized 
 */
static inline void list_splice_init(struct list_head *list, struct list_head *head)
{
    if (!list_is_empty(list)) {
        _list_splice(list, head, head->next);
        INIT_LIST_HEAD(list);
    }
}

/*
 * Join two lists list will be added before the list is reinitialized 
 */
static inline void list_splice_tail_init(struct list_head *list, struct list_head *head)
{
    if (!list_is_empty(list)) {
        _list_splice(list, head->prev, head);
        INIT_LIST_HEAD(list);
    }
}

/*
 * Gets the structure in which this list entry resides
 * PTR: pointing &struct list_head
 * Type: structure type of the list head
 * Member: The member name of the list head in the structure
 */
#define list_entry(ptr,type,member) \
        container_of(ptr,type,member)

/*
 * Gets the first element in the list
 */
#define list_first_entry(ptr,type,member) \
        list_entry(((ptr)->next),type,member)

/*
 * Gets the last element in the list
 */
#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

/*
 * Gets the first element of the list, or NULL if the list is empty
 */
#define list_first_entry_or_null(ptr, type, member) ({ 		  \
	struct list_head *head__ = (ptr); 						  \
	struct list_head *pos__ = READ_ONCE(head__->next); 		  \
	pos__ != head__ ? list_entry(pos__, type, member) : NULL; \
})

/*
 * Gets the next element
 */
#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

/*
 * Gets the previous element
 */
#define list_prev_entry(pos, member) \
	list_entry((pos)->member.prev, typeof(*(pos)), member)

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_continue - continue iteration over a list
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 *
 * Continue to iterate over a list, continuing after the current position.
 */
#define list_for_each_continue(pos, head) \
	for (pos = pos->next; pos != (head); pos = pos->next)


/**
 * list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * list_for_each_safe - Safe iteration list, that is, when iterating with this macro, objects of the current iteration can be removed 
 * @pos:	the &struct list_head to use as a loop cursor.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define list_for_each_safe(pos, n, head) 					\
	for (pos = (head)->next, n = pos->next; pos != (head);  \
		pos = n, n = pos->next)

/**
 * list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop cursor.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define list_for_each_prev_safe(pos, n, head) 	\
	for (pos = (head)->prev, n = pos->prev; 	\
	     pos != (head); 						\
	     pos = n, n = pos->prev)

/**
 * list_entry_is_head - test if the entry points to the head of the list
 * @pos:	the type * to cursor
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define list_entry_is_head(pos, head, member)				\
	(&pos->member == (head))

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define list_for_each_entry(pos, head, member)					\
	for (pos = list_first_entry(head, typeof(*pos), member);	\
	     !list_entry_is_head(pos, head, member);				\
	     pos = list_next_entry(pos, member))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define list_for_each_entry_reverse(pos, head, member)			\
	for (pos = list_last_entry(head, typeof(*pos), member);		\
	     !list_entry_is_head(pos, head, member); 				\
	     pos = list_prev_entry(pos, member))

/**
 * list_prepare_entry - prepare a pos entry for use in list_for_each_entry_continue()
 * @pos:	the type * to use as a start point
 * @head:	the head of the list
 * @member:	the name of the list_head within the struct.
 *
 * Prepares a pos entry for use as a start point in list_for_each_entry_continue().
 */
#define list_prepare_entry(pos, head, member) \
	((pos) ? : list_entry(head, typeof(*pos), member))

/**
 * list_for_each_entry_continue - continue iteration over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define list_for_each_entry_continue(pos, head, member) 	\
	for (pos = list_next_entry(pos, member);				\
	     !list_entry_is_head(pos, head, member);			\
	     pos = list_next_entry(pos, member))

/**
 * list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define list_for_each_entry_continue_reverse(pos, head, member)		\
	for (pos = list_prev_entry(pos, member);						\
	     !list_entry_is_head(pos, head, member);					\
	     pos = list_prev_entry(pos, member))
/**
 * list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define list_for_each_entry_from(pos, head, member) 			\
	for (; !list_entry_is_head(pos, head, member);				\
	     pos = list_next_entry(pos, member))

/**
 * list_for_each_entry_from_reverse - iterate backwards over list of given type
 *                                    from the current point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate backwards over list of given type, continuing from current position.
 */
#define list_for_each_entry_from_reverse(pos, head, member)		\
	for (; !list_entry_is_head(pos, head, member);				\
	     pos = list_prev_entry(pos, member))


/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_first_entry(head, typeof(*pos), member),	\
		n = list_next_entry(pos, member);						\
	     !list_entry_is_head(pos, head, member); 				\
	     pos = n, n = list_next_entry(n, member))


/**
 * list_for_each_entry_safe_continue - continue list iteration safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define list_for_each_entry_safe_continue(pos, n, head, member) 		\
	for (pos = list_next_entry(pos, member), 							\
		n = list_next_entry(pos, member);								\
	     !list_entry_is_head(pos, head, member);						\
	     pos = n, n = list_next_entry(n, member))

/**
 * list_for_each_entry_safe_from - iterate over list from current point safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define list_for_each_entry_safe_from(pos, n, head, member) 			\
	for (n = list_next_entry(pos, member);								\
	     !list_entry_is_head(pos, head, member);						\
	     pos = n, n = list_next_entry(n, member))

/**
 * list_for_each_entry_safe_reverse - iterate backwards over list safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define list_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = list_last_entry(head, typeof(*pos), member),			\
		n = list_prev_entry(pos, member);							\
	     !list_entry_is_head(pos, head, member); 					\
	     pos = n, n = list_prev_entry(n, member))

/**
 * list_safe_reset_next - reset a stale list_for_each_entry_safe loop
 * @pos:	the loop cursor used in the list_for_each_entry_safe loop
 * @n:		temporary storage used in list_for_each_entry_safe
 * @member:	the name of the list_head within the struct.
 *
 * list_safe_reset_next is not safe to use in general if the list may be
 * modified concurrently (eg. the lock is dropped in the loop body). An
 * exception to this is if the cursor element (pos) is pinned in the list,
 * and list_safe_reset_next is called after re-taking the lock and before
 * completing the current iteration of the loop body.
 */
#define list_safe_reset_next(pos, n, member)				\
	n = list_next_entry(pos, member)

#endif /* LIST_H */