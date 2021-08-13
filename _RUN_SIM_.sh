set -x

for load in 0.6
do
for seed in 1 2 3 4 5 6 7 8 18 20
do
	#use tail mark method
	./waf --run "examples/large-scale-sim/large-scale-sim --load=${load} --randomSeed=${seed} --queuedisc=RedQueueDisc --RandomMark=false --Buffer=250p"

	#use random mark method
	#./waf --run "examples/large-scale-sim/large-scale-sim --load=${load} --randomSeed=${seed} --queuedisc=RedQueueDisc --RandomMark=true --Buffer=250p"

	#use front mark method
	./waf --run "examples/large-scale-sim/large-scale-sim --load=${load} --randomSeed=${seed} --queuedisc=RedQueueFrontDisc --Buffer=250p"

done
done
