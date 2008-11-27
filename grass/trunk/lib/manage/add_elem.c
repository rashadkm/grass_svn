#include "list.h"
int add_element(const char *elem, const char *desc)
{
    int n;
    int nelem;

    if (*desc == 0)
	desc = elem;

    n = nlist - 1;
    nelem = list[n].nelem++;
    list[n].element = G_realloc(list[n].element, (nelem + 1) * sizeof(const char *));
    list[n].element[nelem] = G_store(elem);
    list[n].desc = G_realloc(list[n].desc, (nelem + 1) * sizeof(const char *));
    list[n].desc[nelem] = G_store(desc);

    return 0;
}
