#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <microhttpd.h>
// #include "py1.h"

#define PORT 8080
#define BUFFER_SIZE 1000000

typedef struct MHD_Response MHD_Response;
unsigned long last_request_id = 0;

struct qs
{
    char gx[20];
    char name[10];
};
struct qsxs
{
    struct qs *relation[100];
    int x;
};
struct lxr
{
    char name[20];
    char number[12];
    struct lxr *next;
    struct lxr *countnext;
    char jj[100];
    int count;
    struct qsxs *mf;
};

lxr **txl = (lxr **)malloc(sizeof(lxr *) * 100);

// 姓名搜索
lxr *search(char *s, int n)
{
    int i = 0;
    lxr *target = (lxr *)malloc(sizeof(lxr));
    for (i = 0; txl[i] != NULL && txl[i]->name[0] != s[0]; i++)
        ;
    if (txl[i] != NULL)
    {
        target = txl[i]->next;
        int j;
        for (j = 0; target != NULL && s[j] != '\0'; j++)
        {
            if (target->name[j] != s[j])
            {
                target = target->next;
                j = 0;
            }
        };
        if (s[j] == '\0')
            return target;
        else
            return NULL;
    }
    else
        return NULL;
}

// 电话号码搜索
lxr *numbersearch(char *s, int n)
{
    lxr *target = NULL;

    // 假设 txl 是一个包含多个联系人节点的数组或者链表
    for (int i = 0; txl[i] != NULL; i++)
    {
        target = txl[i]; // 从 txl[i] 开始
        int j = 0;

        // 遍历每个联系人节点
        while (target != NULL)
        {
            for (j = 0; target->number[j] != '\0' && s[j] != '\0'; j++)
            {
                if (target->number[j] != s[j])
                {
                    break; // 找到不匹配的字符，退出内层循环
                }
            }

            // 检查是否完全匹配
            if (target->number[j] == '\0' && s[j] == '\0')
            {
                return target; // 返回匹配的联系人节点
            }

            // 继续检查下一个联系人节点
            target = target->next;
        }
    }

    return NULL; // 如果没有找到匹配的电话号码，返回 NULL
}

// 读取数据.txt
void create()
{
    for (int i = 0; i < 100; i++)
        txl[i] = NULL;

    FILE *pf = fopen("数据.txt", "r");
    while (1)
    {
        lxr *new1 = (lxr *)malloc(sizeof(lxr));
        qsxs *m = (qsxs *)malloc(sizeof(qsxs));
        new1->mf = m;
        if ((fscanf(pf, "%s %s %s %d %d ", new1->name, new1->number, new1->jj, &new1->count, &new1->mf->x)) != EOF)
        {
            for (int l = 0; l < new1->mf->x; l++)
            {
                qs *q = (qs *)malloc(sizeof(qs));
                fscanf(pf, "%s %s", q->gx, q->name);
                new1->mf->relation[l] = q;
            }
            new1->countnext = NULL;
            new1->next = NULL;
            int i;
            for (i = 0; txl[i] != NULL && txl[i]->name[0] != new1->name[0]; i++)
                ;
            if (txl[i] == NULL)
            {
                lxr *n = (lxr *)malloc(sizeof(lxr));
                n->name[0] = new1->name[0];
                n->count = 1;
                n->next = new1;
                txl[i] = n;
            }
            else
            {
                lxr *m = (lxr *)malloc(sizeof(lxr));
                m = txl[i]->next;
                while (m->next != NULL)
                {
                    m = m->next;
                }
                m->next = new1;
                txl[i]->count++;
                for (int j = i; j > 0 && txl[j - 1]->count < txl[j]->count; j--)
                {
                    lxr *h = txl[j];
                    txl[j] = txl[j - 1];
                    txl[j - 1] = h;
                }
            }
        }
        else
            break;
    }
}

// 保存数据
void save()
{
    lxr *z = (lxr *)malloc(sizeof(lxr));
    lxr *new1 = (lxr *)malloc(sizeof(lxr));
    lxr *k = (lxr *)malloc(sizeof(lxr));
    FILE *pf = fopen("数据.txt", "w");
    if (pf == NULL)
    {
        perror("fopen");
    }
    for (int i = 0; txl[i] != NULL; i++)
    {
        k = txl[i]->next;
        while (k)
        {
            fprintf(pf, "%s %s %s %d %d ", k->name, k->number, k->jj, k->count, k->mf->x);
            for (int j = 0; j < k->mf->x; j++)
                fprintf(pf, "%s %s ", k->mf->relation[j]->gx, k->mf->relation[j]->name);
            k = k->next;
        }
    }
    fclose(pf);
    pf = NULL;
}

// 函数用于提取数组
void extractArray(const char *json, const char *key, char **array, int size)
{
    char *start = strstr(json, key);
    if (start)
    {
        start = strstr(start, "[");
        if (start)
        {
            start++; // 跳过 '['

            char *end = strchr(start, ']');
            if (end)
            {
                *end = '\0'; // 临时替换 ']' 为字符串结束符

                int i = 0;
                char *token = strtok(start, ","); // 分割数组元素
                while (token && i < size)
                {
                    // 跳过引号
                    if (token[0] == '"')
                    {
                        token++; // 跳过开头的引号
                    }

                    // 查找结束引号并替换为 '\0'
                    char *valueEnd = strchr(token, '"');
                    if (valueEnd)
                    {
                        *valueEnd = '\0'; // 去除结束引号
                    }

                    // 将元素添加到数组
                    array[i++] = strdup(token); // 复制字符串到数组
                    token = strtok(NULL, ",");  // 继续分割下一个元素
                }

                // 恢复 ']' 字符
                *end = ']';
            }
        }
    }
}

// count比较函数
int compare_by_count(const void *a, const void *b)
{
    lxr *contact_a = *(lxr **)a;
    lxr *contact_b = *(lxr **)b;

    return contact_b->count - contact_a->count; // 按 count 降序排序
}
// pinyin比较函数
// int compare_by_pinyin(const void *a, const void *b)
// {
//     lxr *contact_a = *(lxr **)a;
//     lxr *contact_b = *(lxr **)b;
//
//     char outbuf1[HZ2PY_OUTPUT_BUF_ARRAY_SIZE];
//     char outbuf2[HZ2PY_OUTPUT_BUF_ARRAY_SIZE];
//     memset(outbuf1, '\0', sizeof(outbuf1));
//     memset(outbuf2, '\0', sizeof(outbuf2));
//
//     if (is_utf8_string(contact_a->name))
//     {
//         pinyin_utf8(contact_a->name, outbuf1);
//     }
//     else
//     {
//         pinyin_gb2312(contact_a->name, outbuf1, false, false, true, true, true);
//     }
//
//     if (is_utf8_string(contact_b->name))
//     {
//         pinyin_utf8(contact_b->name, outbuf2);
//     }
//     else
//     {
//         pinyin_gb2312(contact_b->name, outbuf2, false, false, true, true, true);
//     }
//
//     return strcmp(outbuf1, outbuf2); // 按拼音字典序排序
// }

// HTTP 请求处理器
static MHD_Result request_handler(void *cls, struct MHD_Connection *connection,
                                  const char *url, const char *method, const char *version,
                                  const char *upload_data, size_t *upload_data_size, void **ptr)
{
    static int aptr;
    if (&aptr != *ptr)
    {
        *ptr = &aptr;
        return MHD_YES;
    }
    *ptr = NULL;

    char response_text[BUFFER_SIZE] = {0};
    int status_code = MHD_HTTP_OK;

    if (strcmp(method, "OPTIONS") == 0)
    {
        struct MHD_Response *response = MHD_create_response_from_buffer(0, (void *)"", MHD_RESPMEM_MUST_COPY);
        MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
        MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, DELETE , PUT, OPTIONS");
        MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");
        MHD_add_response_header(response, "Access-Control-Max-Age", "3600");
        int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret == MHD_YES ? MHD_YES : MHD_NO;
    }

    // 解析请求标识
    unsigned long request_id = (unsigned long)time(NULL); // 使用当前时间作为唯一标识
    if (request_id == last_request_id)
    {
        // 如果请求ID重复，直接返回
        snprintf(response_text, BUFFER_SIZE, "{\"message\":\"重复的请求，已被忽略\"}");
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_text), response_text, MHD_RESPMEM_MUST_COPY);
        MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
        MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return MHD_YES;
    }
    last_request_id = request_id; // 更新最近处理的请求ID

    // 根据 URL 参数处理不同操作
    if (strcmp(method, "GET") == 0)
    {
        if (strstr(url, "operation=list") != NULL) // 列出所有联系人
        {
            printf("传输联系人列表(默认)。。。\n");
            char *json = (char *)malloc(BUFFER_SIZE);
            if (json)
            {
                strcpy(json, "[");
                for (int i = 0; txl[i] != NULL; i++)
                {
                    lxr *current = txl[i]->next;
                    while (current)
                    {
                        char lxr_json[BUFFER_SIZE];
                        snprintf(lxr_json, BUFFER_SIZE, "{\"name\":\"%s\",\"number\":\"%s\",\"jj\":\"%s\",\"count\":%d,\"relationnumber\":%d}",
                                 current->name, current->number, current->jj, current->count, current->mf->x);
                        strcat(json, lxr_json);
                        if (current->next || txl[i + 1] != NULL)
                            strcat(json, ",");
                        current = current->next;
                    }
                }
                strcat(json, "]");
                snprintf(response_text, BUFFER_SIZE, "%s", json);
                free(json);
            }
            else
            {
                status_code = MHD_HTTP_INTERNAL_SERVER_ERROR;
            }
        }
        else if (strstr(url, "operation=countlist") != NULL) // 列出所有联系人
        {
            printf("传输联系人列表(count)。。。\n");

            // 计算联系人总数
            int count = 0;
            for (int i = 0; txl[i] != NULL; i++)
            {
                lxr *current = txl[i]->next;
                while (current)
                {
                    count++;
                    current = current->next;
                }
            }

            // 创建联系人指针数组
            lxr **contact_array = (lxr **)malloc(count * sizeof(lxr *));
            int index = 0;
            for (int i = 0; txl[i] != NULL; i++)
            {
                lxr *current = txl[i]->next;
                while (current)
                {
                    contact_array[index++] = current;
                    current = current->next;
                }
            }

            // 按 count 排序联系人数组
            qsort(contact_array, count, sizeof(lxr *), compare_by_count);

            // 创建 JSON 响应
            char *json = (char *)malloc(BUFFER_SIZE);
            if (json)
            {
                strcpy(json, "[");
                for (int i = 0; i < count; i++)
                {
                    lxr *current = contact_array[i];
                    char lxr_json[BUFFER_SIZE];
                    snprintf(lxr_json, BUFFER_SIZE, "{\"name\":\"%s\",\"number\":\"%s\",\"jj\":\"%s\",\"count\":%d,\"relationnumber\":%d}",
                             current->name, current->number, current->jj, current->count, current->mf->x);
                    strcat(json, lxr_json);
                    if (i < count - 1)
                    {
                        strcat(json, ",");
                    }
                }
                strcat(json, "]");
                snprintf(response_text, BUFFER_SIZE, "%s", json);
                free(json);
            }
            else
            {
                status_code = MHD_HTTP_INTERNAL_SERVER_ERROR;
            }

            // 释放内存
            free(contact_array);
        }
        else if (strstr(url, "operation=pinyinlist") != NULL) // 列出所有联系人
        {
            printf("传输联系人列表(pinyin)。。。\n");

            // 计算联系人总数
            // int count = 0;
            // for (int i = 0; txl[i] != NULL; i++)
            // {
            //     lxr *current = txl[i]->next;
            //     while (current)
            //     {
            //         count++;
            //         current = current->next;
            //     }
            // }

            // // 创建联系人数组
            // lxr **contact_array = (lxr **)malloc(count * sizeof(lxr *));
            // int index = 0;
            // for (int i = 0; txl[i] != NULL; i++)
            // {
            //     lxr *current = txl[i]->next;
            //     while (current)
            //     {
            //         contact_array[index++] = current;
            //         current = current->next;
            //     }
            // }

            // // 对联系人按拼音排序
            // qsort(contact_array, count, sizeof(lxr *), compare_by_pinyin);

            // // 构造 JSON 数据
            // char *json = (char *)malloc(BUFFER_SIZE);
            // if (json)
            // {
            //     strcpy(json, "[");
            //     for (int i = 0; i < count; i++)
            //     {
            //         lxr *current = contact_array[i];
            //         char lxr_json[BUFFER_SIZE];
            //         snprintf(lxr_json, BUFFER_SIZE, "{\"name\":\"%s\",\"number\":\"%s\",\"jj\":\"%s\",\"count\":%d,\"relationnumber\":%d}",
            //                  current->name, current->number, current->jj, current->count, current->mf->x);
            //         strcat(json, lxr_json);
            //         if (i < count - 1)
            //             strcat(json, ",");
            //     }
            //     strcat(json, "]");
            //     snprintf(response_text, BUFFER_SIZE, "%s", json);
            //     free(json);
            // }
            // else
            // {
            //     status_code = MHD_HTTP_INTERNAL_SERVER_ERROR;
            // }

            // // 释放内存
            // free(contact_array);
        }
    }
    else if (strcmp(method, "POST") == 0)
    {
        if (strstr(url, "operation=add") != NULL && *upload_data_size != 0) // 添加联系人
        {
            printf("添加联系人中。。。\n");
            char name[20], number[12], jj[10000];
            int relationnumber;

            sscanf(upload_data, "{\"name\":\"%[^\"]\",\"number\":\"%[^\"]\",\"jj\":\"%[^\"]\",\"relationnumber\":%d", name, number, jj, &relationnumber);
            printf(upload_data);
            printf("\n");
            char *gx[relationnumber];
            char *gxname[relationnumber];

            extractArray(upload_data, "\"gxname\"", gxname, relationnumber);
            extractArray(upload_data, "\"gx\"", gx, relationnumber);

            if (search(name, 20) == NULL)
            {
                printf("可以添加联系人");
                lxr *new1 = (lxr *)malloc(sizeof(lxr));
                new1->mf = (qsxs *)malloc(sizeof(qsxs));
                strcpy(new1->name, name);
                strcpy(new1->number, number);
                strcpy(new1->jj, jj);
                new1->count = 0;
                new1->countnext = NULL;
                new1->mf->x = 0;

                for (int i = 0; i < 100; i++)
                {
                    new1->mf->relation[i] = NULL;
                }
                for (int i = 0; i < relationnumber; i++)
                {
                    if (gx[i] && gxname[i])
                    {
                        struct qs *q = (struct qs *)malloc(sizeof(struct qs));
                        // 复制亲属号码和姓名
                        strcpy(q->gx, gx[i]);       // 亲属的号码
                        strcpy(q->name, gxname[i]); // 亲属的姓名
                        printf("ok");

                        // 将亲属关系添加到联系人
                        int index = new1->mf->x;
                        if (index < 100)
                        {
                            new1->mf->relation[index] = q; // 存储亲属关系
                            new1->mf->x++;                 // 增加已存储的亲属数量
                        }
                        printf("kk");
                    }
                }

                new1->next = NULL;

                int i;
                for (i = 0; txl[i] != NULL && txl[i]->name[0] != new1->name[0]; i++)
                    ;
                if (txl[i] == NULL)
                {
                    lxr *n = (lxr *)malloc(sizeof(lxr));
                    n->name[0] = new1->name[0];
                    n->count = 1;
                    n->next = new1;
                    txl[i] = n;
                }
                else
                {
                    lxr *m = (lxr *)malloc(sizeof(lxr));
                    m = txl[i]->next;
                    while (m->next != NULL)
                    {
                        m = m->next;
                    }
                    m->next = new1;
                    txl[i]->count++;
                    for (int j = i; j > 0 && txl[j - 1]->count < txl[j]->count; j--)
                    {
                        lxr *h = txl[j];
                        txl[j] = txl[j - 1];
                        txl[j - 1] = h;
                    }

                    printf("联系人添加成功");
                    snprintf(response_text, BUFFER_SIZE, "{\"message\":\"联系人添加成功\"}");
                }
            }
            else
            {
                printf("表中已有该联系人。\n");
                snprintf(response_text, BUFFER_SIZE, "{\"message\":\"联系人已存在\"}");
                status_code = MHD_HTTP_CONFLICT;
            }
            // *upload_data_size = 0;
            save();
        }
        else if (strstr(url, "operation=delete") != NULL && *upload_data_size != 0) // 删除联系人
        {
            printf("删除联系人。。。\n");
            char c[20];
            sscanf(upload_data, "{\"name\":\"%[^\"]\"", c);

            int i;
            lxr *target = (lxr *)malloc(sizeof(lxr));
            lxr *pre = (lxr *)malloc(sizeof(lxr));
            for (i = 0; txl[i] != NULL && txl[i]->name[0] != c[0]; i++)
                ;
            if (txl[i] != NULL)
            {
                target = txl[i]->next;
                pre = txl[i];
                int j;
                for (j = 0; target != NULL && c[j] != '\0'; j++)
                {
                    if (target->name[j] != c[j])
                    {
                        pre = target;
                        target = target->next;
                        j = 0;
                    }
                };
                if (c[j] == '\0')
                {
                    pre->next = target->next;
                    txl[i]->count--;
                    if (txl[i]->count == 0)
                        for (int k = i; txl[k] != NULL; k++)
                            txl[k] = txl[k + 1];

                    printf("成功删除%s\n", c);
                    snprintf(response_text, BUFFER_SIZE, "{\"message\":\"联系人 '%s' 删除成功\"}", c);
                }
                else
                    printf("通讯录中没有该联系人\n");
            }
            else
                printf("通讯录中没有该联系人\n");
            save();
        }
        else if (strstr(url, "operation=call") != NULL && *upload_data_size != 0) // 拨打联系人
        {
            printf("拨打联系人。。。\n");
            printf(upload_data);
            char a[12];
            sscanf(upload_data, "{\"number\":\"%[^\"]\"", a);
            lxr *l = numbersearch(a, 12);
            printf("%s", a);
            if (!l)
                printf("通讯录中无该联系人。");
            else
                l->count++;
            save();
        }
        else if (strstr(url, "operation=edit") != NULL && *upload_data_size != 0) // 编辑联系人
        {
            printf("修改联系人中。。。\n");

            char name[20], number[12], jj[10000];
            int relationnumber;

            // 解析传入的数据
            sscanf(upload_data, "{\"name\":\"%[^\"]\",\"number\":\"%[^\"]\",\"jj\":\"%[^\"]\",\"relationnumber\":%d", name, number, jj, &relationnumber);

            char *gx[relationnumber];
            char *gxname[relationnumber];

            // 提取 gx 和 gxname
            extractArray(upload_data, "\"gxname\"", gxname, relationnumber);
            extractArray(upload_data, "\"gx\"", gx, relationnumber);

            // 查找联系人是否存在
            lxr *existingContact = search(name, 20);
            if (existingContact != NULL)
            {
                // 如果联系人存在，更新其信息
                strcpy(existingContact->number, number);
                strcpy(existingContact->jj, jj);
                // 更新亲属关系
                existingContact->mf->x = 0; // 重新计数
                if (relationnumber > 0)
                {

                    for (int i = 0; i < relationnumber; i++)
                    {
                        if (gx[i] && gxname[i])
                        {
                            // 创建新的亲属关系
                            struct qs *q = (struct qs *)malloc(sizeof(struct qs));
                            strcpy(q->gx, gx[i]);
                            strcpy(q->name, gxname[i]);

                            // 将亲属关系添加到联系人
                            int index = existingContact->mf->x;
                            if (index < 100)
                            {
                                existingContact->mf->relation[index] = q;
                                existingContact->mf->x++; // 增加已存储的亲属数量
                            }
                        }
                    }
                }

                // 联系人更新成功
                printf("联系人修改成功");
                snprintf(response_text, BUFFER_SIZE, "{\"message\":\"联系人修改成功\"}");
            }
            else
            {
                // 如果联系人不存在
                printf("表中没有该联系人。\n");
                snprintf(response_text, BUFFER_SIZE, "{\"message\":\"联系人不存在\"}");
                status_code = MHD_HTTP_NOT_FOUND;
            }

            // 保存数据
            save();
        }
    }
    else
    {
        status_code = MHD_HTTP_NOT_FOUND;
    }

    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_text), response_text, MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
    int ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    return ret == MHD_YES ? MHD_YES : MHD_NO;
}

// 主函数
int main()
{
    create();
    struct MHD_Daemon *daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, &request_handler, NULL, MHD_OPTION_END);
    if (!daemon)
    {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }
    printf("Server running on port %d...\n", PORT);
    getchar();
    MHD_stop_daemon(daemon);
    return 0;
}
