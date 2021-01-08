#ifndef __BALANCED_BST_H__
#define __BALANCED_BST_H__

#include "bst.h"

#include <functional>

template <typename T, typename Compare = std::less<T>>
class balanced_bst : public bst<T, Compare>
{
  private:
    using bst_node = typename bst<T, Compare>::bst_node;
    using bst = bst<T, Compare>;

  public:
    balanced_bst() = default;
    balanced_bst(const balanced_bst& source);
    balanced_bst(balanced_bst&& source);

  protected:
    void left_rotate(bst_node* node);
    void right_rotate(bst_node* node);

    virtual void fixup_insert(bst_node* node) = 0;
    virtual void fixup_erase(bst_node* node) = 0;
};

template <typename T, typename Compare>
balanced_bst<T, Compare>::balanced_bst(const balanced_bst& source)
    : bst(source)
{
}

template <typename T, typename Compare>
balanced_bst<T, Compare>::balanced_bst(balanced_bst&& source)
    : bst(source)
{
}

template <typename T, typename Compare>
void balanced_bst<T, Compare>::left_rotate(bst_node* node)
{
    bst_node* child  = node->right;
    node->right      = child->left;
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

template <typename T, typename Compare>
void balanced_bst<T, Compare>::right_rotate(bst_node* node)
{
    bst_node* child  = node->left;
    node->left       = child->right;
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
