#cat example
cat ./test-dir/test.c

#cd example
cd ./test-dir

#piping example
echo hello | grep hello

#redirection examples
cat < ./test-dir/test.c > ./receive.txt
./receive.txt < ./a.out

#tricky piping precedence example
give.c | receive.txt > out.c

#conditional examples
cat ./test-dir/test.c
then cat ./test-dir/receive.c

cat ./test-dir/fake.c
else cat ./test-dir/test.c