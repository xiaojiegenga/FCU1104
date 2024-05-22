#include "list.h"

LIST_LINK * list_init()
{
    LIST_LINK *temp = malloc(sizeof(LIST_LINK));
    temp->next = NULL;
    return temp;
}

void list_add(LIST_LINK* head, LIST_LINK *info)
{

        info->next = head->next;
        head->next = info;

}

LIST_LINK * list_for_each(LIST_LINK* head, char *name)
{//找到线程名字，并返回指向该线程的tmp
    LIST_LINK * tmp = NULL;
    tmp = head;
    while(tmp->next != NULL)
    {
        if(strncmp(tmp->elem.name, name, strlen(name)) == 0)
           return tmp;
        tmp = tmp->next;
    }
    if(strncmp(tmp->elem.name, name, strlen(name)) == 0)
        return tmp;
    else
        return NULL;
}
