From erwin@andreasen.org Fri Jul 26 19:55:13 1996
Date: Fri, 26 Jul 1996 19:55:12 +0200 (MET DST)
From: erwin@andreasen.org
X-Sender: erwin@shire.shire.dk
To: MERC/Envy Mailing List <merc-l@webnexus.com>
Subject: More on signals: catching SIGSEGV
Message-ID: <Pine.LNX.3.91.960726155315.938A-100000@shire.shire.dk>
MIME-Version: 1.0
Content-Type: TEXT/PLAIN; charset=US-ASCII
Status: RO
X-Status: 

Most of the time, a segmentation fault produces a core file, which makes 
it easy to figure out what happened and which command did it. Sometimes, 
however, I've found the stack garbled, probably because of a buffer 
overflowing or something like swapping arguments to strcpy :)

I've added the below code to my server then: every time someone the MUD 
interprets a command, it saves a copy of it in a global buffer, including 
who did the command and where.

A handler is set up for SIGSEGV; if that error occurs and that global 
buffer is not empty, the handler writes the contents of that buffer into 
a file, then aborts the process.

What happens then is up to you: if you are using my board v2 system, you 
could call make_note (as shown below) at next boot and then post a note 
that shows the command. If you don't, you could modify the startup 
script to append the contents of that file to the log file.

I've also added a function that is called when exit() is called - there 
is a number of places where the MUD just exits, with no core or anything. 
IMHO most of them should be replace with abort() so you can examine the 
core in a debugger.

You will probably also need to clear the contents of last_command before 
rebooting or shutdown or other commands that all exit without any error 
being present (in Envy, after boot_db() returns).

To install the signal handler, call install_other_handlers() before entering 
the while (!merc_down) loop for the first time. Use 

last_command[0] = NUL; 

before exit'ing the MUD with error-level 0.

Note that after dispatching the command, I still save whatever was done 
in the last_command buffer, but put a (Finished) before it - so that you 
can see that the MUD finished interpretting the command, but it was still 
the last command executed and might have had something to do with the crash.

PS: There is something called "on_exit", that can install a handler like 
atexit, but that handler gets the errorlevel as parameter, how portable 
is that? Nothing about standard/conformability in my man page.

PPS: MSL = MAX_STRING_LENGTH

In merc.h:

extern char last_command [MSL];

#define LAST_COMMAND_FILE "last_command.txt"

In interp.c:

char last_command [MSL]; /* Global variable to hold the last input line */

In interpret():

    /* Record the command */

    sprintf (last_command, "[%5d] %s in [%5d] %s: %s",
        IS_NPC(ch) ? ch->pIndexData->vnum : 0,
        IS_NPC(ch) ? ch->short_descr : ch->name,
        ch->in_room ? ch->in_room->vnum : 0,
        ch->in_room ? ch->in_room->name : "(not in a room)",
        logline);

    /*
     * Dispatch the command.
     */
    (*cmd_table[cmd].do_fun) ( ch, argument);

    /* Record that the command was the last done, but it is finished */
    sprintf (last_command, "(Finished) [%5d] %s in [%5d] %s: %s",
        IS_NPC(ch) ? ch->pIndexData->vnum : 0,
        IS_NPC(ch) ? ch->short_descr : ch->name,
        ch->in_room ? ch->in_room->vnum : 0,
        ch->in_room ? ch->in_room->name : "(not in a room)",
        logline);


/* In comm.c or wherever */

/* Write last command */
void write_last_command ()
{
    int fd;

    /* Return if no last command - set before normal exit */
    if (!last_command[0])
        return;

    fd = open (LAST_COMMAND_FILE, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);

    if (fd < 0)
        return;

    write (fd, last_command, strlen(last_command));
    write (fd, "\n", 1);
    close (fd);
}

void nasty_signal_handler (int no)
{
    write_last_command();
    return;
}


/* Call this before starting the game_loop */
void install_other_handlers ()
{
    last_command [0] = NUL;

    if (atexit (write_last_command) != 0)
    {
        perror ("install_other_handlers:atexit");
        exit (1);
    }

    /* should probably check return code here */
    signal (SIGSEGV, nasty_signal_handler);

    /* Possibly other signals could be caught? */
}


/* In db.c, if you are using the board v2 code */
/* Add a call to this after booting the database */

/* Find out the last thing that happened before a crash */
void examine_last_command ()
{
    FILE *fp;
    char buf [MSL];

    fp = fopen (LAST_COMMAND_FILE, "r");
    if (!fp)
        return;

    fscanf (fp, "%[^\n]", buf);
    fclose (fp);
    unlink (LAST_COMMAND_FILE);

    make_note ("Implementor", "Q", "Imm", "Last recorded command before crash",
}


