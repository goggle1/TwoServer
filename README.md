TwoServer
=========

TwoServer is one server frame, study nginx server.

1. TwoServer vs nginx.
nginx is multi-process, TwoServer is multi-thread.

2. TwoServer vs OneServer.
OneServer contains one EventThread, N TaskThread, one TimerThread.
TwoServer contains only N TaskThreads.


