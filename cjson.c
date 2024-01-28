#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include "cjson.h"

// 使用函数指针，将cJSON_malloc指向malloc函数，从而完成内存申请，
// static设置为静态函数 将其连接属性设置为内部，即仅可在当前文件访问，调用cJSON_malloc等价于调用malloc
static void *(*cJSON_malloc)(size_t sz) = malloc;
//作用同前面的cJSON_malloc，注意函数指针的类型要与指向的函数的类型完全相同，返回值和参数列表
static void (*cJSON_free)(void *ptr) = free;

// 将字符串的内容存放到动态内存指定的位置
static char* cJSON_strdup(const char* str)  // 复制一段字符串
{
    size_t len;
    char* copy;
    // 加1的原因是需要多申请一个字节的长度存放'\0'
    len = strlen(str) + 1;
    if (!(copy = (char*)cJSON_malloc(len))){
        return 0;
    }
    // 从str中赋值len长度的内容，会自动在copy后面加'\0',而strncpy则需要手动赋字符串末尾为'\0'
    memcpy(copy,str,len);
    return copy;
}

static void suffix_object(cJSON *prev,cJSON* item){
    prev->next = item;
    item->prev = prev;
}

void cJSON_InitHooks(cJSON_Hooks* hooks){
    if (!hooks)
    {
        cJSON_malloc = malloc;
        cJSON_free = free;
        return;
    }
    // 如果定义了，自使用自定义的内存管理函数
    cJSON_malloc = (hooks->malloc_fn)?hooks->malloc_fn:malloc;
	cJSON_free	 = (hooks->free_fn)?hooks->free_fn:free;
}

// 下面是创建基本类型节点的方法
// 创建一个节点
static cJSON *cJSON_New_Item(void){
    // 申请分配内存
    cJSON* node = (cJSON*)cJSON_malloc(sizeof(cJSON));
    if (node) // 如果内存分配成功，则初始化节点
    {
        memset(node,0,sizeof(cJSON));
    }
    return node;
}

// 创建一个空类型的节点
cJSON* cJSON_CreateNull(void){
    // 所有创建json节点的底层逻辑都是先创建一个空内容的节点
    cJSON *item = cJSON_New_Item();
    if (item)
    {
        // 指定节点的类型
        item->type = cJSON_NULL;
    }
    return item;
}

// 创建一个bool节点
cJSON* cJSON_CreateBool(int b){
    cJSON* item = cJSON_New_Item();
    if (item)
    {
        item->type = b ? cJSON_True : cJSON_False;
    }
    return item;
}

cJSON* cJSON_CreateTrue(void){
    return cJSON_CreateBool(1);
}

cJSON* cJSON_CreateFalse(void){
    return cJSON_CreateBool(0);
}

cJSON* cJSON_CreateNumber(double num){
    cJSON* item = cJSON_New_Item();
    if (item)
    {
        item->type = cJSON_Number;
        item->valuedouble = num;
        item->valuedoint = (int) num;
    }
    return item;
}

cJSON* cJSON_CreateString(const char* string){
    cJSON* item = cJSON_New_Item();
    if (item)
    {
        item->type = cJSON_String;
        item->valuestring = cJSON_strdup(string);
    }
    return item;
}

cJSON* cJSON_CreateArray(void){
    cJSON* item = cJSON_New_Item();
    if (item)
    {
        item->type = cJSON_Array;
    }
    return item;
}

cJSON* cJSON_CreateObject(void){
    cJSON* item = cJSON_New_Item();
    if (item)
    {
        item->type = cJSON_Object;
    }
    return item;
}

/* Create Arrays: */
cJSON *cJSON_CreateIntArray(const int *numbers, int count)
{
    int i;
    cJSON *n = 0, *p = 0, *a = cJSON_CreateArray();
    for (i = 0; a && i < count; i++)
    {
        n = cJSON_CreateNumber(numbers[i]);
        if (!i)
            a->child = n;
        else
            suffix_object(p, n);
        p = n;
    }
    return a;
}
cJSON *cJSON_CreateFloatArray(const float *numbers, int count)
{
    int i;
    cJSON *n = 0, *p = 0, *a = cJSON_CreateArray();
    for (i = 0; a && i < count; i++)
    {
        n = cJSON_CreateNumber(numbers[i]);
        if (!i)
            a->child = n;
        else
            suffix_object(p, n);
        p = n;
    }
    return a;
}
cJSON *cJSON_CreateDoubleArray(const double *numbers, int count)
{
    int i;
    cJSON *n = 0, *p = 0, *a = cJSON_CreateArray();
    for (i = 0; a && i < count; i++)
    {
        n = cJSON_CreateNumber(numbers[i]);
        if (!i)
            a->child = n;
        else
            suffix_object(p, n);
        p = n;
    }
    return a;
}
cJSON *cJSON_CreateStringArray(const char **strings, int count)
{
    int i;
    cJSON *n = 0, *p = 0, *a = cJSON_CreateArray();
    for (i = 0; a && i < count; i++)
    {
        n = cJSON_CreateString(strings[i]);
        if (!i)
            a->child = n;
        else
            suffix_object(p, n);
        p = n;
    }
    return a;
}

// 添加子节点 object为根节点，string为子节点键，item为子节点
void cJSON_AddItemToObject(cJSON *object,const char *string,cJSON *item){
    if (!item)
    {
        return;
    }
    // 判断子节点的键是否在之前已经指向了一块内存
    if (item->string)
    {
        // 如果已经分配过内存，释放此段内存
        cJSON_free(item->string);
    }
    // 为string分配一段内存，item->string 指向这段内存
    item->string = cJSON_strdup(string);
    // 调用cJSON_AddItemToArray向root添加子节点。
    cJSON_AddItemToArray(object,item);
}

void cJSON_AddItemToArray(cJSON* array,cJSON* item){
    cJSON *c = array->child;
    if (!item)
    {
        return;
    }
    if (!c)
    {
        array->child = item;
    }else{
        while (c && c->next)
        {
            c = c->next;
        }
        suffix_object(c,item);   
    }
}

// printbuffer结构体，主要用来支持格式化打印的时候进行动态内存的分配
typedef struct 
{
    // 字符串指针
    char* buffer;
    // 长度
    int length;
    // 位置偏置
    int offset;
} printbuffer;

/*返回 一个比x大的n(其中n是2的幂),并且是最小的幂，说白了就是将一个数后边所有的位都置1然后再+1*/
//比如输入1返回2，输入3返回4，输入5返回8，输入9返回16，输入17返回32，比当前数大的最小2^n,这个函数的目的是支持动态内存的分配
static int pow2gt(int x){
    --x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

// 个人认为：这个函数的目的是为buffer申请内存，由于计算机是使用2进制，所以使用pow2gt函数来确定申请空间的大小
// 以下是参看csdn博主的说法
// 这里的目的是为了在格式化输出内容的时候，分配更多的动态内存用来存储换行符、制表符等奇怪的字符
/* ensure 函数是一个协助 printbuffer 分配内存的一个函数  
 * len 表示当前字符串的字符串起始偏移量 即 newbuffer+p->offset 起始的
 */
static char* ensure(printbuffer *p,int needed){
    char *newbuffer;
    int newsize;
    //传入参数合法性检测
    if (!p || p->buffer)
    {
        return 0;
    }
    //需要额外分配的内存 也就是偏移量
    needed += p->offset;
    //内存够用直接返回
    if (needed <= p->length)
    {
        return p->buffer + p->offset;
    }
    //内存不够用，申请malloc出新内存 放buffer里面的内容
    newsize = pow2gt(needed);
    newbuffer = (char*)cJSON_malloc(newsize);
    if (!newbuffer)
    {
        cJSON_free(p->buffer);
        p->length = 0;
        p->buffer = 0;
        return 0;
    }
    cJSON_free(p->buffer);
    p->length = newsize;
    p->buffer = newbuffer;
    return newbuffer + p->offset;
}

// 将给的数字类型结构体转换为字符串存储到p缓冲区中，若p为null就申请空间
static int update(printbuffer *p)
{
	char *str;
	if (!p || !p->buffer) return 0;
	str=p->buffer+p->offset;
	return p->offset+strlen(str);
}

// 打印键值对中，值的内容为数字的cJSON结构体
static char* print_number(cJSON* item,printbuffer* p){
    char *str = 0;
    double d = item->valuedouble;
    // 对于是0的字符，直接分配内存
    if (d == 0)
    {
        if (p)
        {
            // 分配两个字节的内存的原因是因为需要存储'\0'字符
            str = ensure(p,2);
        }else{
            // 同上
            str = (char*)cJSON_malloc(2);
        }
        if (str)
        {
            // 分配成功，将数据存储到指定的内存
            strcpy(str,"0");
        }
    // 浮点数和其对应的整型值小于一个非常小的数，可以把它看成整数
    }else if (fabs(((double)item->valuedoint) - d) <= DBL_EPSILON &&
              d <= INT_MAX && d >= INT_MIN)
    {
        if (p)
        {
            //int型数据，最大长度为21个字符
            str = ensure(p,21);
        }else{
            str = (char*)cJSON_malloc(21);
        }
        if (str)
        {
            // 将valueint中的数字变为字符串存储到str中
            sprintf(str,"%d",item->valuedoint);
        }
    // 若是浮点数，则申请更大的内存
    }else{
        if (p)
        {
            str = ensure(p,64);
        }else{
            str = (char*)cJSON_malloc(64);
        }
        if (str)
        {
            // fabs求浮点数的绝对值,当floor(d)与d的差距很小的时候，只打印floor(d)部分，小数点以下占0位、
            if (fabs(floor(d) - d) <= DBL_EPSILON && fabs(d) < 1.0e60)
            {
                sprintf(str,"%.0f",d);
            // 当数非常小或非常大的时候（使用小数形式难表示的时候）按照e格式的科学计数法的方式输出
            }else if (fabs(d) < 1.0e-6 || fabs(d) > 1.0e9)
            {
                sprintf(str,"%e",d);
            //	否则按照平时的打印方式打印即可
            }else{
                sprintf(str,"%f",d);
            }
        }
    }
    return str;
}

//打印字符串类型的结点
static char* print_string_ptr(const char* str,printbuffer* p){
    const char *ptr;
    char* ptr2,*out;
    int len = 0,flag = 0;
    unsigned char token;
    // 如果传入的字符串是空的，则存储字符串标志\"和\"
    if (!str)
    {
        if (p)
        {
            out = ensure(p,3);
        }else{
            out = (char*)cJSON_malloc(3);
        }
        if (!out)
        {
            return 0;
        }
        //字符串为空的话存入引号"",两个字符
        strcpy(out,"\"\"");
        return out;
    }
    // 判断字符串是否包含特殊字符
    for (ptr = str;*ptr;ptr++){
        flag |= ((*ptr > 0 && *ptr < 32) || (*ptr == '\"') || (*ptr == '\\')) ? 1 : 0;
    }
    // 若没有特殊字符，则直接在字符串两边加入双引号即可
    if (!flag)
    {
        len = ptr - str;
        if (p)
        {
            out = ensure(p,len + 3);
        }else{
            out = (char*)cJSON_malloc(len + 3);
        }
        if (!out)
        {
            return 0;
        }
        ptr2 = out;
        *ptr2++ = '\"';
        strcpy(ptr2,str);
        ptr2[len] = '\"';
        ptr2[len + 1] = 0;
        // 这里由于ptr2指向out一样的内存，因此当改变ptr2指向的内容后，out的内容也会跟着变
        return out;
    }
    // 指向str
    ptr = str;
    //这里的作用是计算为了存储字符串需要多少个字节的内存，使用len来记录。
    while ((token = *ptr) && ++len)
    {
        // 注意这里在while和if else语句中都进行了对len进行＋的操作，
        // 主要是因为这些奇怪的字符需要加斜杠\,while中的++Len操作就是获取存储\需要的内存数。
        // strchr对字符串中的单个字符进行查找，找到了的话返回指向对应的位置指针，否则返回NULL
        if (strchr("\"\\\b\f\n\r\t", token))
        {
            //这里相当于每找到"\"\\\b\f\n\r\t"中的一个字符，len就加2
            len++;
        }else if (token < 2)
        {
            //这里相当于每找到ASCII为32以内的字符，len就加6
            len += 5;
        }
        //获取下一个字符
        ptr++;
    }
    if (p)
    {
        out = ensure(p,len + 3);
    }else{
        out = (char*)cJSON_malloc(len + 3);
    }
    if (!out)
    {
        return 0;
    }
    ptr2 = out;
    ptr = str;
    *ptr2++ = '\"';
    while (*ptr)
    {
        if ((unsigned char)*ptr > 31 && *ptr != '\"' && *ptr != '\\')
        {
            //常规字符,直接将字符存储到ptr2中
            *ptr2++ = *ptr++;
        }else{
            // 奇怪的字符，一般以是\+字符的组合，故先存储一个'\\'字符
			*ptr2++ = '\\';
			switch (token = *ptr++) {
			case '\\': // 存储'\\'
				*ptr2++ = '\\';
				break;
			case '\"': //存储'\"'
				*ptr2++ = '\"';
				break;
			case '\b'://存储'b'
				*ptr2++ = 'b';
				break;
			case '\f'://存储'f'
				*ptr2++ = 'f';
				break;
			case '\n'://存储'n'
				*ptr2++ = 'n';
				break;
			case '\r'://存储'r'
				*ptr2++ = 'r';
				break;
			case '\t'://存储't'
				*ptr2++ = 't';
				break;
			default:
				sprintf(ptr2, "u%04x", token);
                //奇怪的字符，除了存储'\\',还需再移动五个位置，存储它
				ptr2 += 5; 
				break;
			}
        }
    }
    // 在末尾添加字符串结束标志
    *ptr2++ = '\"';
    *ptr2++ = 0;
    return out;
}

static char* print_string(cJSON* item,printbuffer* p){
    return print_string_ptr(item->valuestring,p);
}

/* Predeclare these prototypes. */
static const char *parse_value(cJSON *item,const char *value);
static char *print_value(cJSON *item,int depth,int fmt,printbuffer *p);
static const char *parse_array(cJSON *item,const char *value);
static char *print_array(cJSON *item,int depth,int fmt,printbuffer *p);
static const char *parse_object(cJSON *item,const char *value);
static char *print_object(cJSON *item,int depth,int fmt,printbuffer *p);

// 无格式打印
char* cJSON_Print(cJSON* item){
    return print_value(item,0,1,0);
}

// 格式化打印
char* cJSON_PrintUnformatted(cJSON* item){
    return print_value(item,0,0,0);
}

// 使用buffer打印
char *cJSON_PrintBuffered(cJSON *item,int prebuffer,int fmt){
    printbuffer p;
    p.buffer = (char*)cJSON_malloc(prebuffer);
    p.length = prebuffer;
    p.offset = 0;
    return print_value(item,0,fmt,&p);
    return p.buffer;
}

static char* print_value(cJSON* item,int depth,int fmt,printbuffer* p){
    char* out = 0;
    // 判断传入结点是否存在
    if (!item)
    {
        return 0;
    }
    if (p)
    {
        switch ((item->type) & 255)
        {
        case cJSON_NULL:{
            out = ensure(p,5);
            if (out)
            {
                strcpy(out,"null");
            }
            break;
        }
        case cJSON_False:{
            out = ensure(p,6);
            if (out)
            {
                strcpy(out,"false");
            }
            break;
        }
        case cJSON_True:{
            out = ensure(p,5);
            if (out)
            {
                strcpy(out,"true");
            }
            break;
        }
        case cJSON_Number:
            out = print_number(item,p);
            break;
        case cJSON_String:
            out = print_string(item,p);
            break;
        case cJSON_Array:
            out = print_array(item,depth,fmt,p);
            break;
        case cJSON_Object:
            out = print_object(item,depth,fmt,p);
            break;
        default:
            break;
        }
    }else{
        switch ((item->type) & 255)
        {
        case cJSON_NULL:
			out = cJSON_strdup("null"); // cJSON_strdup调用malloc分配相应的内存存储"null"
			break;
		case cJSON_False:
			out = cJSON_strdup("false");// cJSON_strdup调用malloc分配相应的内存存储"false"
			break;
		case cJSON_True:
			out = cJSON_strdup("true");// cJSON_strdup调用malloc分配相应的内存存储"true"
			break;
		case cJSON_Number:
			out = print_number(item, 0);  // 将item中的值的数字转化为字符串，并分配内存
			break;
		case cJSON_String:
			out = print_string(item, 0);// 分配动态内存，将item中值的内容存放到指定的位置
			break;
		case cJSON_Array:  // 这里会进入子层，这边代码讲解的时候默认depth和fmt均为0
			out = print_array(item, depth, fmt, 0);
			break;
		case cJSON_Object: // 这里会进入子层，边代码讲解的时候默认depth和fmt均为0
			out = print_object(item, depth, fmt, 0);
			break;
		}
    }
    return out;
}

// 处理结点类型为array的结构
static char* print_array(cJSON* item,int depth,int fmt,printbuffer* p){
    // 存储array中的字符串内容
    char** entries;
    char* out = 0,*ptr,*ret;
    int len = 5;
    // 进入子节点，array类似于object自身的结点只是一个空壳子，其子节点存储内容
    cJSON *child = item->child;
    //  numentries记录子节点的个数
    int numertries = 0,i = 0,fail = 0;
    size_t tmplen = 0;

    /* 查看数组里有多少个元素 */
    while (child)
    {
        numertries++;
        //子节点之间通过next连接
        child = child->next;
    }
    /* 明确处理numentries == 0 */
    // 处理空数组
    // 如果不含有子节点，即只含有一个空壳子，至少也要打印一个“[]”
    if (!numertries)
    {
        if (p)
        {
            // 格式化输出的时候，调用的内存申请方式，一般申请更多的内存
            out = ensure(p,3);
        }else{
            // 申请三个字节的内存，存储这个空壳子
            out = (char*)cJSON_malloc(3);
        }
        if (out)
        {
            // 存储内容
            strcpy(out,"[]");
        }
        return out;
    }
    // 当含有子节点时的处理过程
    // 这一部分和else处理的目的相同，只不过考虑了格式化输出有更多的细节考虑
    if (p)
    {
        /* 组成数组的输出形式 */
        i = p->offset;
        ptr = ensure(p,1);
        if (!ptr)
        {
            return 0;
        }
        *ptr = '[';
        p->offset++;
        child = item->child;
        while (child && !fail)
        {
            print_value(child,depth + 1,fmt,p);
            p->offset = update(p);
            if (child->next)
            {
                len = fmt ? 2 : 1;
                ptr = ensure(p,len + 1);
                if (!ptr)
                {
                    return 0;
                }
                *ptr++ = ',';
                if (fmt)
                {
                    *ptr++ = ' ';
                }
                *ptr = 0;
                p->offset += len;
            }
            child = child->next;
        }
        ptr = ensure(p,2);
        if (!ptr)
        {
            return 0;
        }
        *ptr++ = ']';
        *ptr = 0;
        out = (p->buffer) + i;
    }else{
        entries = (char**) cJSON_malloc(numertries * sizeof(char*));
        if (!entries)
        {
            return 0;
        }
        memset(entries,0,numertries * sizeof(char*));
        child = item->child;
        while (child && !fail)
        {
            ret = print_value(child,depth + 1,fmt,0);
            entries[i++] = ret;
            if (ret)
            {
                len += strlen(ret) + 2 + (fmt ? 1 : 0);
            }else{
                fail = 1;
            }
            child = child->next;
        }
        if (!fail)
        {
            out = (char*)cJSON_malloc(len);
        }
        if (!out)
        {
            fail = 1;
        }
        if (fail)
        {
            for(int i = 0;i < numertries;i++){
                if (entries[i])
                {
                    cJSON_free(entries[i]);
                }
                cJSON_free(entries);
                return 0;
            }
        }
        *out = '[';
        ptr = out + 1;
        *ptr = 0;
        for (int i = 0;i < numertries;i++){
            tmplen = strlen(entries[i]);
            memcpy(ptr,entries[i],tmplen);
            ptr += tmplen;
            if (i != numertries - 1)
            {
                *ptr++ = ',';
                if (fmt)
                {
                    *ptr++ = ' ';
                }
                *ptr = 0;
            }
            cJSON_free(entries[i]);
            
        }
        cJSON_free(entries);
        *ptr++ = ']';
        *ptr++ = 0;
    }
    return out;
}

/* 打印object类型结点. */
// 这个函数有递归调用，每到一个type类型是cJSON_Object，即类型值为6的话，就会进入递归从而获得子节点的键与值object
static char* print_object(cJSON* item, int depth, int fmt,  printbuffer* p)
{
    // 保存每一个结点数据的内容，entries存储的是"键",names存储的是对应的值，注意这里的指针为两重指针
	char **entries = 0, **names = 0;
    // out为输出的字符串，其它为辅助字符串
	char *out = 0, *ptr, *ret, *str;  
	int len = 7, i = 0, j;
	cJSON *child = item->child;
    // numentries用来统计含有多少子节点，主要为了给后面存储字符串申请内容
	int numentries = 0, fail = 0; 
	size_t tmplen = 0;
 
	/* 统计有多少个子结点. */
	while (child){
		numentries++;
        child = child->next;
    }
	/* 明确处理空对象的情况*/
    //空的时候至少也要存储'{','}'和'\0'
	if (!numentries) { 
		if (p)
			out = ensure(p, fmt ? depth + 4 : 3);
		else
			out = (char *) cJSON_malloc(fmt ? depth + 4 : 3);
		if (!out)
			return 0;
		ptr = out;
		*ptr++ = '{';
		if (fmt) {
			*ptr++ = '\n';
			for (i = 0; i < depth; i++)
				*ptr++ = '\t';
		}
		*ptr++ = '}';
		*ptr++ = 0;
		return out;// 当object不含有子节点的时候，至少会输出大括号{}
	}
	if (p) {//这里进行一些格式处理细节太多，不看了，知道是调整输出的格式即可
		/* 组成输出形式: */
		i = p->offset;
		len = fmt ? 2 : 1;
		ptr = ensure(p, len + 1);
		if (!ptr)
			return 0;
		*ptr++ = '{';
		if (fmt)
			*ptr++ = '\n';
		*ptr = 0;
		p->offset += len;
		child = item->child;
		depth++;
		while (child) {
			if (fmt) {
				ptr = ensure(p, depth);
				if (!ptr)
					return 0;
				for (j = 0; j < depth; j++)
					*ptr++ = '\t';
				p->offset += depth;
			}
			print_string_ptr(child->string, p);
			p->offset = update(p);
 
			len = fmt ? 2 : 1;
			ptr = ensure(p, len);
			if (!ptr)
				return 0;
			*ptr++ = ':';
			if (fmt)
				*ptr++ = '\t';
			p->offset += len;
 
			print_value(child, depth, fmt, p);
			p->offset = update(p);
 
			len = (fmt ? 1 : 0) + (child->next ? 1 : 0);
			ptr = ensure(p, len + 1);
			if (!ptr)
				return 0;
			if (child->next)
				*ptr++ = ',';
			if (fmt)
				*ptr++ = '\n';
			*ptr = 0;
			p->offset += len;
			child = child->next;
		}
		ptr = ensure(p, fmt ? (depth + 1) : 2);
		if (!ptr)
			return 0;
		if (fmt)
			for (i = 0; i < depth - 1; i++)
				*ptr++ = '\t';
		*ptr++ = '}';
		*ptr = 0;
		out = (p->buffer) + i;
	} else {
		/*为对象和名称分配空间，存储键和值对应的字符串 */
		entries =
		    (char **) cJSON_malloc(numentries * sizeof(char *));
		if (!entries)
			return 0;
		names =
		    (char **) cJSON_malloc(numentries * sizeof(char *));
		if (!names) {  // 出错释放内存
			cJSON_free(entries);
			return 0;
		}
		memset(entries, 0, sizeof(char *) * numentries);  // 指定内容初始化为0
		memset(names, 0, sizeof(char *) * numentries);
 
		/* 将所有结果收集到数组: */
		child = item->child;  //进入子节点
		depth++;
		if (fmt)  // 指定了格式的话，len需要增加，以申请更多的内存
			len += depth;
		while (child && !fail) {   // 如果存储数据的过程中，出现了输出为空的话，则置fail为1，跳出循环
			names[i] = str =
			    print_string_ptr(child->string, 0);  // 存储对应结点的键
			entries[i++] = ret =   // 存储对应结点的值，当child的type为cJSON_Object的时候会进入子节点，处理完后，将其内容放回，放入entries，i++的目的是存储下一个结点的内容，这里不能写成++i；
			    print_value(child, depth, fmt, 0);
			if (str && ret) // 由于后面需要将各个结点的内容，串在一起输出，因此这里计算一下每个子节点的长度，从而计算出后续所需要的动态内存
				len +=
				    strlen(ret) + strlen(str) + 2 +
				    (fmt ? 2 + depth : 0);
			else
				fail = 1;
			child = child->next;// 处理同一级的下一个结点，同一级的结点之间通过next和prev进行相连
		}
 
		if (!fail)  // 存储键值的过程中如果没有出错的话，申请一段动态内存
			out = (char *) cJSON_malloc(len);
		if (!out)  // 动态内存申请不成功的话，依旧设置fail为1，这样就可以进入接下来的if从而释放之前申请的动态内存
			fail = 1;
 
	
		if (fail) {
			for (i = 0; i < numentries; i++) {
				if (names[i]) // 对每个一级指针的内容逐渐释放其内存，仅当其内存的确存在才释放，注意内存释放的顺序
					cJSON_free(names[i]);
				if (entries[i])
					cJSON_free(entries[i]);
			}
			cJSON_free(names);  // 释放存储了一组字符串的动态内存
			cJSON_free(entries);
			return 0;
		}
 
		/* 组成输出形式: */
        // 把names和entries的内容组成字符串输出，同时释放了其对应的动态内存，这样后面仅仅需要释放out指向的动态内存即可   
		*out = '{';
		ptr = out + 1;
		if (fmt)
			*ptr++ = '\n';
		*ptr = 0;
		for (i = 0; i < numentries; i++) { //for循环依次回去键数组和值数组中对应的内容。
			if (fmt)
				for (j = 0; j < depth; j++)
					*ptr++ = '\t';
			tmplen = strlen(names[i]);
			memcpy(ptr, names[i], tmplen);
			ptr += tmplen;
			*ptr++ = ':';
			if (fmt)
				*ptr++ = '\t';
			strcpy(ptr, entries[i]);
			ptr += strlen(entries[i]);
			if (i != numentries - 1)
				*ptr++ = ',';
			if (fmt)
				*ptr++ = '\n';
			*ptr = 0;
			cJSON_free(names[i]);
			cJSON_free(entries[i]);
		}
 
		cJSON_free(names);
		cJSON_free(entries);
		if (fmt)
			for (i = 0; i < depth - 1; i++)
				*ptr++ = '\t';
		*ptr++ = '}';
		*ptr++ = 0;
	}
	return out;
}

// 传入需要删除的root指针，cjOSN结构形式
void cJSON_Delete(cJSON *c)
{
	cJSON *next;
	while (c)
	{
        //通过while循环释放同一层结点的动态内存
		next=c->next; 
		// cJSON_IsReference = 0001 0000 0000,下面的类型按位与均为0
        // 如果当前结点的child成员不为0，说明存在下一层结构（type为object或array），这个时候递归的进入下一层，逐层释放
		if (!(c->type&cJSON_IsReference) && c->child){
            cJSON_Delete(c->child);
        }
        // 释放存储“值”的内容的动态内存
		if (!(c->type&cJSON_IsReference) && c->valuestring){
             cJSON_free(c->valuestring);
        }
        //释放存储“键”的内容的动态内存
		if (!(c->type&cJSON_StringIsConst) && c->string){
             cJSON_free(c->string);
        }
        //释放cJSON结构体的动态内存
		cJSON_free(c);
		c=next;
	}
}

//全局变量，ep指向了出错的字符串，当需要获得出错的字符串的时候，可通过调用函数cJSON_GetErrorPtr()查看
//未初始化全局不可修改指定内存内容的指针,全局常量字符串，内容不可改变，但是指针指向的位置可以改变，间接改变存储的内容
static const char* ep;

const char* cJSON_GetErrorPtr(void){
    return ep;
}

/* Parse the input text into an unescaped cstring, and populate item. */
static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

// 编码格式转换
static unsigned parse_hex4(const char* str){
    unsigned h = 0;
    if (*str >= '0' && *str <= '9')
    {
        h += (*str) - '0';
    }else if (*str >= 'A' && *str <= 'F')
    {
        h += 10 + (*str) - 'A';
    }else if (*str >= 'a' && *str <= 'f')
    {
        h += 10 + (*str) - 'a';
    }else{
        return 0;
    }
    h = h << 4;
    str++;
    if (*str >= '0' && *str <= '9')
    {
        h += (*str) - '0';
    }else if (*str >= 'A' && *str <= 'F')
    {
        h += 10 + (*str) - 'A';
    }else if (*str >= 'a' && *str <= 'f')
    {
        h += 10 + (*str) - 'a';
    }else{
        return 0;
    }
    h = h << 4;
    str++;
    if (*str >= '0' && *str <= '9')
    {
        h += (*str) - '0';
    }else if (*str >= 'A' && *str <= 'F')
    {
        h += 10 + (*str) - 'A';
    }else if (*str >= 'a' && *str <= 'f')
    {
        h += 10 + (*str) - 'a';
    }else{
        return 0;
    }
    return h;
}

// 解析字符串类型
static const char* parse_string(cJSON* item,const char* str){
    // 跳过开头的'\"'字符
    const char *ptr = str + 1;
    char *ptr2;
    char *out;
    int len = 0;
    unsigned uc,uc2;

    // 传入的不是字符串则出错，ep指向出错内容。
    if (*str!= '\"')
    {
        ep = str;
        return 0;
    }
    //假设传入的字符串为这个"\"Jack (\\\"Bee\\\") Nimble\", \n\"format\""，
	//这段代码能够统计出jack(\"Bee\")Nimble的长度，方便接下来为存储这段字符申请内存，注意\\该表字符\,\"代表字符"
    while (*ptr != '\"' && *ptr && ++len)
    {
        // 一次跳过两个字符，因为默认\\和\"是一起出现的
        if (*ptr++ == '\\')
        {
            ptr++;
        }
    }
    // 多申请一个内容存储结尾字符'\0'
    out = (char*)cJSON_malloc(len + 1);
    if (!out)
    {
        return 0;
    }
    
    ptr = str + 1;
    //ptr2指向out，通过改变ptr2中的内容，改变out中的内容
    ptr2 = out;
    while (*ptr != '\"' && * ptr)
    {
        // 把ptr中的内存赋值给ptr2
        if (*ptr != '\\')
        {
            *ptr2++ = *ptr++;
        }else{
            // 先多移动一个字符，在开始存储
            ptr++;
            // 存储特殊字符
            switch (*ptr)
            {
            case 'b':
                *ptr2++ = '\b';
                break;
            
            case 'f':
                *ptr2++ = '\f';
                break;

            case 'n':
                *ptr2++ = '\n';
                break;
            
            case 'r':
                *ptr2++ = '\r';
                break;

            case 't':
                *ptr2++ = '\t';
                break;
            
            case 'u':
                //这里主要是编码转换的，了解一下就好
                uc = parse_hex4(ptr + 1);
                ptr += 4;
                if ((uc >= 0xDC00 && uc <= 0xDFFF) || uc == 0)
                {
                    break;
                }
                if (uc >= 0xDB800 && uc <= 0xDBFF)
                {
                    if (ptr[1] != '\\' || ptr[2] != 'u')
                    {
                        break;
                    }
                    uc2 = parse_hex4(ptr + 3);
                    ptr += 6;
                    if (uc2 < 0xDC00 || uc2 > 0xDFFF)
                    {
                        break;
                    }
                    uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
                }
                len = 4;
                if (uc < 0x80)
                {
                    len = 1;
                }else if (uc < 0x800)
                {
                    len = 2;
                }else if (uc < 0x10000)
                {
                    len = 3;
                }
                ptr2 += len;
                
                switch (len)
                {
                case 4:
                    *--ptr2 = ((uc | 0x80) & 0xBF);
                    uc >>= 6;
                    break;
                case 3:
                    *--ptr2 = ((uc | 0x80) & 0xBF);
                    uc >>= 6;
                case 2:
                    *--ptr2 = ((uc | 0x80) & 0xBF);
                    uc >>= 6;
                case 1:
                    *--ptr2 = (uc | firstByteMark[len]);
                }
                ptr2 += len;
                break;

            default:
                //直接复制对应的内容
                *ptr2++ = *ptr;
                break;
            }
            ptr++;
        }
    }
    *ptr2 = 0;
    if (*ptr == '\"')
    {
        ptr++;
    }
    // 设置item
    item->valuestring = out;
    item->type = cJSON_String;
    // 返回剩余的字符串
    return ptr;
}


static const char* parse_number(cJSON* item,const char* num){
    // sign是符号位，signsubscale是科学计数法中的符号，如 10e-10，10e+10
    double n = 0,sign = 1,scale = 0;
    int subscale = 0,signsubscale = 1;
    //记录负号
    if (*num == '-')
    {
        sign = -1;
        num++;
    }
    //0的话直接移动，比如00001直接打印1就行
    if (*num == '0')
    {
        num++;
    }
    // 数字式1到9的话，得到其对应的值
    if (*num >= '1' && *num <= '9')
    {
        do
        {
            n = (n * 10.0) + (*num++ - '0');
        } while (*num >= '0' && *num <= '9');
    }
    //说明带有小数
    if (*num == '.' && num[1] >= '0' && num[1] <= '9')
    {
        num++;
        do
        {
            n = (n * 10.0) + (*num++ - '0');
            // 后面调整小数点的位置
            scale--;
        } while (*num >= '0' && *num <= '9');
    }
    //科计数法的部分
    if (*num == 'e' || *num == 'E')
    {
        num++;
        if (*num == '+')
        {
            num++;
        //e后面为-号
        }else if (*num == '-')
        {
            signsubscale = -1;
            num++;
        }
        while (*num >= '0' && *num <= '9')
        {
            subscale = (subscale * 10) + (*num++ - '0');
        }
    }
    //得到n的结果
    n = sign * n * pow(10.0,(scale + subscale * signsubscale));
    //存储到指定的位置
    item->valuedouble = n;
    item->valuedoint = (int)n;
    //设置类型
    item->type = cJSON_Number;
    return num;
}

// 将字符串存储到指定的item成员里面
static const char* parse_value(cJSON* item,const char* value){
    if (!value)
    {
        return 0;
    }
    // 函数多了一个参数n，限制了对字符串的访问，相对来说安全一些。
    /*
	如果返回值 < 0，则表示 str1 小于 str2。
	如果返回值 > 0，则表示 str2 小于 str1。
	如果返回值 = 0，则表示 str1 等于 str2。
	*/
    // 等于null指针向下移动4个
    if (!strncmp(value,"null",4))
    {
        item->type = cJSON_NULL;
        return value + 4;
    }
    // 等于false指针向下移动5个
    if (!strncmp(value,"false",5))
    {
        item->type = cJSON_False;
        return value + 5;
    }
    // 等于true指针向下移动4个
    if (!strncmp(value,"true",4))
    {
        item->type = cJSON_True;
        item->valuedoint = 1;
        return value + 4;
    }
    // 字符串开始标识符，则调用此函数
    if (*value == '\"')
    {
        return parse_string(item,value);
    }
    // 数值开始标识符则调用此函数
    if (*value == '-' || (*value >= '0' && *value <= '9'))
    {
        return parse_number(item,value);
    }
    //array开始标识符则调用此函数
    if (*value == '[')
    {
        return parse_array(item,value);
    }
    // object开始标识符则调用此函数 
    if (*value == '{')
    {
        return parse_object(item,value);
    }
    //都不匹配，则出错，ep指向这段字符串，返回0
    ep = value;
    return 0;
}


// 跳过一个字符串中开头包含ASCII码<=32的字符，空格字符也会被跳过
static const char* skip(const char* in){
    // 字符指针存在，当前字符不是'\0'，且<= 32;
    while (in && *in && (unsigned char)*in <= 32)
    {
        in++;
    }
    return in;
}

static const char* parse_array(cJSON* item,const char* value){
    cJSON* child;
    //出错，指向出错的位置
    if (*value != '[')
    {
        ep = value;
        return 0;
    }
    // 设置item的类型
    item->type = cJSON_Array;
    // value + 1跳过 [,skip跳过开头ASCII<=32的字符
    value = skip(value + 1);
    // 说明是空的array
    if (*value == ']')
    {
        return value + 1;
    }
    // array和第一个结点之间通过child进行连接
    item->child = child = cJSON_New_Item();
    // 内存分配失败
    if (!item->child)
    {
        return 0;
    }
    //进行递归调用，处理array中的第一个结点
    value = skip(parse_value(child,skip(value)));
    // 如果value没有后续了，则解析结束
    if (!value)
    {
        return 0;
    }
    // []中的字符串以,号隔开，故每处理一个结点，接下来的字符都是逗号
    while (*value == ',')
    {
        cJSON* new_item;
        // 内存分配失败
        if (!(new_item = cJSON_New_Item()))
        {
            return 0;
        }
        child->next = new_item;
        //将同一级结点之间使用next和prev指针串起来，并且指向下一个结构继续处理
        new_item->prev = child;
        child = new_item;
        //递归的构建array中的每个结点
        value = skip(parse_value(child,skip(value + 1)));
        // 如果value没有后续了，则解析结束
        if (!value)
        {
            return 0;
        }
    }
    // 遇到array结尾标志，向下移动
    if (*value == ']')
    {
        return value + 1;
    }
    // 如果不是正确的结尾标志，则说明出错，ep指向错误字符，并且解析结束
    ep = value;
    return 0;
}

// 处理*value为'{'的情况
static const char* parse_object(cJSON* item,const char* value){
    cJSON *child;
    // 出错的话，打印出错位置
    if (*value != '{')
    {
        ep = value;
        return 0;
    }
    // 设置item的类型
    item->type = cJSON_Object;
    // 向下移动一格，并且跳过开头ASCII<=32的字符
    value = skip(value + 1);
    //空字符，指向下一个
    if (*value == '}')
    {
        return value + 1;
    }
    // 申请内存，object中的第一个结点
    item->child = child = cJSON_New_Item();
    if (!item->child)
    {
        return 0;
    }
    // 处理第一个结点
    value = skip(parse_string(child,skip(value)));
    if (!value)
    {
        return 0;
    }
    //将值中的内容赋值给对应的键
    child->string = child->valuestring;
    child->valuestring = 0;
    // 键和值之间通过:号连接。
    if (*value != ':')
    {
        ep = value;
        return 0;
    }
    //递归调用函数向下处理，用来添加值
    value = skip(parse_value(child,skip(value + 1)));
    if (!value)
    {
        return 0;
    }
    //同一层结点之间，通过,号隔开,每次处理完一部分字符串剩余的字符串以,号开始
    while (*value ==',')
    {
        cJSON *new_item;
        //内存申请失败
        if (!(new_item = cJSON_New_Item()))
        {
           return 0;
        }
         child->next = new_item;
        new_item->prev = child;
        child = new_item;
        //将同一级结点之间使用next和prev指针串起来，并且指向下一个结构继续处理
        value = skip(parse_string(child,skip(value + 1)));
        if (!value)
        {
            return 0;
        }
        //给键赋值
        child->string = child->valuestring;
        child->valuestring = 0;
        // 先将第一个子节点的"值"内容，赋给结点中的键，后面会从重新赋值
        if (*value != ':')
        {
            ep = value;
            return 0;
        }
        //递归的处理子节点，给值赋值
        value = skip(parse_value(child,skip(value + 1)));
        if (!value)
        {
            return 0;
        }   
    }
    if (*value == '}')
    {
        return value + 1;
    }
    ep = value;
    return 0;
}

// value为需要解析的文本，require_null_terminated为一个标识符，
// return_parse_end为一个二级指针，用于获取出现错误部分的字符串，由于需要在函数内部改变其指向，故设置为二级指针（一级指针没效果，推出函数即消失）
cJSON* cJSON_ParseWithOpts(const char* value,const char** return_parse_end,int require_null_terminated){
    const char* end = 0;
    // 创建一个结构体，用于存储字符串中的内容
    cJSON *c = cJSON_New_Item();
    // 全局变量，ep指向了出错的字符串，当需要获得出错的字符串的时候，可通过调用函数cJSON_GetErrorPtr()查看
    ep = 0;
    /* memory fail */
    if (!c)
    {
        return 0;
    }
    end = parse_value(c,skip(value));
    //出错的话释放内存，ep = 0
    if (!end)
    {
        cJSON_Delete(c);
        return 0;
    }
    // 假设设置了这个标志位，要求最终的结束字符是null
    if (require_null_terminated)
    {
        // 跳过开头的字符
        end = skip(end);
        //如果依旧存在字符，说明要解析的文本不符合规定，若输入为\"nullyuanlixiang\"，通过parse_value后，end = "yuanlixiang",不符合以"null"结尾的要求，故释放内存，同时是ep指向出错的字符串
        if (*end)
        {
            cJSON_Delete(c);
            ep = end;
            return 0;
        }
    }
    // 二维指针，获得解析完毕的情况下end中的内容
    if (return_parse_end)
    {
        *return_parse_end = end;
    }
    //返回结构体c，通过递归已经将结构体串成一个链
    return c;
}

// cJSON_Parse通过调用cJSON_ParseWithOpts实现
cJSON* cJSON_Parse(const char* value){
    return cJSON_ParseWithOpts(value,0,0);
}

cJSON* cJSON_DetachItemFromArray(cJSON *array,int which){
    cJSON *c = array->child;
    while (c && which > 0)
    {
        c = c->next;
        which--;
    }
    if (!c)
    {
        return 0;
    }
    if (c->prev)
    {
        c->prev->next = c->next;
    }
    if (c->next)
    {
        c->next->prev = c->prev;
    }
    if (c == array->child)
    {
        array->child = c->next;
    }
    c->prev = c->next = 0;
    return c;
}

void cJSON_DeleteItemFromArray(cJSON* array,int which){
    cJSON_Delete(cJSON_DetachItemFromArray(array,which));
}

static int cJSON_strcasecmp(const char* s1,const char* s2){
    if (!s1)
    {
        return (s1 == s2) ? 0 : 1;
        if (s2)
        {
            return 1;
        }
        for(;tolower(*s1) == tolower(*s2);++s1,++s2){
            if (*s1 == 0)
            {
                return 0;
            }
            
        }
        return tolower(*(const unsigned char *)s1) - tolower(*(const unsigned char *)s2);
    }
    
}

cJSON* cJSON_DetachItemFromObject(cJSON *object,const char *string){
    int i = 0;
    cJSON* c = object->child;
    while (c && cJSON_strcasecmp(c->string,string))
    {
        i++;
        c = c->next;
    }
    if (c)
    {
        return cJSON_DetachItemFromArray(object,i);
    }
    return 0;
}

void cJSON_DeleteItemFromObject(cJSON *object,const char* string){
    cJSON_Delete(cJSON_DetachItemFromObject(object,string));
}

// 返回节点的个数,注意是某个特顶层的，其子层和父层的结点不会被计算
int cJSON_GetArraySize(cJSON *array){
    cJSON *c = array->child;
    int i = 0;
    while (c)
    {
        i++;
        c = c->next;
    }
    return i;
}

// 返回array中第item个节点的地址，同样是特定层的结点的地址
cJSON* cJSON_GetArrayItem(cJSON* array,int item){
    cJSON *c = array->child;
    while (c && item > 0)
    {
        item--;
        c = c->next;
    }
    return c;
}

cJSON *cJSON_GetObjectItem(cJSON* object,const char* string){
    cJSON *c = object->child;
    while (c && cJSON_strcasecmp(c->string,string))
    {
        c = c->next;
    }
    return c;
}

//在链表中的指定位置插入一个新结点，array或object的child指向的元素为第一个结构，which从0开始
void cJSON_InsertItemInArray(cJSON* array,int which,cJSON* newitem){
    cJSON *c = array->child;
    // 找到which位置
    // 找到要添加的位置
    while (c && which > 0)
    {
        c = c->next;
        which--;
    }
    // 添加新的节点到array中
    // 为空的话说明直接添加到最后面
    if (!c)
    {
        cJSON_AddItemToArray(array,newitem);
        return;
    }
    // 在链的中间的某个部位添加结点
    // 将链表节点进行挂接
    newitem->next = c;
    newitem->prev = c->prev;
    c->prev = newitem;
    // 处理arry的第一个结点，child指向的位置
    if (c == array->child)
    {
        array->child = newitem;
    }else{// 添加到其它位置的时候
        newitem->prev->next = newitem;
    }
    
}

// array中的替换节点操作，用新的节点替换原有的某一个节点
void cJSON_ReplaceItemInArray(cJSON* array,int which,cJSON* newitem){
    cJSON* c = array->child;
    // 找到which位置
    while (c && which > 0)
    {
        c = c->next;
        which--;
    }
    if (!c)
    {
        return;
    }
    // 进行挂接
    newitem->next = c->next;
    newitem->prev = c->prev;
    // 处理非NULL情况
    if (newitem->next)
    {
        newitem->next->prev = newitem;
    }
    // 替换的结点为array的第一个子节点，child指向的结点
    if (c == array->child)
    {
        array->child = newitem;
    }else{// 处理其它替换结点的情况
        newitem->prev->next = newitem;
    }
    // 删除替换的节点
    c->next = c->prev = 0;
    //释放替换下来的结点的内存
    cJSON_Delete(c);
}

// object替换节点操作
// 在object对应的链中替换结点，将新结点替换原来的结点，根据键的内容找到对应的就结点，（不区分大小写）
void cJSON_ReplaceItemInObject(cJSON* object,const char* string,cJSON* newitem){
    int i = 0;
    cJSON *c = object->child;
    //找到所需要替换的结点的位置，调用cJSON_ReplaceItemInArray实现替换
    while (c && cJSON_strcasecmp(c->string,string))
    {
        i++;
        c = c->next;
    }
    // 调用array中的结点替换方式进行替换
    if (c)
    {
        newitem->string = cJSON_strdup(string);
        cJSON_ReplaceItemInArray(object,i,newitem);
    }
}

// 拷贝副本操作，后面的recurse用于判断是否进行递归copy多个结点，如果启用了recurse会把后面的整条链拷贝一份，当然这个时候传入的item必须是一个object或array
cJSON *cJSON_Duplicate(cJSON *item,int recurse){
    cJSON *newitem,*cptr,*nptr = 0,*newchild;
    if (!item)
    {
        return 0;
    }
    //创建一个新结点
    newitem = cJSON_New_Item();
    if (!newitem)
    {
        return 0;
    }
    // 将指定结点的内容赋给刚申请的结点
    newitem->type = item->type & (~cJSON_IsReference);
    newitem->valuedoint = item->valuedoint;
    newitem->valuedouble = item->valuedouble;
    if (item->valuestring)
    {
        //动态内存分配存储字符串的内容
        newitem->valuestring = cJSON_strdup(item->valuestring);
        if (!newitem->valuestring)
        {
            //出错释放内存
            cJSON_Delete(newitem);
            return 0;
        }
    }
    if (item->string)
    {
        newitem->string = cJSON_strdup(item->string);
        if (!newitem->string)
        {
            cJSON_free(newitem);
            return 0;
        }
    }
    //加入不需要递归拷贝的话，直接返回刚创建的结点
    if (!recurse)
    {
        return newitem;
    }
    cptr = item->child;
    //如果存在内容，说明item是一个含有子节点的object或array
    while (cptr)
    {
        newchild = cJSON_Duplicate(cptr,1);
        if (!newchild)
        {
            cJSON_Delete(newitem);
            return 0;
        }
        if (nptr)
        {
            nptr->next = newchild;
            newchild->prev = nptr;
            // 移动当前指针，继续向下处理
            nptr = newchild;
        }
        else{// 当copy的是第一个结点时，child的内容
            newitem->child = newchild;
            // 移动当前指针
            nptr = newchild;
        }
        //循环处理某一层的结点。
        cptr = cptr->next;
    }
    return newitem;
}
