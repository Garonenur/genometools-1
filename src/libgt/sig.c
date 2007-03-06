/*
  Copyright (c) 2006 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2006 Center for Bioinformatics, University of Hamburg
  See LICENSE file or http://genometools.org/license.html for license details.
*/

#include <libgt/sig.h>
#include <libgt/xposix.h>

void sig_register_all(void (*func)(int sigraised))
{
  /* POSIX */
  (void) xsignal(SIGABRT, func);
  (void) xsignal(SIGBUS, func);
  (void) xsignal(SIGFPE, func);
  (void) xsignal(SIGHUP, func);
  (void) xsignal(SIGILL, func);
  (void) xsignal(SIGINT, func);
  (void) xsignal(SIGPIPE, func);
  (void) xsignal(SIGQUIT, func);
  (void) xsignal(SIGSEGV, func);
  (void) xsignal(SIGTERM, func);
  (void) xsignal(SIGTSTP, func);
  (void) xsignal(SIGTTIN, func);
  (void) xsignal(SIGTTOU, func);
  (void) xsignal(SIGUSR1, func);
  (void) xsignal(SIGUSR2, func);
  (void) xsignal(SIGSYS, func);
  (void) xsignal(SIGXCPU, func);
  (void) xsignal(SIGXFSZ, func);

  /* OpenBSD */
#ifdef SIGEMT
  (void) xsignal(SIGEMT, func);
#endif
}

void sig_unregister_all(void)
{
  /* POSIX */
  (void) xsignal(SIGABRT, SIG_DFL);
  (void) xsignal(SIGBUS, SIG_DFL);
  (void) xsignal(SIGFPE, SIG_DFL);
  (void) xsignal(SIGHUP, SIG_DFL);
  (void) xsignal(SIGILL, SIG_DFL);
  (void) xsignal(SIGINT, SIG_DFL);
  (void) xsignal(SIGPIPE, SIG_DFL);
  (void) xsignal(SIGQUIT, SIG_DFL);
  (void) xsignal(SIGSEGV, SIG_DFL);
  (void) xsignal(SIGTERM, SIG_DFL);
  (void) xsignal(SIGTSTP, SIG_DFL);
  (void) xsignal(SIGTTIN, SIG_DFL);
  (void) xsignal(SIGTTOU, SIG_DFL);
  (void) xsignal(SIGUSR1, SIG_DFL);
  (void) xsignal(SIGUSR2, SIG_DFL);
  (void) xsignal(SIGSYS, SIG_DFL);
  (void) xsignal(SIGXCPU, SIG_DFL);
  (void) xsignal(SIGXFSZ, SIG_DFL);

  /* OpenBSD */
#ifdef SIGEMT
  (void) xsignal(SIGEMT, SIG_DFL);
#endif
}
