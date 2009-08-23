#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/lib/queue.h"

int main(int argc, char **argv)
{
	/*
	 * Declaramos la cabeza de la TAILQ
	 * entry = tipo de elemenot que vamos a encolar 
	 */
	TAILQ_HEAD(tailhead, entry) head;

	/* Declaramos una entrada de la TAILQ agregandole los campos que queramos */
	struct entry
	{
		TAILQ_ENTRY(entry) entries; /* Tail queue. */
		int e;
	}*n1, *n2, *np;
	
	/* Inicializamos la TAILQ */
	TAILQ_INIT(&head);
	
	n1 = malloc(sizeof(struct entry));
	n1->e = 1;
	/* Insert at the head. */
	TAILQ_INSERT_HEAD(&head, n1, entries);
	
	n1 = malloc(sizeof(struct entry));
	n1->e = 2;
	/* Insert at the tail. */
	TAILQ_INSERT_TAIL(&head, n1, entries);
	
	n2 = malloc(sizeof(struct entry));
	n2->e = 3;
	/* Insert after. */
	TAILQ_INSERT_AFTER(&head, n1, n2, entries);
	
	n2 = malloc(sizeof(struct entry));
	n2->e = 4;
	/* Insert before. */
	TAILQ_INSERT_BEFORE(n1, n2, entries);
	
	/* Forward traversal. */
	TAILQ_FOREACH(np, &head, entries)
	{
		printf("%d ", np->e);	
	}

	puts("");
	
	/* Reverse traversal. */
	TAILQ_FOREACH_REVERSE(np, &head, tailhead, entries)
	{
		printf("%d ", np->e);
	}

	/* Delete. */
	while (TAILQ_FIRST(&head) != NULL)
	{
		TAILQ_REMOVE(&head, TAILQ_FIRST(&head), entries);
	}
	
	if (TAILQ_EMPTY(&head))
	{
		printf("nothing to do\n");
	}
	
	return 0;
}
