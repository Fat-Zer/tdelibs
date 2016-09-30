
clean_up() {
	rm -f batch.stdout shell.stdout shell.returns batch.returns
}

clean_up

../dcopserver --nofork &
DCOP_SERVER_PID=$!

die() {
    kill $DCOP_SERVER_PID
    echo "$1"
    exit 1;
}

echo '* Running batch mode'
./dcop_test --batch >batch.stdout || die "Failed to run dcop_test"

echo -n '* Starting test app '
./dcop_test >shell.stdout &
DCOP_TEST_PID=$!

while ! ../client/dcop | grep -q "TestApp-$DCOP_TEST_PID"; do
    echo -n '.'
    sleep 2
    kill -0 "$DCOP_TEST_PID" || die "dcop_test died unexpectadly"
done

echo ' started'

echo '* Running driver mode'
./driver "TestApp-$DCOP_TEST_PID" >driver.stdout || die "Failed to start driver"

echo '* Running shell mode'
source ./shell.generated >shell.returns

echo -n '* Comparing ... '

compare()
{
if ! diff -q --strip-trailing-cr $1 $2; then
	echo "FAILED:"
	diff -u $1 $2
	die "$1 and $2 are different";
fi
}

compare batch.stdout shell.stdout
compare batch.stdout driver.stdout
compare batch.returns shell.returns
compare batch.returns driver.returns

clean_up

kill $DCOP_SERVER_PID
echo "Passed"
exit 0;
