#include <time.h>
#include <malloc.h>
//#include <sys/sysinfo.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <panel.h>
#include "info.h"


#define NLINES 10
#define NCOLS 40
#define DELAY 1000*100

void init_wins(WINDOW** wins, int n);

struct ti {
    int o;
    long w;
};

void draw_title(int n);

int main()
{
    struct info inf;
    struct cpuinfo ci;
    process_list plist;
    plist.init = 0;
    mounts_list mlist;
    mlist.inst = 0;
    int len = 0;
    int tmr2 = 0;
    uint32_t flag_opt_1 = 0;
    char* user_name;
    WINDOW* my_wins[4];
    PANEL*  my_panels[4];
    PANEL*  top;
    char* PNAMES[] = { "general information",
                       "tasks running",
                       "disks usage and state",
                       "about and settings"
                     };
    int ch;
    int i;
    int pl;
    int row_first = 0;
    int it_pl = 0;

    /* getting partoions info*/
    part_stat partions;
    status_t st = get_part_stat(&partions);
    if (st != ok) {
        return -1;
    }
    get_mounts_stat(&mlist, 100);
    user_name = getlogin();
    /* инициализация ncurses */
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, 1);
    keypad(stdscr, TRUE);

    /* инициализация цветовой палитры */
    if (!has_colors()) {
        endwin();
        printf("\nОшибка! Не поддерживаются цвета\n");
        return 1;
    }
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_GREEN);
    init_pair(3, COLOR_WHITE,  COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_WHITE, COLOR_BLACK);
    // инициализируются окна
    init_wins(my_wins, 4);
    // создание панелей на основе созданных окон
    my_panels[0] = new_panel(my_wins[0]);
    my_panels[1] = new_panel(my_wins[1]);
    my_panels[2] = new_panel(my_wins[2]);
    my_panels[3] = new_panel(my_wins[3]);
    // устанавливаем указатели на следующее окно
    // для перехода при нажатии Tab на следующее окно
    set_panel_userptr(my_panels[0], my_panels[1]);
    set_panel_userptr(my_panels[1], my_panels[2]);
    set_panel_userptr(my_panels[2], my_panels[3]);
    set_panel_userptr(my_panels[3], my_panels[0]);

    // обновление и прорисовка
    update_panels();
    mvprintw(0, 0, "General    Tasks    Disks    About");
    mvprintw(LINES - 1, 0, "Tab - next panel F10 - Exit");
    doupdate();
    // т.к. панель с индексом 2 создавалась последней
    // значит она и будет верхней
    top = my_panels[3];
    pl = 3;
    draw_title(pl);
    // цикл обработки сообщений с клавиатуры
    while (1) {
        ch = getch();
        if (ch == KEY_F(10) || ch == 'q') break;

        switch (ch) {
        case '\t': // переход на следующую панель
            top = (PANEL*)panel_userptr(top);
            top_panel(top);

            if (pl == 3) {
                pl = 0;    // if last panel cnt switch it to 0
            } else {
                pl++;
            }
            draw_title(pl);
            break;
        case KEY_DOWN: { //
            if (pl == 1 && (row_first + LINES - 3) < (plist.num_nodes)) {
                row_first++;
            }
            if (pl == 1 && it_pl < plist.num_nodes) {
                it_pl++;
            }
        }
        break;
        case KEY_UP: { //
            if (pl == 1 && row_first > 0) {
                row_first--;
            }
            if (pl == 1 && it_pl > 0) {
                it_pl--;
            }
        }
        break;
        case KEY_F(8):
        case KEY_DL: {
            if (strcmp(plist.procs[it_pl].user_name, "root") && pl == 1) {
                int pid = plist.procs[it_pl].pid;
                kill(pid, SIGTERM);
            }
        }
        break;
        case ' ': {
            if (pl == 3) flag_opt_1 = ~flag_opt_1;
        }
        break;


        }
        wclear(top->win);
        //
        box(top->win, 0 , 0);
        wmove(top->win, 0, 2);

        wprintw(top->win, " %s ", PNAMES[pl]);
        switch (pl) {
        case 0: {
            mvprintw(LINES - 1, 0, "Tab - next panel F10 - Exit\t\t\t\t");
            // 0 первая панель с  общей инфомацией
            unsigned int hour, min, sec, rm;
            hour = inf.uptime / 3600;
            rm = inf.uptime % 3600;
            min = rm / 60;
            sec = rm % 60;

            uint32_t line = 1;
            wmove(top->win, line++, 1);
            wprintw(top->win, " Processor:\t %s", ci.name);
            wmove(top->win, line++, 1);
            wprintw(top->win, " Frequency:\t %u Mhz", ci.mhz);
            wmove(top->win, line++, 1);
            wprintw(top->win, " CPU load:\t %.1f%%", inf.load * 100);
            wmove(top->win, line++, 1);
            wprintw(top->win, " Memused1:\t %u MB", (inf.mi.memtotal - inf.mi.memfree - inf.mi.buffers - inf.mi.cashed) / 1024);
            wmove(top->win, line++, 1);
            wprintw(top->win, " Memused2:\t %u MB", (inf.mi.memtotal - inf.mi.memavailable) / 1024);
            wmove(top->win, line++, 1);
            wprintw(top->win, " Swapused:\t %u MB", (inf.mi.swaptotal - inf.mi.swapfree) / 1024);
            wmove(top->win, line++, 1);
            wprintw(top->win, " System are running for : %u hours %u minutes %u seconds", hour, min, sec);
            wmove(top->win, line++, 1);
            if (user_name)
                wprintw(top->win, " Currently logged user: %s", user_name);
            else
                wprintw(top->win, " Currently logged user: info unavailable");
            wmove(top->win, 0, 0);
        }
        break;
        case 1: {
            //
            mvprintw(LINES - 1, 0, "Tab - next panel F8 - Kill process F10 - Exit");
            int i;
            int start, end;
            int k = 1;
            if (plist.num_nodes < (LINES - 4)) {
                start = 0;
                end = plist.num_nodes - 1;
            } else {
                start = row_first;
                end = start + LINES - 4;
            }
            wmove(top->win, 0, 1);
            wprintw(top->win, " PID\tUSER\tCPU\tMEM\tPROGRAM  ");
            for (i = start; i < end; i++) {
                wmove(top->win, k, 1);
                wprintw(top->win, " %d\t%s\t%.2lf\t%.2lf  %s", plist.procs[i].pid, plist.procs[i].user_name, plist.procs[i].cpu, plist.procs[i].mem, plist.procs[i].cdm_str);
                if (i == it_pl) {
                    if (!strcmp(plist.procs[i].user_name, "root")) {
                        mvwchgat(top->win, k, 1, COLS - 3, A_UNDERLINE, 4, NULL);
                    } else {
                        mvwchgat(top->win, k, 1, COLS - 3, A_UNDERLINE, 5, NULL);
                    }
                }
                k++;
            }
        }
        break;
        case 2: {
            mvprintw(LINES - 1, 0, "Tab - next panel F10 - Exit\t\t\t\t\t");
            int i;
            wmove(top->win, 1, 1);
            wprintw(top->win, "Device\t\tFS\tTot(MB)\tAva(MB)\tUse\tMounted");
            for (i = 0; i < mlist.num_nodes; i++) {
                wmove(top->win, i + 2, 1);
                char str[256];
                uint32_t total = mlist.mounts[i].blocks;
                uint32_t ava = mlist.mounts[i].blocks_available;
                uint32_t use = mlist.mounts[i].use;
                sprintf(str, "%-s  %-8.8s\t%-u\t%-u\t%u%%\t%-s", mlist.mounts[i].fs_dev, mlist.mounts[i].type, total / 1024\
                        , ava / 1024, use, mlist.mounts[i].mnt_to);
                wprintw(top->win, "%s", str);
            }
        }
        break;
        case 3: {
            wmove(top->win, 2, 2);
            wprintw(top->win, "Program monit writen by hexlotar and ratuil");
            wmove(top->win, 3, 2);
            wprintw(top->win, "Now only one setting privided: ");
            wmove(top->win, 4, 2);
            wprintw(top->win, "show only user tasks: [ ]");
            wmove(top->win, 4, 25);
            if (flag_opt_1) {
                mvwaddch(top->win, 4, 25, 'X' | COLOR_PAIR(4));
            } else {
                mvwaddch(top->win, 4, 25, ' ' | COLOR_PAIR(4));
            }
            wmove(top->win, 4, 25);

        }
        break;
        }
        if (pl != 3)
            wmove(top->win, 1, 1);
        update_panels();
        doupdate();
        struct ti delay;
        delay.o = 0;
        delay.w = DELAY;
        // there we get first
        get_fir();
        len++;

        usleep(DELAY);
        if (len > 5) {
            //every 1000 seconds get second
            get_sec(&inf);

            if (plist.init) {
                destr(&plist);
            }
            if (flag_opt_1)
                get_p_stat(&plist, 512, user_name);
            else
                get_p_stat(&plist, 512, NULL);
            len = 0;

        }
        tmr2++;
        if (tmr2 > 50 * 60 * 5) {
            if (mlist.inst) {
                destr_mounts(&mlist);
            }
            get_mounts_stat(&mlist, 100);
            tmr2 = 0;
        }
        get_cpu_info(&ci);
        //sleep(1);
    }
    // уничтожение созданных панелей и окон
    for (i = 0; i < 4; ++i) {
        del_panel(my_panels[i]);
        delwin(my_wins[i]);
    }
    //завершение программы
    endwin();
    return 0;
}

// инициализируются окна
void init_wins(WINDOW** wins, int n)
{
    int x, y, i;

    y = 1;
    x = 0;

    for (i = 0; i < n; ++i) {
        wins[i] = newwin(LINES - 2, COLS, y, x);
        wbkgdset(wins[i], COLOR_PAIR(1));
        wclear(wins[i]);
        wrefresh(wins[i]);
    }
}

void draw_title(int n)
{
    switch (n) {
    case 0:
        mvchgat(0, 0, 7, A_BOLD, 2, NULL);
        mvchgat(0, 11, 5, 0, 3, NULL);
        mvchgat(0, 20, 5, 0, 3, NULL);
        mvchgat(0, 29, 5, 0, 3, NULL);
        break;
    case 1:
        mvchgat(0, 0, 7, 0, 3, NULL);
        mvchgat(0, 11, 5, A_BOLD, 2, NULL);
        mvchgat(0, 20, 5, 0, 3, NULL);
        mvchgat(0, 29, 5, 0, 3, NULL);
        break;
    case 2:
        mvchgat(0, 0, 7, 0, 3, NULL);
        mvchgat(0, 11, 5, 0, 3, NULL);
        mvchgat(0, 20, 5, A_BOLD, 2, NULL);
        mvchgat(0, 29, 5, 0, 3, NULL);
        break;
    case 3:
        mvchgat(0, 0, 7, 0, 3, NULL);
        mvchgat(0, 11, 5, 0, 3, NULL);
        mvchgat(0, 20, 5, 0, 3, NULL);
        mvchgat(0, 29, 5, A_BOLD, 2, NULL);
        break;
    }
}


