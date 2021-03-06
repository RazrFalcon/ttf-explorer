#ifndef TTFCORE_H
#define TTFCORE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct ttfcore_tree ttfcore_tree;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

bool ttfcore_parse_data(const char *data, int32_t len, ttfcore_tree **tree);
void ttfcore_free_tree(ttfcore_tree *tree);
const char *ttfcore_tree_warnings(const ttfcore_tree *tree, uintptr_t *len);
uint32_t ttfcore_tree_item_parent(const ttfcore_tree *tree, uint32_t id);
uint32_t ttfcore_tree_item_child_at(const ttfcore_tree *tree, uint32_t parent_id, uint32_t row);
uint32_t ttfcore_tree_item_child_index(const ttfcore_tree *tree, uint32_t id);
uint32_t ttfcore_tree_item_children_count(const ttfcore_tree *tree, uint32_t id);
bool ttfcore_tree_item_has_children(const ttfcore_tree *tree, uint32_t id);
const char *ttfcore_tree_item_title(const ttfcore_tree *tree, uint32_t id, uintptr_t *len);
uint8_t ttfcore_tree_item_title_type(const ttfcore_tree *tree, uint32_t id);
int ttfcore_tree_item_index(const ttfcore_tree *tree, uint32_t id);
const char *ttfcore_tree_item_value(const ttfcore_tree *tree, uint32_t id, uintptr_t *len);
uint8_t ttfcore_tree_item_value_type(const ttfcore_tree *tree, uint32_t id);
void ttfcore_tree_item_range(const ttfcore_tree *tree, uint32_t id, uint32_t *start, uint32_t *end);
void ttfcore_tree_collect_ranges(const ttfcore_tree *tree, void *data, void (*p)(void*, uint32_t, uint32_t));
uint32_t ttfcore_tree_item_at_byte(const ttfcore_tree *tree, uint32_t offset);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif /* TTFCORE_H */
