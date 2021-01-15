#ifndef __RB_TREE_H__
#define __RB_TREE_H__

#include "balanced_bst.h"

#include <functional>
#include <memory>

#ifndef _P_UNUSED_
#define _P_UNUSED_ __attribute__((unsed))
#endif

template <typename T, typename Compare = std::less<T>,
          typename Allocator = std::allocator<T>>
class rb_tree : public balanced_bst<T, Compare, Allocator>
{
  private:
    using bst_node     = typename bst<T, Compare, Allocator>::bst_node;
    using bst          = bst<T, Compare, Allocator>;
    using balanced_bst = balanced_bst<T, Compare, Allocator>;

    using node_color              = bool;
    const static node_color red   = false;
    const static node_color black = true;

    struct rb_node;

  public:
    rb_tree() = default;
    rb_tree(const rb_tree& source);
    rb_tree(rb_tree&& source);
    virtual ~rb_tree() = default;

  private:
    using node_allocator = typename std::allocator_traits<
        Allocator>::template rebind_alloc<rb_node>;
    using alloc_traits = std::allocator_traits<node_allocator>;

    inline rb_node* resolve(bst_node* node);
    inline rb_node* parent(bst_node* node);
    inline rb_node* uncle(bst_node* node);
    inline rb_node* grand_parent(bst_node* node);
    inline node_color& color(bst_node* node);

    virtual rb_node* make_node(const T& data) override;
    virtual rb_node* copy_node(bst_node* node) override;

    virtual void base_insert(bst_node* node) override;
    virtual void fixup_insert(bst_node* node) override;

    inline virtual void erase_single_child_node(bst_node* node,
                                                bst_node* replacement) override;
    inline virtual void erase_double_child_node(bst_node* node,
                                                bst_node* replacement) override;

    virtual void destroy_node(bst_node* node) override;

    virtual void fixup_erase(bst_node* node) override;

    node_allocator alloc;
};

template <typename T, typename Compare, typename Allocator>
struct rb_tree<T, Compare, Allocator>::rb_node : public bst::bst_node {
    node_color color = red;

    rb_node() = default;
    rb_node(const T& data);
    rb_node(const rb_node& source);
};

template <typename T, typename Compare, typename Allocator>
rb_tree<T, Compare, Allocator>::rb_tree(const rb_tree& source)
    : balanced_bst(source)
{
}

template <typename T, typename Compare, typename Allocator>
rb_tree<T, Compare, Allocator>::rb_tree(rb_tree&& source) : balanced_bst(source)
{
}

template <typename T, typename Compare, typename Allocator>
rb_tree<T, Compare, Allocator>::rb_node::rb_node(const T& data)
    : bst::bst_node(data)
{
}

template <typename T, typename Compare, typename Allocator>
rb_tree<T, Compare, Allocator>::rb_node::rb_node(const rb_node& source)
    : bst::bst_node(source), color(source.color)
{
}

template <typename T, typename Compare, typename Allocator>
typename rb_tree<T, Compare, Allocator>::rb_node*
rb_tree<T, Compare, Allocator>::resolve(bst_node* node)
{
    return static_cast<rb_node*>(node);
}

template <typename T, typename Compare, typename Allocator>
typename rb_tree<T, Compare, Allocator>::rb_node*
rb_tree<T, Compare, Allocator>::parent(bst_node* node)
{
    return resolve(node->parent);
}

template <typename T, typename Compare, typename Allocator>
typename rb_tree<T, Compare, Allocator>::rb_node*
rb_tree<T, Compare, Allocator>::uncle(bst_node* node)
{
    if (node->parent == node->parent->parent->left)
        return resolve(node->parent->parent->right);
    else
        return resolve(node->parent->parent->left);
}

template <typename T, typename Compare, typename Allocator>
typename rb_tree<T, Compare, Allocator>::rb_node*
rb_tree<T, Compare, Allocator>::grand_parent(bst_node* node)
{
    return resolve(node->parent->parent);
}

template <typename T, typename Compare, typename Allocator>
typename rb_tree<T, Compare, Allocator>::node_color&
rb_tree<T, Compare, Allocator>::color(bst_node* node)
{
    static node_color null_color = black;

    if (node)
        return resolve(node)->color;
    return null_color;
}

template <typename T, typename Compare, typename Allocator>
typename rb_tree<T, Compare, Allocator>::rb_node*
rb_tree<T, Compare, Allocator>::make_node(const T& data)
{
    rb_node* node = alloc_traits::allocate(alloc, 1);
    alloc_traits::construct(alloc, node, data);
    return node;
}

template <typename T, typename Compare, typename Allocator>
typename rb_tree<T, Compare, Allocator>::rb_node*
rb_tree<T, Compare, Allocator>::copy_node(bst_node* node)
{
    rb_node* copy = alloc_traits::allocate(alloc, 1);
    alloc_traits::construct(alloc, copy, *resolve(node));
    return copy;
}

template <typename T, typename Compare, typename Allocator>
void rb_tree<T, Compare, Allocator>::base_insert(bst_node* node)
{
    bst::base_insert(node);
    fixup_insert(node);
}

template <typename T, typename Compare, typename Allocator>
void rb_tree<T, Compare, Allocator>::fixup_insert(bst_node* node)
{
    while (color(parent(node)) == red) {
        bool case_2 = node == parent(node)->left;

        auto rotate_2 = &rb_tree::right_rotate;
        auto rotate_3 = &rb_tree::left_rotate;

        if (parent(node) == grand_parent(node)->left) {
            case_2   = node == parent(node)->right;
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

    color(bst::root) = black;
}

template <typename T, typename Compare, typename Allocator>
void rb_tree<T, Compare, Allocator>::erase_single_child_node(
    bst_node* node, bst_node* replacement)
{
    bst::erase_single_child_node(node, replacement);
    if (color(node) == black)
        fixup_erase(replacement);
}

template <typename T, typename Compare, typename Allocator>
void rb_tree<T, Compare, Allocator>::erase_double_child_node(
    bst_node* node, bst_node* replacement)
{
    bst_node* to_fixup = replacement->right;
    if (replacement->parent == node && to_fixup)
        to_fixup->parent = replacement;
    bst::erase_double_child_node(node, replacement);
    node_color replacement_color = color(replacement);
    color(replacement)           = color(node);
    if (replacement_color == black)
        fixup_erase(to_fixup);
}

template <typename T, typename Compare, typename Allocator>
void rb_tree<T, Compare, Allocator>::destroy_node(bst_node* node)
{
    rb_node* resolved = resolve(node);
    alloc_traits::destroy(alloc, resolved);
    alloc_traits::deallocate(alloc, resolved, 1);
}

template <typename T, typename Compare, typename Allocator>
void rb_tree<T, Compare, Allocator>::fixup_erase(bst_node* node)
{
    while (node && node != bst::root && color(node) == black) {
        if (node == node->parent->left) {
            bst_node* node_uncle = uncle(node);
            if (color(node_uncle) == red) {
                color(node_uncle)   = black;
                color(parent(node)) = red;
                balanced_bst::left_rotate(node->parent);
                node_uncle = uncle(node);
            }
            if (color(node_uncle->left) == black &&
                color(node_uncle->right) == black) {
                color(node_uncle) = red;
                node              = node->parent;
            } else {
                if (color(node_uncle->right) == black) {
                    color(node_uncle->left) = black;
                    color(node_uncle)       = red;
                    balanced_bst::right_rotate(node_uncle);
                    node_uncle = node->parent->right;
                }
                color(node_uncle)   = color(parent(node));
                color(parent(node)) = black;
                balanced_bst::left_rotate(node->parent);
                node = bst::root;
            }
        } else {
            bst_node* node_uncle = uncle(node);
            if (color(node_uncle) == red) {
                color(node_uncle)   = black;
                color(parent(node)) = red;
                balanced_bst::right_rotate(node->parent);
                node_uncle = uncle(node);
            }
            if (color(node_uncle->left) == black &&
                color(node_uncle->right) == black) {
                color(node_uncle) = red;
                node              = node->parent;
            } else {
                if (color(node_uncle->left) == black) {
                    color(node_uncle->right) = black;
                    color(node_uncle)        = red;
                    balanced_bst::left_rotate(node_uncle);
                    node_uncle = node->parent->left;
                }
                color(node_uncle)   = color(parent(node));
                color(parent(node)) = black;
                balanced_bst::right_rotate(node->parent);
                node = bst::root;
            }
        }
    }
    color(node) = black;
}

#endif
