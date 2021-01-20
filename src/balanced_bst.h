#ifndef __BALANCED_BST_H__
#define __BALANCED_BST_H__

#include "bst.h"

#include <functional>
#include <memory>

template <typename T, typename Compare = std::less<T>,
          typename Allocator = std::allocator<T>>
class balanced_bst : public bst<T, Compare, Allocator>
{
  private:
    using bst_node = typename bst<T, Compare, Allocator>::bst_node;
    using bst      = bst<T, Compare, Allocator>;

  public:
    balanced_bst() = default;
    balanced_bst(balanced_bst&& source);

  protected:
    void left_rotate(bst_node* node);
    void right_rotate(bst_node* node);

    virtual void fixup_insert(bst_node* node) = 0;
    virtual void fixup_erase(bst_node* node)  = 0;
};

template <typename T, typename Compare, typename Allocator>
balanced_bst<T, Compare, Allocator>::balanced_bst(balanced_bst&& source)
    : bst(source)
{
}

template <typename T, typename Compare, typename Allocator>
void balanced_bst<T, Compare, Allocator>::left_rotate(bst_node* node)
{
    bst_node* child = node->right;
    node->right     = child->left;
    if (child->left)
        child->left->parent = node;
    child->parent = node->parent;
    if (!node->parent)
        bst::root = child;
    else if (node == node->parent->left)
        node->parent->left = child;
    else
        node->parent->right = child;
    child->left  = node;
    node->parent = child;
}

template <typename T, typename Compare, typename Allocator>
void balanced_bst<T, Compare, Allocator>::right_rotate(bst_node* node)
{
    bst_node* child = node->left;
    node->left      = child->right;
    if (child->right)
        child->right->parent = node;
    child->parent = node->parent;
    if (!node->parent)
        bst::root = child;
    else if (node == node->parent->right)
        node->parent->right = child;
    else
        node->parent->left = child;
    child->right = node;
    node->parent = child;
}

#endif
