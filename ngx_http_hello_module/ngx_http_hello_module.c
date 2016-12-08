#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
	ngx_str_t hello_string;
	ngx_int_t hello_counter;
}ngx_http_hello_loc_conf_t;

static ngx_int_t ngx_http_hello_init(ngx_conf_t *cf);
static void *ngx_http_hello_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_hello_string(ngx_conf_t *cf, ngx_command_t *cmd,void *conf);
static char *ngx_http_hello_counter(ngx_conf_t *cf,ngx_command_t *cmd,void *conf);

/* 模块配置指令*/
static ngx_command_t ngx_http_hello_commands[] = {
	{
		ngx_string("hello_string"), // 配置指令的名称
		NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS|NGX_CONF_TAKE1, //配置的类型
		ngx_http_hello_string,   //设置命令初始函数
		NGX_HTTP_LOC_CONF_OFFSET,
		offsetof(ngx_http_hello_loc_conf_t,hello_string),
		NULL
	},

	{
		ngx_string("hello_counter"),
		NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
		ngx_http_hello_counter,
		NGX_HTTP_LOC_CONF_OFFSET,
		offsetof(ngx_http_hello_loc_conf_t,hello_counter),
		NULL
	},
	ngx_null_command
}; //实例化命令数组，以ngx_null_command结尾

static int ngx_hello_visited_times = 0;
static ngx_http_module_t ngx_http_hello_module_ctx = {
	NULL,                 // preconfiguration 在创建和读取该模块的配置信息之前被调用
	ngx_http_hello_init,  // postconfiguration 在创建和读取该模块的配置信息之后被调用
	NULL,                 // create main configuration
	NULL,                 // init main configuration
	NULL,                 // create server configuration
	NULL,                 // merge server configuration
	ngx_http_hello_create_loc_conf,  // create location configuration
	NULL                  //merge location configuration
};//实例化模块上下文结构

ngx_module_t ngx_http_hello_module = {
	NGX_MODULE_V1,
	&ngx_http_hello_module_ctx, // 主要的作用是分配内存空间和对parent进行继承，为指令参数的提取最好准备
	ngx_http_hello_commands,  //主要来对指令进行定义和规范，如有没有带参数，并转化读入指令传进来的参数，然后将合适的值保存到配置结构体（自己定义的一个结构）
	NGX_HTTP_MODULE,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NGX_MODULE_V1_PADDING
}; // 模块信息

static ngx_int_t ngx_http_hello_handler(ngx_http_request_t *r)
{
	ngx_int_t rc;
	ngx_buf_t *b;
	ngx_chain_t out;
	ngx_http_hello_loc_conf_t *my_conf;
	u_char ngx_hello_string[1024] = {0};
	ngx_uint_t content_length = 0;

	ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"ngx_http_hello_handler is called!");
	//获得当前的配置结构体
	my_conf = ngx_http_get_module_loc_conf(r,ngx_http_hello_module);

	if(my_conf->hello_string.len == 0) {
		ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "hello_string is empty!");
		return NGX_DECLINED;
	}

	if(my_conf->hello_counter == NGX_CONF_UNSET || my_conf->hello_counter == 0) {
		ngx_sprintf(ngx_hello_string, "%s", my_conf->hello_string.data);
	} else {
		ngx_sprintf(ngx_hello_string, "%s Visited Times:%d", my_conf->hello_string.data, ++ngx_hello_visited_times);
	}

	ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "hello_string:%s", ngx_hello_string);
	content_length = ngx_strlen(ngx_hello_string);
	 /* we response to 'GET' and 'HEAD' requests only */
	if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
			return NGX_HTTP_NOT_ALLOWED;
	}

	/* discard request body, since we don't need it here */
	rc = ngx_http_discard_request_body(r);
	if (rc != NGX_OK) {
	   return rc;
	}

	/* set the 'Content-type' header */
	ngx_str_set(&r->headers_out.content_type,"text/html");

	/* send the header only, if the request type is http 'HEAD' */

	if (r->method == NGX_HTTP_HEAD) {
		r->headers_out.status = NGX_HTTP_OK;
		r->headers_out.content_length_n = content_length;
		return ngx_http_send_header(r);
	}

	/* allocate a buffer for your response body */
	b = ngx_pcalloc(r->pool,sizeof(ngx_buf_t));
	if (b == NULL) {
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}

	/* attach this buffer to the buffer chain */
	out.buf = b;
	out.next = NULL;

	/* adjust the pointers of the buffer */
	b->pos = ngx_hello_string;
	b->last = ngx_hello_string + content_length;
	b->memory = 1;    /* this buffer is in memory */
	b->last_buf = 1;  /* this is the last buffer in the buffer chain */
	
	/* set the status line */
	r->headers_out.status = NGX_HTTP_OK;
	r->headers_out.content_length_n = content_length;

	/* send the headers of your response  */
	rc = ngx_http_send_header(r);
	if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
	    return rc;
	}

	/* send the buffer chain of your response */
	return ngx_http_output_filter(r, &out);
}

static void *ngx_http_hello_create_loc_conf(ngx_conf_t *cf)
{
	ngx_http_hello_loc_conf_t *loc_conf = NULL;
	loc_conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_hello_loc_conf_t));

	if (loc_conf == NULL)
	{
	   return NULL;
	}

	ngx_str_null(&loc_conf->hello_string);
	loc_conf->hello_counter = NGX_CONF_UNSET;
	return loc_conf;
}
//实现hello_string命令的初始化函数
static char* ngx_http_hello_string(ngx_conf_t *cf, ngx_command_t *cmd,void *conf)
{
	ngx_http_hello_loc_conf_t* local_conf;

	local_conf = conf;
	char *rv = ngx_conf_set_str_slot(cf,cmd,conf);

	ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "hello_string:%s", local_conf->hello_string.data);
	return rv;
}
//实现hello_counter命令的初始化函数
static char* ngx_http_hello_counter(ngx_conf_t *cf, ngx_command_t *cmd,void *conf)
{
	ngx_http_hello_loc_conf_t* local_conf;

	local_conf = conf;
    char* rv = NULL;
    rv = ngx_conf_set_flag_slot(cf, cmd, conf);

    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "hello_counter:%d", local_conf->hello_counter);
	return rv;
}
// 指定命令的真正处理函数
static ngx_int_t ngx_http_hello_init(ngx_conf_t *cf)
{
	ngx_http_handler_pt *h;
	ngx_http_core_main_conf_t  *cmcf;
	cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
	h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
	if (h == NULL){		                   
		return NGX_ERROR;
	}

	*h = ngx_http_hello_handler;

	return NGX_OK;
}
