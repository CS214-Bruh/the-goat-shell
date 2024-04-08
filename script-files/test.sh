#cat example
cat ./test-dir/test.c

#cd example
cd test-dir

#piping example
give.c | out.c

#redirection examples
cat < ./test-dir/test.c > receive.txt
./test-dir/receive.txt < ./test-dir/out.c

#tricky piping precedence example
give.c | receive.txt > out.c

#conditional examples
cat ./test-dir/test.c
then cat ./test-dir/receive.c

cat ./test-dir/fake.c
else cat ./test-dir/test.c