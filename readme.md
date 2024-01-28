# cJSON项目

cJSON项目有四大模版组成：创建模块、打印模块、解析模块、增删改查模块，还有一些小模块：内存管理模块。

## json数据类型

json类型定义如下：

```c++
typedef struct cJSON

{

​    // next指向下一条json数据，prev指向上一条json数据

​    struct cJSON *next,*prev;

​    // 数据对象或者object会在一个键值下有多个实体

​    struct cJSON *child;

​    // json数据类型，值为宏定义

​    int type;

​    // json数据类型为字符串类型，存储字符串

​    char* valuestring;

​    // json数据类型为number类型时，存储数据

​    int valuedoint;

​    double valuedouble;

​    // 键值的名字

​    char* string;

} cJSON;
```

json数据类型结构是树形结构。

## 打印模块

打印模块的顶层函数有三个：

```c++
/* Render a cJSON entity to text for transfer/storage. Free the char* when finished. */
extern char *cJSON_Print(cJSON *item);
/* Render a cJSON entity to text for transfer/storage without any formatting. Free the char* when finished. */
extern char *cJSON_PrintUnformatted(cJSON *item);
/* Render a cJSON entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
extern char *cJSON_PrintBuffered(cJSON *item,int prebuffer,int fmt);
```

这三个函数通过调用静态函数print_value(cJSON* item,int depth,int fmt,printbuffer* p)

*item是指向待解析json数据的顶点节点

depth指深度

fmt是标识符选项，fmt=1是格式化打印，fmt=0是无格式打印

p是缓冲区

## 创建模块

创建模块函数如下：

```c++
#define cJSON_AddNullToObject(object,name)		cJSON_AddItemToObject(object, name, cJSON_CreateNull())
#define cJSON_AddTrueToObject(object,name)		cJSON_AddItemToObject(object, name, cJSON_CreateTrue())
#define cJSON_AddFalseToObject(object,name)		cJSON_AddItemToObject(object, name, cJSON_CreateFalse())
#define cJSON_AddBoolToObject(object,name,b)	cJSON_AddItemToObject(object, name, cJSON_CreateBool(b))
#define cJSON_AddNumberToObject(object,name,n)	cJSON_AddItemToObject(object, name, cJSON_CreateNumber(n))
#define cJSON_AddStringToObject(object,name,s)	cJSON_AddItemToObject(object, name, cJSON_CreateString(s))

/* These calls create a cJSON item of the appropriate type. */
extern cJSON *cJSON_CreateNull(void);
extern cJSON *cJSON_CreateTrue(void);
extern cJSON *cJSON_CreateFalse(void);
extern cJSON *cJSON_CreateBool(int b);
extern cJSON *cJSON_CreateNumber(double num);
extern cJSON *cJSON_CreateString(const char *string);
extern cJSON *cJSON_CreateArray(void);
extern cJSON *cJSON_CreateObject(void);

/* These utilities create an Array of count items. */
extern cJSON *cJSON_CreateIntArray(const int *numbers,int count);
extern cJSON *cJSON_CreateFloatArray(const float *numbers,int count);
extern cJSON *cJSON_CreateDoubleArray(const double *numbers,int count);
extern cJSON *cJSON_CreateStringArray(const char **strings,int count);

/* Append item to the specified array/object. */
extern void cJSON_AddItemToArray(cJSON *array, cJSON *item);
extern void	cJSON_AddItemToObject(cJSON *object,const char *string,cJSON *item);
```

由于json数据类型是树形结构，所以一般是首先调用cJSON_CreateObject()函数，再根据需求调用其他函数。

## 解析模块

解析模块函数如下：

```c++
extern cJSON* cJSON_ParseWithOpts(const char* value,const char** return_parse_end,int require_null_terminated);
extern cJSON* cJSON_Parse(const char* value);
extern const char* cJSON_GetErrorPtr(void);
```

cJSON_Parse()函数调用cJSON_ParseWithOpts()函数，实现json数据解析功能。同时解析过程中，维护一个ep指针，指向解析过程中出错的位置，并且使用cJSON_GetErrorPtr得到ep指针。

其他模块较为简单。

## 引用

[小白的第一个C语言博客项目-cJSON源码详解](https://blog.csdn.net/qq_36160429/article/details/109330528)

[cJSON](https://sourceforge.net/projects/cjson/)