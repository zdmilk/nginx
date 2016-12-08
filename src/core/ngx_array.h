
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

/*
 *  ngx_array_t 基本数组结构
 *
 *
 * */

#ifndef _NGX_ARRAY_H_INCLUDED_
#define _NGX_ARRAY_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

/*nginx的数组结构*/
typedef struct {
    void        *elts;      //数组数据区起始位置
    ngx_uint_t   nelts;     //实际存放的元素个数
    size_t       size;      //每个元素大小 
    ngx_uint_t   nalloc;    /*数组的容量。表示该数组在不引发扩容的前提下，可以最多存储的元素的个数。当nelts增长到达nalloc 时，
							如果再往此数组中存储元素，则会引发数组的扩容。数组的i容量将会扩展到原有容量的2倍大小。实际上是分配新的一块内存，
							新的一块内存的大小是原有内存大小的2倍。原有的数据会被拷贝到新的一块内存中*/
    ngx_pool_t  *pool;     //该数组在此内存池中分配
} ngx_array_t;

/*
 *	sizeof(ngx_ayyay_t) = 20   nginx的数组要从内存池中分配。将分配nalloc个大小为size的空间，实际分配的大小为(nalloc * size)
 * */

ngx_array_t *ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size);  //创建数组  
void ngx_array_destroy(ngx_array_t *a);                                   //销毁数组 
// 向数组中添加元素
void *ngx_array_push(ngx_array_t *a);                                     
void *ngx_array_push_n(ngx_array_t *a, ngx_uint_t n);

//初始化数组 
static ngx_inline ngx_int_t
ngx_array_init(ngx_array_t *array, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    /*
     * set "array->nelts" before "array->elts", otherwise MSVC thinks
     * that "array->nelts" may be used without having been initialized
     */

    array->nelts = 0;
    array->size = size;
    array->nalloc = n;
    array->pool = pool;

	/* 分配n*size大小的区域作为数组数据区 */
    array->elts = ngx_palloc(pool, n * size);
    if (array->elts == NULL) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


#endif /* _NGX_ARRAY_H_INCLUDED_ */
