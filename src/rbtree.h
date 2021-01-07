#ifndef __RB_TREE_H__
#define __RB_TREE_H__

#include "tree.h"

#include <functional>

#ifndef _P_UNUSED_
#define _P_UNUSED_ __attribute__((unsed))
#endif

template <typename T, typename Compare = std::less<T>>
class rb_tree : public bst<T, Compare>
{
  private:
    using bst_node = typename bst<T, Compare>::bst_node;
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

    virtual rb_node* make_node(const T& data) override;

    virtual void base_insert(bst_node* node) override;
    void fixup_insert(bst_node* node);

    inline virtual bst_node* erase_case_no_left(bst_node* node) override;
    inline virtual bst_node* erase_case_no_right(bst_node* node) override;
    inline virtual void erase_case_parent_of_successor(bst_node* temp, bst_node* successor) override;
    inline virtual bst_node* erase_case_two_child(bst_node* node) override;
    virtual void base_erase(bst_node* node) override;

  public:
    using value_type     = typename bst::value_type;
    using iterator       = typename bst::iterator;
    using const_iterator = typename bst::const_iterator;

    rb_tree() = default;
    rb_tree(const rb_tree& source);
    rb_tree(rb_tree&& source);
};

template <typename T, typename Compare>
struct rb_tree<T, Compare>::rb_node : public bst::bst_node {
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
typename rb_tree<T, Compare>::rb_node* rb_tree<T, Compare>::make_node(const T& data)
{
    return new rb_node{data};
}

template <typename T, typename Compare>
void rb_tree<T, Compare>::base_insert(bst_node* node)
{
    bst::base_insert(node);
    fixup_insert(node);
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
            rotate_2 = &rb_tree::left_rotate;
            rotate_3 = &rb_tree::right_rotate;
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
typename rb_tree<T, Compare>::bst_node* rb_tree<T, Compare>::erase_case_no_left(bst_node* node)
{
    bst::erase_case_no_left(node);
    return node->right;
}

template <typename T, typename Compare>
typename rb_tree<T, Compare>::bst_node* rb_tree<T, Compare>::erase_case_no_right(bst_node* node)
{
    bst::erase_case_no_right(node);
    return node->left;
}

template <typename T, typename Compare>
void rb_tree<T, Compare>::erase_case_parent_of_successor(bst_node* temp, bst_node* successor)
{
    temp->parent = successor;
}

template <typename T, typename Compare>
typename rb_tree<T, Compare>::bst_node* rb_tree<T, Compare>::erase_case_two_child(bst_node* node)
{
    bst_node* successor = bst::subtree_min(node);
    node_color succ_color = color(resolve(successor));
    bst_node* succ_right = successor->right;
    if (successor->parent == node)
        erase_case_parent_of_successor(succ_right, successor);
    else
        bst::erase_case_not_parent(node, successor);

    successor->left = node->left;
    successor->left->parent = successor;
    color(resolve(successor)) = color(resolve(node));
    color(resolve(node)) = succ_color;

    return succ_right;
}

template <typename T, typename Compare>
void rb_tree<T, Compare>::base_erase(bst_node* node)
{
    bst_node* to_fix;
    if (!node->left)
        to_fix = erase_case_no_left(node);
    else if (!node->right)
        to_fix = erase_case_no_right(node);
    else
        to_fix = erase_case_two_child(node);
    if (color(resolve(node)) == black)
        /* delete_fixup(to_fix); */

    delete node;
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
rb_tree<T, Compare>::rb_node::rb_node(const T& data) : bst::bst_node(data)
{
}

template <typename T, typename Compare>
rb_tree<T, Compare>::rb_node::rb_node(const rb_node& source)
    : bst::bst_node(source)
{
}

#endif
