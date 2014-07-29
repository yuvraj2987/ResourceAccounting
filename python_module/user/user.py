import sys, os
import time,timeModule
import curses
 
def get_time():
    ts=timeModule.time()
    ret = ts[0]+(ts[1]+0.0)/1000000000
    return ret

def main():
    isoutside=0
    if (len(sys.argv)>1):
        if (sys.argv[1]=="-o"):
            isoutside=1

    start_time = time.time()
    filename="../log/log_"+time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime(start_time))+"_user.log"
    fuser=open (filename, "w")

    stdscr = curses.initscr()
    curses.cbreak()
    stdscr.keypad(1)

    stdscr.addstr(0,0,"recording user activity...")
    stdscr.addstr(1,0,"   i for inside")
    stdscr.addstr(2,0,"   o for inside")
    stdscr.addstr(3,0,"   m for moving")
    stdscr.addstr(4,0,"   c for custom")
    stdscr.addstr(5,0,"   q for quit")
    stdscr.addstr(6,0,"---------------------------")

    s="started "
    
    if (isoutside):
        s+="outside\n"
    else:
        s+="inside\n"
    
    stdscr.addstr(7,0,s)
    stdscr.addstr(10,0,"pressed key: ")
    stdscr.refresh()
    
    fuser.write("#time #0-inside/1-outside/2-moving/3-custom")
    fuser.write("%.6f %d\n" %(get_time(),isoutside))   


    key = ''
    
    curses.noecho()

    while key != ord('q'):
        key = stdscr.getch()
        stdscr.addch(10,13,key)
        stdscr.refresh()
        if (key==ord('i')):
            fuser.write("%.6f 0\n" %get_time())
        if (key==ord('o')):
            fuser.write("%.6f 1\n" %get_time())
        if (key==ord('m')):
            fuser.write("%.6f 2\n" %get_time())
        if (key==ord('c')):
            fuser.write("%.6f 3\n" %get_time())


    curses.endwin()
    fuser.flush()

if __name__ == '__main__':
    main()

