#ifdef __LINUX__
#include <execinfo.h>
#include <ucontext.h>
#include <signal.h>
#include <cxxabi.h>
#include <cstdlib>
#include <cstdio>




/* This structure mirrors the one found in /usr/include/asm/ucontext.h */
typedef struct _sig_ucontext 
{
	unsigned long uc_flags;
	ucontext_t*	  uc_link;
	stack_t       uc_stack;
	sigcontext    uc_mcontext;
	sigset_t      uc_sigmask;
} sig_ucontext_t;


void crit_err_hdlr(int sig_num, siginfo_t* info, void* ucontext)
{
    sig_ucontext_t* uc;
    void* caller_address;
	void* array[50];
	int   size;
	char** messages;
	char* mangled_name = 0;
	char* offset_begin = 0;
	char* offset_end   = 0;

	char* real_name    = 0;
	int   status       = 0xFF;
	char* chosen_name  = 0;
	bool  cond;


	uc = (sig_ucontext_t*)ucontext;
	caller_address = (void*)uc->uc_mcontext.rip; /* x86-64 specific */
	
	std::std::fprintf(stderr, "signal %d (%s), address is %p from %p\n\n",
		sig_num,
		strsignal(sig_num),
		info->si_addr,
		caller_address
	);


    size 	 = backtrace(array, 50);
    array[1] = caller_address;
    messages = backtrace_symbols(array, size);    


    /* skip first stack frame (points here) */
    for (int i = 1; i < size && messages != NULL; ++i)
    {
        /* find parantheses and +address offset surrounding mangled name */
        for (char *p = messages[i]; *p; ++p)
        {
            // if 		(*p == '(')  { mangled_name = p;  		}
            // else if (*p == '+')  { offset_begin = p; 		}
            // else if (*p == ')')  { offset_end   = p; break; }
			cond = (*p == '('); mangled_name = (char*)(cond * (size_t)p + !cond * (size_t)mangled_name);
			cond = (*p == '+'); offset_begin = (char*)(cond * (size_t)p + !cond * (size_t)offset_begin);
			cond = (*p == ')'); offset_end   = (char*)(cond * (size_t)p + !cond * (size_t)offset_end  );
			if(cond) break;
		}


        if (mangled_name && offset_begin && offset_end && mangled_name < offset_begin)
        { /* if the line could be processed, attempt to demangle the symbol */
            *mangled_name++ = '\0';
            *offset_begin++ = '\0';
            *offset_end++   = '\0';
            real_name   = abi::__cxa_demangle(mangled_name, 0, 0, &status);

			chosen_name = (char*)( (status == 0) * (size_t)real_name + (status != 0) * (size_t)mangled_name );
			std::fprintf(stderr, "[bt]: (%d) %s : %s+%s%s\n",
				i,
				messages[i],
				chosen_name,
				offset_begin,
				offset_end
			);
            free(real_name);


        } else { /* Otherwise print the whole line */
			std::fprintf(stderr, "[bt]: (%d) %s\n", i, messages[i]);
        }
	}
	std::fprintf(stderr, "\n");


    free(messages);
    exit(EXIT_FAILURE);
}


int enable_sigaction_stacktrace() 
{
	struct sigaction sigact;
	sigact.sa_sigaction = crit_err_hdlr;
	sigact.sa_flags = SA_RESTART | SA_SIGINFO;

	int zeroisgood = sigaction(SIGSEGV, &sigact, nullptr);
 	if(zeroisgood) { /* not good codepath lol */
		std::fprintf(stderr, "error setting signal handler for %d (%s)\n", SIGSEGV, strsignal(SIGSEGV));
	}

	return !zeroisgood;
}


void override_stacktrace();


#endif