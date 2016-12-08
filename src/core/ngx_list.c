
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


ngx_list_t *
ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    ngx_list_t  *list;

    list = ngx_palloc(pool, sizeof(ngx_list_t));  //从内存池中分配链表头
    if (list == NULL) {
        return NULL;
    }

    if (ngx_list_init(list, pool, n, size) != NGX_OK) {
        return NULL;
    }

    return list; //返回链表头的起始位置 
}


void *
ngx_list_push(ngx_list_t *l)
{
    void             *elt;
    ngx_list_part_t  *last;

    last = l->last; //获得最后一个节点

    if (last->nelts == l->nalloc) {    //链表数据区满 

        /* the last part is full, allocate a new list part */

        last = ngx_palloc(l->pool, sizeof(ngx_list_part_t));   //分配节点（list part）
        if (last == NULL) {
            return NULL;
        }

        last->elts = ngx_palloc(l->pool, l->nalloc * l->size);  //分配该节点(part)的数据区
        if (last->elts == NULL) {
            return NULL;
        }

        last->nelts = 0;
        last->next = NULL;

        l->last->next = last;   //将分配的list part插入链表  
        l->last = last;         //并修改list头的last指针 
    }

    elt = (char *) last->elts + l->size * last->nelts;  //计算下一个数据在链表数据区中的位置  
    last->nelts++;   //实际存放的数据个数加1 

    return elt;     //返回该位置 
}
