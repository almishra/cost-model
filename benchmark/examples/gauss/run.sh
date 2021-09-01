cd /gpfs/projects/ChapmanGroup/alok/git/cost-model/benchmark/examples/gauss

for((i=32; i<=8192; i+=32))
do
  echo ./gs_offload $i output_${i};
  ./gs_offload $i output_${i};
done
