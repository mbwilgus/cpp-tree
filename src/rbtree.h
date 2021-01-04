#ifndef __RB_TREE_H__
#define __RB_TREE_H__

#include "tree.h"

#include <functional>
#include <type_traits>

#ifndef _P_UNUSED_
#define _P_UNUSED_ __attribute__((unsed))
#endif

template <typename T, typename Compare = std::less<T>>
class rb_tree : public bst<T, Compare>
{
  private:
    using bst_node = typename bst<T, Compare>::node_base;
    using bst      = bst<T, Compare>;

    using node_color              = bool;
    const static node_color red   = false;
    const static node_color black = true;

    struct rb_node;

    inline rb_node* resolve(bst_node* node);
    inline rb_node* parent(bst_node* node);
    inline rb_node* uncle(bst_node* node);
    inline rb_node* grand_parent(bst_node* node);
    inline node_color& color(rb_node* node);

    void fixup_insert(bst_node* node);

  public:
    using value_type     = typename bst::value_type;
    using iterator       = typename bst::iterator;
    using const_iterator = typename bst::const_iterator;

    rb_tree() = default;
    rb_tree(const rb_tree& source);
    rb_tree(rb_tree&& source);

    virtual iterator insert(const_iterator pos, const T& data);
    virtual iterator insert(const T& data);
};

template <typename T, typename Compare>
struct rb_tree<T, Compare>::rb_node : public bst::node_base {
    node_color color = red;

    rb_node() = default;
    rb_node(const T& data);
    rb_node(const rb_node& source);
};

template <typename T, typename Compare>
typename rb_tree<T, Compare>::rb_node*
rb_tree<T, Compare>::resolve(bst_node* node)
{
    return static_cast<rb_node*>(node);
}

template <typename T, typename Compare>
typename rb_tree<T, Compare>::rb_node*
rb_tree<T, Compare>::parent(bst_node* node)
{
    return resolve(node->parent);
}

template <typename T, typename Compare>
typename rb_tree<T, Compare>::rb_node*
rb_tree<T, Compare>::uncle(bst_node* node)
{
    if (node->parent == node->parent->parent->left)
        return resolve(node->parent->parent->right);
    else
        return resolve(node->parent->parent->left);
}

template <typename T, typename Compare>
typename rb_tree<T, Compare>::rb_node*
rb_tree<T, Compare>::grand_parent(bst_node* node)
{
    return resolve(node->parent->parent);
}

template <typename T, typename Compare>
typename rb_tree<T, Compare>::node_color&
rb_tree<T, Compare>::color(rb_node* node)
{
    static node_color null_color = black;

    if (node)
        return node->color;
    return null_color;
}

template <typename T, typename Compare>
void rb_tree<T, Compare>::fixup_insert(bst_node* node)
{
    while (color(parent(node)) == red) {
        bool case_2 = node == parent(node)->left;

        auto rotate_2 = &rb_tree::right_rotate;
        auto rotate_3 = &rb_tree::left_rotate;

        if (parent(node) == grand_parent(node)->left) {
            case_2 = node == parent(node)->right;
            std::swap(rotate_2, rotate_3);
        }

        if (color(uncle(node)) == red) {
            color(parent(node))       = black;
            color(uncle(node))        = black;
            color(grand_parent(node)) = red;

            node = grand_parent(node);
        } else {
            if (case_2) {
                node = parent(node);
                (*this.*rotate_2)(node);
            }
            color(parent(node))       = black;
            color(grand_parent(node)) = red;
            (*this.*rotate_3)(grand_parent(node));
        }
    }

    color(resolve(bst::root())) = black;
}

template <typename T, typename Compare>
rb_tree<T, Compare>::rb_tree(const rb_tree& source) : bst(source)
{
}

template <typename T, typename Compare>
rb_tree<T, Compare>::rb_tree(rb_tree&& source) : bst(source)
{
}

template <typename T, typename Compare>
typename rb_tree<T, Compare>::iterator
rb_tree<T, Compare>::insert(_P_UNUSED_ const_iterator pos, const T& data)
{
    return insert(data);
}

template <typename T, typename Compare>
typename rb_tree<T, Compare>::iterator
rb_tree<T, Compare>::insert(const T& data)
{
    rb_node* node = new rb_node{data};
    bst::base_insert(node);
    fixup_insert(node);
    return iterator{node};
}

template <typename T, typename Compare>
rb_tree<T, Compare>::rb_node::rb_node(const T& data) : bst::node_base(data)
{
}

template <typename T, typename Compare>
rb_tree<T, Compare>::rb_node::rb_node(const rb_node& source)
    : bst::node_base(source)
{
}

#endif
