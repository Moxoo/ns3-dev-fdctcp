set -x

mkdir RESULT
rm RESULT/*

for load in 0.4 0.5 0.6 0.7 0.8 0.9
do
for seed in 1 2 3 4 5 6 7 8 9 10
do
	#use tail mark method
	./waf --run "examples/large-scale-sim/simulation-large-scale --load=${load} --randomSeed=${seed} --RandomMark=false"
	#use random mark method
	./waf --run "examples/large-scale-sim/simulation-large-scale --load=${load} --randomSeed=${seed} --RandomMark=true"

done
done
