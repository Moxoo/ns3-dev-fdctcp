set -x

for sendNum in 10
do
for seed in 1 2 3 4 5 6 7 8 9 10
do
	#use tail mark method
	./waf --run "scratch/long_short --transport_prot=TcpDctcp --SNum=${sendNum} --queuedisc=RedQueueDisc --randomSeed=${seed}"

	#use random mark method
	./waf --run "scratch/long_short --transport_prot=TcpDctcp --SNum=${sendNum} --queuedisc=RedQueueDisc --randommark=true --randomSeed=${seed}"

	#use fron mark method
	./waf --run "scratch/long_short --transport_prot=TcpDctcp --SNum=${sendNum} --queuedisc=RedQueueFrontDisc --randomSeed=${seed}"
done
done
