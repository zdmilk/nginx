
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_LIST_H_INCLUDED_
#define _NGX_LIST_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
/*  nginx容器—链表 */

typedef struct ngx_list_part_s  ngx_list_part_t;

//链表节点结构
struct ngx_list_part_s {
    void             *elts;     //指向该节点实际的数据区(该数据区中可以存放nalloc个大小为size的元素)
    ngx_uint_t        nelts;    //实际存放的元素个数 
    ngx_list_part_t  *next;     //指向下一个节点 
};

//链表头结构 
typedef struct {
    ngx_list_part_t  *last;    //指向链表最后一个节点(part)
    ngx_list_part_t   part;    //链表头中包含的第一个节点(part)
    size_t            size;    //每个元素大小
    ngx_uint_t        nalloc;  //链表所含空间个数，即实际分配的小空间的个数 
    ngx_pool_t       *pool;    //该链表节点空间在此内存池中分配
} ngx_list_t;

// sizeof(ngx_list_t)=28，sizeof(ngx_list_part_t)=12
// nginx的链表也要从内存池中分配。对于每一个节点(list part)将分配nalloc个大小为size的小空间，实际分配的大小为(nalloc * size)

ngx_list_t *ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size);

static ngx_inline ngx_int_t
ngx_list_init(ngx_list_t *list, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    list->part.elts = ngx_palloc(pool, n * size);  //分配n*size大小的区域作为链表数据区
    if (list->part.elts == NULL) {
        return NGX_ERROR;
    }

    list->part.nelts = 0;
    list->part.next = NULL;
    list->last = &list->part;
    list->size = size;
    list->nalloc = n;
    list->pool = pool;

    return NGX_OK;
}


/*
 *
 *  the iteration through the list:
 *
 *  part = &list.part;
 *  data = part->elts;
 *
 *  for (i = 0 ;; i++) {
 *
 *      if (i >= part->nelts) {
 *          if (part->next == NULL) {
 *              break;
 *          }
 *
 *          part = part->next;
 *          data = part->elts;
 *          i = 0;
 *      }
 *
 *      ...  data[i] ...
 *
 *  }
 */


void *ngx_list_push(ngx_list_t *list);


#endif /* _NGX_LIST_H_INCLUDED_ */
