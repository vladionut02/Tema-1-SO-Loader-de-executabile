/*
 * Loader Implementation
 *
 * 2022, Operating Systems
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

#include "exec_parser.h"
static so_exec_t *exec;

static int pageSize;
static struct sigaction old_action;
static int file_descr;
typedef struct {
	int *mapped;
	int nr;
} TPage;

static void segv_handler(int signum, siginfo_t *info, void *context)
{
	char *addr;
	int rc;
	int flag = 0;
	int offset;

	//  Verificam daca semnalul este SIGSEGV 
	if (signum != SIGSEGV) {
		old_action.sa_sigaction(signum, info, context);
		return;
	}
	/* Obtain from siginfo_t the memory location
    	 which caused the page fault */

	addr = (char*)info->si_addr;

	/* Parcurgem toate segmentele si verificam daca 
	   adresa pe care o cautam exista in intervalul segmentului curent */

	for (int i = 0; i < exec->segments_no; i++) {
        so_seg_t* segment = &exec->segments[i];
		TPage* page = (TPage*)segment->data;
		if (page->mapped == NULL) {
			page->mapped = malloc((segment->mem_size / pageSize + 1)*sizeof(int));
			if (page->mapped == NULL) {
				perror("eroare alocare");
				exit(EXIT_FAILURE);
			}
		}
        if (addr >= segment->vaddr && addr <= segment->vaddr + segment->mem_size) {
			
			/* Obtain the page which caused the page fault */

            int page_nr = (unsigned int)(addr - segment->vaddr) / pageSize;
				offset = segment->offset + page_nr * pageSize;

				/* Check if page is already mapped
					If it is, call default handler*/

				for (int j = 0 ; j < page->nr; j++){
					if (page->mapped[j] == page_nr){
						flag = 1;
					}
				}
				if (flag == 1){
					old_action.sa_sigaction(signum, info, context);
					return;
				}
                void* mapped_page = mmap((void*) segment->vaddr + page_nr * pageSize, 
                                         pageSize, 
                                         PROT_WRITE, 
										 MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
                if (mapped_page == MAP_FAILED) {
                    perror("error mmap");
                    exit(EXIT_FAILURE);
                }
				if (segment->file_size > page_nr * pageSize) {
					if (segment->file_size < (page_nr + 1) * pageSize) {
						lseek(file_descr, offset, SEEK_SET);
						rc = read(file_descr, mapped_page,
						segment->file_size - page_nr * pageSize);
						if (rc == -1) {
							perror("eroare citire");
							exit(EXIT_FAILURE);
							}
				} 	else {
					lseek(file_descr, offset, SEEK_SET);
					rc = read(file_descr, mapped_page, pageSize);
					if (rc == -1) {
						perror("eroare citire2");
						exit(EXIT_FAILURE);
					}
				}
			}
			 /* Increase protection */
			rc = mprotect(mapped_page, pageSize,
				segment->perm);
			if (rc == -1) {
				perror("error mprotect");
				exit(EXIT_FAILURE);
			}

			page->mapped[page->nr] = page_nr;
			page->nr++;
			return;
		}
	}

	/* Nu face parte dintr-un segment cunoscut,
	   apelam handlerul default 
		*/
	old_action.sa_sigaction(signum, info, context);
}

int so_init_loader(void)
{
	pageSize = getpagesize();
	int rc;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = segv_handler;
	sa.sa_flags = SA_SIGINFO;
	rc = sigaction(SIGSEGV, &sa, NULL);
	if (rc < 0) {
		perror("sigaction");
		return -1;
	}
	return 0;
}

int so_execute(char *path, char *argv[])
{
	file_descr = open(path, O_RDONLY);
	if (file_descr == -1) {
		perror("error opening the file");
		exit(EXIT_FAILURE);
	}
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	for (int i = 0; i < exec->segments_no; i++) {
		exec->segments[i].data =
				malloc(sizeof(TPage));
		if (exec->segments[i].data == NULL) {
			perror("error mem alloc");
			exit(EXIT_FAILURE);
		}
	
	}
	so_start_exec(exec, argv);

	return -1;
}
