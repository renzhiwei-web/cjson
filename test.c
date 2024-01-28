#include <stdio.h>
#include <stdlib.h>
#include "cjson.h"

cJSON* create_objects(){
    cJSON* root = cJSON_CreateObject();// 类似于创建了一个大括号
	cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
	cJSON* fmt=cJSON_CreateObject();
	cJSON_AddItemToObject(root, "format", fmt);
	cJSON_AddStringToObject(fmt,"type",		"rect");
	cJSON_AddNumberToObject(fmt,"width",		1920);
	cJSON_AddNumberToObject(fmt,"height",		1080);
	cJSON_AddFalseToObject (fmt,"interlace");
	cJSON_AddNumberToObject(fmt,"frame rate",	24);
	return root;
}

int main (int argc, const char * argv[]){
    cJSON* root = create_objects();
	char *s1 = cJSON_Print(root); // 带有格式的打印，换行符、制表符、空格等
    char *s2 = cJSON_PrintUnformatted(root); // 不带格式的打印，效果可以看后面的输出结果
    char *text;
    cJSON* json;
    if(root)
        cJSON_Delete(root);
    // 判断是否转化为字符串成功，成功的话打印并释放动态内存
    if(s1)
    {
       printf(" 带有格式的输出结果:\n%s \n",s1);
       free(s1);
    }
    // 判断是否转化为字符串成功，成功的话打印并释放动态内存
    if(s2)
    {
       printf(" 不带格式的输出结果:\n%s \n",s2);
       free(s2);
    }
    text = "{\n\"name\": \"Jack (\\\"Bee\\\") Nimble\", \n\"format\": {\"type\":       \"rect\", \n\"width\":      1920, \n\"height\":     1080, \n\"interlace\":  false,\"frame rate\": 24\n}\n}";
    json=cJSON_Parse(text);//通过此函数将字符串存储到cJSON的结构块中，并且结构块之间相连
    char* s3 = cJSON_PrintUnformatted(json);
    printf("%s \n",s3);
    free(text);
    cJSON_Delete(json);
    free(s3);
    return 0;
}