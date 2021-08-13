set -x

for sendNum in $(seq 1 60)

do
	#use mixed mark method
	./waf --run "scratch/incast --transport_prot=TcpDctcp --sendNum=${sendNum} --queuedisc=RedQueueMMDisc"

	#use tail mark method
	#./waf --run "scratch/incast --transport_prot=TcpDctcp --sendNum=${sendNum} --queuedisc=RedQueueDisc"

	#use random mark method
	#./waf --run "scratch/incast --transport_prot=TcpDctcp --sendNum=${sendNum} --queuedisc=RedQueueDisc --randommark=true"

	#use fron mark method
	#./waf --run "scratch/incast --transport_prot=TcpDctcp --sendNum=${sendNum} --queuedisc=RedQueueFrontDisc"

	#use CUBIC and FIFO queue
	#./waf --run "scratch/incast --transport_prot=TcpCubic --sendNum=${sendNum}"
done
