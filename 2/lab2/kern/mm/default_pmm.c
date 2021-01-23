#include <pmm.h>
#include <list.h>
#include <string.h>
#include <default_pmm.h>

//空闲块管理数据结构
free_area_t free_area;

#define free_list (free_area.free_list)             //双向链表指针
#define nr_free (free_area.nr_free)                 //记录当前空闲页的个数的无符号整型变量nr_free

//对free_area_t的双向链表和空闲块的数目进行初始化
static void
default_init(void) {
    list_init(&free_list);
    nr_free = 0;
}

//初始化空闲页链表,初始化每一个空闲页，然后计算空闲页的总数
/*
具体的过程就是​先传入物理页基地址和物理页的个数(个数必须大于0)，
然后对每一块物理页进行设置：先判断是否为保留页，如果不是，则进行下一步。
将标志位清0，连续空页个数清0，然后将标志位设置为1，将引用此物理页的虚拟页的个数清0。
然后再加入空闲链表。最后计算空闲页的个数，修改物理基地址页的property的个数为n
*/
static void
default_init_memmap(struct Page *base, size_t n) {  //基地址和物理页数量
    assert(n > 0);
    struct Page *p = base;
    for (; p != base + n; p ++) {                   //处理每一页
        assert(PageReserved(p));                    //是否为保留页
        p->flags = p->property = 0;                 //标志位清零, 空闲块数量
        set_page_ref(p, 0);                         //将引用此物理页的虚拟页的个数清0
    }
    base->property = n;                             //开头页面的空闲块数量设置为n
    SetPageProperty(base);      
    nr_free += n;                                   //新增空闲页个数nr_free
    list_add_before(&free_list, &(base->page_link));//将新增的Page加在双向链表指针中
}


//FIRST FIT分配
/*
FIRST FIT分配, 从空闲页块的链表中去遍历，找到第一块大小大于n的块，
然后分配出来，把它从空闲页链表中除去，然后如果有多余的，把分完剩下的部分再次加入会空闲页链表中

首先判断空闲页的大小是否大于所需的页块大小。
然后遍历整个空闲链表。如果找到合适的空闲页，即p->property >= n（从该页开始，连续的空闲页数量大于n）
即可认为可分配，重新设置标志位。具体操作是调用SetPageReserved(pp)和ClearPageProperty(pp)，设置当前页面预留，
以及清空该页面的连续空闲页面数量值。然后从空闲链表，即free_area_t中，记录空闲页的链表，删除此项。

如果当前空闲页的大小大于所需大小。则分割页块。具体操作就是，刚刚分配了n个页，如果分配完了，还有连续的空间，
则在最后分配的那个页的下一个页（未分配），更新它的连续空闲页值。如果正好合适，则不进行操作。
最后计算剩余空闲页个数并返回分配的页块地址。
*/
static struct Page *
default_alloc_pages(size_t n) {
    assert(n > 0);                                              
    if (n > nr_free) {                                              //检查空闲页的大小是否大于所需的页块大小
        return NULL;
    }
    struct Page *page = NULL;
    list_entry_t *le = &free_list;
    while ((le = list_next(le)) != &free_list) {                    //遍历链表找到合适的空闲页
        struct Page *p = le2page(le, page_link);
        if (p->property >= n) {                                     //第一个目标页
            page = p;
            break;
        }
    }
    if (page != NULL) {                                             //如果页的空闲块数量大于n 执行分割
        if (page->property > n) {
            struct Page *p = page + n;                              //指针后移n单位
            p->property = page->property - n;                       //数量减n
            SetPageProperty(p);                                     //调SetPageProperty设置当前页面预留
            list_add_after(&(page->page_link), &(p->page_link));    //链接被分割之后剩下的空闲块地址
        }
        list_del(&(page->page_link));                               //删除pageLink链接
        nr_free -= n;                                               //更新空闲页个数
        ClearPageProperty(page);                                    //清空该页面的连续空闲页面数量值
    }
    return page;
}

//释放页
/*
首先检查基地址所在的页是否为预留，如果不是预留页，那么说明它已经是free状态，无法再次free，只有处在占用的页，才能有free操作
接着，声明一个页p，p遍历一遍整个物理空间，直到遍历到base所在位置停止，开始释放操作
找到了这个基地址之后，将空闲页重新加进来（之前在分配的时候删除），设置一系列标记位
检查合并, 如果插入基地址附近的高地址或低地址可以合并，那么需要更新相应的连续空闲页数量，向高合并和向低合并。
*/
static void
default_free_pages(struct Page *base, size_t n) {
    assert(n > 0); 
    //assert(PageReserved(base));                         //检查基地址所在的页是否为预留                                  
    struct Page *p = base;
    for (; p != base + n; p ++) {                       //遍历整个物理空间
        assert(!PageReserved(p) && !PageProperty(p));   //检查是否是预留或者head页,内核保留无法分配释放, 只有PageProperty有效才是!free状态
        p->flags = 0;                                   //设置flage标志
        set_page_ref(p, 0);                             //将引用此物理页的虚拟页的个数清0, 这里类似与初始化的设置
    }
    base->property = n;                                 //设置空闲块数量
    SetPageProperty(base);                              //调SetPageProperty设置当前页面预留仅仅head
    list_entry_t *le = list_next(&free_list);           //新建一个管理结构,为链表头的子节点
    while (le != &free_list) {                          //遍历
        p = le2page(le, page_link);                     //新页le2page就是初始化为一个Page结构
        le = list_next(le);                             //取下一个节点

        //向高地址合并
        if (base + base->property == p) {               //如果到达当前页的尾部地址,执行合并
            base->property += p->property;              //增加头页的property
            ClearPageProperty(p);                       //清空该页面的连续空闲页面数量值
            list_del(&(p->page_link));                  //删除旧的索引
        }
        //向低地址合并
        else if (p + p->property == base) {             //同上
            p->property += base->property;              //增加头页的property
            ClearPageProperty(base);                    //clear 
            base = p;                                   //迭代交换base
            list_del(&(p->page_link));                  //删除旧的索引
        }
    }

    //检查合并结果是否发生了错误
    nr_free += n;                                       //新增空闲快数量
    le = list_next(&free_list);                         
    while (le != &free_list) {                          
        p = le2page(le, page_link);
        if (base + base->property <= p) {               //如果当前页的尾部地址小于等于p
            assert(base + base->property != p);         //检查是否不等于p
            break;
        }
        le = list_next(le);
    }
    list_add_before(le, &(base->page_link));            //将新增的Page加在双向链表指针中
}

static size_t
default_nr_free_pages(void) {
    return nr_free;
}



static void
basic_check(void) {
    struct Page *p0, *p1, *p2;
    p0 = p1 = p2 = NULL;
    assert((p0 = alloc_page()) != NULL);
    assert((p1 = alloc_page()) != NULL);
    assert((p2 = alloc_page()) != NULL);

    assert(p0 != p1 && p0 != p2 && p1 != p2);
    assert(page_ref(p0) == 0 && page_ref(p1) == 0 && page_ref(p2) == 0);

    assert(page2pa(p0) < npage * PGSIZE);
    assert(page2pa(p1) < npage * PGSIZE);
    assert(page2pa(p2) < npage * PGSIZE);

    list_entry_t free_list_store = free_list;
    list_init(&free_list);
    assert(list_empty(&free_list));

    unsigned int nr_free_store = nr_free;
    nr_free = 0;

    assert(alloc_page() == NULL);

    free_page(p0);
    free_page(p1);
    free_page(p2);
    assert(nr_free == 3);

    assert((p0 = alloc_page()) != NULL);
    assert((p1 = alloc_page()) != NULL);
    assert((p2 = alloc_page()) != NULL);

    assert(alloc_page() == NULL);

    free_page(p0);
    assert(!list_empty(&free_list));

    struct Page *p;
    assert((p = alloc_page()) == p0);
    assert(alloc_page() == NULL);

    assert(nr_free == 0);
    free_list = free_list_store;
    nr_free = nr_free_store;

    free_page(p);
    free_page(p1);
    free_page(p2);
}

// LAB2: below code is used to check the first fit allocation algorithm (your EXERCISE 1) 
// NOTICE: You SHOULD NOT CHANGE basic_check, default_check functions!
static void
default_check(void) {
    int count = 0, total = 0;
    list_entry_t *le = &free_list;
    while ((le = list_next(le)) != &free_list) {
        struct Page *p = le2page(le, page_link);
        assert(PageProperty(p));
        count ++, total += p->property;
    }
    assert(total == nr_free_pages());

    basic_check();

    struct Page *p0 = alloc_pages(5), *p1, *p2;
    assert(p0 != NULL);
    assert(!PageProperty(p0));

    list_entry_t free_list_store = free_list;
    list_init(&free_list);
    assert(list_empty(&free_list));
    assert(alloc_page() == NULL);

    unsigned int nr_free_store = nr_free;
    nr_free = 0;

    free_pages(p0 + 2, 3);
    assert(alloc_pages(4) == NULL);
    assert(PageProperty(p0 + 2) && p0[2].property == 3);
    assert((p1 = alloc_pages(3)) != NULL);
    assert(alloc_page() == NULL);
    assert(p0 + 2 == p1);

    p2 = p0 + 1;
    free_page(p0);
    free_pages(p1, 3);
    assert(PageProperty(p0) && p0->property == 1);
    assert(PageProperty(p1) && p1->property == 3);

    assert((p0 = alloc_page()) == p2 - 1);
    free_page(p0);
    assert((p0 = alloc_pages(2)) == p2 + 1);

    free_pages(p0, 2);
    free_page(p2);

    assert((p0 = alloc_pages(5)) != NULL);
    assert(alloc_page() == NULL);

    assert(nr_free == 0);
    nr_free = nr_free_store;

    free_list = free_list_store;
    free_pages(p0, 5);

    le = &free_list;
    while ((le = list_next(le)) != &free_list) {
        struct Page *p = le2page(le, page_link);
        count --, total -= p->property;
    }
    assert(count == 0);
    assert(total == 0);
}

const struct pmm_manager default_pmm_manager = {
    .name = "default_pmm_manager",
    .init = default_init,
    .init_memmap = default_init_memmap,
    .alloc_pages = default_alloc_pages,
    .free_pages = default_free_pages,
    .nr_free_pages = default_nr_free_pages,
    .check = default_check,
};


