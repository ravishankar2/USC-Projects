#include <stdlib.h>

#include "cs402.h"

#include "my402list.h"

int My402ListInsertBefore(My402List * list, void * data, My402ListElem * elem)
{
    if(elem == NULL)
        return My402ListPrepend(list, data);
    else
    {
        My402ListElem * new_elem = (My402ListElem *)malloc(sizeof(My402ListElem));
        if(!new_elem)
            return 0;
        My402ListElem * prev = elem -> prev;
        new_elem -> obj = data;
        prev -> next = new_elem;
        elem -> prev = new_elem;
        new_elem -> next = elem;
        new_elem -> prev = prev;
        list -> num_members++;
        return 1;
    }
}

int My402ListInsertAfter(My402List * list, void * data, My402ListElem * elem)
{
    if(elem == NULL)
        return My402ListAppend(list, data);
    else
    {
        My402ListElem * new_elem = (My402ListElem *)malloc(sizeof(My402ListElem));
        if(!new_elem)
            return 0;
        My402ListElem * next = elem -> next;
        new_elem -> obj = data;
        next -> prev = new_elem;
        elem -> next = new_elem;
        new_elem -> next = next;
        new_elem -> prev = elem;
        list -> num_members++;
        return 1;
    }
}

void My402ListUnlinkAll(My402List * list)
{
    if(My402ListEmpty(list))
        return;
    My402ListElem * cur = My402ListFirst(list);
    while(cur)
    {
        My402ListElem * del = cur;
        cur = My402ListNext(list, cur);
        My402ListUnlink(list, del);
    }
    (list -> anchor).next = &(list -> anchor);
    (list -> anchor).prev = &(list -> anchor);
}

void My402ListUnlink(My402List * list, My402ListElem * elem)
{
    if(My402ListEmpty(list))
        return;
    My402ListElem * prev = elem -> prev;
    My402ListElem * next = elem -> next;
    prev -> next = next;
    next -> prev = prev;
    list -> num_members--;
    free(elem);
}

int My402ListPrepend(My402List * list, void * data)
{
    // Create the new node
    My402ListElem * elem;
    elem = (My402ListElem *)malloc(sizeof(My402ListElem));
    if(!elem) // Allocate memory failed
        return 0;
    elem -> obj = data;
    // Insert the node
    My402ListElem * first_elem = My402ListFirst(list);
    if(first_elem)
    {
        elem -> prev = &(list -> anchor);
        elem -> next = first_elem;
        (list -> anchor).next = elem;
        first_elem -> prev = elem;
        list -> num_members++;
    }
    else // List is empty
    {
        elem -> next = &(list -> anchor);
        elem -> prev = &(list -> anchor);
        (list -> anchor).next = elem;
        (list -> anchor).prev = elem;
        list -> num_members++;
    }
    return 1;
}

int My402ListAppend(My402List * list, void * data)
{
    // Create the new node
    My402ListElem * elem;
    elem = (My402ListElem *)malloc(sizeof(My402ListElem));
    if(!elem) // Allocate memory failed
        return 0;
    elem -> obj = data;
    // Insert the node
    My402ListElem * last_elem = My402ListLast(list);
    if(last_elem)
    {
        elem -> next = &(list -> anchor);
        elem -> prev = last_elem;
        (list -> anchor).prev = elem;
        last_elem -> next = elem;
        list -> num_members++;
    }
    else // List is empty
    {
        elem -> next = &(list -> anchor);
        elem -> prev = &(list -> anchor);
        (list -> anchor).next = elem;
        (list -> anchor).prev = elem;
        list -> num_members++;
    }
    return 1;
}

My402ListElem * My402ListFind(My402List * list, void * data)
{
    My402ListElem * elem = My402ListFirst(list);
    while(elem)
    {
        if(elem -> obj == data)
            return elem;
        elem = My402ListNext(list, elem);
    }
    return NULL;
}

My402ListElem * My402ListPrev(My402List * list, My402ListElem * cur)
{
    if(cur == My402ListFirst(list))
        return NULL;
    else
        return cur -> prev;
}

My402ListElem * My402ListNext(My402List * list, My402ListElem * cur)
{
    if(cur == My402ListLast(list))
        return NULL;
    else
        return cur -> next;
}

My402ListElem * My402ListLast(My402List * list)
{
    if(My402ListEmpty(list))
        return NULL;
    else
        return (list -> anchor).prev;
}

My402ListElem * My402ListFirst(My402List * list)
{
    if(My402ListEmpty(list))
        return NULL;
    else
        return (list -> anchor).next;
}

int My402ListEmpty(My402List * list)
{
    return list -> num_members <= 0;
}

int My402ListLength(My402List * list)
{
    return list -> num_members;
}

int My402ListInit(My402List * list)
{
    (list -> anchor).next = &(list -> anchor);
    (list -> anchor).prev = &(list -> anchor);
    (list -> anchor).obj = NULL;
    list -> num_members = 0;
    return 1;
}



