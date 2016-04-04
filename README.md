pwr2prv
=========

pwr2prv is a small command line application to convert and merge dcdb power
readings to BSC's
(http://www.bsc.es/computer-sciences/performance-tools/paraver) Paraver
performance analysis tool format.

Compilation
-----------
	./configure
	make

Usage
-----
	Usage: pwr2prv [OPTION...]
		-m, --merge-prv=STRING	Merge with paraver trace
		-p, --power=STRING	Power trace
		-o, --output=STRING	Output filename
		-t, --offset=INT	Initial timestamp (ns)
		-v, --verbose		Be verbose
	
	Help options:
		-?, --help		Show this help message
		--usage			Display brief usage message
