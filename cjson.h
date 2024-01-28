// 避免头文件重复引用
#ifndef cjson__h
#define cjson__h

#ifdef __cplusplus
// 兼容C语言
extern "C"
{
#endif

// 定义json数据类型
#define cJSON_False 0
#define cJSON_True 1
#define cJSON_NULL 2
#define cJSON_Number 3
#define cJSON_String 4
#define cJSON_Array 5
#define cJSON_Object 6

#define cJSON_IsReference 256
#define cJSON_StringIsConst 512

typedef struct cJSON
{
    // next指向下一条json数据，prev指向上一条json数据
    struct cJSON *next,*prev;
    // 数据对象或者object会在一个键值下有多个实体
    struct cJSON *child;
    // json数据类型，值为宏定义
    int type;
    // json数据类型为字符串类型，存储字符串
    char* valuestring;
    // json数据类型为number类型时，存储数据
    int valuedoint;
    double valuedouble;
    // 键值的名字
    char* string;
} cJSON;

typedef struct cJSON_Hooks {
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
} cJSON_Hooks;

/* Supply malloc, realloc and free functions to cJSON */
extern void cJSON_InitHooks(cJSON_Hooks* hooks);

#define cJSON_AddNullToObject(object,name)		cJSON_AddItemToObject(object, name, cJSON_CreateNull())
#define cJSON_AddTrueToObject(object,name)		cJSON_AddItemToObject(object, name, cJSON_CreateTrue())
#define cJSON_AddFalseToObject(object,name)		cJSON_AddItemToObject(object, name, cJSON_CreateFalse())
#define cJSON_AddBoolToObject(object,name,b)	cJSON_AddItemToObject(object, name, cJSON_CreateBool(b))
#define cJSON_AddNumberToObject(object,name,n)	cJSON_AddItemToObject(object, name, cJSON_CreateNumber(n))
#define cJSON_AddStringToObject(object,name,s)	cJSON_AddItemToObject(object, name, cJSON_CreateString(s))

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define cJSON_SetIntValue(object,val)			((object)?(object)->valuedoint=(object)->valuedouble=(val):(val))
#define cJSON_SetNumberValue(object,val)		((object)?(object)->valuedoint=(object)->valuedouble=(val):(val))

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

/* Render a cJSON entity to text for transfer/storage. Free the char* when finished. */
extern char *cJSON_Print(cJSON *item);
/* Render a cJSON entity to text for transfer/storage without any formatting. Free the char* when finished. */
extern char *cJSON_PrintUnformatted(cJSON *item);
/* Render a cJSON entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
extern char *cJSON_PrintBuffered(cJSON *item,int prebuffer,int fmt);

extern void cJSON_Delete(cJSON *c);
extern void cJSON_DeleteItemFromArray(cJSON* array,int which);
extern void cJSON_DeleteItemFromObject(cJSON *object,const char* string);
extern cJSON* cJSON_DetachItemFromArray(cJSON *array,int which);
extern cJSON* cJSON_DetachItemFromObject(cJSON *object,const char *string);


extern cJSON* cJSON_ParseWithOpts(const char* value,const char** return_parse_end,int require_null_terminated);

extern cJSON* cJSON_Parse(const char* value);

extern const char* cJSON_GetErrorPtr(void);

extern int cJSON_GetArraySize(cJSON *array);
extern cJSON* cJSON_GetArrayItem(cJSON* array,int item);
extern void cJSON_InsertItemInArray(cJSON* array,int which,cJSON* newitem);
extern void cJSON_ReplaceItemInArray(cJSON* array,int which,cJSON* newitem);
extern void cJSON_ReplaceItemInObject(cJSON* object,const char* string,cJSON* newitem);

extern cJSON *cJSON_Duplicate(cJSON *item,int recurse);

#ifdef __cplusplus
}
#endif

#endif