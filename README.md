Small reproducer for a puzzling observation of the time it takes to read a (big)
raw data file : time is big *and* quite dependent on the reading method used.

cmake .
make
./read $HOME/cernbox/o2muon/data-de819-sr90-20200206.raw 20

The 20 means create a 20GB dummy file to be used as a poor's man (and probably
wrong) approach to SSD cache wiping between tests (using `dd if=...`)

The results on a MacBook Pro (i.e. with a SSD drive) are  below. You'll observe
that the method all (i.e. reading sequentially everything instead of using one
of the SEEK methods) is *much* better in this case.

(the data-de819-sr90-20200206.raw file can be taken from <https://cernbox.cern.ch/index.php/s/PiIVnjRZNyI7JoZ>)

```
$ ./read /Users/laurent/cernbox/o2muon/data-de819-sr90-20200206.raw 20
[100%] Built target read
Creating bigfile dummy-big-file.txt of size 20 GB ...: 21474836480 bytes in    8.6 seconds  2386 MB/s
Reading bigfile with blockSize=512
: 21474836480 bytes in   47.3 seconds   433 MB/s
Reading bigfile with blockSize=1024
: 21474836480 bytes in   26.6 seconds   769 MB/s
Reading bigfile with blockSize=2048
: 21474836480 bytes in   17.9 seconds  1142 MB/s
Reading bigfile with blockSize=4096
: 21474836480 bytes in   11.4 seconds  1796 MB/s
Reading bigfile with blockSize=8192
: 21474836480 bytes in    8.5 seconds  2408 MB/s
Reading bigfile with blockSize=16384
: 21474836480 bytes in    7.2 seconds  2825 MB/s
Reading dummy-big-file.txt...Reading bigfile with blockSize=8192
: 21474836480 bytes in    8.2 seconds  2510 MB/s
Reading /Users/laurent/cernbox/o2muon/data-de819-sr90-20200206.raw(method all)...: 10485170176 bytes in    4.4 seconds  2284 MB/s
Reading dummy-big-file.txt...Reading bigfile with blockSize=8192
: 21474836480 bytes in    7.9 seconds  2583 MB/s
Reading /Users/laurent/cernbox/o2muon/data-de819-sr90-20200206.raw(method set)...: 10485170176 bytes in   89.8 seconds   111 MB/s
Reading dummy-big-file.txt...Reading bigfile with blockSize=8192
: 21474836480 bytes in    8.6 seconds  2389 MB/s
Reading /Users/laurent/cernbox/o2muon/data-de819-sr90-20200206.raw(method cur)...: 10485170176 bytes in   79.6 seconds   126 MB/s
Reading dummy-big-file.txt...Reading bigfile with blockSize=8192
: 21474836480 bytes in    8.3 seconds  2454 MB/s
Reading /Users/laurent/cernbox/o2muon/data-de819-sr90-20200206.raw(method all)...: 10485170176 bytes in    4.5 seconds  2201 MB/s
Reading dummy-big-file.txt...Reading bigfile with blockSize=8192
: 21474836480 bytes in    8.1 seconds  2516 MB/s
Reading /Users/laurent/cernbox/o2muon/data-de819-sr90-20200206.raw(method cur)...: 10485170176 bytes in   77.3 seconds   129 MB/s
Reading dummy-big-file.txt...Reading bigfile with blockSize=8192
: 21474836480 bytes in    8.6 seconds  2383 MB/s
Reading /Users/laurent/cernbox/o2muon/data-de819-sr90-20200206.raw(method set)...: 10485170176 bytes in   94.4 seconds   106 MB/s
Reading dummy-big-file.txt...Reading bigfile with blockSize=8192
: 21474836480 bytes in    8.3 seconds  2453 MB/s
Reading /Users/laurent/cernbox/o2muon/data-de819-sr90-20200206.raw(method cur)...: 10485170176 bytes in   70.0 seconds   143 MB/s
Reading dummy-big-file.txt...Reading bigfile with blockSize=8192
: 21474836480 bytes in    8.4 seconds  2445 MB/s
Reading /Users/laurent/cernbox/o2muon/data-de819-sr90-20200206.raw(method all)...: 10485170176 bytes in    4.5 seconds  2219 MB/s

```

While on a Linux machine (with a spinning drive) are : 

    $ ./read /home/aphecetche/data/data-de819-sr90-20200206.raw 20
    Creating bigfile dummy-big-file.txt of size 20 GB ...: 21474836480 bytes in  151.6 seconds   135 MB/s
    Reading bigfile with blockSize=512
    : 21474836480 bytes in   56.3 seconds   364 MB/s
    Reading bigfile with blockSize=1024
    : 21474836480 bytes in   29.5 seconds   694 MB/s
    Reading bigfile with blockSize=2048
    : 21474836480 bytes in   16.6 seconds  1236 MB/s
    Reading bigfile with blockSize=4096
    : 21474836480 bytes in   10.0 seconds  2050 MB/s
    Reading bigfile with blockSize=8192
    : 21474836480 bytes in    6.9 seconds  2984 MB/s
    Reading bigfile with blockSize=16384
    : 21474836480 bytes in    5.7 seconds  3616 MB/s
    Reading dummy-big-file.txt...Reading bigfile with blockSize=8192
    : 21474836480 bytes in    6.8 seconds  2990 MB/s
    Reading /home/aphecetche/data/data-de819-sr90-20200206.raw(method all)...: 10485170176 bytes in  118.7 seconds    84 MB/s
    Reading dummy-big-file.txt...Reading bigfile with blockSize=8192
    : 21474836480 bytes in    7.4 seconds  2766 MB/s
    Reading /home/aphecetche/data/data-de819-sr90-20200206.raw(method set)...: 10485170176 bytes in    2.9 seconds  3426 MB/s
    Reading dummy-big-file.txt...Reading bigfile with blockSize=8192
    : 21474836480 bytes in    7.2 seconds  2851 MB/s
    Reading /home/aphecetche/data/data-de819-sr90-20200206.raw(method cur)...: 10485170176 bytes in    3.1 seconds  3250 MB/s
    Reading dummy-big-file.txt...Reading bigfile with blockSize=8192
    : 21474836480 bytes in    7.1 seconds  2871 MB/s
    Reading /home/aphecetche/data/data-de819-sr90-20200206.raw(method all)...: 10485170176 bytes in    4.3 seconds  2329 MB/s
    Reading dummy-big-file.txt...Reading bigfile with blockSize=8192
    : 21474836480 bytes in    6.9 seconds  2966 MB/s
    Reading /home/aphecetche/data/data-de819-sr90-20200206.raw(method cur)...: 10485170176 bytes in    2.7 seconds  3742 MB/s
    Reading dummy-big-file.txt...Reading bigfile with blockSize=8192
    : 21474836480 bytes in    7.2 seconds  2841 MB/s
    Reading /home/aphecetche/data/data-de819-sr90-20200206.raw(method set)...: 10485170176 bytes in    2.9 seconds  3481 MB/s
    Reading dummy-big-file.txt...Reading bigfile with blockSize=8192
    : 21474836480 bytes in    7.2 seconds  2859 MB/s
    Reading /home/aphecetche/data/data-de819-sr90-20200206.raw(method cur)...: 10485170176 bytes in    2.9 seconds  3475 MB/s
    Reading dummy-big-file.txt...Reading bigfile with blockSize=8192
    : 21474836480 bytes in    7.2 seconds  2860 MB/s
    Reading /home/aphecetche/data/data-de819-sr90-20200206.raw(method all)...: 10485170176 bytes in    3.9 seconds  2588 MB/s
