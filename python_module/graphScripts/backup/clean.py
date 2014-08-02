fin = open ('../log/scheduler.txt','r')
fout = open ('clean.txt','w')
errors = open('errors.log','w')

last_time = 0
num_err=0

for line in fin:
    if not line.startswith('#'):
        splt=line.strip().split()
        timestamp = splt[3][:-1]
        try:
            float(timestamp)
            if timestamp > last_time:
                fout.write(line)
                last_time=timestamp
        except ValueError:
            num_err=num_err + 1
            errors.write(line)

print str(num_err) + " lines skipped and logged in errors.log"


