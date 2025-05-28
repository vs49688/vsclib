/*
 * vsclib
 * https://{github.com,codeberg.org}/vs49688/vsclib
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) 2021 Zane van Iperen (zane@zanevaniperen.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef VSCLIB_ALLOCATOR_INTERNAL_HPP_
#define VSCLIB_ALLOCATOR_INTERNAL_HPP_

#include <type_traits>
#include <cstdint>
#include <vsclib/resource.h>

/**
 * \brief ResNode is a quick'n'dirty linked-list inspired by BRender's br_simple_list.
 *
 * \remark I will eventually do an equivalent C void version, of which this should be replaced with.
 */
template <typename T>
struct ResNode {
    T  *next;
    T **prev;
};

template <typename T>
using is_res_node_t = std::enable_if_t<std::is_base_of_v<ResNode<T>, T>>;

template <typename T /*, typename = is_res_node_t<T>*/>
struct ResList {
    T *head;
};

template <typename T, typename = is_res_node_t<T>>
static bool resnode_inserted(ResNode<T> *n)
{
    return n->prev != nullptr;
}

template <typename T, typename = is_res_node_t<T>>
static T *resnode_head(ResList<T> *list)
{
    return list->head;
}

template <typename T, typename = is_res_node_t<T>>
static T *resnode_next(ResNode<T> *n)
{
    return n->next;
}

template <typename T, typename = is_res_node_t<T>>
static T *resnode_remove(T *node)
{
    *(node->prev) = node->next;

    if(node->next)
        node->next->prev = node->prev;

    node->prev = nullptr;
    node->next = nullptr;

    return node;
}

template <typename T, typename = is_res_node_t<T>>
static void resnode_add_head(ResList<T> *list, ResNode<T> *node)
{
    node->next = list->head;
    node->prev = &list->head;

    if(node->next)
        node->next->prev = &node->next;

    list->head = static_cast<T *>(node);
}

/**
 * \brief The base header. THIS MUST BE AT THE _END_ OF EVERY OTHER HEADER.
 */
typedef struct BaseHeader {
    size_t size;
    union {
        struct {
            size_t align_power : 8;
            size_t is_resource : 1;
            size_t in_free : 1;
            size_t reserved : VSC_SIZE_T_BITSIZE - 10;
        };
        size_t _pad;
    };
    uintptr_t sig;
} BaseHeader;

/**
 * \brief The standard header. This only exists so it can be used by code that needs "hdr->base"
 */
struct MemHeader {
    constexpr static uintptr_t kSignature = 0xFEED5EEDFEED5EEDu; /* Formerly Chuck's */

    BaseHeader base;
};
static_assert(std::is_trivially_copyable_v<MemHeader>);

/**
 * \brief The resource header.
 */
struct ResourceHeader : ResNode<ResourceHeader> {
    constexpr static uintptr_t kSignature = 0xDEADBABEDEADBABE;

    ResList<ResourceHeader>   children;
    VscResourceDestructorProc destructor;
    BaseHeader                base;
};
static_assert(std::is_trivially_copyable_v<ResourceHeader>);

#endif /* VSCLIB_ALLOCATOR_INTERNAL_HPP_ */
