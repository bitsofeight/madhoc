#Makefil för Aodv daemon.

#Val av C-kompilator
CC = gcc

#Kompileringsflaggor
#Flags -DLOGMSG and -DDEBUG
CFLAGS = -O3 -Wall -I./ -DLOGMSG -I/usr/include/pcap

#Extra bibliotek
LIBS = -lpcap

#Filer som ingår
OBJS = RT.o rrep.o rreq.o timer.o to_rreq.o aodv_daemon.o gen_rrep.o rerr.o rreq_list.o update_reverse.o utils.o uio.o gen_rreq.o logmsg.o find_inactives.o krtable.o packetcap.o


#Regler

all : aodv_daemon manpages

aodv_daemon :	$(OBJS)
		$(CC)  -o aodv_daemon $(OBJS)  $(LIBS)
manpages:	
		groff -t -mdoc -Tascii man/man8/aodv_daemon.mdoc > man/man8/aodv_daemon.8
		gzip man/man8/aodv_daemon.8 
clean:
		rm -f *.o *~ \#*
		rm -f man/man8/*.8  man/man8/*~ man/man8/*.gz
		rm -f aodv_daemon

#Beroenden
RT.o : RT.h rt_entry_list.h rt_entry.h precursor.h krtable.h
rrep.o : rrep.h RT.h utils.h rt_entry.h info.h aodv.h krtable.h
rreq.o : rreq.h RT.h utils.h gen_rrep.h rreq_list.h update_reverse.h aodv.h rt_entry.h info.h
timer.o : timer.h utils.h
to_rreq.o : to_rreq.h timer.h RT.h
aodv_daemon.o : rreq.h rrep.h RT.h timer.h to_rreq.h aodv.h info.h logmsg.h
gen_rrep.o : gen_rrep.h RT.h utils.h rt_entry.h info.h aodv.h
gen_rreq.o : gen_rreq.h RT.h utils.h rt_entry.h info.h aodv.h timer.h to_rreq.h
rerr.o : utils.h RT.h aodv.h rt_entry.h info.h
rreq_list.o : rreq_list.h utils.h
update_reverse.o : RT.h utils.h rt_entry.h info.h aodv.h krtable.h
utils.o : utils.h info.h aodv.h logmsg.h
uio.o : uio.h aodv.h info.h
logmsg.o : aodv.h utils.h
find_inactives.o : rerr.h aodv.h utils.h rt_entry.h info.h RT.h
packetcap.o : RT.h utils.h








