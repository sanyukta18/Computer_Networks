new_dnsserver.c :
compile: gcc new_dnsserver.c -o ser
run:  ./ser

dnsclient.c : 
compile:  gcc dnsclient.c -o udpcli
run:  ./udpcli

newdnsclient.c : 
compile: gcc newdnsclient.c -o tcpcli
run:   ./tcpcli

