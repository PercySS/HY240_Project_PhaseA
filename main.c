/***************************************************************
 *
 * file: main.c
 *
 * @Author  Dimitrios Makrogiannis (csd4676@csd.uoc.gr)
 * @Version Final
 *
 * @brief   Main function for the needs of CS-240 project 2022.Prof. Panagiota Fatourou.
 * @see     Compile with command: gcc -ansi main.c pss.c -o run
 ***************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include "pss.h"

#define BUFFER_SIZE 1024 /* Maximum length of a line in input file */

/* Uncomment the following line to enable debugging prints
 * or comment to disable it */
#define DEBUG
#ifdef DEBUG
#define DPRINT(...) fprintf(stderr, __VA_ARGS__);
#else /* DEBUG */
#define DPRINT(...)
#endif /* DEBUG */

/**
 * @brief Parsing event arguments from buffer
 *
 * @param buff String of current line in the test file
 * @param int_arr_size Pointer to integer that represents the number of arguments in the test after the Event char
 *
 * @return 0 on success
 *         1 on failure
 */
static int *event_args(char *buff, unsigned int *int_arr_size)
{
	int *int_arr;
	char event = '\0';
	int value;
	char *p = buff + 1;
	unsigned int len = 0;
	unsigned int count_args = 1;
	unsigned int i = 0;

	sscanf(buff, "%c", &event);

	while (sscanf(p, "%d%n", &value, &len) == 1 && value != -1)
	{
		count_args++;
		p += len;
	}

	p = buff + 1;
	(*int_arr_size) = count_args;
	int_arr = malloc(count_args * sizeof(int));

	while (sscanf(p, "%d%n", &value, &len) == 1)
	{
		int_arr[i++] = value;
		p += len;
		if (value == -1)
		{
			break;
		}
	}

	return int_arr;
}

/* Function Declarations */
void Print_Subscriber_List(struct SubInfo *);
void Print_Group_Subscriber_List(struct Group *);

/**
 * @brief The main function
 *
 * @param argc Number of arguments
 * @param argv Argument vector
 *
 * @return 0 on success
 *         1 on failure
 */
int main(int argc, char **argv)
{
	int MG = 0;
	FILE *fin = NULL;
	char buff[BUFFER_SIZE], event = '\0';
	struct Group *G;   /* Πίνακας Group */
	struct SubInfo *S; /* Πρώτο στοιχείο λίστας συνδρομητών */

	/* Check command buff arguments */
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s <no_of_groups> <input_file>\n", argv[0]);

		return EXIT_FAILURE;
	}

	/* BONUS: Set he number of groups dynamically */
	MG = atoi(argv[1]);
	G = (struct Group *)malloc(MG * sizeof(struct Group));

	/* Open input file */
	if ((fin = fopen(argv[2], "r")) == NULL)
	{
		fprintf(stderr, "\n Could not open file: %s\n", argv[1]);
		perror("Opening test file\n");

		return EXIT_FAILURE;
	}

	/* Initializations */
	initialize(G, MG, &S);

	/* Read input file buff-by-buff and handle the events */
	while (fgets(buff, BUFFER_SIZE, fin))
	{
		DPRINT("\n>>> Event: %s\n", buff);
		switch (buff[0])
		{
		/* Comment */
		case '#':
			break;

		/* Insert_Info
		 * I <itm> <iId> <gId1> <gId2> ... <gIdk> -1 */
		case 'I':
		{
			int itm;
			int iId;
			unsigned int num_of_gids;
			int *event_args_arr;
			int *gids_arr;

			event_args_arr = event_args(buff, &num_of_gids);
			itm = event_args_arr[0];
			iId = event_args_arr[1];
			gids_arr = event_args_arr + 2;
			num_of_gids -= 2;

			if (Insert_Info(G, MG, itm, iId, gids_arr, num_of_gids) == 0)
			{
				DPRINT("%c <%d> <%d> DONE\n", buff[0], itm, iId);

				int i;
				for (i = 0; i < MG && G[i].gId != -1; i++)
				{
					DPRINT("  GROUPID = <%d>", G[i].gId);

					struct Info *curr = G[i].gfirst;

					DPRINT(", INFOLIST = <");

					while (curr != NULL)
					{
						DPRINT("%d", curr->iId);
						curr = curr->inext;

						if (curr != NULL)
						{
							DPRINT(",");
						}
					}

					DPRINT(">\n");
				}
			}
			else
			{
				fprintf(stderr, "%s failed\n", buff);
			}

			num_of_gids = 0;

			free(event_args_arr);

			break;
		}

		/* Subscriber_Registration
		 * S <sΤΜ> <sId> <gId1> <gId2> ... <gIdm> -1  */
		case 'S':
		{
			int sTM, sId;
			unsigned int num_of_gids;
			int *event_args_arr;
			int *gids_arr;

			event_args_arr = event_args(buff, &num_of_gids);
			sTM = event_args_arr[0];
			sId = event_args_arr[1];
			gids_arr = event_args_arr + 2;
			num_of_gids -= 2;

			if (Subscriber_Registration(G, MG, &S, sTM, sId, gids_arr, num_of_gids) == 0)
			{
				DPRINT("%c <%d> <%d> DONE\n", buff[0], sTM, sId);

				Print_Subscriber_List(S);
				int i;
				for (i = 0; i < MG && G[i].gId != -1; i++)
				{
					if (G[i].ggsub == NULL)
					{
						continue;
					}

					Print_Group_Subscriber_List(&G[i]);
				}
			}
			else
			{
				fprintf(stderr, "%s failed\n", buff);
			}

			num_of_gids = 0;

			free(event_args_arr);

			break;
		}

		/* Consume
		 * C <sId> */
		case 'C':
		{
			int sId;
			sscanf(buff, "%c %d", &event, &sId);

			/* Keeping a copy of the last infos that were consumed */
			struct Info **sgp_copy = (struct Info **)malloc(MG * sizeof(struct Info *));

			/* Find the subscriber object */
			struct SubInfo *subscriber = S;
			while (subscriber != NULL)
			{
				if (subscriber->sId == sId)
				{
					break;
				}

				subscriber = subscriber->snext;
			}

			if (subscriber == NULL)
			{
				fprintf(stderr, "%s failed\n", buff);
				break;
			}
			else
			{
				/* Keep a copy of the subscriber's group list */
				int i;
				for (i = 0; i < MG; i++)
				{
					sgp_copy[i] = subscriber->sgp[i];
				}
			}

			if (Consume(subscriber, G, MG, sId) == 0)
			{
				DPRINT("%c <%d> DONE\n", event, sId);

				int i;
				for (i = 0; i < MG; i++)
				{
					struct Group group = G[i];
					struct Info *lastInfo = sgp_copy[i];
					struct Info *cur = NULL;

					/* Not interested in this group */
					if (lastInfo == (struct Info *)1)
					{
						continue;
					}

					DPRINT("  GROUPID = <%d>", group.gId);

					cur = lastInfo == NULL ? group.gfirst : lastInfo->inext;

					DPRINT(", INFOLIST = <");

					if (cur != NULL)
					{
						while (cur->inext != NULL)
						{
							DPRINT("%d, ", cur->iId);
							cur = cur->inext;
						}

						DPRINT("%d>", cur->iId);
						DPRINT(", NEWSGP = <%d>\n", cur->iId);
					}
					else
					{
						DPRINT(">");

						if (lastInfo == NULL)
						{
							DPRINT(", NEWSGP = <>\n");
						}
						else
						{
							DPRINT(", NEWSGP = <%d>\n", lastInfo->iId);
						}
					}
				}
			}
			else
			{
				fprintf(stderr, "%c %d failed\n", event, sId);
			}
			break;
		}

		/* Delete_Subscriber
		 * D <sId>: */
		case 'D':
		{
			int sId;
			sscanf(buff, "%c %d", &event, &sId);

			if (Delete_Subscriber(G, MG, &S, sId) == 0)
			{
				DPRINT("%c <%d> DONE\n", event, sId);

				Print_Subscriber_List(S);
				int i;
				for (i = 0; i < MG && G[i].gId != -1; i++)
				{
					/* if (G[i].ggsub == NULL)
					{
						continue;
					} */

					Print_Group_Subscriber_List(&G[i]);
				}
			}
			else
			{
				fprintf(stderr, "%c %d failed\n", event, sId);
			}
			break;
		}

		/* Print
		 * P */
		case 'P':
		{
			if (Print_all(G, S) == 0)
			{
				DPRINT("%c DONE\n", buff[0]);

				int group_count = 0, i;
				for (i = 0; i < MG && G[i].gId != -1; i++)
				{
					group_count++;
					DPRINT("  GROUPID = <%d>", G[i].gId);

					struct Info *currInfo = G[i].gfirst;

					DPRINT(", INFOLIST = <");

					while (currInfo != NULL)
					{
						DPRINT("%d", currInfo->iId);
						currInfo = currInfo->inext;

						if (currInfo != NULL)
						{
							DPRINT(",");
						}
					}

					DPRINT(">");

					struct Subscription *currSubscription = G[i].ggsub;

					DPRINT(", SUBLIST = <");

					while (currSubscription != NULL)
					{
						DPRINT("%d", currSubscription->sId);
						currSubscription = currSubscription->snext;

						if (currSubscription != NULL)
						{
							DPRINT(",");
						}
					}

					DPRINT(">\n");
				}

				Print_Subscriber_List(S);

				struct SubInfo *currSubscriber = S;

				int subscriber_count = 0;
				while (currSubscriber != NULL)
				{
					subscriber_count++;
					DPRINT("  SUBSCRIBERID = <%d>", currSubscriber->sId);

					DPRINT(", GROUPLIST = <");

					int first = 1, i;
					for (i = 0; i < MG && G[i].gId != -1; i++)
					{
						if (currSubscriber->sgp[i] != (struct Info *)1)
						{
							if (first)
							{
								first = 0;
							}
							else
							{
								DPRINT(",");
							}

							DPRINT("%d", G[i].gId);
						}
					}

					DPRINT(">\n");

					currSubscriber = currSubscriber->snext;
				}

				DPRINT("  NO_GROUPS = <%d>, NO_SUBSCRIBERS = <%d>", group_count, subscriber_count);
			}
			else
			{
				fprintf(stderr, "%c failed\n", buff[0]);
			}

			break;
		}

		/* Empty line */
		case '\n':
			break;

		/* Ignore everything else */
		default:
			DPRINT("Ignoring line: %s \n", buff);
			break;
		}
	}

	free_all(&G, MG, &S);

	return (EXIT_SUCCESS);
}

void Print_Subscriber_List(struct SubInfo *S)
{
	DPRINT("  SUBSCRIBERLIST = <");
	struct SubInfo *curr = S;

	while (curr != NULL)
	{
		DPRINT("%d", curr->sId);

		curr = curr->snext;

		if (curr != NULL)
		{
			DPRINT(",");
		}
	}

	DPRINT(">\n");
}

void Print_Group_Subscriber_List(struct Group *G)
{
	DPRINT("  GROUPID = <%d>", G->gId);

	struct Subscription *curr = G->ggsub;

	DPRINT(", SUBLIST = <");

	while (curr != NULL)
	{
		DPRINT("%d", curr->sId);
		curr = curr->snext;

		if (curr != NULL)
		{
			DPRINT(",");
		}
	}

	DPRINT(">\n");
}
