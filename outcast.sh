set -x

for f_num in 1 2 4 6 8 10 12 14 16

do
	#use tail mark method
	#./waf --run "scratch/outcast --transport_prot=TcpDctcp --f_num=${f_num} --queuedisc=RedQueueDisc"

	#use random mark method
	./waf --run "scratch/outcast --transport_prot=TcpDctcp --f_num=${f_num} --queuedisc=RedQueueDisc --randommark=true"

	#use fron mark method
	#./waf --run "scratch/outcast --transport_prot=TcpDctcp --f_num=${f_num} --queuedisc=RedQueueFrontDisc"

done
