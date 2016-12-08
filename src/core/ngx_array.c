
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


ngx_array_t *
ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size)
{
    ngx_array_t *a;
	/* 为结构体分配空间(数组头)*/
    a = ngx_palloc(p, sizeof(ngx_array_t));
    if (a == NULL) {
        return NULL;
    }

    if (ngx_array_init(a, p, n, size) != NGX_OK) {
        return NULL;
    }

    return a; //返回数组头的起始位置 
}

/*销毁数组的操作实现如下，包括销毁数组数据区和数组头。这里的销毁动作实际上就是修改内存池的last指针，并没有调用free等释放内存的操作，维护效率高*/
void
ngx_array_destroy(ngx_array_t *a)
{
    ngx_pool_t  *p;

    p = a->pool;
	
	/*销毁顺序不能换,先销毁数据区，再销毁数组结构体*/
    if ((u_char *) a->elts + a->size * a->nalloc == p->d.last) {  //先销毁数组数据区
        p->d.last -= a->size * a->nalloc;    //设置内存池的last指针
    }

    if ((u_char *) a + sizeof(ngx_array_t) == p->d.last) {    //接着销毁数组头
        p->d.last = (u_char *) a;  //接着销毁数组头
    }
}
/*
 *向数组添加元素的操作有两个，ngx_array_push和ngx_array_push_n，分别添加一个和多个元素
 *实际的添加操作并不在这两个函数中完成
 *例如: ngx_array_push返回可以在该数组数据区中添加这个元素的位置，ngx_array_push_n则返回可以在该数组数据区中添加n个元素的起始位置
 *而添加操作即在获得添加位置之后进行
 * */

void *
ngx_array_push(ngx_array_t *a)
{
    void        *elt, *new;
    size_t       size;
    ngx_pool_t  *p;

    if (a->nelts == a->nalloc) {

        /* the array is full */

        size = a->size * a->nalloc;  // 数据区大小

        p = a->pool;

        if ((u_char *) a->elts + size == p->d.last  //若内存池的last指针指向数组数据区的末尾
            && p->d.last + a->size <= p->d.end)    // 且内存池未使用的空间能够分配一个数组中单个元素大小的空间
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */

            p->d.last += a->size; // 分配数组一个元素的大小的空间
            a->nalloc++;   //数组的容量加1

        } else {
            /* allocate a new array */

            new = ngx_palloc(p, 2 * size);  //扩展数组数据区为原来的2倍  
            if (new == NULL) {
                return NULL;
            }

            ngx_memcpy(new, a->elts, size);  //将原来数据区的内容拷贝到新的数据区
            a->elts = new;  
            a->nalloc *= 2;  //注意：此处转移数据后，并未释放原来的数据区，内存池将统一释放
        }
    }

    elt = (u_char *) a->elts + a->size * a->nelts;  //elt指向数组数据区中存在的数据的末尾 
    a->nelts++;  //实际存放的元素个数加1

    return elt;   //返回该末尾指针，即下一个元素应该存放的位置
}
/*
 *向数组中添加元素实际上也是在修该内存池的last指针(若数组数据区满)及数组头信息，
 *即使数组满了，需要扩展数据区内容，也只需要内存拷贝完成，并不需要数据的移动操作，效率高
 * */

void *
ngx_array_push_n(ngx_array_t *a, ngx_uint_t n)
{
    void        *elt, *new;
    size_t       size;
    ngx_uint_t   nalloc;
    ngx_pool_t  *p;

    size = n * a->size;

    if (a->nelts + n > a->nalloc) {

        /* the array is full */

        p = a->pool;

        if ((u_char *) a->elts + a->size * a->nalloc == p->d.last
            && p->d.last + size <= p->d.end)
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */

            p->d.last += size;
            a->nalloc += n;

        } else {
            /* allocate a new array */

            nalloc = 2 * ((n >= a->nalloc) ? n : a->nalloc);

            new = ngx_palloc(p, nalloc * a->size);
            if (new == NULL) {
                return NULL;
            }

            ngx_memcpy(new, a->elts, a->nelts * a->size);
            a->elts = new;
            a->nalloc = nalloc;
        }
    }

    elt = (u_char *) a->elts + a->size * a->nelts;
    a->nelts += n;

    return elt;
}
