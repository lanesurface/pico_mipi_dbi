
/**
 * ========================
 *          ll.h
 * ========================
 * 
 * Linked List Utility
 * 
 * Author(s): Lane W Surface
 * Created:   2025-09-01
 * License:   MIT
 * 
 * Copyright Surface EP, LLC 2025.
 */

struct ll_node {
  struct ll_node * next;
  void * data;
};

struct linked_list {
  struct ll_node * head, * tail;
};

extern ll_node *
list_get_next (void);

extern struct linked_list
list_make_from_array (void * data, size_t len);

extern void 
list_destroy (linked_list * lst);