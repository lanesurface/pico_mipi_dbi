
/**
 * ========================
 *          ll.h
 * ========================
 * 
 * Linked List Utility
 * 
 * Author(s): Lane W Surface
 * Created:   2025-01-09
 * License:   MIT
 * 
 * Copyright Surface EP, LLC 2025.
 */

struct ll_node {
  struct ll_node * next, * prev;
  void * data;
};

struct linked_list {
  struct ll_node * head, * tail;
};

extern struct ll_node *
list_get_next (void);

extern struct linked_list
list_make_from_array (void * data, size_t len);

extern ssize_t
list_find_ll_item (void * data);

extern void 
list_destroy (struct linked_list * lst);