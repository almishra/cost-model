module load cuda/10.1.243

for i in `ls kernel_9*.out`;
do
  log=$(echo $i | cut -f 1 -d '.');
  echo -ne "${log}...";
  if [ ! -f /gpfs/alpine/world-shared/csc401/alokm/gpu/log_$log ];
  then
    /ccs/home/alokm/cost-model/test/gpu/$i /gpfs/alpine/world-shared/csc401/alokm/gpu/log_$log;
  else
    echo -ne "Exists."
  fi;
#  if [ -f /gpfs/alpine/world-shared/csc401/alokm/gpu/1log_$log ];
#  then
#    rm /gpfs/alpine/world-shared/csc401/alokm/gpu/log_$log
#  fi
#  /ccs/home/alokm/cost-model/test/gpu/$i /gpfs/alpine/world-shared/csc401/alokm/gpu/1log_$log;
  echo "";
done
