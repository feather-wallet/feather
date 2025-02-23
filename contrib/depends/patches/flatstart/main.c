extern long m_symlink(const char *target, const char *linkpath);
extern long m_mkdir(const char *pathname, unsigned int mode);
extern long m_execve(const char *filename, char *const argv[], char *const envp[]);
extern void m_exit(int status);

void start_c(int argc, char **argv, char **envp) {
    m_symlink("/app/gnu", "/gnu");

    // Qt expects certs to be here, see: qtbase/src/network/ssl/qsslsocket.cpp
    m_mkdir("/etc/ssl", 0755);
    m_symlink("/app/profile/etc/ssl/certs", "/etc/ssl/certs");

    // Stops Qt from complaining about not running in a utf-8 locale
    m_mkdir("/run/current-system", 0755);
    m_symlink("/app/profile/lib/locale", "/run/current-system/locale");

    // Launch Feather
    const char *exec_path = "/app/feather";
    int new_argc = argc;
    char *new_argv[new_argc + 1];

    new_argv[0] = (char *)exec_path;
    for (int i = 1; i < new_argc; i++) {
        new_argv[i] = argv[i];
    }
    new_argv[new_argc] = 0;

    long ret = m_execve(exec_path, new_argv, envp);

    m_exit((int)ret);
}
